/// \file
/// \brief QF/C++ port to Win32 API
/// \cond
///***************************************************************************
/// Last updated for version 5.4.2
/// Last updated on  2015-06-05
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

// Local objects *************************************************************
static CRITICAL_SECTION l_win32CritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static bool  l_isRunning;      // flag indicating when QF is running

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
    l_isRunning = false;   // terminate the main (ticker) thread
}
//****************************************************************************
void QF::thread_(QMActive *act) {
    // loop until m_thread is cleared in QMActive::stop()
    do {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    } while (act->m_thread != NULL);

    act->unsubscribeAll(); // make sure that no events are subscribed
    QF::remove_(act);  // remove this object from any subscriptions
    CloseHandle(act->m_osObject); // cleanup the OS event
    delete[] act->m_eQueue.m_ring;  // free the fudged queue storage
}
//****************************************************************************
// helper function to match the signature expeced by CreateThread() Win32 API
static DWORD WINAPI ao_thread(LPVOID me) {
    QF::thread_(static_cast<QMActive *>(me));
    return static_cast<DWORD>(0); // return success
}
//****************************************************************************
int_t QF::run(void) {
    onStartup();  // startup callback

    l_isRunning = true; // QF is running

    // set the ticker thread priority below normal to prevent
    // flooding other threads with time events when the machine
    // is very busy.
    //
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    do {
        Sleep(l_tickMsec);  // wait for the tick interval
        QF_onClockTick();   // clock tick callback (must call QF_TICK_X())
    } while (l_isRunning);

    onCleanup();            // cleanup callback
    QS_EXIT();              // cleanup the QSPY connection
    //DeleteCriticalSection(&l_win32CritSect);
    //free all "fudged" event pools...
    return static_cast<int_t>(0); // return success
}
//****************************************************************************
void QF_setTickRate(uint32_t ticksPerSec) {
    l_tickMsec = 1000UL / ticksPerSec;
}
//****************************************************************************
void QF_setWin32Prio(QMActive *act, int_t win32Prio) {
    if (act->getThread() == (HANDLE)0) {  // thread not created yet?
        act->getOsObject() = (void *)win32Prio; // store the priority for later
    }
    else {
        SetThreadPriority(act->getThread(), win32Prio);
    }
}
//****************************************************************************
void QMActive::start(uint_fast8_t prio,
                     QEvt const *qSto[], uint_fast16_t qLen,
                     void *stkSto, uint_fast16_t stkSize,
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
    //
    Q_ASSERT_ID(710, static_cast<uint32_t>(qLen) * QF_WIN32_FUDGE_FACTOR
                     < USHRT_MAX);
    // fudge the queue length
    uint_fast16_t fudgedQLen = qLen * QF_WIN32_FUDGE_FACTOR;
    QEvt const **fudgedQSto = new QEvt const *[fudgedQLen]; // fudged queue storage
    // allocation must succeed
    Q_ASSERT_ID(720, fudgedQSto != static_cast<QEvt const * *>(0));
    m_eQueue.init(fudgedQSto, fudgedQLen);

    // save osObject as integer, in case it contains the Win32 priority
    int win32Prio = (m_osObject != static_cast<void *>(0))
                    ? reinterpret_cast<int>(m_osObject)
                    : THREAD_PRIORITY_NORMAL;

    /* create the Win32 "event" to throttle the AO's event queue */
    m_osObject = CreateEvent(NULL, FALSE, FALSE, NULL);

    this->init(ie); // execute initial transition (virtual call)
    QS_FLUSH(); /* flush the QS trace buffer to the host */

    // stack size not provided?
    if (stkSize == 0U) {
        stkSize = 1024U; // NOTE: will be rounded up to the nearest page
    }
    m_thread = CreateThread(NULL, stkSize, &ao_thread, this, 0, NULL);
    Q_ASSERT_ID(730, m_thread != NULL); // thread must be created

    SetThreadPriority(m_thread, win32Prio);
}
//****************************************************************************
void QMActive::stop(void) {
    m_thread = static_cast<HANDLE>(0);  // stop the QActive::run() loop
}

} // namespace QP
