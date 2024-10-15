//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// The QP/C software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2024-09-26
//! @version Last updated for: @ref qpcpp_8_0_0
//!
//! @file
//! @brief QF/C++ port to ThreadX (a.k.a. Azure RTOS), generic C++11 compiler

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

//============================================================================
namespace { // anonymous namespace with local definitions

Q_DEFINE_THIS_MODULE("qf_port")

static void thread_function(ULONG thread_input); // prototype
static void thread_function(ULONG thread_input) { // ThreadX signature
    QP::QActive::evtLoop_(reinterpret_cast<QP::QActive *>(thread_input));
}

} // anonymous namespace

// namespace QP ==============================================================
namespace QP {

//............................................................................
void QF::init() {
    bzero_(&QF::priv_,             sizeof(QF::priv_));
    bzero_(&QActive::registry_[0], sizeof(QActive::registry_));
    // nothing to do for ThreadX
}
//............................................................................
int QF::run() {
    onStartup();    // QF callback to configure and start interrupts

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();

    return 0; // return success
}
//............................................................................
void QF::stop() {
    onCleanup(); // cleanup callback
}

// thread for active objects -------------------------------------------------
void QActive::evtLoop_(QActive *act) {
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
}
//............................................................................
void QActive::start(QPrioSpec const prioSpec,
                    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U); // QF preemption-thre.
    register_(); // make QF aware of this AO

    // create the ThreadX message queue for the AO
    UINT tx_err = tx_queue_create(&m_eQueue,
        m_thread.tx_thread_name,
        TX_1_ULONG,
        static_cast<VOID *>(qSto),
        static_cast<ULONG>(qLen * sizeof(QEvtPtr)));

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(110, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    UINT tx_prio = QF_TO_TX_PRIO_MAP(m_prio);
    UINT tx_pt   = QF_TO_TX_PRIO_MAP(m_pthre);
    tx_err = tx_thread_create(
        &m_thread, // ThreadX thread control block
        m_thread.tx_thread_name, // unique thread name
        &thread_function,        // thread function
        reinterpret_cast<ULONG>(this), // thread parameter
        stkSto,                  // stack start
        stkSize,                 // stack size in bytes
        tx_prio,                 // ThreadX priority
        tx_pt,                   // ThreadX preempt-threshold, see NOTE1
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(120, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();
}
//............................................................................
void QActive::setAttr(std::uint32_t const attr1, void const *attr2) {
    // this function must be called before QActive::start(),
    // which implies that m_thread.tx_thread_name must not be used yet;
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(150, m_thread.tx_thread_name == nullptr);

    switch (attr1) {
        case THREAD_NAME_ATTR:
            // temporarily store the name, cast 'const' away
            m_thread.tx_thread_name = static_cast<char *>(
                                          const_cast<void *>(attr2));
            break;
        // ...
        default:
            break;
    }
    QF_CRIT_EXIT();
}
//............................................................................
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != nullptr);
#ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(201, e->verify_());
#endif // ndef Q_UNSAFE

    std::uint_fast16_t nFree =
        static_cast<std::uint_fast16_t>(m_eQueue.tx_queue_available_storage);

    bool status;
    if (margin == QF::NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_INCRIT(210); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()

        if (e->getPoolNum_() != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        QF_CRIT_EXIT();

        UINT tx_err = tx_queue_send(&m_eQueue, const_cast<QEvt**>(&e),
                                    TX_NO_WAIT);
        QF_CRIT_ENTRY();
        // posting to the ThreadX message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(220, tx_err == TX_SUCCESS);
        QF_CRIT_EXIT();
    }
    else {

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()

        QF_CRIT_EXIT();
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(300, e != nullptr);
#ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(301, e->verify_());
#endif // ndef Q_UNSAFE


    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
        QS_EQC_PRE(m_eQueue.tx_queue_available_storage); // # free
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    if (e->getPoolNum_() != 0U) { // is it a pool event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
    QF_CRIT_EXIT();

    UINT tx_err = tx_queue_front_send(&m_eQueue, const_cast<QEvt**>(&e),
                                      TX_NO_WAIT);

    QF_CRIT_ENTRY();
    // LIFO posting must succeed, see NOTE3
    Q_ASSERT_INCRIT(310, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    QEvtPtr e;
    UINT tx_err = tx_queue_receive(&m_eQueue, (VOID *)&e, TX_WAIT_FOREVER);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(710, tx_err == TX_SUCCESS);

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();          // timestamp
        QS_SIG_PRE(e->sig);     // the signal of this event
        QS_OBJ_PRE(this);       // this active object
        QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
        QS_EQC_PRE(m_eQueue.tx_queue_available_storage); // # free
    QS_END_PRE()
    QF_CRIT_EXIT();

    return e;
}

//............................................................................
void QFSchedLock::lock(std::uint_fast8_t prio) {

    m_lockHolder = tx_thread_identify();

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // must be in thread context, so current TX thread must be valid
    Q_REQUIRE_INCRIT(800, m_lockHolder != nullptr);
    QF_CRIT_EXIT();

    // change the preemption threshold of the current thread
    UINT tx_err = tx_thread_preemption_change(m_lockHolder,
                     QF_TO_TX_PRIO_MAP(prio),
                     &m_prevThre);

    if (tx_err == TX_SUCCESS) {
        m_lockPrio = prio;

        QF_CRIT_ENTRY();
        QS_BEGIN_PRE(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE(); // timestamp
            QS_2U8_PRE(TX_TO_QF_PRIO_MAP(m_prevThre),
                        prio); // new lock prio
        QS_END_PRE()
        QF_CRIT_EXIT();
    }
    else if (tx_err == TX_THRESH_ERROR) {
        // threshold was greater than (lower prio) than the current prio
        m_lockPrio = 0U; // threshold not changed
    }
    else {
        // no other errors are tolerated
        QF_CRIT_ENTRY();
        Q_ERROR_INCRIT(810);
        //QF_CRIT_EXIT();
    }
}

//............................................................................
void QFSchedLock::unlock(void) const {

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // the lock holder must be valid and the scheduler must be locked
    Q_REQUIRE_INCRIT(900, (m_lockHolder != nullptr)
                          && (m_lockPrio != 0U));

    QS_BEGIN_PRE(QS_SCHED_UNLOCK, 0U)
        QS_TIME_PRE(); // timestamp
        QS_2U8_PRE(m_lockPrio, // prev lock prio
                    TX_TO_QF_PRIO_MAP(m_prevThre)); // new lock prio
    QS_END_PRE()
    QF_CRIT_EXIT();

    // restore the preemption threshold of the lock holder
    UINT old_thre;
    UINT tx_err = tx_thread_preemption_change(m_lockHolder, m_prevThre,
                                              &old_thre);
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(910, tx_err == TX_SUCCESS);
    QF_CRIT_EXIT();
}

} // namespace QP

//============================================================================
// NOTE1:
// When QF preemption threshold is NOT specified (upper byte of prio is 0),
// QActive::register_() will set m_pthre equal to m_prio, which means
// that tx_pt == tx_prio. In that case the call to ThreadX tx_thread_create()
// will disable the ThreadX preemption-threshold for this thread.
//
// NOTE3:
// The event posting to ThreadX message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
