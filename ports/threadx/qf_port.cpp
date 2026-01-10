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

//============================================================================
namespace { // anonymous namespace with local definitions

Q_DEFINE_THIS_MODULE("qf_port")

//----------------------------------------------------------------------------
static void thread_main(ULONG thread_input);  // prototype
static void thread_main(ULONG thread_input) { // ThreadX signature
    QP::QActive::evtLoop_(reinterpret_cast<QP::QActive *>(thread_input));
}

} // anonymous namespace

//============================================================================
namespace QP {

// Active Object customization...

//............................................................................
bool QActive::postx_(
    QEvt const * const e,
    std::uint_fast16_t const margin,
    void const * const sender) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the event to post must not be NULL
    Q_REQUIRE_INCRIT(100, e != nullptr);

    // the number of free slots available in the embOS queue
    std::uint_fast16_t nFree =
        static_cast<std::uint_fast16_t>(m_eQueue.tx_queue_available_storage);

    bool status = ((margin == QF::NO_MARGIN)
        || (nFree > static_cast<QEQueueCtr>(margin)));
    if (status) { // should try to post the event?
#if (QF_MAX_EPOOL > 0U)
        if (e->poolNum_ != 0U) { // is it a mutable event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
#endif // (QF_MAX_EPOOL > 0U)

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(0U);     // min # free entries (unknown)
        QS_END_PRE()

        QF_CRIT_EXIT(); // exit crit.sect. before calling Zephyr API

        // post the event to the ThreadX event queue, see NOTE3
        status = (tx_queue_send(&m_eQueue, const_cast<QEvt**>(&e), TX_NO_WAIT)
                  == TX_SUCCESS);
    }

    if (!status) { // event NOT posted?
        QF_CRIT_ENTRY();

        // posting is allowed to fail only when margin != QF_NO_MARGIN
        Q_ASSERT_INCRIT(130, margin != QF::NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(margin); // margin requested
        QS_END_PRE()

        QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
        QF::gc(e); // recycle the event to avoid a leak
#endif
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the posted event must be be valid (which includes not NULL)
    Q_REQUIRE_INCRIT(200, e != nullptr);

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
        QS_EQC_PRE(m_eQueue.tx_queue_available_storage); // # free
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

    UINT const err = tx_queue_front_send(&m_eQueue,
        const_cast<QEvt**>(&e), TX_NO_WAIT);

#ifndef Q_UNSAFE
    QF_CRIT_ENTRY();
    // LIFO posting to ThreadX mailbox must succeed, see NOTE3
    Q_ASSERT_INCRIT(230, err == TX_SUCCESS);
    QF_CRIT_EXIT();
#else
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
QEvt const *QActive::get_() noexcept {
    // wait for an event (forever)
    QEvtPtr e;
    UINT const err = tx_queue_receive(&m_eQueue,
                     static_cast<VOID *>(&e), TX_WAIT_FOREVER);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    Q_ASSERT_INCRIT(310, err == TX_SUCCESS); // queue-get must succeed
#else
    Q_UNUSED_PAR(err);
#endif

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
        QS_EQC_PRE(m_eQueue.tx_queue_available_storage); // # free entries
    QS_END_PRE()

    QF_CRIT_EXIT();

    return e;
}
//............................................................................
std::uint16_t QActive::getQueueUse(std::uint_fast8_t const prio) noexcept {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueFree(std::uint_fast8_t const prio) noexcept {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueMin(std::uint_fast8_t const prio) noexcept {
    Q_UNUSED_PAR(prio);
    return 0U; // minimum free entries in a queue not supported in this RTOS
}

//............................................................................
void QActive::start(
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
    void * const stkSto, std::uint_fast16_t const stkSize,
    void const * const par)
{
    // create the ThreadX message queue for the AO
    UINT err = tx_queue_create(&m_eQueue,
        m_thread.tx_thread_name,
        TX_1_ULONG,
        static_cast<VOID *>(qSto),
        static_cast<ULONG>(qLen * sizeof(QEvtPtr)));

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // the ThreadX queue must be created correctly
    Q_ASSERT_INCRIT(400, err == TX_SUCCESS);
    QF_CRIT_EXIT();


    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U); // QF preemption-thre.
    register_(); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // ThreadX priority, see NOTE1
    UINT tx_prio = QF_TO_TX_PRIO_MAP(m_prio);
    UINT tx_pt   = QF_TO_TX_PRIO_MAP(m_pthre);
    err = tx_thread_create(
        &m_thread, // ThreadX thread control block
        m_thread.tx_thread_name, // unique thread name
        &thread_main,            // thread function
        reinterpret_cast<ULONG>(this), // thread parameter
        stkSto,                  // stack start
        stkSize,                 // stack size in bytes
        tx_prio,                 // ThreadX priority
        tx_pt,                   // ThreadX preempt-threshold, see NOTE1
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    QF_CRIT_ENTRY();
    // ThreadX task must be created correctly
    Q_ASSERT_INCRIT(490, err == TX_SUCCESS);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
void QActive::setAttr(std::uint32_t const attr1, void const *attr2) {
    // NOTE: this function must be called before QActive::start(),
    // which implies that m_thread.tx_thread_name must not be used yet;
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(500, m_thread.tx_thread_name == nullptr);

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
void QActive::evtLoop_(QActive *act) {
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
}

//----------------------------------------------------------------------------
namespace QF {

//............................................................................
void init() {
    // nothing to do for ThreadX
}
//............................................................................
int_t run() {
    onStartup();    // QF callback to configure and start interrupts

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();
#endif // Q_SPY

    return 0; // return success
}
//............................................................................
void stop() {
    onCleanup(); // cleanup callback
}

} // namespace QF

//............................................................................
void QFSchedLock::lock(std::uint_fast8_t prio) {

    m_lockHolder = tx_thread_identify();

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // must be in thread context, so current TX thread must be valid
    Q_REQUIRE_INCRIT(800, m_lockHolder != nullptr);
    QF_CRIT_EXIT();

    // change the preemption threshold of the current thread
    UINT const err = tx_thread_preemption_change(m_lockHolder,
                     QF_TO_TX_PRIO_MAP(prio),
                     &m_prevThre);

    if (err == TX_SUCCESS) {
        m_lockPrio = prio;

        QS_CRIT_ENTRY();
        QS_BEGIN_PRE(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE(); // timestamp
            QS_2U8_PRE(TX_TO_QF_PRIO_MAP(m_prevThre),
                        prio); // new lock prio
        QS_END_PRE()
        QS_CRIT_EXIT();
    }
    else if (err == TX_THRESH_ERROR) {
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
    UINT const err = tx_thread_preemption_change(m_lockHolder, m_prevThre,
                                              &old_thre);
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(910, err == TX_SUCCESS);
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
