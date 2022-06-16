//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief Cooperative QV kernel, definition of QP::QV_readySet_ and
//! implementation of kernel-specific functions.

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope internal interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QV_HPP
    #error "Source file included in a project NOT based on the QV kernel"
#endif // QV_HPP

//============================================================================
namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qv")

} // unnamed namespace

//============================================================================
extern "C" {

// Package-scope objects .....................................................
QPSet  QV_readySet_;  // ready set of AOs

} // extern "C"

//============================================================================
namespace QP {

void QF::init(void) {
    QF_maxPool_      = 0U;
    QF_subscrList_   = nullptr;
    QF_maxPubSignal_ = 0;

    bzero(&QF::timeEvtHead_[0], sizeof(QF::timeEvtHead_));
    bzero(&active_[0], sizeof(active_));
    bzero(&QV_readySet_, sizeof(QV_readySet_));

#ifdef QV_INIT
    QV_INIT(); // port-specific initialization of the QV kernel
#endif
}

//............................................................................
void QF::stop(void) {
    onCleanup(); // cleanup callback
    // nothing else to do for the "vanilla" kernel
}

//............................................................................
int_t QF::run(void) {
#ifdef Q_SPY
    std::uint_fast8_t pprev = 0U; // previous priority
#endif

    onStartup(); // startup callback

    // the combined event-loop and background-loop of the QV kernel...
    QF_INT_DISABLE();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_NOCRIT_PRE_(QS_QF_RUN, 0U)
    QS_END_NOCRIT_PRE_()

    for (;;) {

        // find the maximum priority AO ready to run
        if (QV_readySet_.notEmpty()) {
            std::uint_fast8_t const p = QV_readySet_.findMax();
            QActive * const a = active_[p];

#ifdef Q_SPY
            QS_BEGIN_NOCRIT_PRE_(QS_SCHED_NEXT, a->m_prio)
                QS_TIME_PRE_(); // timestamp
                QS_2U8_PRE_(p, pprev);// scheduled prio & previous prio
            QS_END_NOCRIT_PRE_()

            pprev = p; // update previous priority
#endif // Q_SPY

            QF_INT_ENABLE();

            // perform the run-to-completion (RTC) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The "Vanialla" kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const * const e = a->get_();
            a->dispatch(e, a->m_prio);
            gc(e);

            QF_INT_DISABLE();

            if (a->m_eQueue.isEmpty()) { // empty queue?
                QV_readySet_.rmove(p);
            }
        }
        else { // no AO ready to run --> idle
#ifdef Q_SPY
            if (pprev != 0U) {
                QS_BEGIN_NOCRIT_PRE_(QS_SCHED_IDLE, 0U)
                    QS_TIME_PRE_();    // timestamp
                    QS_U8_PRE_(pprev); // previous prio
                QS_END_NOCRIT_PRE_()

                pprev = 0U; // update previous prio
            }
#endif // Q_SPY

            // QV::onIdle() must be called with interrupts DISABLED because
            // the determination of the idle condition (no events in the
            // queues) can change at any time by an interrupt posting events
            // to a queue. QV::onIdle() MUST enable interrupts internally,
            // perhaps at the same time as putting the CPU into a power-saving
            // mode.
            QP::QV::onIdle();

            QF_INT_DISABLE();
        }
    }
#ifdef __GNUC__ // GNU compiler?
    return 0;
#endif
}

//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    static_cast<void>(stkSize); // unused paramteter in the QV port

    //! @pre the priority must be in range and the stack storage must not
    //! be provided, because the QV kernel does not need per-AO stacks.
    //!
    Q_REQUIRE_ID(500,
        (0U < prio) && (prio <= QF_MAX_ACTIVE)
        && (stkSto == nullptr));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = static_cast<std::uint8_t>(prio);  // set the QF prio of this AO

    QF::add_(this); // make QF aware of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    QS_FLUSH(); // flush the trace buffer to the host
}

} // namespace QP
