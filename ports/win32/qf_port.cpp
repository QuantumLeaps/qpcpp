/// @file
/// @brief QF/C++ port to Win32 API (multi-threaded)
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-01-23
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond
///
#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#include <limits.h>         // limits of dynamic range for integers
#include <conio.h>          // console input/output

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects *************************************************************
static CRITICAL_SECTION l_win32CritSect;
static CRITICAL_SECTION l_startupCritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static int_t l_tickPrio = 50;  // default priority of the "ticker" thread
static bool  l_isRunning;      // flag indicating when QF is running

//****************************************************************************
void QF::init(void) {
    InitializeCriticalSection(&l_win32CritSect);

    // initialize and enter the startup critical section object to block
    // any active objects started before calling QF::run()
    InitializeCriticalSection(&l_startupCritSect);
    EnterCriticalSection(&l_startupCritSect);

    // clear the internal QF variables, so that the framework can (re)start
    // correctly even if the startup code is not called to clear the
    // uninitialized data (as is required by the C++ Standard).
    extern std::uint_fast8_t QF_maxPool_;
    QF_maxPool_ = 0U;
    bzero(&QF::timeEvtHead_[0], sizeof(QF::timeEvtHead_));
    bzero(&active_[0],          sizeof(active_));
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
    l_isRunning = false; // terminate the main (ticker) thread
}
//****************************************************************************
void QF::thread_(QActive *act) {
    // block this thread until the startup critical section is exited
    // from QF::run()
    EnterCriticalSection(&l_startupCritSect);
    LeaveCriticalSection(&l_startupCritSect);

    // event-loop
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
}
//****************************************************************************
// helper function to match the signature expeced by CreateThread() Win32 API
static DWORD WINAPI ao_thread(LPVOID me) {
    QF::thread_(static_cast<QActive *>(me));
    return static_cast<DWORD>(0); // return success
}
//****************************************************************************
int_t QF::run(void) {

    onStartup(); // application-specific startup callback

    // leave the startup critical section to unblock any active objects
    // started before calling QF::run()
    LeaveCriticalSection(&l_startupCritSect);

    l_isRunning = true; // QF is running

    // set the ticker (this thread) priority according to selection made in
    // QF_setTickRate()
    //
    int threadPrio = THREAD_PRIORITY_NORMAL;
    if (l_tickPrio < 33) {
        threadPrio = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if (l_tickPrio > 66) {
        threadPrio = THREAD_PRIORITY_ABOVE_NORMAL;
    }
    SetThreadPriority(GetCurrentThread(), threadPrio);

    do {
        Sleep(l_tickMsec);  // wait for the tick interval
        QF_onClockTick();   // clock tick callback (must call QF_TICK_X())
    } while (l_isRunning);

    onCleanup();            // cleanup callback
    QS_EXIT();              // cleanup the QSPY connection
    //DeleteCriticalSection(&l_startupCritSect);
    //DeleteCriticalSection(&l_win32CritSect);
    return 0; // return success
}
//****************************************************************************
void QF_setTickRate(std::uint32_t ticksPerSec, int_t tickPrio) {
    Q_REQUIRE_ID(600, ticksPerSec != 0U);
    l_tickMsec = 1000UL / ticksPerSec;
    l_tickPrio = tickPrio;
}
//****************************************************************************
void QF_setWin32Prio(QActive *act, int_t win32Prio) {
    HANDLE win32thread = static_cast<HANDLE>(act->getThread());

    // thread must be already created, see QActive::start()
    Q_REQUIRE_ID(700, win32thread != static_cast<HANDLE>(0));
    SetThreadPriority(win32thread, win32Prio);
}

//............................................................................
void QF_consoleSetup(void) {
}
//............................................................................
void QF_consoleCleanup(void) {
}
//............................................................................
int QF_consoleGetKey(void) {
    if (_kbhit()) { // any key pressed?
        return static_cast<int>(_getwch());
    }
    return 0;
}
//............................................................................
int QF_consoleWaitForKey(void) {
    return static_cast<int>(_getwch());
}

//****************************************************************************
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    (void)stkSize; // unused paramteter in the Win32 port

    Q_REQUIRE_ID(800, (0U < prio)  /* priority...*/
        && (prio <= QF_MAX_ACTIVE) /*...in range */
        && (stkSto == nullptr));  // statck storage must NOT...
                                   // ... be provided
    m_eQueue.init(qSto, qLen);
    m_prio = prio;  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO

    // save osObject as integer, in case it contains the Win32 priority
    //int win32Prio = (m_osObject != nullptr)
    //    ? reinterpret_cast<intptr_t>(m_osObject)
    //    : THREAD_PRIORITY_NORMAL;

    // create the Win32 "event" to throttle the AO's event queue
    m_osObject = CreateEvent(NULL, FALSE, FALSE, NULL);

    this->init(par); // execute initial transition (virtual call)
    QS_FLUSH(); // flush the QS trace buffer to the host

    // create a Win32 thread for the AO;
    // The thread is created with THREAD_PRIORITY_NORMAL
    m_thread = CreateThread(
        NULL,
        (stkSize < 1024U ? 1024U : stkSize),
        &ao_thread,
        this,
        0,
        NULL);
    Q_ENSURE_ID(830, m_thread != static_cast<HANDLE>(0)); // must succeed
}

} // namespace QP

