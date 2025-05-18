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
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_ps")
} // unnamed namespace

namespace QP {

QSubscrList * QActive::subscrList_;
QSignal QActive::maxPubSignal_;

//............................................................................
void QActive::psInit(
    QSubscrList * const subscrSto,
    enum_t const maxSignal) noexcept
{
    Q_REQUIRE_INCRIT(100, subscrSto != nullptr);
    Q_REQUIRE_INCRIT(110, maxSignal >= Q_USER_SIG);

    subscrList_   = subscrSto;
    maxPubSignal_ = static_cast<QSignal>(maxSignal);

    // initialize the subscriber list
    for (enum_t sig = 0; sig < maxSignal; ++sig) {
        subscrSto[sig].m_set.setEmpty();
    }
}

//............................................................................
void QActive::publish_(
    QEvt const * const e,
    void const * const sender,
    std::uint_fast8_t const qsId) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != nullptr);

    QSignal const sig = e->sig;
    Q_REQUIRE_INCRIT(240, sig < maxPubSignal_);

    // make a local, modifiable copy of the subscriber set
    QPSet subscrSet = subscrList_[sig].m_set;

    QS_BEGIN_PRE(QS_QF_PUBLISH, qsId)
        QS_TIME_PRE();          // the timestamp
        QS_OBJ_PRE(sender);     // the sender object
        QS_SIG_PRE(sig);        // the signal of the event
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
    QS_END_PRE()

    // is it a mutable event?
    if (e->poolNum_ != 0U) {
        // NOTE: The reference counter of a mutable event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF::gc()) decrements the reference counter and
        // recycles the event if the counter drops to zero. This covers the
        // case when the event was published without any subscribers.
        Q_ASSERT_INCRIT(260, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e);
    }

    QF_CRIT_EXIT();

    if (subscrSet.notEmpty()) { // any subscribers?
        // highest-prio subscriber
        std::uint8_t p = static_cast<std::uint8_t>(subscrSet.findMax());

        QF_CRIT_ENTRY();

        QActive *a = registry_[p];
        Q_ASSERT_INCRIT(300, a != nullptr);

        QF_CRIT_EXIT();

        QF_SCHED_STAT_
        QF_SCHED_LOCK_(p); // lock the scheduler up to AO's prio

        std::uint_fast8_t lbound = QF_MAX_ACTIVE + 1U; // fixed loop bound
        for (;;) { // loop over all subscribers

            // POST() asserts internally if the queue overflows
            a->POST(e, sender);

            subscrSet.remove(p); // remove the handled subscriber
            if (subscrSet.isEmpty()) {  // no more subscribers?
                break;
            }

            // highest-prio subscriber
            p = static_cast<std::uint8_t>(subscrSet.findMax());

            QF_CRIT_ENTRY();

            a = registry_[p];
            // the AO must be registered with the framework
            Q_ASSERT_INCRIT(340, a != nullptr);

            --lbound; // fixed loop bound
            Q_INVARIANT_INCRIT(370, lbound > 0U);

            QF_CRIT_EXIT();
        }

        QF_SCHED_UNLOCK_(); // unlock the scheduler
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
#if (QF_MAX_EPOOL > 0U)
    QF::gc(e); // recycle the event to avoid a leak
#endif
}

//............................................................................
void QActive::subscribe(enum_t const sig) const noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // check this AO ("this") and the registration...
    std::uint8_t const p = m_prio;
    Q_REQUIRE_INCRIT(420, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(440, this == registry_[p]);

    // check sig parameter and subscription list...
    Q_REQUIRE_INCRIT(460, sig >= Q_USER_SIG);
    Q_REQUIRE_INCRIT(480, static_cast<QSignal>(sig) < maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_SUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(this); // this active object
    QS_END_PRE()

    // insert the prio. into the subscriber set
    subscrList_[sig].m_set.insert(p);

    QF_CRIT_EXIT();
}

//............................................................................
void QActive::unsubscribe(enum_t const sig) const noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // check this AO ("this") and the registration...
    std::uint8_t const p = m_prio;
    Q_REQUIRE_INCRIT(520, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(540, this == registry_[p]);

    // check sig parameter and subscription list...
    Q_REQUIRE_INCRIT(560, sig >= Q_USER_SIG);
    Q_REQUIRE_INCRIT(580, static_cast<QSignal>(sig) < maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_UNSUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(this); // this active object
    QS_END_PRE()

    // remove the prio. from the subscriber set
    subscrList_[sig].m_set.remove(p);

    QF_CRIT_EXIT();
}

//............................................................................
void QActive::unsubscribeAll() const noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // check this AO ("this") and the registration...
    std::uint8_t const p = m_prio;
    Q_REQUIRE_INCRIT(620, (0U < p) && (p <= QF_MAX_ACTIVE));
    Q_REQUIRE_INCRIT(640, this == registry_[p]);

    // check maxPubSignal_ and subscription list...
    QSignal const maxPubSig = maxPubSignal_;
    Q_REQUIRE_INCRIT(670, maxPubSig >= static_cast<QSignal>(Q_USER_SIG));

    QF_CRIT_EXIT();

    for (QSignal sig = static_cast<QSignal>(Q_USER_SIG);
         sig < maxPubSig;
         ++sig)
    {
        QF_CRIT_ENTRY();

        if (subscrList_[sig].m_set.hasElement(p)) {
            subscrList_[sig].m_set.remove(p);
            QS_BEGIN_PRE(QS_QF_ACTIVE_UNSUBSCRIBE, p)
                QS_TIME_PRE();    // timestamp
                QS_SIG_PRE(sig);  // the signal of this event
                QS_OBJ_PRE(this); // this active object
            QS_END_PRE()
        }
        QF_CRIT_EXIT();

        QF_CRIT_EXIT_NOP(); // prevent merging critical sections
    }
}

} // namespace QP
