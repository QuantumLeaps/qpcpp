/// @file
/// @brief QF/C++ port to uC/OS-III RTOS, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 6.9.3
/// Last updated on  2021-04-08
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

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
static void task_function(void *pdata); // uC/OS-III task signature

//............................................................................
void QF::init(void) {
    OS_ERR err;
    OSInit(&err);        // initialize uC/OS-III
    Q_ASSERT_ID(50, err == OS_ERR_NONE);
}
//............................................................................
int_t QF::run(void) {
    onStartup();     // configure & start interrupts, see NOTE0

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()
    OS_ERR err;
    OSStart(&err);       // start uC/OS-III multitasking
    Q_ASSERT_ID(100, err == OS_ERR_NONE);  
    return 0; // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}

//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    /* Retrieve attributes for task indirectly with m_thread data members*/
    void * task_name = static_cast<void *>( m_thread.StkPtr);
    OS_OPT  * task_opt = static_cast<OS_OPT*>(static_cast<void *>( m_thread.ExtPtr));

    OS_ERR err;

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object
    

    init(par, m_prio); // take the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

#if (OS_CFG_TASK_Q_EN  == 0u) //If os doesnt use internal Task queue
    // create uC/OS-III queue and make sure it was created correctly
    char qName[] = "qAOxx";
    qName[3] = '0' + (m_prio / 10U);
    qName[4] = '0' + (m_prio % 10U);
    OSQCreate(&m_eQueue, qName,qLen,&err);
    Q_ASSERT_ID(210, err == OS_ERR_NONE);
#endif

    // map from QP to uC/OS-III priority
    OS_PRIO p_ucos = static_cast<OS_PRIO>(QF_MAX_ACTIVE - m_prio);
    const CPU_STK_SIZE stk_size = (stkSize/sizeof(CPU_STK));

    // create AO's task...
    //
    // NOTE: The call to uC/OS-III API OSTaskCreate() assumes that the
    // pointer to the top-of-stack (ptos) is at the end of the provided
    // stack memory. This is correct only for CPUs with downward-growing
    // stack, but must be changed for CPUs with upward-growing stack
    //
#if OS_STK_GROWTH == 1u
    CPU_STK * pStack = static_cast<CPU_STK*>(stkSto);
#else
    CPU_STK * pStack = &static_cast<CPU_STK*>(stkSto)[stk_size-1];           
#endif
    OSTaskCreate(&m_thread,
                 static_cast<CPU_CHAR *>(task_name),
                 task_function,
                 this,
                 p_ucos, // the unique QP priority is the task id
                 pStack,
                 stk_size/10, //Stack limit
                 stk_size,// size in OS_STK units
#if (OS_CFG_TASK_Q_EN > 0u)
                 static_cast<OS_MSG_QTY>(qLen), //internal Task message Queue, used only if 
#else
                 static_cast<OS_MSG_QTY>(0),
#endif
                 static_cast<OS_TICK>(0), //time_quanta
                 static_cast<void *>(0), //TCB extension
                 *task_opt,
                 &err);

    // uC/OS-III task must be created correctly
    Q_ENSURE_ID(220, err == OS_ERR_NONE);
}
//............................................................................
// NOTE: This function must be called BEFORE starting an active object
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    // this function must be called before QACTIVE_START(),
    // which implies that me->m_thread must not be used yet;
    switch (attr1) {
        case TASK_NAME_ATTR:
           Q_ASSERT_ID(300, m_thread.StkPtr == nullptr);
           // temporarily store the name in StkPtr, cast 'const' away
           m_thread.StkPtr = static_cast<CPU_STK *>(
                           const_cast<void *>(attr2));
           break;
       case TASK_OPT_ATTR:
       default:
            Q_ASSERT_ID(310, m_thread.ExtPtr == nullptr);
            // temporarily store the task options in ExtPtr, cast 'const' away
            m_thread.ExtPtr =  const_cast<void *>(attr2);
            break;
       
    }
}

// thread for active objects -------------------------------------------------
void QF::thread_(QActive *act) {
    // event-loop
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
}
//............................................................................
static void task_function(void *pdata) { // uC/OS-III task signature
    QActive *act = reinterpret_cast<QActive *>(pdata);

    QF::thread_(act);
    QF::remove_(act); // remove this object from QF
    OS_ERR err;
    OSTaskDel(&act->m_thread,&err); // make uC/OS-III forget about this task
}
//............................................................................
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e,
                    std::uint_fast16_t const margin) noexcept
#else
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
#endif
{
    bool status;
    std::uint_fast16_t nFree;
    QF_CRIT_STAT_

    QF_CRIT_E_();
#if (OS_CFG_TASK_Q_EN > 0u) //If os uses internal queue
    nFree = static_cast<std::uint_fast16_t>(
            m_thread.MsgQ.NbrEntriesSize
             - m_thread.MsgQ.NbrEntries);
#else
    nFree = static_cast<std::uint_fast16_t>(
            m_eQueue.MsgQ.NbrEntriesSize
             - m_eQueue.MsgQ.NbrEntries);
#endif

  
    if (margin == QF_NO_MARGIN) {
        if (nFree > static_cast<QEQueueCtr>(0)) {
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
        OS_ERR err;
#if (OS_CFG_TASK_Q_EN > 0u) //If os uses internal queue
        OSTaskQPost(&m_thread,
#else
        OSQPost (   &m_eQueue,
#endif
                    const_cast<QEvt *>(e), //Message to send
                    sizeof(QEvt), //Message size
                    OS_OPT_POST_FIFO,
                    &err);
        // posting the event to uC/OS-III message queue must succeed
        Q_ALLEGE_ID(720, err  == OS_ERR_NONE);
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
#if (OS_CFG_TASK_Q_EN > 0u) //If os uses internal queue
        QS_EQC_PRE_(m_thread.MsgQ.NbrEntriesSize
                    - m_thread.MsgQ.NbrEntries);
#else
        QS_EQC_PRE_(m_eQueue.MsgQ.NbrEntriesSize
                    - m_eQueue.MsgQ.NbrEntries);
#endif
       
        QS_EQC_PRE_(0U);      // min # free (unknown)
    QS_END_NOCRIT_PRE_()

    if (e->poolId_ != 0U) { // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QF_CRIT_X_();
    OS_ERR err;
    
#if (OS_CFG_TASK_Q_EN > 0u) //If os uses internal queue
    OSTaskQPost(&m_thread,
#else
    OSQPost (   &m_eQueue,
#endif
                const_cast<QEvt *>(e), //Message to send
                sizeof(QEvt), //Message size
                OS_OPT_POST_LIFO,
                &err);
    // posting the event to uC/OS-III message queue must succeed
    Q_ALLEGE_ID(810, err == OS_ERR_NONE);
}
//............................................................................

QEvt const *QActive::get_(void) noexcept {
    OS_ERR err;
    QS_CRIT_STAT_
    OS_MSG_SIZE msg_size;
    CPU_TS ts;
    QEvt const *e = static_cast<QEvt const *>(
#if (OS_CFG_TASK_Q_EN > 0u) //If os uses internal queue
    OSTaskQPend(
#else
    OSQPend (   &m_eQueue,
#endif
                    0U,         //timeout
                    OS_OPT_PEND_BLOCKING, //Pend type
                    &msg_size, 
                    &ts, //Timestamp
                    &err));
    Q_ASSERT_ID(910, err == OS_ERR_NONE);

    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
                              // # free entries
#if (OS_CFG_TASK_Q_EN > 0u) //If os uses internal queue
        QS_EQC_PRE_(m_thread.MsgQ.NbrEntriesSize
                    - m_thread.MsgQ.NbrEntries);
#else
        QS_EQC_PRE_(m_eQueue.MsgQ.NbrEntriesSize
                    - m_eQueue.MsgQ.NbrEntries);
#endif
    QS_END_PRE_()

    return e;
}

} // namespace QP

///***************************************************************************
// NOTE0:
// The QF_onStartup() should enter the critical section before configuring
// and starting interrupts and it should NOT exit the critical section.
// Thus the interrupts cannot fire until uC/OS-III starts multitasking
// in OSStart(). This is to prevent a (narrow) time window in which interrupts
// could make some tasks ready to run, but the OS would not be ready yet
// to perform context switch.
//
// NOTE1:
// The member QActive.thread is set to the uC/OS-III task options in the
// function QF_setUCosTaskAttr(), which must be called **before**
// QActive::start().
//

