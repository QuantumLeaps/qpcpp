//****************************************************************************
// Product: QF/C++ generic port to embOS
// Last updated for version 5.3.0
// Last updated on  2014-06-27
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
void QF::init(void) {
    OS_InitKern();  // initialize embOS
    OS_InitHW();    // initialize the hardware used by embOS
}
//............................................................................
int_t QF::run(void) {

    onStartup(); // startup callback
    OS_Start();  // start multitasking; NOTE: QS_start() does not return

    return static_cast<int_t>(0); // return success (just in case)
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}
//............................................................................
extern "C" void thread_function(void *pdata) { // embOS thread signature
    QF::thread_(static_cast<QActive *>(pdata));
}
//............................................................................
void QActive::start(uint_fast8_t const prio,
                    QEvt const *qSto[], uint_fast16_t const qLen,
                    void * const stkSto, uint_fast16_t const stkSize,
                    QEvt const * const ie)
{
    // create the embOS message box for the AO
    OS_CreateMB(&m_eQueue,
                static_cast<OS_U16>(sizeof(QEvt *)),
                static_cast<OS_UINT>(qLen),
                static_cast<void *>(&qSto[0]));

    m_prio = static_cast<uint8_t>(prio); // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO
    this->init(ie); // execute initial transition (virtual call)

    QS_FLUSH();     // flush the trace buffer to the host


    // create AO's task...
    OS_CreateTaskEx(&m_thread,
                    "AO",
                    static_cast<OS_PRIO>(prio), // the same numbering as QP
                    &thread_function,
                    static_cast<void OS_STACKPTR *>(stkSto),
                    static_cast<OS_UINT>(stkSize),
                    static_cast<OS_UINT>(0), // no AOs at the same prio
                    static_cast<void *>(this));
}

//............................................................................
void QF::thread_(QActive *act) {

#if defined (__ARM7EM__) && (__CORE__ == __ARM7EM__) && defined (__ARMVFP__)
    // does the task use the FPU? see NOTE1
    if ((act->m_osObject & QF_TASK_USES_FPU) != static_cast<uint_fast8_t>(0)){
        OS_ExtendTaskContext_VFP();
    }
#endif

    // event-loop of an active object thread
    act->m_osObject = static_cast<uint_fast8_t>(1);  // enable thread loop
    while (act->m_osObject != static_cast<uint_fast8_t>(0)) {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }

    QF::remove_(act);        // remove this object from the framework
    OS_DeleteMB(&act->m_eQueue);
    OS_TerminateTask(&act->m_thread);
}
//............................................................................
void QActive::stop(void) {
    m_osObject = static_cast<uint_fast8_t>(0); // stop the AO thread loop
}
//............................................................................
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin)
#else
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin,
                    void const * const sender)
#endif // Q_SPY
{
    bool status;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    uint_fast16_t nFree = (uint_fast16_t)(m_eQueue.maxMsg - m_eQueue.nofMsg);

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
        // posting to the embOS mailbox must succeed, see NOTE2
        Q_ALLEGE(OS_PutMailCond(&m_eQueue,
                                static_cast<OS_CONST_PTR void *>(&e))
                 == static_cast<char>(0));

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
        QS_EQC_(static_cast<QEQueueCtr>(m_eQueue.maxMsg - m_eQueue.nofMsg));
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
    QS_END_NOCRIT_()

    if (e->poolId_ != u8_0) {   // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    // posting to the embOS mailbox must succeed, see NOTE2
    Q_ALLEGE(OS_PutMailFrontCond(&m_eQueue,
                                 static_cast<OS_CONST_PTR void *>(&e))
             == static_cast<char>(0));

    QF_CRIT_EXIT_();
}
//............................................................................
QEvt const *QActive::get_(void) {
    QEvt const *e;
    QS_CRIT_STAT_

    OS_GetMail(&m_eQueue, static_cast<void *>(&e));

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::priv_.aoObjFilter, this)
        QS_TIME_();             // timestamp
        QS_SIG_(e->sig);        // the signal of this event
        QS_OBJ_(this);          // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id/ref Ctr of the event
        // # free entries
        QS_EQC_(static_cast<QEQueueCtr>(m_eQueue.maxMsg - m_eQueue.nofMsg));
    QS_END_()

    return e;
}

} // namespace QP


