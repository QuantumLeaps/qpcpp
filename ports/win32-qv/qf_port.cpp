//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
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
//! @date Last updated on: 2023-11-30
//! @version Last updated for: @ref qpcpp_7_3_1
//!
//! @file
//! @brief QF/C++ port to Win32 API (single-threaded, like the QV kernel)

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
#include <conio.h>          // console input/output

namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects =============================================================
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static int   l_tickPrio = 50;  // default priority of the "ticker" thread
static bool  l_isRunning;      // flag indicating when QF is running

//============================================================================
static DWORD WINAPI ticker_thread(LPVOID arg); // prototype
static DWORD WINAPI ticker_thread(LPVOID arg) { // for CreateThread()
    Q_UNUSED_PAR(arg);

    int threadPrio = THREAD_PRIORITY_NORMAL;

    // set the ticker thread priority according to selection made in
    // QF_setTickRate()
    if (l_tickPrio < 33) {
        threadPrio = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if (l_tickPrio > 66) {
        threadPrio = THREAD_PRIORITY_ABOVE_NORMAL;
    }

    SetThreadPriority(GetCurrentThread(), threadPrio);

    while (l_isRunning) { // the clock tick loop...
        Sleep(l_tickMsec); // wait for the tick interval
        QP::QF::onClockTick(); // clock tick callback (must call QTimeEvt::TICK_X())
    }

    return 0U; // return success
}

} // unnamed local namespace

//============================================================================
namespace QP {
namespace QF {

QPSet readySet_;
QPSet readySet_dis_;
HANDLE win32Event_; // Win32 event to signal events

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
    win32Event_ = CreateEvent(NULL, FALSE, FALSE, NULL);

    readySet_.setEmpty();
#ifndef Q_UNSAFE
    readySet_.update_(&readySet_dis_);
#endif

}

//............................................................................
int run() {
    l_isRunning = true; // QF is running

    onStartup(); // application-specific startup callback

    QF_CRIT_STAT

    if (l_tickMsec != 0U) { // system clock tick configured?
        // create the ticker thread...
        HANDLE ticker = CreateThread(NULL, 1024, &ticker_thread,
                                     nullptr, 0U, NULL);
        QF_CRIT_ENTRY();
        Q_ASSERT_INCRIT(310, ticker != static_cast<HANDLE>(0));
        QF_CRIT_EXIT();
    }

    // the combined event-loop and background-loop of the QV kernel
    QF_CRIT_ENTRY();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    while (l_isRunning) {
        Q_ASSERT_INCRIT(300, readySet_.verify_(&readySet_dis_));

        // find the maximum priority AO ready to run
        if (readySet_.notEmpty()) {
            std::uint_fast8_t p = readySet_.findMax();
            QActive *a = QActive::registry_[p];

            // the active object 'a' must still be registered in QF
            // (e.g., it must not be stopped)
            Q_ASSERT_INCRIT(320, a != nullptr);
            QF_CRIT_EXIT();

            QEvt const *e = a->get_();
            // dispatch event (virtual call)
            a->dispatch(e, a->getPrio());
            QF::gc(e);

            QF_CRIT_ENTRY();
            if (a->getEQueue().isEmpty()) { // empty queue?
                readySet_.remove(p);
#ifndef Q_UNSAFE
                readySet_.update_(&readySet_dis_);
#endif
            }
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the Win32-QV port does not do busy-waiting
            // for events. Instead, the Win32-QV port efficiently waits until
            // QP events become available.
            QF_CRIT_EXIT();

            WaitForSingleObject(win32Event_, INFINITE);

            QF_CRIT_ENTRY();
        }
    }
    QF_CRIT_EXIT();
    onCleanup(); // cleanup callback
    QS_EXIT();   // cleanup the QSPY connection

    //CloseHandle(win32Event_);
    //DeleteCriticalSection(&l_win32CritSect);

    return 0; // return success
}
//............................................................................
void stop() {
    l_isRunning = false; // this will exit the main event-loop

    // unblock the event-loop so it can terminate
    readySet_.insert(1U);
#ifndef Q_UNSAFE
    readySet_.update_(&readySet_dis_);
#endif
    SetEvent(win32Event_);
}
//............................................................................
void setTickRate(std::uint32_t ticksPerSec, int tickPrio) {
    if (ticksPerSec != 0U) {
        l_tickMsec = 1000UL / ticksPerSec;
    }
    else {
        l_tickMsec = 0U; // means NO system clock tick
    }
    l_tickPrio = tickPrio;
}

//............................................................................
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

} // namespace QF

// QActive functions =========================================================

void QActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // no per-AO stack needed for this port
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(600, stkSto == nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used in this port)
    register_(); // register this AO

    m_eQueue.init(qSto, qLen);

    // top-most initial tran. (virtual call)
    this->init(par, m_prio);
    QS_FLUSH(); // flush the QS trace buffer to the host
}

//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    unsubscribeAll();

    // make sure the AO is no longer in "ready set"
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF::readySet_.remove(m_prio);
#ifndef Q_UNSAFE
    QF::readySet_.update_(&QF::readySet_dis_);
#endif
    QF_CRIT_EXIT();

    unregister_(); // remove this AO from QF
}
#endif

} // namespace QP

