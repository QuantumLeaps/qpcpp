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

// protection against including this source file in a wrong project
#ifndef QK_HPP_
    #error Source file included in a project NOT based on the QK kernel
#endif // QK_HPP_

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qk")
} // unnamed namespace

namespace QP {
namespace QK {

//............................................................................
QSchedStatus schedLock(std::uint_fast8_t const ceiling) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(100, !QK_ISR_CONTEXT_());

    // first store the previous lock prio
    QSchedStatus stat = 0xFFU; // assume scheduler NOT locked
    if (ceiling > QK_priv_.lockCeil) { // raising the lock ceiling?
        QS_BEGIN_PRE(QS_SCHED_LOCK, QK_priv_.actPrio)
            QS_TIME_PRE();   // timestamp
            // the previous lock ceiling & new lock ceiling
            QS_2U8_PRE(QK_priv_.lockCeil,
                       static_cast<std::uint8_t>(ceiling));
        QS_END_PRE()

        // previous status of the lock
        stat = static_cast<QSchedStatus>(QK_priv_.lockCeil);

        // new status of the lock
        QK_priv_.lockCeil = static_cast<std::uint8_t>(ceiling);
    }
    QF_CRIT_EXIT();

    return stat; // return the status to be saved in a stack variable
}

//............................................................................
void schedUnlock(QSchedStatus const prevCeil) noexcept {
    // has the scheduler been actually locked by the last QK::schedLock()?
    if (prevCeil != 0xFFU) {
        QF_CRIT_STAT
        QF_CRIT_ENTRY();

        Q_REQUIRE_INCRIT(200, !QK_ISR_CONTEXT_());
        Q_REQUIRE_INCRIT(210, QK_priv_.lockCeil > prevCeil);

        QS_BEGIN_PRE(QS_SCHED_UNLOCK, QK_priv_.actPrio)
            QS_TIME_PRE(); // timestamp
            // current lock ceiling (old), previous lock ceiling (new)
            QS_2U8_PRE(QK_priv_.lockCeil,
                       static_cast<std::uint8_t>(prevCeil));
        QS_END_PRE()

        // restore the previous lock ceiling
        QK_priv_.lockCeil = prevCeil;

        // find if any AOs should be run after unlocking the scheduler
        if (QK_sched_() != 0U) { // preemption needed?
            QK_activate_(); // activate any unlocked AOs
        }

        QF_CRIT_EXIT();
    }
}

} // namespace QK

//============================================================================
extern "C" {

QK_Attr QK_priv_;

//............................................................................
std::uint_fast8_t QK_sched_() noexcept {
    // NOTE: this function is entered with interrupts DISABLED

    std::uint8_t p = 0U; // assume NO activation needed
    if (QK_priv_.readySet.notEmpty()) {
        // find the highest-prio AO with non-empty event queue
        p = static_cast<std::uint8_t>(QK_priv_.readySet.findMax());

        // is the AO's prio. below the active preemption-threshold?
        if (p <= QK_priv_.actThre) {
            p = 0U; // no activation needed
        }
        else {
            // is the AO's prio. below the lock-ceiling?
            if (p <= QK_priv_.lockCeil) {
                p = 0U; // no activation needed
            }
            else {
                QK_priv_.nextPrio = p; // next AO to run
            }
        }
    }

    return p;
}

//............................................................................
std::uint_fast8_t QK_sched_act_(
    QP::QActive const * const act,
    std::uint_fast8_t const pthre_in) noexcept
{
    // NOTE: this function is entered with interrupts DISABLED

    std::uint8_t p = act->m_prio;
    if (act->m_eQueue.isEmpty()) { // empty queue?
        QK_priv_.readySet.remove(p);
    }

    if (QK_priv_.readySet.isEmpty()) {
        p = 0U; // no activation needed
    }
    else {
        // find new highest-prio AO ready to run...
        p = static_cast<std::uint8_t>(QK_priv_.readySet.findMax());
        // NOTE: p is guaranteed to be <= QF_MAX_ACTIVE

        // is the new prio. below the initial preemption-threshold?
        if (p <= pthre_in) {
            p = 0U; // no activation needed
        }
        else {
            // is the AO's prio. below the lock preemption-threshold?
            if (p <= QK_priv_.lockCeil) {
                p = 0U; // no activation needed
            }
        }
    }

    return p;
}

//............................................................................
void QK_activate_() {
    // NOTE: this function is entered with interrupts DISABLED

    std::uint8_t const prio_in = QK_priv_.actPrio; // save initial prio.
    std::uint8_t p = QK_priv_.nextPrio; // next prio to run

    Q_REQUIRE_INCRIT(520, prio_in <= QF_MAX_ACTIVE);
    Q_REQUIRE_INCRIT(530, (0U < p) && (p <= QF_MAX_ACTIVE));

#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
    std::uint8_t pprev = prio_in;
#endif // QF_ON_CONTEXT_SW || Q_SPY

    QK_priv_.nextPrio = 0U; // clear for the next time

    std::uint8_t pthre_in = 0U; // assume preempting the idle thread
    if (prio_in != 0U) { // preempting NOT the idle thread
        QP::QActive const * const a = QP::QActive::registry_[prio_in];
        Q_ASSERT_INCRIT(540, a != nullptr);

        pthre_in = a->m_pthre;
    }

    // loop until no more ready-to-run AOs of higher pthre than the initial
    do  {
        QP::QActive * const a = QP::QActive::registry_[p];
        Q_ASSERT_INCRIT(570, a != nullptr); // the AO must be registered
        std::uint8_t const pthre = a->m_pthre;

        // set new active prio. and preemption-threshold
        QK_priv_.actPrio = p;
        QK_priv_.actThre = pthre;

#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
        if (p != pprev) { // changing threads?

            QS_BEGIN_PRE(QP::QS_SCHED_NEXT, p)
                QS_TIME_PRE();     // timestamp
                QS_2U8_PRE(p, pprev);
            QS_END_PRE()

#ifdef QF_ON_CONTEXT_SW
            QF_onContextSw(QP::QActive::registry_[pprev], a);
#endif // QF_ON_CONTEXT_SW

            pprev = p; // update previous prio.
        }
#endif // QF_ON_CONTEXT_SW || Q_SPY

        QF_INT_ENABLE(); // unconditionally enable interrupts

        QP::QEvt const * const e = a->get_();

        // dispatch event (virtual call)
        a->dispatch(e, a->m_prio);
#if (QF_MAX_EPOOL > 0U)
        QP::QF::gc(e);
#endif

        // determine the next highest-prio. AO ready to run...
        QF_INT_DISABLE(); // unconditionally disable interrupts
        p = QK_sched_act_(a, pthre_in); // schedule next AO

    } while (p != 0U);

    // restore the active prio. and preemption-threshold
    QK_priv_.actPrio = prio_in;
    QK_priv_.actThre = pthre_in;

#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
    if (prio_in != 0U) { // resuming an active object?
        QS_BEGIN_PRE(QP::QS_SCHED_NEXT, prio_in)
            QS_TIME_PRE();     // timestamp
            // prio. of the resumed AO, previous prio.
            QS_2U8_PRE(prio_in, pprev);
        QS_END_PRE()

#ifdef QF_ON_CONTEXT_SW
        QF_onContextSw(QP::QActive::registry_[pprev],
                       QP::QActive::registry_[prio_in]);
#endif // QF_ON_CONTEXT_SW
    }
    else {  // resuming prio.==0 --> idle
        QS_BEGIN_PRE(QP::QS_SCHED_IDLE, pprev)
            QS_TIME_PRE();     // timestamp
            QS_U8_PRE(pprev);  // previous prio.
        QS_END_PRE()

#ifdef QF_ON_CONTEXT_SW
        QF_onContextSw(QP::QActive::registry_[pprev], nullptr);
#endif // QF_ON_CONTEXT_SW
    }

#endif // QF_ON_CONTEXT_SW || Q_SPY
}
} // extern "C"

//............................................................................
namespace QF {

//............................................................................
void init() {
    bzero_(&QF::priv_,                 sizeof(QF::priv_));
    bzero_(&QK_priv_,                  sizeof(QK_priv_));
    bzero_(&QActive::registry_[0],     sizeof(QActive::registry_));

    // setup the QK scheduler as initially locked and not running
    QK_priv_.lockCeil = (QF_MAX_ACTIVE + 1U); // scheduler locked

#ifdef QK_INIT
    QK_INIT(); // port-specific initialization of the QK kernel
#endif
}

//............................................................................
void stop() {
    onCleanup(); // application-specific cleanup callback
    // nothing else to do for the preemptive QK kernel
}

//............................................................................
int_t run() {
    QF_INT_DISABLE();
#ifdef Q_SPY
    // produce the QS_QF_RUN trace record
    QS::beginRec_(QS_REC_NUM_(QS_QF_RUN));
    QS::endRec_();
#endif // Q_SPY

#ifdef QK_START
    QK_START(); // port-specific startup of the QK kernel
#endif

    QK_priv_.lockCeil = 0U; // unlock the QK scheduler

#ifdef QF_ON_CONTEXT_SW
    // officially switch to the idle context
    QF_onContextSw(nullptr, QActive::registry_[QK_priv_.nextPrio]);
#endif

    // activate AOs to process events posted so far
    if (QK_sched_() != 0U) {
        QK_activate_();
    }

    QF_INT_ENABLE();

    onStartup(); // app. callback: configure and enable interrupts

    for (;;) { // QK idle loop...
        QK::onIdle(); // application-specific QK on-idle callback
    }

#ifdef __GNUC__  // GNU compiler?
    return 0;
#endif
}

} // namespace QF

//............................................................................
void QActive::start(
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);  // not needed in QK
    Q_UNUSED_PAR(stkSize); // not needed in QK

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(300, stkSto == nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-prio.
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U);   // preemption-thre.
    register_(); // make QF aware of this AO

    m_eQueue.init(qSto, qLen); // init the built-in queue

    // top-most initial tran. (virtual call)
    this->init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // See if this AO needs to be scheduled if QK is already running
    QF_CRIT_ENTRY();
    if (QK_sched_() != 0U) { // activation needed?
        QK_activate_();
    }
    QF_CRIT_EXIT();
}

} // namespace QP
