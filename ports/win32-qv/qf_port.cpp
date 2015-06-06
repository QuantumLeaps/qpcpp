/// \file
/// \brief QF/C++ port to Win32 API with cooperative QV scheduler (win32-qv)
/// \cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-03-14
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"       // QF package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

#include <limits.h>       // limits of dynamic range for integers


namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

/* Global objects ==========================================================*/
QPSet64 QV_readySet_;     // QV-ready set of active objects
HANDLE  QV_win32Event_;   // Win32 event to signal events

// Local objects *************************************************************
static CRITICAL_SECTION l_win32CritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static bool  l_isRunning;      // flag indicating when QF is running

static DWORD WINAPI ticker_thread(LPVOID arg);

//****************************************************************************
void QF::init(void) {
    InitializeCriticalSection(&l_win32CritSect);
}
//****************************************************************************
void QF_enterCriticalSection_(void) {
    EnterCriticalSection(&l_win32CritSect);
}
//****************************************************************************
void QF_leaveCriticalSection_(void) {
    LeaveCriticalSection(&l_win32CritSect);
}
//****************************************************************************
void QF::stop(void) {
    l_isRunning = false; // terminate the main event-loop thread
    SetEvent(QV_win32Event_); // unblock the event-loop so it can terminate
}
//****************************************************************************
int_t QF::run(void) {
    onStartup(); // application-specific startup callback

    l_isRunning = true; /* QF is running */

    // create the ticker thread...
    HANDLE ticker = CreateThread(NULL, 1024, &ticker_thread,
                                 static_cast<void *>(0), 0U, NULL);
    // thread must be created
    Q_ASSERT_ID(310, ticker != static_cast<HANDLE>(0));

    // the combined event-loop and background-loop of the QV kernel */
    while (l_isRunning) {
        QF_INT_DISABLE();
        if (QV_readySet_.notEmpty()) {
            uint_fast8_t p = QV_readySet_.findMax();
            QMActive *a = active_[p];
            QF_INT_ENABLE();

            // perform the run-to-completion (RTS) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The "Vanialla" kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const *e = a->get_();
            a->dispatch(e);
            gc(e);
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the Win32-QV port does not do busy-waiting
            // for events. Instead, the Win32-QV port efficiently waits until
            // QP events become available.
            QF_INT_ENABLE();
            (void)WaitForSingleObject(QV_win32Event_, (DWORD)INFINITE);
        }
    }
    onCleanup();  // cleanup callback
    QS_EXIT();    // cleanup the QSPY connection

    //CloseHandle(QV_win32Event_);
    //DeleteCriticalSection(&l_win32CritSect);
    //free all "fudged" event pools...
    return static_cast<int_t>(0);
}
//****************************************************************************
void QF_setTickRate(uint32_t ticksPerSec) {
    l_tickMsec = 1000UL / ticksPerSec;
}
//****************************************************************************
void QMActive::start(uint_fast8_t prio,
                     QEvt const *qSto[], uint_fast16_t qLen,
                     void *stkSto, uint_fast16_t /*stkSize*/,
                     QEvt const *ie)
{
    Q_REQUIRE_ID(700, (static_cast<uint_fast8_t>(0) < prio) /* priority...*/
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)) /*... in range */
        && (qSto != static_cast<QEvt const **>(0)) /* queue storage... */
        && (qLen > static_cast<uint_fast16_t>(0))  /* ... must be provided */
        && (stkSto == static_cast<void *>(0)));    /* statck storage must NOT...
                                                    * ... be provided */

    m_prio = prio;  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO

    // ignore the original storage for the event queue 'qSto' and
    // instead allocate an oversized "fudged" storage for the queue.
    // See also NOTE2 in qf_port.h.
    Q_ASSERT_ID(710, static_cast<uint32_t>(qLen) * QF_WIN32_FUDGE_FACTOR
                     < USHRT_MAX);
    // fudged the queue length
    uint_fast16_t fudgedQLen = qLen * QF_WIN32_FUDGE_FACTOR;
    // fudged queue storage
    void *fudgedQSto = new QEvt*[fudgedQLen];
    // allocation must succeed
    Q_ASSERT_ID(720, fudgedQSto != static_cast<void *>(0));
    m_eQueue.init(static_cast<QEvt const **>(fudgedQSto), fudgedQLen);

    this->init(ie); // execute initial transition (virtual call)
    QS_FLUSH();     // flush the QS trace buffer to the host
}
//****************************************************************************
void QMActive::stop(void) {
    unsubscribeAll();
    QF::remove_(this);
    delete[] m_eQueue.m_ring; // free the fudged queue storage
}

//****************************************************************************
static DWORD WINAPI ticker_thread(LPVOID /*arg*/) { // for CreateThread()
    // set the ticker thread priority below normal to prevent
    // flooding other threads with time events when the machine
    // is very busy.
    //
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    while (l_isRunning) {
        Sleep(l_tickMsec); // wait for the tick interval
        QF_onClockTick();  // clock tick callback (must call QF_TICK_X())
    }
    return static_cast<DWORD>(0); // return success
}

} // namespace QP
