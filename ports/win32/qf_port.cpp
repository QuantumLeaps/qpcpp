//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#include <climits>          // limits of dynamic range for integers

namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects =============================================================
static CRITICAL_SECTION l_startupCritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static int   l_tickPrio = 50;  // default priority of the "ticker" thread
static bool  l_isRunning;      // flag indicating when QF is running

//............................................................................
// helper function to match the signature expected by CreateThread() Win32 API
static DWORD WINAPI ao_thread(LPVOID me) {
    QP::QActive::evtLoop_(static_cast<QP::QActive *>(me));
    return static_cast<DWORD>(0); // return success
}

} // unnamed local namespace

//============================================================================
namespace QP {
namespace QF {

static CRITICAL_SECTION l_win32CritSect;
static int_t l_critSectNest;   // critical section nesting up-down counter

//............................................................................
void enterCriticalSection_() {
    if (l_isRunning) {
        EnterCriticalSection(&l_win32CritSect);
        Q_ASSERT_INCRIT(100, l_critSectNest == 0); // NO nesting of crit.sect!
        ++l_critSectNest;
    }
}
//............................................................................
void leaveCriticalSection_() {
    if (l_isRunning) {
        Q_ASSERT_INCRIT(200, l_critSectNest == 1); // crit.sect. must ballace!
        if ((--l_critSectNest) == 0) {
            LeaveCriticalSection(&l_win32CritSect);
        }
    }
}

//............................................................................
void init() {
    InitializeCriticalSection(&l_win32CritSect);

    // initialize and enter the startup critical section object to block
    // any active objects started before calling QF::run()
    InitializeCriticalSection(&l_startupCritSect);
    EnterCriticalSection(&l_startupCritSect);
}

//............................................................................
int run() {

    onStartup(); // application-specific startup callback

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    // leave the startup critical section to unblock any active objects
    // started before calling QF::run()
    LeaveCriticalSection(&l_startupCritSect);

    l_isRunning = true; // QF is running

    // set the ticker (this thread) priority according to selection made in
    // QF::setTickRate()
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
        onClockTick();     // must call QTimeEvt::TICK_X()
    }

    onCleanup(); // cleanup callback
    QS_EXIT();   // cleanup the QSPY connection

    //DeleteCriticalSection(&l_startupCritSect);
    //DeleteCriticalSection(&l_win32CritSect);
    return 0; // return success
}
//............................................................................
void stop() {
    l_isRunning = false; // terminate the main (ticker) thread
}
//............................................................................
void setTickRate(std::uint32_t ticksPerSec, int tickPrio) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(600, ticksPerSec != 0U);
    QF_CRIT_EXIT();

    l_tickMsec = 1000UL / ticksPerSec;
    l_tickPrio = tickPrio;
}

// console access ============================================================
#ifdef QF_CONSOLE

#include <conio.h>          // console input/output

void consoleSetup() {
}
//............................................................................
void consoleCleanup() {
}
//............................................................................
int consoleGetKey() {
    if (_kbhit()) { // any key pressed?
        return static_cast<int>(_getwch());
    }
    return 0;
}
//............................................................................
int consoleWaitForKey(void) {
    return static_cast<int>(_getwch());
}

#endif // #ifdef QF_CONSOLE

} // namespace QF

// QActive functions =========================================================

void QActive::start(QPrioSpec const prioSpec,
    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
    void * const stkSto, std::uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // no external AO-stack storage needed for this port
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(800, stkSto == nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used in QF, but in Win32)
    register_();  // register this AO

    // create the Win32 "event" to throttle the AO's event queue
    m_osObject = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_eQueue.init(qSto, qLen);

    // top-most initial tran. (virtual call)
    this->init(par, m_prio);
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
    QF_CRIT_ENTRY();
    Q_ENSURE_INCRIT(830, m_thread != nullptr); // must succeed
    QF_CRIT_EXIT();

    // set the priority of the Win32 thread based on the
    // "prio-threshold" field provided in the `prioSpec` parameter
    int win32Prio;
    switch ((prioSpec >> 8U) & 0xFFU) {
        case 1U:
            win32Prio = THREAD_PRIORITY_LOWEST;
            break;
        case 2U:
            win32Prio = THREAD_PRIORITY_NORMAL;
            break;
        case 3U:
            win32Prio = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        default:
            win32Prio = THREAD_PRIORITY_NORMAL;
            break;
    }
    SetThreadPriority(m_thread, win32Prio);
}

//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    if (QActive_subscrList_ != nullptr) {
        unsubscribeAll(); // unsubscribe this AO from all events
    }
    m_thread = nullptr; // stop the thread loop (see QF::thread_)
}
#endif

//............................................................................
void QActive::evtLoop_(QActive *act) {
    // block this thread until the startup critical section is exited
    // from QF::run()
    EnterCriticalSection(&l_startupCritSect);
    LeaveCriticalSection(&l_startupCritSect);

#ifdef QACTIVE_CAN_STOP
    while (act->m_thread)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    act->unregister_(); // remove this object from QF
#endif
}

} // namespace QP

