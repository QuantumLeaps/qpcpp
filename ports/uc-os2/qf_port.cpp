//============================================================================
// QF/C++ port to uC-OS2 RTOS, generic C++11 compiler
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
//! @date Last updated on: 2022-08-28
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QF/C++ port to uC-OS2, generic C++11 compiler

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"
#include "qassert.h"
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------
static void task_function(void *pdata); // uC-OS2 task signature

//............................................................................
void QF::init(void) {
    OSInit();  // initialize uC-OS2
}
//............................................................................
int_t QF::run(void) {
    onStartup();     // configure & start interrupts, see NOTE0

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    OSStart();       // start uC-OS2 multitasking
    Q_ERROR_ID(100); // OSStart() should never return
    return 0; // this unreachable return keeps the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}

//............................................................................
void QActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    m_prio = static_cast<std::uint8_t>(prioSpec & 0xFF); // QF-priority
    register_(); // make QF aware of this AO

    // task name to be passed to OSTaskCreateExt()
    void * const task_name = static_cast<void *>(m_eQueue);

    // create uC-OS2 queue and make sure it was created correctly
    m_eQueue = OSQCreate((void **)qSto, qLen);
    Q_ASSERT_ID(210, m_eQueue != nullptr);

    init(par, m_prio); // take the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // map from QP to uC-OS2 priority
    INT8U const p_ucos = static_cast<INT8U>(QF_MAX_ACTIVE - m_prio);

    // create AO's task...
    //
    // NOTE: The call to uC-OS2 API OSTaskCreateExt() assumes that the
    // pointer to the top-of-stack (ptos) is at the end of the provided
    // stack memory. This is correct only for CPUs with downward-growing
    // stack, but must be changed for CPUs with upward-growing stack
    //
    INT8U const err = OSTaskCreateExt(
        &task_function, // the task function
        this,     // the 'pdata' parameter
#if OS_STK_GROWTH
        &static_cast<OS_STK *>(stkSto)[(stkSize/sizeof(OS_STK)) - 1], // ptos
#else
        static_cast<OS_STK *>(stkSto), // ptos
#endif
        p_ucos,                    // uC-OS2 task priority
        static_cast<INT16U>(m_prio), // the unique QP priority is the task id
#if OS_STK_GROWTH
        static_cast<OS_STK *>(stkSto), // pbos
#else
        &static_cast<OS_STK *>(stkSto)[(stkSize/sizeof(OS_STK)) - 1], // pbos
#endif
        static_cast<INT32U>(stkSize/sizeof(OS_STK)),// size in OS_STK units
        task_name,                 // pext
        static_cast<INT16U>(m_thread)); // task options, see NOTE1

    // uC-OS2 task must be created correctly
    Q_ENSURE_ID(220, err == OS_ERR_NONE);
}
//............................................................................
// NOTE: This function must be called BEFORE starting an active object
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    switch (attr1) {
        case TASK_NAME_ATTR:
           // this function must be called before QACTIVE_START(),
           // which implies that me->eQueue must not be used yet;
           Q_ASSERT_ID(300, m_eQueue == nullptr);
           // temporarily store the name, cast 'const' away
           m_eQueue = static_cast<OS_EVENT *>(
                           const_cast<void *>(attr2));
            break;
        // ...
        default:
            m_thread = attr1;
            break;
    }
}

// thread for active objects -------------------------------------------------
void QActive::thread_(QActive *act) {
    // event-loop
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
    act->unregister_(); // remove this object from QF
    OSTaskDel(OS_PRIO_SELF); // make uC-OS2 forget about this task
}
//............................................................................
static void task_function(void *pdata) { // uC-OS2 task signature
    QActive::thread_(reinterpret_cast<QActive *>(pdata));
}
//............................................................................
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
{
    QF_CRIT_STAT_

    QF_CRIT_E_();
    std::uint_fast16_t const nFree = static_cast<std::uint_fast16_t>(
        reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSQSize
         - reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSNMsgs);

    bool status;
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_ID(710); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);  // # free entries
            QS_EQC_PRE_(0U);     // min # free (unknown)
        QS_END_NOCRIT_PRE_()

        if (e->poolId_ != 0U) { // is it a pool event?
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        QF_CRIT_X_();

        // posting the event to uC-OS2 message queue must succeed
        Q_ALLEGE_ID(720,
            OSQPost(m_eQueue, const_cast<QEvt *>(e)) == OS_ERR_NONE);
    }
    else {

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);  // # free entries
            QS_EQC_PRE_(0U);     // min # free (unknown)
        QS_END_NOCRIT_PRE_()

        QF_CRIT_X_();
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
                              // # free entries
        QS_EQC_PRE_(reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
                    - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries);
        QS_EQC_PRE_(0U);      // min # free (unknown)
    QS_END_NOCRIT_PRE_()

    if (e->poolId_ != 0U) { // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QF_CRIT_X_();

    // posting the event to uC-OS2 message queue must succeed
    Q_ALLEGE_ID(810,
        OSQPostFront(m_eQueue, const_cast<QEvt *>(e)) == OS_ERR_NONE);
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    INT8U err;
    QEvt const *e = static_cast<QEvt const *>(
        OSQPend(static_cast<OS_EVENT *>(m_eQueue), 0U, &err));
    Q_ASSERT_ID(910, err == OS_ERR_NONE);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
                              // # free entries
        QS_EQC_PRE_(reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
                    - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries);
    QS_END_PRE_()

    return e;
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

