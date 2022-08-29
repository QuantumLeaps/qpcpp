//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-08-28
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QF/C++ port to Win32 API (multi-threaded)

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

namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects .............................................................
static CRITICAL_SECTION l_win32CritSect;
static CRITICAL_SECTION l_startupCritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static int_t l_tickPrio = 50;  // default priority of the "ticker" thread
static bool  l_isRunning;      // flag indicating when QF is running

//............................................................................
// helper function to match the signature expeced by CreateThread() Win32 API
static DWORD WINAPI ao_thread(LPVOID me) {
    QP::QActive::thread_(static_cast<QP::QActive *>(me));
    return static_cast<DWORD>(0); // return success
}

} // unnamed local namespace

//============================================================================
namespace QP {

//............................................................................
void QF::init(void) {
    InitializeCriticalSection(&l_win32CritSect);

    // initialize and enter the startup critical section object to block
    // any active objects started before calling QF::run()
    InitializeCriticalSection(&l_startupCritSect);
    EnterCriticalSection(&l_startupCritSect);
}
//............................................................................
void QF::enterCriticalSection_(void) {
    EnterCriticalSection(&l_win32CritSect);
}
//............................................................................
void QF::leaveCriticalSection_(void) {
    LeaveCriticalSection(&l_win32CritSect);
}
//............................................................................
void QF::stop(void) {
    l_isRunning = false; // terminate the main (ticker) thread
}
//............................................................................
void QActive::thread_(QActive *act) {
    // block this thread until the startup critical section is exited
    // from QF::run()
    EnterCriticalSection(&l_startupCritSect);
    LeaveCriticalSection(&l_startupCritSect);

#ifdef QF_ACTIVE_STOP
    while (act->m_thread)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QF_ACTIVE_STOP
    act->unregister_(); // remove this object from QF
#endif
}

//............................................................................
int_t QF::run(void) {

    onStartup(); // application-specific startup callback

    // produce the QS_QF_RUN trace record
    QS_BEGIN_NOCRIT_PRE_(QS_QF_RUN, 0U)
    QS_END_NOCRIT_PRE_()

    // leave the startup critical section to unblock any active objects
    // started before calling QF::run()
    LeaveCriticalSection(&l_startupCritSect);

    l_isRunning = true; // QF is running

    // set the ticker (this thread) priority according to selection made in
    // QF::setTickRate()
    //
    int threadPrio = THREAD_PRIORITY_NORMAL;
    if (l_tickPrio < 33) {
        threadPrio = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if (l_tickPrio > 66) {
        threadPrio = THREAD_PRIORITY_ABOVE_NORMAL;
    }
    SetThreadPriority(GetCurrentThread(), threadPrio);

    while (l_isRunning) {
        Sleep(l_tickMsec); // wait for the tick interval
        onClockTick();     // clock tick callback (must call QF::TICK_X())
    }

    onCleanup(); // cleanup callback
    QS_EXIT();   // cleanup the QSPY connection
    //DeleteCriticalSection(&l_startupCritSect);
    //DeleteCriticalSection(&l_win32CritSect);
    return 0; // return success
}
//............................................................................
void QF::setTickRate(std::uint32_t ticksPerSec, int_t tickPrio) {
    Q_REQUIRE_ID(600, ticksPerSec != 0U);
    l_tickMsec = 1000UL / ticksPerSec;
    l_tickPrio = tickPrio;
}
//............................................................................
void QF::setWin32Prio(QActive *act, int_t win32Prio) {
    HANDLE win32thread = static_cast<HANDLE>(act->getThread());

    // thread must be already created, see QActive::start()
    Q_REQUIRE_ID(700, win32thread != static_cast<HANDLE>(0));
    SetThreadPriority(win32thread, win32Prio);
}

//............................................................................
void QF::consoleSetup(void) {
}
//............................................................................
void QF::consoleCleanup(void) {
}
//............................................................................
int QF::consoleGetKey(void) {
    if (_kbhit()) { // any key pressed?
        return static_cast<int>(_getwch());
    }
    return 0;
}
//............................................................................
int QF::consoleWaitForKey(void) {
    return static_cast<int>(_getwch());
}

//============================================================================
void QActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // no need for external stack storage in this port
    Q_REQUIRE_ID(800, stkSto == nullptr);

    m_prio = static_cast<std::uint8_t>(prioSpec & 0xFF); // QF-priority
    register_(); // make QF aware of this AO

    m_eQueue.init(qSto, qLen);

    // save osObject as integer, in case it contains the Win32 priority
    //int win32Prio = (m_osObject != nullptr)
    //    ? reinterpret_cast<intptr_t>(m_osObject)
    //    : THREAD_PRIORITY_NORMAL;

    // create the Win32 "event" to throttle the AO's event queue
    m_osObject = CreateEvent(NULL, FALSE, FALSE, NULL);

    this->init(par, m_prio); // execute initial transition (virtual call)
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
    Q_ENSURE_ID(830, m_thread != nullptr); // must succeed
}
//............................................................................
#ifdef QF_ACTIVE_STOP
void QActive::stop(void) {
    unsubscribeAll(); // unsubscribe this AO from all events
    m_thread = nullptr; // stop the thread loop (see QF::thread_)
}
#endif

} // namespace QP

