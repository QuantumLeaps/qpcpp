/// \file
/// \brief QF/C++ port to Win32 API with cooperative QV scheduler (win32-qv)
/// \cond
///***************************************************************************
/// Last updated for version 5.8.2
/// Last updated on  2016-12-22
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
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
QPSet  QV_readySet_;   // QV-ready set of active objects
HANDLE QV_win32Event_; // Win32 event to signal events

// Local objects *************************************************************
static CRITICAL_SECTION l_win32CritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static bool  l_isRunning;      // flag indicating when QF is running

static DWORD WINAPI ticker_thread(LPVOID arg);

//****************************************************************************
void QF::init(void) {
    InitializeCriticalSection(&l_win32CritSect);

    // clear the internal QF variables, so that the framework can (re)start
    // correctly even if the startup code is not called to clear the
    // uninitialized data (as is required by the C++ Standard).
    extern uint_fast8_t QF_maxPool_;
    QF_maxPool_ = static_cast<uint_fast8_t>(0);
    bzero(&QF::timeEvtHead_[0],
          static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));
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
    QF_INT_DISABLE();
    while (l_isRunning) {

        if (QV_readySet_.notEmpty()) {
            uint_fast8_t p = QV_readySet_.findMax();
            QActive *a = active_[p];
            QF_INT_ENABLE();

            // the active object 'a' must still be registered in QF
            // (e.g., it must not be stopped)
            Q_ASSERT_ID(320, a != static_cast<QActive *>(0));

            // perform the run-to-completion (RTS) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The "Vanialla" kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const *e = a->get_();
            a->dispatch(e);
            gc(e);

            QF_INT_DISABLE();

            if (a->m_eQueue.isEmpty()) { /* empty queue? */
                QV_readySet_.remove(p);
            }
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the Win32-QV port does not do busy-waiting
            // for events. Instead, the Win32-QV port efficiently waits until
            // QP events become available.
            QF_INT_ENABLE();
            (void)WaitForSingleObject(QV_win32Event_, (DWORD)INFINITE);

            QF_INT_DISABLE();
        }
    }
    QF_INT_ENABLE();
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
void QActive::start(uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t /*stkSize*/,
                    QEvt const *ie)
{
    Q_REQUIRE_ID(700, (static_cast<uint_fast8_t>(0) < prio) /* priority...*/
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)) /*... in range */
        && (stkSto == static_cast<void *>(0)));    /* statck storage must NOT...
                                                    * ... be provided */

    m_prio = prio;  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO

    m_eQueue.init(qSto, qLen);


    this->init(ie); // execute initial transition (virtual call)
    QS_FLUSH();     // flush the QS trace buffer to the host
}
//****************************************************************************
void QActive::stop(void) {
    unsubscribeAll();
    QF::remove_(this);
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
