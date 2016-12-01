/// @file
/// @brief QF/C++ port to uC/OS-II (V2.92) kernel, all supported compilers
/// @cond
////**************************************************************************
/// Last updated for version 5.8.0
/// Last updated on  2016-11-19
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// http://www.state-machine.com
/// mailto:info@state-machine.com
////**************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
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
    OSStart();       // start uC/OS-II multitasking
    Q_ERROR_ID(100); // OSStart() should never return
    return static_cast<int_t>(0); // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}
//............................................................................
void QF_setUCosTaskAttr(QActive *act, uint32_t attr) {
    act->getThread() = attr;
}

//............................................................................
void QActive::start(uint_fast8_t prio,
                     QEvt const *qSto[], uint_fast16_t qLen,
                     void *stkSto, uint_fast16_t stkSize,
                     QEvt const *ie)
{
    // create uC/OS-II queue and make sure it was created correctly
    m_eQueue = OSQCreate((void **)qSto, qLen);
    Q_ASSERT_ID(210, m_eQueue != static_cast<OS_EVENT *>(0));

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object
    init(ie);       // thake the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // map from QP to uC/OS-II priority
    INT8U p_ucos = static_cast<INT8U>(QF_MAX_ACTIVE - m_prio);

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
        &(((OS_STK *)stkSto)[(stkSize / sizeof(OS_STK)) - 1]), // ptos
        p_ucos,                 // uC/OS-II task priority
        static_cast<INT16U>(prio), // the unique QP priority is the task id
        static_cast<OS_STK *>(stkSto),  // pbos
        static_cast<INT32U>(stkSize/sizeof(OS_STK)),// size in OS_STK units
        static_cast<void *>(0),         // pext
        static_cast<INT16U>(m_thread)); // task options, see NOTE1

    // uC/OS-II task must be created correctly
    Q_ENSURE_ID(220, err == OS_ERR_NONE);
}

// thread for active objects -------------------------------------------------
void QF::thread_(QActive *act) {
    // enable thread-loop, see NOTE2
    act->m_thread = static_cast<uint32_t>(1);  // set event-loop control
    do {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    } while (act->m_thread != static_cast<uint32_t>(0));
    act->unsubscribeAll();
    INT8U err;
    OSQDel(act->m_eQueue, OS_DEL_ALWAYS, &err); // cleanup the uC/OS-II queue
    Q_ENSURE_ID(300, err == OS_ERR_NONE); // must be cleaned up correctly
}
//............................................................................
static void task_function(void *pdata) { // uC/OS-II task signature
    QActive *act = reinterpret_cast<QActive *>(pdata);

    QF::thread_(act);
    QF::remove_(act); // remove this object from QF
    OSTaskDel(OS_PRIO_SELF); // make uC/OS-II forget about this task
}
//............................................................................
void QActive::stop() {
    m_thread = static_cast<uint32_t>(0); // stop the thread loop
}
//............................................................................
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin)
#else
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin,
                     void const * const sender)
#endif
{
    bool status;
    uint_fast16_t nFree;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    nFree = static_cast<uint_fast16_t>(
        reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSQSize
         - reinterpret_cast<OS_Q_DATA *>(m_eQueue)->OSNMsgs);

    if (nFree > margin) {
        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::priv_.aoObjFilter, this)
            QS_TIME_();             // timestamp
            QS_OBJ_(sender);        // the sender object
            QS_SIG_(e->sig);        // the signal of the event
            QS_OBJ_(this);          // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        if (e->poolId_ != static_cast<uint8_t>(0)) { // is it a pool event?
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        QF_CRIT_EXIT_();

        // posting the event to uC/OS-II message queue must succeed, NOTE3
        Q_ALLEGE_ID(710,
            OSQPost(m_eQueue, const_cast<QEvt *>(e)) == OS_ERR_NONE);

        status = true; // report success
    }
    else {
        // can tolerate dropping evts?
        Q_ASSERT_ID(720, margin != static_cast<uint_fast16_t>(0));

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT,
                         QS::priv_.aoObjFilter, this)
            QS_TIME_();             // timestamp
            QS_OBJ_(sender);        // the sender object
            QS_SIG_(e->sig);        // the signal of the event
            QS_OBJ_(this);          // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        QF_CRIT_EXIT_();

        status = false; // report failure
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS::priv_.aoObjFilter, this)
        QS_TIME_();             // timestamp
        QS_SIG_(e->sig);        // the signal of this event
        QS_OBJ_(this);          // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
        // # free entries
        QS_EQC_(static_cast<QEQueueCtr>(
            reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
            - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries));
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free entries (unknown)
    QS_END_NOCRIT_()

    if (e->poolId_ != static_cast<uint8_t>(0)) { // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QF_CRIT_EXIT_();

    // posting the event to uC/OS-II message queue must succeed, NOTE3
    Q_ALLEGE_ID(810,
        OSQPostFront(m_eQueue, const_cast<QEvt *>(e)) == OS_ERR_NONE);
}
//............................................................................
QEvt const *QActive::get_(void) {
    INT8U err;
    QS_CRIT_STAT_

    QEvt const *e = static_cast<QEvt const *>(
        OSQPend(static_cast<OS_EVENT *>(m_eQueue), 0U, &err));
    Q_ASSERT_ID(910, err == OS_ERR_NONE);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::priv_.aoObjFilter, this)
        QS_TIME_();             // timestamp
        QS_SIG_(e->sig);        // the signal of this event
        QS_OBJ_(this);          // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
        // # free entries
        QS_EQC_(static_cast<QEQueueCtr>(
            reinterpret_cast<OS_Q *>(m_eQueue)->OSQSize
            - reinterpret_cast<OS_Q *>(m_eQueue)->OSQEntries));
    QS_END_()

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
// QACTIVE_START().
//
// NOTE2:
// The member QActive.thread is reused as the loop control variable,
// because the task options are alredy applied.
//
// NOTE3:
// The following uC/OS-II OSQPost() API is called inside a critical section,
// but this is OK, because uC/OS-II critical sections are designed to nest.
//
