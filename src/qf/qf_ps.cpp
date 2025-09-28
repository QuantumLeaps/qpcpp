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

QSubscrList * QActive_subscrList_;
QSignal QActive_maxPubSignal_;

//............................................................................
void QActive::psInit(
    QSubscrList * const subscrSto,
    QSignal const maxSignal) noexcept
{
    // provided subscSto must be valid
    Q_REQUIRE_INCRIT(100, subscrSto != nullptr);

    // provided maximum of subscribed signals must be >= Q_USER_SIG
    Q_REQUIRE_INCRIT(110, maxSignal >= Q_USER_SIG);

    QActive_subscrList_   = subscrSto;
    QActive_maxPubSignal_ = static_cast<QSignal>(maxSignal);

    // initialize all signals in the subscriber list...
    for (QSignal sig = 0U; sig < maxSignal; ++sig) {
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

    // the published event must be valid
    Q_REQUIRE_INCRIT(200, e != nullptr);

    QSignal const sig = e->sig;

    // published event signal must not exceed the maximum
    Q_REQUIRE_INCRIT(240, sig < QActive_maxPubSignal_);

    // make a local, modifiable copy of the subscriber set
    QPSet subscrSet = QActive_subscrList_[sig].m_set;

    QS_BEGIN_PRE(QS_QF_PUBLISH, qsId)
        QS_TIME_PRE();          // the timestamp
        QS_OBJ_PRE(sender);     // the sender object
        QS_SIG_PRE(sig);        // the signal of the event
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
    QS_END_PRE()

    if (e->poolNum_ != 0U) { // is it a mutable event?
        // NOTE: The reference counter of a mutable event is incremented to
        // prevent premature recycling of the event while multicasting is
        // still in progress. The garbage collector step (QF_gc()) at the
        // end of the function decrements the reference counter and recycles
        // the event if the counter drops to zero. This covers the case when
        // event was published without any subscribers.
        QEvt_refCtr_inc_(e);
    }

    QF_CRIT_EXIT();

    if (subscrSet.notEmpty()) { // any subscribers?
        multicast_(&subscrSet, e, sender); // multicast to all
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
#if (QF_MAX_EPOOL > 0U)
    QF::gc(e); // recycle the event to avoid a leak
#endif
}

//............................................................................
void QActive::multicast_(
    QPSet * const subscrSet,
    QEvt const * const e,
    void const * const sender)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    // highest-prio subscriber ('subscrSet' guaranteed to be NOT empty)
    std::uint8_t p = static_cast<std::uint8_t>(subscrSet->findMax());

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // p != 0 is guaranteed as the result of QPSet_findMax()
    Q_ASSERT_INCRIT(300, p <= QF_MAX_ACTIVE);
    QActive *a = QActive_registry_[p];

    // the active object must be registered (started)
    Q_ASSERT_INCRIT(310, a != nullptr);

    QF_CRIT_EXIT();

    QF_SCHED_STAT_
    QF_SCHED_LOCK_(p); // lock the scheduler up to AO's prio

    // NOTE: the following loop does not need the fixed loop bound check
    // because the local subscriber set 'subscrSet' can hold at most
    // QF_MAX_ACTIVE elements (rounded up to the nearest 8), which are
    // removed one by one at every pass.
    for (;;) { // loop over all subscribers

        // POST() asserts internally if the queue overflows
        a->POST(e, sender);

        subscrSet->remove(p); // remove the handled subscriber
        if (subscrSet->isEmpty()) {  // no more subscribers?
            break;
        }

        // find the next highest-prio subscriber
        p = static_cast<std::uint8_t>(subscrSet->findMax());

        QF_CRIT_ENTRY();

        a = QActive_registry_[p];

        // the AO must be registered with the framework
        Q_ASSERT_INCRIT(340, a != nullptr);

        QF_CRIT_EXIT();
    }

    QF_SCHED_UNLOCK_(); // unlock the scheduler
}

//............................................................................
void QActive::subscribe(QSignal const sig) const noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    std::uint8_t const p = m_prio;

    // the AO's prio. must be in range
    Q_REQUIRE_INCRIT(420, (0U < p) && (p <= QF_MAX_ACTIVE));

    // the subscriber AO must be registered (started)
    Q_REQUIRE_INCRIT(440, this == QActive_registry_[p]);

    // the sig parameter must not overlap reserved signals
    Q_REQUIRE_INCRIT(460, sig >= Q_USER_SIG);

    // the subscribed signal must be below the maximum of published signals
    Q_REQUIRE_INCRIT(480, static_cast<QSignal>(sig) < QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_SUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(this); // this active object
    QS_END_PRE()

    // insert the AO's prio. into the subscriber set for the signal
    QActive_subscrList_[sig].m_set.insert(p);

    QF_CRIT_EXIT();
}

//............................................................................
void QActive::unsubscribe(QSignal const sig) const noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    std::uint8_t const p = m_prio;

    // the AO's prio. must be in range
    Q_REQUIRE_INCRIT(520, (0U < p) && (p <= QF_MAX_ACTIVE));

    // the subscriber AO must be registered (started)
    Q_REQUIRE_INCRIT(540, this == QActive_registry_[p]);

    // the sig parameter must not overlap reserved signals
    Q_REQUIRE_INCRIT(560, sig >= Q_USER_SIG);
    Q_REQUIRE_INCRIT(580, static_cast<QSignal>(sig) < QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_ACTIVE_UNSUBSCRIBE, p)
        QS_TIME_PRE();    // timestamp
        QS_SIG_PRE(sig);  // the signal of this event
        QS_OBJ_PRE(this); // this active object
    QS_END_PRE()

    // remove the AO's prio. from the subscriber set for the signal
    QActive_subscrList_[sig].m_set.remove(p);

    QF_CRIT_EXIT();
}

//............................................................................
void QActive::unsubscribeAll() const noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    std::uint8_t const p = m_prio;

    // the AO's prio. must be in range
    Q_REQUIRE_INCRIT(620, (0U < p) && (p <= QF_MAX_ACTIVE));

    // the subscriber AO must be registered (started)
    Q_REQUIRE_INCRIT(640, this == QActive_registry_[p]);

    QSignal const maxPubSig = QActive_maxPubSignal_;

    // the maximum of published signals must not overlap the reserved signals
    Q_REQUIRE_INCRIT(670, maxPubSig >= static_cast<QSignal>(Q_USER_SIG));

    QF_CRIT_EXIT();

    // remove this AO's prio. from subscriber lists of all published signals
    for (QSignal sig = static_cast<QSignal>(Q_USER_SIG);
         sig < maxPubSig;
         ++sig)
    {
        QF_CRIT_ENTRY();

        if (QActive_subscrList_[sig].m_set.hasElement(p)) {
            // remove the AO's prio. from the subscriber set for the signal
            QActive_subscrList_[sig].m_set.remove(p);

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
