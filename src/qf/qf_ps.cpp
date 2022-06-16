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
//! @brief QF/C++ Publish-Subscribe services
//! definitions.

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {

Q_DEFINE_THIS_MODULE("qf_ps")

} // unnamed namespace

//============================================================================
namespace QP {

// Package-scope objects .....................................................
QSubscrList *QF_subscrList_;
enum_t QF_maxPubSignal_;

//............................................................................
void QF::psInit(QSubscrList * const subscrSto,
                enum_t const maxSignal) noexcept
{
    QF_subscrList_   = subscrSto;
    QF_maxPubSignal_ = maxSignal;

    // zero the subscriber list, so that the framework can start correctly
    // even if the startup code fails to clear the uninitialized data
    // (as is required by the C++ Standard)
    bzero(subscrSto, static_cast<unsigned>(maxSignal) * sizeof(QSubscrList));
}

//............................................................................
#ifndef Q_SPY
void QF::publish_(QEvt const * const e) noexcept {
#else
void QF::publish_(QEvt const * const e,
                  void const * const sender,
                  std::uint_fast8_t const qs_id) noexcept
{
#endif
    //! @pre the published signal must be within the configured range
    Q_REQUIRE_ID(100, static_cast<enum_t>(e->sig) < QF_maxPubSignal_);

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_PUBLISH, qs_id)
        QS_TIME_PRE_();                      // the timestamp
        QS_OBJ_PRE_(sender);                 // the sender object
        QS_SIG_PRE_(e->sig);                 // the signal of the event
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
    QS_END_NOCRIT_PRE_()

    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        // NOTE: The reference counter of a dynamic event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF::gc()) decrements the reference counter and
        // recycles the event if the counter drops to zero. This covers the
        // case when the event was published without any subscribers.
        //
        QF_EVT_REF_CTR_INC_(e);
    }

    // make a local, modifiable copy of the subscriber list
    QPSet subscrList = QF_subscrList_[e->sig];
    QF_CRIT_X_();

    if (subscrList.notEmpty()) { // any subscribers?
        // the highest-prio subscriber
        std::uint_fast8_t p = subscrList.findMax();
        QF_SCHED_STAT_

        QF_SCHED_LOCK_(p); // lock the scheduler up to prio 'p'
        do { // loop over all subscribers */
            // the prio of the AO must be registered with the framework
            Q_ASSERT_ID(210, active_[p] != nullptr);

            // POST() asserts internally if the queue overflows
            static_cast<void>(active_[p]->POST(e, sender));

            subscrList.rmove(p); // remove the handled subscriber
            if (subscrList.notEmpty()) {  // still more subscribers?
                p = subscrList.findMax(); // the highest-prio subscriber
            }
            else {
                p = 0U; // no more subscribers
            }
        } while (p != 0U);
        QF_SCHED_UNLOCK_(); // unlock the scheduler
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
    //
    gc(e);
}

//............................................................................
void QActive::subscribe(enum_t const sig) const noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    Q_REQUIRE_ID(300, (Q_USER_SIG <= sig)
              && (sig < QF_maxPubSignal_)
              && (0U < p) && (p <= QF_MAX_ACTIVE)
              && (QF::active_[p] == this));

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_SUBSCRIBE, m_prio)
        QS_TIME_PRE_();    // timestamp
        QS_SIG_PRE_(sig);  // the signal of this event
        QS_OBJ_PRE_(this); // this active object
    QS_END_NOCRIT_PRE_()

    QF_subscrList_[sig].insert(p); // insert into subscriber-list
    QF_CRIT_X_();
}

//............................................................................
void QActive::unsubscribe(enum_t const sig) const noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    //! @pre the signal and the priority must be in range, the AO must also
    // be registered with the framework
    Q_REQUIRE_ID(400, (Q_USER_SIG <= sig)
                      && (sig < QF_maxPubSignal_)
                      && (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (QF::active_[p] == this));

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_UNSUBSCRIBE, m_prio)
        QS_TIME_PRE_();         // timestamp
        QS_SIG_PRE_(sig);       // the signal of this event
        QS_OBJ_PRE_(this);      // this active object
    QS_END_NOCRIT_PRE_()

    QF_subscrList_[sig].rmove(p); // remove from subscriber-list

    QF_CRIT_X_();
}

//............................................................................
void QActive::unsubscribeAll(void) const noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    Q_REQUIRE_ID(500, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (QF::active_[p] == this));

    for (enum_t sig = Q_USER_SIG; sig < QF_maxPubSignal_; ++sig) {
        QF_CRIT_STAT_
        QF_CRIT_E_();
        if (QF_subscrList_[sig].hasElement(p)) {
            QF_subscrList_[sig].rmove(p);

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_UNSUBSCRIBE, m_prio)
                QS_TIME_PRE_();     // timestamp
                QS_SIG_PRE_(sig);   // the signal of this event
                QS_OBJ_PRE_(this);  // this active object
            QS_END_NOCRIT_PRE_()

        }
        QF_CRIT_X_();

        // prevent merging critical sections
        QF_CRIT_EXIT_NOP();
    }
}

} // namespace QP
