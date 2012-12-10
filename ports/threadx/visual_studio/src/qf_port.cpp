//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port to ThreadX
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 19, 2012
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

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------

//............................................................................
void QF::init(void) {
}
//............................................................................
int16_t QF::run(void) {
    static TX_TIMER qf_tick_timer;
    Q_ALLEGE(tx_timer_create(&qf_tick_timer,           // ThreadX timer object
                 "QF",                                    // name of the timer
                 (VOID (*)(ULONG))&QF::tick,            // expiration function
                 0,                               // expiration function input
                 1,                                           // initial ticks
                 1,                                        // reschedule ticks
                 TX_AUTO_ACTIVATE)             // automatically activate timer
             == TX_SUCCESS);
    return static_cast<int16_t>(0);                          // return success
}
//............................................................................
void QF::stop(void) {
    onCleanup();                                           // cleanup callback
}
//............................................................................
void QF::thread_(QActive *act) {
    do {                  // loop until m_thread is cleared in QActive::stop()
        QEvt const *e = act->get_();                         // wait for event
        act->dispatch(e);     // dispatch to the active object's state machine
        gc(e);          // check if the event is garbage, and collect it if so
    } while (act->m_osObject);
    Q_ALLEGE(tx_queue_delete(&act->m_eQueue) == TX_SUCCESS);  // cleanup queue
    Q_ALLEGE(tx_thread_delete(&act->m_thread) == TX_SUCCESS);// cleanup thread
}
//............................................................................
extern "C" void thread_function(ULONG thread_input) {     // ThreadX signature
    QF::thread_(reinterpret_cast<QActive *>(thread_input));
}
//............................................................................
void QActive::start(uint8_t prio,
                    QEvt const *qSto[], uint32_t qLen,
                    void *stkSto, uint32_t stkSize,
                    QEvt const *ie)
{

                      // allege that the ThreadX queue is created successfully
    Q_ALLEGE(tx_queue_create(&m_eQueue,
                 "Q",
                 TX_1_ULONG,
                 (VOID *)qSto,
                 (ULONG)(qLen * sizeof(ULONG)))
             == TX_SUCCESS);

    m_prio = prio;                                     // save the QF priority
    QF::add_(this);                     // make QF aware of this active object
    init(ie);                                    // execute initial transition

    QS_FLUSH();                          // flush the trace buffer to the host

                                // convert QF priority to the ThreadX priority
        // ThreadX priority corresponding to the QF priority prio
    UINT tx_prio = QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - prio;

    Q_ALLEGE(tx_thread_create(&m_thread,       // ThreadX thread control block
                 "AO",                                          // thread name
                 &thread_function,                          // thread function
                 (ULONG)this,                                  // thread input
                 stkSto,                                        // stack start
                 stkSize,                               // stack size in bytes
                 tx_prio,                                  // ThreadX priority
                 tx_prio,   //preemption threshold disabled (same as priority)
                 TX_NO_TIME_SLICE,
                 TX_AUTO_START)
             == TX_SUCCESS);

    m_osObject = true;                  // indicate that the thread is running
}
//............................................................................
void QActive::stop(void) {
    m_osObject = false;                                // stop the thread loop
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
    Q_ALLEGE(tx_queue_send(&m_eQueue, (VOID *)&e, TX_NO_WAIT)
             == TX_SUCCESS);
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
    Q_ALLEGE(tx_queue_front_send(&m_eQueue, (VOID *)&e, TX_NO_WAIT)
             == TX_SUCCESS);
}
//............................................................................
QEvt const *QActive::get_(void) {
    QEvt const *e;
    Q_ALLEGE(tx_queue_receive(&m_eQueue, (VOID *)&e, TX_WAIT_FOREVER)
             == TX_SUCCESS);
    QS_CRIT_STAT_

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
