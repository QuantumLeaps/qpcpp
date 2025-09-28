//============================================================================
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
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C++ software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C++ in closed-source proprietary applications.
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

//............................................................................
static void task_function(void *pdata); // prototype
static void task_function(void *pdata) { // uC-OS2 task signature
    QP::QActive::evtLoop_(reinterpret_cast<QP::QActive *>(pdata));
}

} // anonymous namespace

// namespace QP ==============================================================
namespace QP {

//............................................................................
void QF::init() {
    OSInit();  // initialize uC-OS2
}
//............................................................................
int QF::run() {
    onStartup();  // QF callback to configure and start interrupts

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();

    OSStart(); // start uC-OS2 multitasking,  should never return

    return 0; // this unreachable return keeps the compiler happy
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
    //act->unregister_(); // remove this object from QF
}

//............................................................................
void QActive::start(QPrioSpec const prioSpec,
    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
    void * const stkSto, std::uint_fast16_t const stkSize,
    void const * const par)
{
    // task name to be passed to OSTaskCreateExt()
    void * const task_name = static_cast<void *>(m_eQueue);

    // create uC-OS2 queue and make sure it was created correctly
    m_eQueue = OSQCreate((void **)qSto, qLen);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // the uC-OS2 queue must be created correctly
    Q_ASSERT_INCRIT(110, m_eQueue != nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used)
    register_(); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // map from QP to uC-OS2 priority
    // The uC-OS2 priority of the AO thread can be specified in two ways:
    //
    // 1. Implictily based on the AO's priority (uC-OS2 uses the reverse
    //    priority numbering scheme than QP). This option is chosen when
    //    the higher-byte of the prioSpec parameter is set to zero.
    //
    // 2. Explicitly as the higher-byte of the prioSpec parameter.
    //    This option is chosen when the prioSpec parameter is not-zero.
    //    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
    //    as 10 and FreeRTOS priority as 5.
    //
    //    NOTE: The explicit uC-OS2 priority is NOT sanity-checked,
    //    so it is the responsibility of the application to ensure that
    //    it is consistent with the AO's priority. An example of
    //    inconsistent setting would be assigning uC-OS2 priorities that
    //    would result in a different relative priritization of AO's threads
    //    than indicated by the AO priorities assigned.
    //
    INT8U ucos2_prio = (prioSpec >> 8U);
    if (ucos2_prio == 0U) {
        ucos2_prio = (INT8U)(OS_LOWEST_PRIO - m_prio);
    }

    // create AO's task...
    //
    // NOTE: The call to uC-OS2 API OSTaskCreateExt() assumes that the
    // pointer to the top-of-stack (ptos) is at the end of the provided
    // stack memory. This is correct only for CPUs with downward-growing
    // stack, but must be changed for CPUs with upward-growing stack
    INT8U const err = OSTaskCreateExt(
        &task_function, // the task function
        this,     // the 'pdata' parameter
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
        task_name,                      // pext
        static_cast<INT16U>(m_thread)); // task options, see NOTE1

    QF_CRIT_ENTRY();
    // uC-OS2 task must be created correctly
    Q_ASSERT_INCRIT(220, err == OS_ERR_NONE);
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
           // this function must be called before QActive_start(),
           // which implies that m_eQueue must not be used yet;
           Q_ASSERT_INCRIT(150, m_eQueue == nullptr);
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
bool QActive::postx_(QEvt const * const e, std::uint_fast16_t const margin,
                     void const * const sender) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != nullptr);

    std::uint_fast16_t const nFree = static_cast<std::uint_fast16_t>(
        reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSQSize
         - reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSNMsgs);

    bool status;
    if (margin == QF::NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false;  // cannot post
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
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()

        if (e->poolNum_ != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        QF_CRIT_EXIT();

        INT8U err = OSQPost(m_eQueue, const_cast<QEvt *>(e));

        QF_CRIT_ENTRY();
        // posting to uC-OS2 message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(220, err == OS_ERR_NONE);
        QF_CRIT_EXIT();

#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
    }
    else {

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(margin);  // margin requested
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

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
                              // # free entries
        QS_EQC_PRE(reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
                    - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries);
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    if (e->poolNum_ != 0U) { // is it a pool event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
    QF_CRIT_EXIT();

    INT8U err = OSQPostFront(m_eQueue, const_cast<QEvt *>(e));

    QF_CRIT_ENTRY();
    // posting to uC-OS2 message queue must succeed, see NOTE3
    Q_ASSERT_INCRIT(310, err == OS_ERR_NONE);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    INT8U err;
    QEvt const *e = static_cast<QEvt const *>(
        OSQPend(static_cast<OS_EVENT *>(m_eQueue), 0U, &err));

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(410, err == OS_ERR_NONE);

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();          // timestamp
        QS_SIG_PRE(e->sig);     // the signal of this event
        QS_OBJ_PRE(this);       // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
                                 // # free entries
        QS_EQC_PRE(reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
                    - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries);
    QS_END_PRE()
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif
    return e;
}
//............................................................................
std::uint16_t QActive::getQueueUse(
    std::uint_fast8_t const prio) noexcept
{
    return 0U; // queue use not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueFree(std::uint_fast8_t const prio) noexcept {
    return 0U; // queue free elements not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueMin(std::uint_fast8_t const prio) noexcept {
    return 0U; // queue minimum not supported in this RTOS
}

} // namespace QP

//============================================================================
// NOTE0:
// The QF_onStartup() should enter the critical section before configuring
// and starting interrupts and it should NOT exit the critical section.
// Thus the interrupts cannot fire until uC-OS2 starts multitasking
// in OSStart(). This is to prevent a (narrow) time window in which interrupts
// could make some tasks ready to run, but the OS would not be ready yet
// to perform context switch.
//
// NOTE1:
// The member QActive.thread is set to the uC-OS2 task options in the
// function QF_setUCosTaskAttr(), which must be called **before**
// QActive::start().
//
// NOTE3:
// The event posting to uC-OS2 message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.

