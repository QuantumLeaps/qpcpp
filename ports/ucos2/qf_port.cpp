//****************************************************************************
// Product: QF/C++ generic port to uC/OS-II
// Last updated for version 5.3.1
// Last updated on  2014-05-29
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, www.state-machine.com.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

//............................................................................
int_t QF::run(void) { //!!! DOS-specific

    // NOTE the QF::onStartup() callback must be invoked from the task level
    OSStart(); // start uC/OS-II multitasking (does not return)

    return static_cast<int_t>(0); // return success (just in case)
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}
//............................................................................
extern "C" void task_function(void *pdata) { // uC/OS-II task signature
    QF::thread_(static_cast<QActive *>(pdata));
}
//............................................................................
// NOTE: The call to uC/OS-II API OSTaskCreateExt() assumes that the
// potinter to the top-of-stack (ptos) is at the end of the provided
// stack memory. This is correct only for CPUs with downward-growing
// stack, but must be changed for CPUs with upward-growing stack
//
void QActive::start(uint_fast8_t const prio,
                    QEvt const *qSto[], uint_fast16_t const qLen,
                    void * const stkSto, uint_fast16_t const stkSize,
                    QEvt const * const ie)
{
    m_eQueue = OSQCreate((void **)(qSto), qLen);
    Q_ASSERT(m_eQueue != (OS_EVENT *)0); // uC/OS-II queue created
    m_prio = static_cast<uint8_t>(prio); // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO

    this->init(ie); // execute initial transition (virtual call)
    QS_FLUSH();     // flush the trace buffer to the host

    // map from QP to uC/OS priority
    INT8U p_ucos = static_cast<INT8U>(QF_MAX_ACTIVE - m_prio);

    // create AO's task...
    INT8U err = OSTaskCreateExt(&task_function, // the task function
             (void *)this,   // the 'pdata' parameter
             &(((OS_STK *)stkSto)[(stkSize / sizeof(OS_STK)) - 1]), // stack
             p_ucos,         // uC/OS-II task priority
             (INT16U)p_ucos, // the unique priority is the task id as well
             (OS_STK *)stkSto,       // pbos
             (INT32U)(stkSize/sizeof(OS_STK)), // stack size in OS_STK units
             (void *)0,      // pext
             (INT16U)OS_TASK_OPT_STK_CLR); // opt
    Q_ASSERT(err == OS_NO_ERR); // uC/OS-II task must be created correctly
}

//............................................................................
void QF::init(void) {
    OSInit(); // initialize uC/OS-II
}
//............................................................................
void QF::thread_(QActive *act) {
    // event-loop of an active object thread
    act->m_thread = static_cast<uint_fast8_t>(1);
    while (act->m_thread != static_cast<uint_fast8_t>(0)) {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
    QF::remove_(act);        // remove this object from the framework
    OSTaskDel(OS_PRIO_SELF); // make uC/OS-II forget about this task
}
//............................................................................
void QActive::stop(void) {
    m_thread = static_cast<uint_fast8_t>(0); // stop the AO task loop

    // cleanup the uC/OS-II queue
    INT8U err;
    OSQDel(m_eQueue, OS_DEL_ALWAYS, &err);
    Q_ASSERT(err == OS_NO_ERR);
}
//............................................................................
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin)
#else
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin,
                    void const * const sender)
#endif // Q_SPY
{
    OS_Q_DATA const *os_q_data =
        reinterpret_cast<OS_Q_DATA const *>(m_eQueue);
    bool status;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();

    uint_fast16_t nFree =
        static_cast<uint_fast16_t>(os_q_data->OSQSize - os_q_data->OSNMsgs);

    if (nFree > margin) {
        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::priv_.aoObjFilter, this)
            QS_TIME_();         // timestamp
            QS_OBJ_(sender);    // the sender object
            QS_SIG_(e->sig);    // the signal of the event
            QS_OBJ_(this);      // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id/ref Ctr of the event
            // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(nFree));
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        if (e->poolId_ != u8_0) {   // is it a pool event?
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }
        Q_ALLEGE(OSQPost(m_eQueue, (void *)e) == OS_NO_ERR);
        status = true; // return success
    }
    else {
        Q_ASSERT(margin != u16_0); // can tolerate dropping evts?

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT, QS::priv_.aoObjFilter,
                         this)
            QS_TIME_();         // timestamp
            QS_OBJ_(sender);    // the sender object
            QS_SIG_(e->sig);    // the signal of the event
            QS_OBJ_(this);      // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id/ref Ctr of the event
            // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(nFree));
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        status = false; // return failure
    }

    QF_CRIT_EXIT_();

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
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id/ref Ctr of the event
        // # free entries
        QS_EQC_(static_cast<QEQueueCtr>(
                reinterpret_cast<OS_Q_DATA const *>(m_eQueue)->OSQSize
                - reinterpret_cast<OS_Q_DATA const *>(m_eQueue)->OSNMsgs));
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
    QS_END_NOCRIT_()

    if (e->poolId_ != u8_0) {   // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    // NOTE: uC/OS-II does not support postig to the front of the
    // queue, so here a regular posting is used. This is not quite OK!
    Q_ALLEGE(OSQPost(m_eQueue, (void *)e) == OS_NO_ERR);

    QF_CRIT_EXIT_();
}
//............................................................................
QEvt const *QActive::get_(void) {
    INT8U err;
    QEvt const *e = static_cast<QEvt const *>(OSQPend(m_eQueue, 0, &err));
    QS_CRIT_STAT_

    Q_ASSERT(err == OS_NO_ERR);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::priv_.aoObjFilter, this)
        QS_TIME_();             // timestamp
        QS_SIG_(e->sig);        // the signal of this event
        QS_OBJ_(this);          // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id/ref Ctr of the event
        // # free entries
        QS_EQC_(static_cast<QEQueueCtr>(
                reinterpret_cast<OS_Q_DATA const *>(m_eQueue)->OSQSize
                - reinterpret_cast<OS_Q_DATA const *>(m_eQueue)->OSNMsgs));
    QS_END_()

    return e;
}

} // namespace QP


