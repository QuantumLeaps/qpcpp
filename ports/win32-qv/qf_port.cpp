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
//! @date Last updated on: 2022-08-29
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QF/C++ port to Win32 API (single-threaded, like the QV kernel)
//!
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

// Local objects *************************************************************
static CRITICAL_SECTION l_win32CritSect;
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static int_t l_tickPrio = 50;  // default priority of the "ticker" thread
static bool  l_isRunning;      // flag indicating when QF is running

static DWORD WINAPI ticker_thread(LPVOID /*arg*/) { // for CreateThread()
    int threadPrio = THREAD_PRIORITY_NORMAL;

    // set the ticker thread priority according to selection made in
    // QF_setTickRate()
    //
    if (l_tickPrio < 33) {
        threadPrio = THREAD_PRIORITY_BELOW_NORMAL;
    }
    else if (l_tickPrio > 66) {
        threadPrio = THREAD_PRIORITY_ABOVE_NORMAL;
    }

    SetThreadPriority(GetCurrentThread(), threadPrio);

    while (l_isRunning) {
        Sleep(l_tickMsec); // wait for the tick interval
        QP::QF::onClockTick();  // clock tick callback (must call TICK_X())
    }
    return 0; // return success
}
} // unnamed local namespace

//============================================================================
namespace QP {

// Global objects ............................................................
HANDLE QV_win32Event_; // Win32 event to signal events

//............................................................................
void QF::init(void) {
    InitializeCriticalSection(&l_win32CritSect);
    QV_win32Event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
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
    l_isRunning = false; // terminate the main event-loop thread
    SetEvent(QV_win32Event_); // unblock the event-loop so it can terminate
}
//............................................................................
int_t QF::run(void) {

    onStartup(); // application-specific startup callback

    l_isRunning = true; // QF is running

    // system clock tick configured?
    if (l_tickMsec != 0U) {
        // create the ticker thread...
        HANDLE ticker = CreateThread(NULL, 1024, &ticker_thread,
                                     nullptr, 0U, NULL);
        // thread must be created
        Q_ASSERT_ID(310, ticker != static_cast<HANDLE>(0));
    }

    // the combined event-loop and background-loop of the QV kernel
    QF_CRIT_STAT_
    QF_CRIT_E_();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_NOCRIT_PRE_(QS_QF_RUN, 0U)
    QS_END_NOCRIT_PRE_()

    while (l_isRunning) {
        // find the maximum priority AO ready to run
        if (QF::readySet_.notEmpty()) {
            std::uint_fast8_t p = QF::readySet_.findMax();
            QActive *a = QActive::registry_[p];
            QF_CRIT_X_();

            // the active object 'a' must still be registered in QF
            // (e.g., it must not be stopped)
            Q_ASSERT_ID(320, a != nullptr);

            // perform the run-to-completion (RTS) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The QV kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const *e = a->get_();
            a->dispatch(e, a->m_prio);
            QF::gc(e);

            QF_CRIT_E_();

            if (a->m_eQueue.isEmpty()) { // empty queue?
                QF::readySet_.remove(p);
            }
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the Win32-QV port does not do busy-waiting
            // for events. Instead, the Win32-QV port efficiently waits until
            // QP events become available.
            QF_CRIT_X_();

            (void)WaitForSingleObject(QV_win32Event_, INFINITE);

            QF_CRIT_E_();
        }
    }
    QF_CRIT_X_();
    QF::onCleanup(); // cleanup callback
    QS_EXIT();       // cleanup the QSPY connection

    //CloseHandle(QV_win32Event_);
    //DeleteCriticalSection(&l_win32CritSect);
    //free all "fudged" event pools...
    return 0; // return success
}
//............................................................................
void QF::setTickRate(std::uint32_t ticksPerSec, int_t tickPrio) {
    if (ticksPerSec != 0U) {
        l_tickMsec = 1000UL / ticksPerSec;
    }
    else {
        l_tickMsec = 0U; // means NO system clock tick
    }
    l_tickPrio = tickPrio;
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
    Q_REQUIRE_ID(600, stkSto == nullptr);

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U); // preemption-thre.
    register_(); // make QF aware of this AO

    m_eQueue.init(qSto, qLen);

    this->init(par, m_prio); // execute initial transition (virtual call)
    QS_FLUSH(); // flush the QS trace buffer to the host
}

//............................................................................
#ifdef QF_ACTIVE_STOP
void QActive::stop(void) {
    unsubscribeAll(); // unsubscribe from all events

    // make sure the AO is no longer in "ready set"
    QF_CRIT_STAT_
    QF_CRIT_E_();
    QF::readySet_.remove(m_prio);
    QF_CRIT_X_();

    unregister_(); // remove this AO from QF
}
#endif

} // namespace QP

