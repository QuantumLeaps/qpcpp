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
//! @brief QXK/C++ preemptive kernel core functions
//! public interface.

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qxk_pkg.hpp"      // QXK package-scope internal interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QXK_HPP
    #error "Source file included in a project NOT based on the QXK kernel"
#endif // QXK_HPP

//============================================================================
namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qxk")

// Local-scope objects .......................................................
class QXKIdleThread : public QP::QActive {
public:
    QXKIdleThread() : QP::QActive(nullptr)
    {}
};

static QXKIdleThread l_idleThread;

} // unnamed namespace

//============================================================================
extern "C" {

QXK_Attr QXK_attr_;   // global attributes of the QXK kernel

} // extern "C"

//============================================================================
namespace QP {

//............................................................................
void QF::init(void) {
    QF_maxPool_      = 0U;
    QF_subscrList_   = nullptr;
    QF_maxPubSignal_ = 0;

    bzero(&timeEvtHead_[0], sizeof(timeEvtHead_));
    bzero(&active_[0],      sizeof(active_));
    bzero(&QXK_attr_,       sizeof(QXK_attr_));
    bzero(&l_idleThread,    sizeof(l_idleThread));

    // setup the QXK scheduler as initially locked and not running
    QXK_attr_.lockPrio = QF_MAX_ACTIVE + 1U;

    // setup the QXK idle loop...
    active_[0] = &l_idleThread; // register the idle thread with QF
    QXK_attr_.idleThread = &l_idleThread; // save the idle thread ptr
    QXK_attr_.actPrio = l_idleThread.m_prio; // set the base priority

#ifdef QXK_INIT
    QXK_INIT(); // port-specific initialization of the QXK kernel
#endif
}

//............................................................................
void QF::stop(void) {
    onCleanup(); // application-specific cleanup callback
    // nothing else to do for the preemptive QXK kernel
}

//............................................................................
//! process all events posted during initialization
static void initial_events(void); // prototype
static void initial_events(void) {
    QXK_attr_.lockPrio = 0U; // unlock the scheduler

    // any active objects need to be scheduled before starting event loop?
    if (QXK_sched_() != 0U) {
        QXK_activate_(); // process all events produced so far
    }
}

//............................................................................
int_t QF::run(void) {
    QF_INT_DISABLE();
    initial_events(); // process all events posted during initialization
    onStartup(); // application-specific startup callback

    // produce the QS_QF_RUN trace record
    QS_BEGIN_NOCRIT_PRE_(QS_QF_RUN, 0U)
    QS_END_NOCRIT_PRE_()

    QF_INT_ENABLE();

    // the QXK idle loop...
    for (;;) {
        QXK::onIdle(); // application-specific QXK idle callback
    }

#ifdef __GNUC__  // GNU compiler?
    return 0;
#endif
}

//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    //! @pre AO cannot be started:
    //! - from an ISR;
    //! - the priority must be in range;
    //! - the stack storage must NOT be provided (because the QXK kernel does
    //! not need per-AO stacks).
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_())
        && (0U < prio) && (prio <= QF_MAX_ACTIVE)
        && (stkSto == nullptr)
        && (stkSize == 0U));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_osObject  = nullptr; // no private stack for AO
    m_prio      = static_cast<std::uint8_t>(prio); // prio of the AO
    m_dynPrio   = static_cast<std::uint8_t>(prio); // dynamic prio of the AO
    QF::add_(this);  // make QF aware of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    QS_FLUSH(); // flush the trace buffer to the host

    // see if this AO needs to be scheduled in case QXK is running
    QF_CRIT_STAT_
    QF_CRIT_E_();
    if (QXK_sched_() != 0U) { // activation needed?
        QXK_activate_();
    }
    QF_CRIT_X_();
}

//............................................................................
QSchedStatus QXK::schedLock(std::uint_fast8_t const ceiling) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    //! @pre The QXK scheduler lock:
    //! - cannot be called from an ISR;
    Q_REQUIRE_ID(400, !QXK_ISR_CONTEXT_());

    // first store the previous lock prio if below the ceiling
    QSchedStatus stat;
    if (static_cast<std::uint_fast8_t>(QXK_attr_.lockPrio) < ceiling) {
        stat = (static_cast<QSchedStatus>(QXK_attr_.lockPrio) << 8U);
        QXK_attr_.lockPrio = static_cast<std::uint8_t>(ceiling);

        QS_BEGIN_NOCRIT_PRE_(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE_(); // timestamp
            // the previous lock prio & new lock prio
            QS_2U8_PRE_(stat, QXK_attr_.lockPrio);
        QS_END_NOCRIT_PRE_()

        // add the previous lock holder priority
        stat |= static_cast<QSchedStatus>(QXK_attr_.lockHolder);
        QXK_attr_.lockHolder = (QXK_attr_.curr != nullptr)
                               ? QXK_attr_.curr->m_prio
                               : 0U;
    }
    else {
       stat = 0xFFU;
    }
    QF_CRIT_X_();

    return stat; // return the status to be saved in a stack variable
}

//............................................................................
void QXK::schedUnlock(QSchedStatus const stat) noexcept {
    // has the scheduler been actually locked by the last QXK_schedLock()?
    if (stat != 0xFFU) {
        std::uint_fast8_t const lockPrio
            = static_cast<std::uint_fast8_t>(QXK_attr_.lockPrio);
        std::uint_fast8_t const prevPrio
            = static_cast<std::uint_fast8_t>(stat >> 8U);
        QF_CRIT_STAT_
        QF_CRIT_E_();

        //! @pre The scheduler cannot be unlocked:
        //! - from the ISR context; and
        //! - the current lock priority must be greater than the previous
        Q_REQUIRE_ID(500, (!QXK_ISR_CONTEXT_())
                          && (lockPrio > prevPrio));

        QS_BEGIN_NOCRIT_PRE_(QS_SCHED_UNLOCK, 0U)
            QS_TIME_PRE_(); // timestamp
            // prio before unlocking & prio after unlocking
            QS_2U8_PRE_(lockPrio, prevPrio);
        QS_END_NOCRIT_PRE_()

        // restore the previous lock priority and lock holder
        QXK_attr_.lockPrio   = static_cast<std::uint8_t>(prevPrio);
        QXK_attr_.lockHolder = static_cast<std::uint8_t>(stat & 0xFFU);

        // find the highest-prio thread ready to run
        if (QXK_sched_() != 0U) { // priority found?
            QXK_activate_(); // activate any unlocked basic threads
        }

        QF_CRIT_X_();
    }
}

} // namespace QP


//============================================================================
extern "C" {

//............................................................................
std::uint_fast8_t QXK_sched_(void) noexcept {
    // find the highest-prio thread ready to run
    std::uint_fast8_t p = QXK_attr_.readySet.findMax();

    // below the lock prio?
    if (p <= static_cast<std::uint_fast8_t>(QXK_attr_.lockPrio)) {
        // dynamic priority of the thread holding the lock
        p = static_cast<std::uint_fast8_t>(
             QP::QF::active_[QXK_attr_.lockHolder]->m_dynPrio);
        if (p != 0U) {
            Q_ASSERT_ID(610, QXK_attr_.readySet.hasElement(p));
        }
    }

    QP::QActive * const next = QP::QF::active_[p];

    // the thread found must be registered in QF
    Q_ASSERT_ID(620, next != nullptr);

    // is the current thread a basic-thread?
    if (QXK_attr_.curr == nullptr) {

        // is next a basic-thread?
        if (next->m_osObject == nullptr) {
            if (p > static_cast<std::uint_fast8_t>(QXK_attr_.actPrio)) {
                QXK_attr_.next = next; // set the next AO to activate
            }
            else {
                QXK_attr_.next = nullptr;
                p = 0U; // no activation needed
            }
        }
        else {  // this is an extened-thread

            QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_NEXT, next->m_prio)
                QS_TIME_PRE_();  // timestamp
                // prio of the next AO & prio of the curr AO
                QS_2U8_PRE_(p, QXK_attr_.actPrio);
            QS_END_NOCRIT_PRE_()

            QXK_attr_.next = next;
            p = 0U; // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
    }
    else { // currently executing an extended-thread

        // is the next thread different from the current?
        if (next != QXK_attr_.curr) {

            QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_NEXT, next->m_prio)
                QS_TIME_PRE_(); // timestamp
                // next prio & current prio
                QS_2U8_PRE_(p, QXK_attr_.curr->m_prio);
            QS_END_NOCRIT_PRE_()

            QXK_attr_.next = next;
            p = 0U; // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
        else { // next is the same as current
            QXK_attr_.next = nullptr; // no need to context-switch
            p = 0U; // no activation needed
        }
    }
    return p;
}

//............................................................................
void QXK_activate_(void) {
    std::uint_fast8_t const pin =
        static_cast<std::uint_fast8_t>(QXK_attr_.actPrio);
    QP::QActive *a = QXK_attr_.next; // the next AO (basic-thread) to run

    // QXK Context switch callback defined or QS tracing enabled?
#if (defined QXK_ON_CONTEXT_SW) || (defined Q_SPY)
    std::uint_fast8_t pprev = pin;
#endif // QXK_ON_CONTEXT_SW || Q_SPY

    //! @pre QXK_attr_.next must be valid
    Q_REQUIRE_ID(700, (a != nullptr) && (pin < QF_MAX_ACTIVE));

    // dynamic priority of the next AO
    std::uint_fast8_t p = static_cast<std::uint_fast8_t>(a->m_dynPrio);

    // loop until no more ready-to-run AOs of higher prio than the initial
    do  {
        a = QP::QF::active_[p]; // obtain the pointer to the AO

        QXK_attr_.actPrio = static_cast<std::uint8_t>(p); // new active prio
        QXK_attr_.next = nullptr; // clear the next AO

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_NEXT, a->m_prio)
            QS_TIME_PRE_();         // timestamp
            // next prio & prev prio
            QS_2U8_PRE_(p, pprev);
        QS_END_NOCRIT_PRE_()

#if (defined QXK_ON_CONTEXT_SW) || (defined Q_SPY)
        if (p != pprev) {  // changing threads?

#ifdef QXK_ON_CONTEXT_SW
            Q_ASSERT_ID(710, pprev < QF_MAX_ACTIVE);

            // context-switch callback
            QXK_onContextSw(((pprev!=0U) ? QP::QF::active_[pprev] : nullptr),
                            a);
#endif // QXK_ON_CONTEXT_SW

             pprev = p; // update previous priority
         }
#endif // QXK_ON_CONTEXT_SW || Q_SPY

        QF_INT_ENABLE(); // unconditionally enable interrupts

        // perform the run-to-completion (RTC) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and QActive_get_() asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QP::QEvt const * const e = a->get_();
        a->dispatch(e, a->m_prio);
        QP::QF::gc(e);

        QF_INT_DISABLE(); // unconditionally disable interrupts

        if (a->m_eQueue.isEmpty()) { // empty queue?
            QXK_attr_.readySet.rmove(p);
        }

        // find new highest-prio AO ready to run...
        // NOTE: this part must match the QXK_sched_(),
        // current is a basic-thread path.
        p = QXK_attr_.readySet.findMax();

        // below scheduler lock?
        if (p <= static_cast<std::uint_fast8_t>(QXK_attr_.lockPrio)) {
            p = static_cast<std::uint_fast8_t>(QXK_attr_.lockHolder);
            if (p != 0U) {
                Q_ASSERT_ID(710, QXK_attr_.readySet.hasElement(p));
            }
        }
        a = QP::QF::active_[p];

        // the AO must be registered in QF
        Q_ASSERT_ID(720, a != nullptr);

        // is the next a basic thread?
        if (a->m_osObject == nullptr) {
            if (p > pin) {
                QXK_attr_.next = a;
            }
            else {
                QXK_attr_.next = nullptr;
                p = 0U; // no activation needed
            }
        }
        else {  // next is the extened thread

            QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_NEXT, a->m_prio)
                QS_TIME_PRE_(); // timestamp
                // next prio & curr prio
                QS_2U8_PRE_(p, QXK_attr_.actPrio);
            QS_END_NOCRIT_PRE_()

            QXK_attr_.next = a;
            p = 0U; // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
    } while (p != 0U); // while activation needed

    QXK_attr_.actPrio = static_cast<std::uint8_t>(pin); // restore base prio

#if (defined QXK_ON_CONTEXT_SW) || (defined Q_SPY)
    if (pin != 0U) { // resuming an active object?
        a = QP::QF::active_[pin]; // the pointer to the preempted AO

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_RESUME, a->m_prio)
            QS_TIME_PRE_();  // timestamp
            QS_2U8_PRE_(pin, pprev); // resumed prio & previous prio
        QS_END_NOCRIT_PRE_()
    }
    else {  // resuming priority==0 --> idle
        a = nullptr;

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_IDLE, 0U)
            QS_TIME_PRE_();    // timestamp
            QS_U8_PRE_(pprev); // previous prio
        QS_END_NOCRIT_PRE_()
    }

#ifdef QXK_ON_CONTEXT_SW
    // context-switch callback
    QXK_onContextSw(QP::QF::active_[pprev], a);
#endif // QXK_ON_CONTEXT_SW

#endif // QXK_ON_CONTEXT_SW || Q_SPY
}

//............................................................................
QP::QActive *QXK_current(void) noexcept {
    //! @pre the QXK kernel must be running
    Q_REQUIRE_ID(800, QXK_attr_.lockPrio <= QF_MAX_ACTIVE);

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QP::QActive *curr = QXK_attr_.curr;
    if (curr == nullptr) { // basic thread?
        curr = QP::QF::active_[QXK_attr_.actPrio];
    }
    QF_CRIT_X_();

    //! @post the current thread must be valid
    Q_ENSURE_ID(890, curr != nullptr);

    return curr;
}

} // extern "C"
