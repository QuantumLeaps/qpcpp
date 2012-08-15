//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port to x86, uC/OS-II, Open Watcom, Large model
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 15, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qf_pkg.h"
#include "qassert.h"

#include <dos.h>                          // for _dos_setvect()/_dos_getvect()

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------
static void interrupt (*l_dosSpareISR)(void);

//............................................................................
void QF::init(void) {
    OSInit();                                           // initialize uC/OS-II
}
//............................................................................
int16_t QF::run(void) {
                                     // install uC/OS-II context switch vector
    l_dosSpareISR = _dos_getvect(uCOS);
    _dos_setvect(uCOS, (void interrupt (*)(void))&OSCtxSw);

    // NOTE the QF::onStartup() callback must be invoked from the task level
    OSStart();                                  // start uC/OS-II multitasking

    return static_cast<int16_t>(0);                          // return success
}
//............................................................................
void QF::stop(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    _dos_setvect(uCOS, l_dosSpareISR);      // restore the original DOS vector
    QF_CRIT_EXIT_();
    onCleanup();                                           // cleanup callback
}
//............................................................................
void QF::thread_(QActive *act) {
    while (act->m_thread != 0U) {     // loop until cleared in QActive::stop()
        QEvt const *e = act->get_();                         // wait for event
        act->dispatch(e);     // dispatch to the active object's state machine
        gc(e);          // check if the event is garbage, and collect it if so
    }
    QF::remove_(act);                 // remove this object from the framework
    OSTaskDel(OS_PRIO_SELF);           // make uC/OS-II forget about this task
}
//............................................................................
extern "C" void task_function(void *pdata) {        // uC/OS-II task signature
    QF::thread_(static_cast<QActive *>(pdata));
}
//............................................................................
void QActive::start(uint8_t prio,
                    QEvt const *qSto[], uint32_t qLen,
                    void *stkSto, uint32_t stkSize,
                    QEvt const *ie)
{
    m_eQueue = OSQCreate((void **)qSto, qLen);
    Q_ASSERT(m_eQueue != (OS_EVENT *)0);             // uC/OS-II queue created
    m_prio = prio;                                      // set the QF priority
    QF::add_(this);                     // make QF aware of this active object
    init(ie);                                // execute the initial transition

    QS_FLUSH();                          // flush the trace buffer to the host

    uint8_t p_ucos = QF_MAX_ACTIVE - m_prio;    // map QF priority to uC/OS-II
    INT8U err = OSTaskCreateExt(&task_function,           // the task function
             this,                                    // the 'pdata' parameter
             &(((OS_STK *)stkSto)[(stkSize / sizeof(OS_STK)) - 1]),    // ptos
             p_ucos,                                 // uC/OS-II task priority
             p_ucos,             // the unique priority is the task id as well
             (OS_STK *)stkSto,                                         // pbos
             stkSize/sizeof(OS_STK),      // size of the stack in OS_STK units
             (void *)0,                                                // pext
             (INT16U)OS_TASK_OPT_STK_CLR);                              // opt
    Q_ASSERT(err == OS_NO_ERR);                       // uC/OS-II task created
    m_thread = 1U;             // to keep the active object task loop spinning
}
//............................................................................
void QActive::stop(void) {
    m_thread = 0U;                         // stop the active object task loop

    INT8U err;
    OSQDel(m_eQueue, OS_DEL_ALWAYS, &err);       // cleanup the uC/OS-II queue
    Q_ASSERT(err == OS_NO_ERR);
}
//............................................................................
#ifndef Q_SPY
void QActive::postFIFO(QEvt const *e) {
#else
void QActive::postFIFO(QEvt const *e, void const *sender) {
#endif

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(sender);                                  // the sender object
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                       // this active object (recipient)
        QS_U8_(QF_EVT_POOL_ID_(e));                // the pool Id of the event
        QS_U8_(QF_EVT_REF_CTR_(e));              // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOCRIT_()

    if (QF_EVT_POOL_ID_(e) != (uint8_t)0) {             // is it a pool event?
        QF_EVT_REF_CTR_INC_(e);             // increment the reference counter
    }
    QF_CRIT_EXIT_();
    Q_ALLEGE(OSQPost((OS_EVENT *)m_eQueue, (void *)e) == OS_NO_ERR);
}
//............................................................................
void QActive::postLIFO(QEvt const *e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(QF_EVT_POOL_ID_(e));                // the pool Id of the event
        QS_U8_(QF_EVT_REF_CTR_(e));              // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOCRIT_()

    if (QF_EVT_POOL_ID_(e) != (uint8_t)0) {             // is it a pool event?
        QF_EVT_REF_CTR_INC_(e);             // increment the reference counter
    }
    QF_CRIT_EXIT_();
    Q_ALLEGE(OSQPostFront((OS_EVENT *)m_eQueue, (void *)e) == OS_NO_ERR);
}
//............................................................................
QEvt const *QActive::get_(void) {
    INT8U err;
    QEvt const *e = (QEvt *)OSQPend((OS_EVENT *)m_eQueue, 0, &err);
    QS_CRIT_STAT_

    Q_ASSERT(err == OS_NO_ERR);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(QF_EVT_POOL_ID_(e));                // the pool Id of the event
        QS_U8_(QF_EVT_REF_CTR_(e));              // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
    QS_END_()

    return e;
}

QP_END_


