/// @file
/// @brief QF/C++ port to uC/OS-II RTOS, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 6.9.2a
/// Last updated on  2021-01-26
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
static void task_function(void *pdata); // uC/OS-II task signature

//............................................................................
void QF::init(void) {
    OSInit();        // initialize uC/OS-II
}
//............................................................................
int_t QF::run(void) {
    onStartup();     // configure & start interrupts, see NOTE0

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    OSStart();       // start uC/OS-II multitasking
    Q_ERROR_ID(100); // OSStart() should never return
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
    // create uC/OS-II queue and make sure it was created correctly
    m_eQueue = OSQCreate((void **)qSto, qLen);
    Q_ASSERT_ID(210, m_eQueue != nullptr);

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object

    init(par, m_prio); // take the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // map from QP to uC/OS-II priority
    INT8U p_ucos = static_cast<INT8U>(QF_MAX_ACTIVE - m_prio);

    // prepare the unique task name of the form "Axx",
    // where xx is a 2-digit QP priority of the task
    char task_name[4];
    task_name[0] = 'A';
    task_name[1] = '0' + (prio / 10U);
    task_name[2] = '0' + (prio % 10U);
    task_name[3] = '\0'; // zero-terminate

    // create AO's task...
    //
    // NOTE: The call to uC/OS-II API OSTaskCreateExt() assumes that the
    // pointer to the top-of-stack (ptos) is at the end of the provided
    // stack memory. This is correct only for CPUs with downward-growing
    // stack, but must be changed for CPUs with upward-growing stack
    //
    INT8U err = OSTaskCreateExt(
        &task_function, // the task function
        this,     // the 'pdata' parameter
#if OS_STK_GROWTH
        &static_cast<OS_STK *>(stkSto)[(stkSize/sizeof(OS_STK)) - 1], // ptos
#else
        static_cast<OS_STK *>(stkSto), // ptos
#endif
        p_ucos,                    // uC/OS-II task priority
        static_cast<INT16U>(prio), // the unique QP priority is the task id
        static_cast<OS_STK *>(stkSto), // pbos
        static_cast<INT32U>(stkSize/sizeof(OS_STK)),// size in OS_STK units
        task_name,                 // pext
        static_cast<INT16U>(m_thread)); // task options, see NOTE1

    // uC/OS-II task must be created correctly
    Q_ENSURE_ID(220, err == OS_ERR_NONE);
}
//............................................................................
// NOTE: This function must be called BEFORE starting an active object
void QActive::setAttr(std::uint32_t attr1, void const * /*attr2*/) {
    m_thread = attr1; // use as temporary
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
static void task_function(void *pdata) { // uC/OS-II task signature
    QActive *act = reinterpret_cast<QActive *>(pdata);

    QF::thread_(act);
    QF::remove_(act); // remove this object from QF
    OSTaskDel(OS_PRIO_SELF); // make uC/OS-II forget about this task
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
    nFree = static_cast<std::uint_fast16_t>(
        reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSQSize
         - reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSNMsgs);

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

        // posting the event to uC/OS-II message queue must succeed
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

    // posting the event to uC/OS-II message queue must succeed
    Q_ALLEGE_ID(810,
        OSQPostFront(m_eQueue, const_cast<QEvt *>(e)) == OS_ERR_NONE);
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    INT8U err;
    QS_CRIT_STAT_

    QEvt const *e = static_cast<QEvt const *>(
        OSQPend(static_cast<OS_EVENT *>(m_eQueue), 0U, &err));
    Q_ASSERT_ID(910, err == OS_ERR_NONE);

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

///***************************************************************************
// NOTE0:
// The QF_onStartup() should enter the critical section before configuring
// and starting interrupts and it should NOT exit the critical section.
// Thus the interrupts cannot fire until uC/OS-II starts multitasking
// in OSStart(). This is to prevent a (narrow) time window in which interrupts
// could make some tasks ready to run, but the OS would not be ready yet
// to perform context switch.
//
// NOTE1:
// The member QActive.thread is set to the uC/OS-II task options in the
// function QF_setUCosTaskAttr(), which must be called **before**
// QActive::start().
//

