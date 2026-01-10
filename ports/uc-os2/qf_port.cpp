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
static void task_main(void *pdata);  // prototype
static void task_main(void *pdata) { // uC-OS2 task signature
    QP::QActive::evtLoop_(reinterpret_cast<QP::QActive *>(pdata));
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

    // the number of free slots available in the uC-OS2 queue
    std::uint_fast16_t const nFree = static_cast<std::uint_fast16_t>(
        reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSQSize
         - reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSNMsgs);

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

        QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

        // post the event to the uC-OS2 event queue, see NOTE3
        status = (OSQPost(m_eQueue, const_cast<QEvt *>(e)) == OS_ERR_NONE);
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
                              // # free entries
        QS_EQC_PRE(reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
                    - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries);
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

    INT8U const err = OSQPostFront(m_eQueue, const_cast<QEvt *>(e));

#ifndef Q_UNSAFE
    QF_CRIT_ENTRY();
    // LIFO posting to uC-OS2 message queue must succeed, see NOTE3
    Q_ASSERT_INCRIT(230, err == OS_ERR_NONE);
    QF_CRIT_EXIT();
#else
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
QEvt const *QActive::get_() noexcept {
    // wait for an event (forever)
    INT8U err;
    QEvt const *e = static_cast<QEvt const *>(
        OSQPend(static_cast<OS_EVENT *>(m_eQueue), 0U, &err));

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    Q_ASSERT_INCRIT(310, err == OS_ERR_NONE); // queue-get must succeed
#else
    Q_UNUSED_PAR(err);
#endif

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
                                 // # free entries
        QS_EQC_PRE(reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
                    - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries);
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
    // extract data temporarily saved in QActive::setAttr()
    void * const task_name = static_cast<void *>(m_eQueue);

    // create the uC-OS2 message queue
    m_eQueue = OSQCreate((void **)qSto, qLen);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // the uC-OS2 queue must be created correctly
    Q_ASSERT_INCRIT(410, m_eQueue != nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used for AO registration)
    register_(); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // uC-OS2 priority, see NOTE1
    INT8U ucos2_prio = (prioSpec >> 8U);
    if (ucos2_prio == 0) {
        ucos2_prio = static_cast<INT8U>(OS_LOWEST_PRIO - m_prio);
    }

    // create the uC-OS2 task for the AO...
    INT8U const err = OSTaskCreateExt(
        &task_main,     // the task function
        this,           // the 'pdata' parameter
#if OS_STK_GROWTH
        &static_cast<OS_STK *>(stkSto)[(stkSize/sizeof(OS_STK)) - 1], // ptos
#else
        static_cast<OS_STK *>(stkSto),  // ptos
#endif
        ucos2_prio,       // uC-OS2 task priority
        static_cast<INT16U>(m_prio),    // the unique AO priority as task ID
#if OS_STK_GROWTH
        static_cast<OS_STK *>(stkSto),  // pbos
#else
        &static_cast<OS_STK *>(stkSto)[(stkSize/sizeof(OS_STK)) - 1], // pbos
#endif
        static_cast<INT32U>(stkSize/sizeof(OS_STK)), // size in OS_STK units
        task_name,           // pext
        static_cast<INT16U>(m_thread)); // task options, see NOTE1

    QF_CRIT_ENTRY();
    // uC-OS2 task must be created correctly
    Q_ASSERT_INCRIT(490, err == OS_ERR_NONE);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    // NOTE: this function must be called *before* QActive_start(),
    // which implies that me->thread.tx_thread_name must not be used yet;
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    switch (attr1) {
        case TASK_NAME_ATTR:
            // m_eQueue must not be used yet
            Q_ASSERT_INCRIT(510, m_eQueue == nullptr);
            // temporarily store the name, cast 'const' away
            m_eQueue = static_cast<OS_EVENT *>(
                           const_cast<void *>(attr2));
            break;
        // ...
        default:
            m_thread = attr1;
            break;
    }
    QF_CRIT_EXIT();
}
//............................................................................
void QActive::evtLoop_(QActive *act) {
    // the event-loop...
    for (;;) { // for-ever
        QEvt const * const e = act->get_(); // BLOCK for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
#if (QF_MAX_EPOOL > 0U)
        QF::gc(e); // check if the event is garbage, and collect it if so
#endif
    }
    //act->unregister_(); // remove this object from QF
}

//============================================================================
namespace QF {

//............................................................................
void init() {
    OSInit();  // initialize uC-OS2
}
//............................................................................
int run() {
    onStartup(); // the startup callback, see NOTE4

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();
#endif // Q_SPY

    OSStart(); // start uC-OS2 multitasking--should never return

    return 0; // this unreachable return keeps the compiler happy
}
//............................................................................
void stop() {
    onCleanup(); // cleanup callback
}

} // namespace QF
} // namespace QP

//============================================================================
// NOTE1:
// The uC-OS2 priority of the AO thread can be specified in two ways:
//
// A. Implicitly based on the AO's priority (uC-OS2 uses the reverse
//    priority numbering scheme than QP). This option is chosen when
//    the higher-byte of the prioSpec parameter is set to zero.
//
// B. Explicitly as the higher-byte of the prioSpec parameter.
//    This option is chosen when the prioSpec parameter is not-zero.
//    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
//    as 10 and uC-OS2 priority as 5.
//
// CAUTION: The explicit uC-OS2 priority is NOT sanity-checked, so it is the
// responsibility of the application to ensure that it is consistent with the
// QP priority. An example of inconsistent setting would be assigning uC-OS2
// priorities that would result in a different relative prioritization of AOs
// than indicated by the QP priorities assigned to the AOs.
//
// NOTE2:
// In the uC-OS2 port, the generic function QActive::setAttr() is used to
// set the options for the uC-OS2 task options and task name.
// CAUTION: QActive_setAttr() needs to be called *before* QActive::start() for
// the given AO.
//
// NOTE3:
// The event posting to uC-OS2 message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
// NOTE4:
// The QF::onStartup() should enter the critical section before configuring
// and starting interrupts and it should NOT exit the critical section.
// Thus the interrupts cannot fire until uC-OS2 starts multitasking
// in OSStart(). This is to prevent a (narrow) time window in which interrupts
// could make some tasks ready to run, but the OS would not be ready yet
// to perform context switch.
//
