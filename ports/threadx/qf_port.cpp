//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port to ThreadX
// Last updated for version 5.3.0
// Last updated on  2014-05-09
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
//////////////////////////////////////////////////////////////////////////////
#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------

//............................................................................
void QF::init(void) {
}
//............................................................................
int_t QF::run(void) {
    onStartup();  // the startup callback
    return static_cast<int_t>(0); // return success
}
//............................................................................
void QF::stop(void) {
    onCleanup(); // the cleanup callback
}
//............................................................................
void QF::thread_(QActive *act) {
    // event loop of the active object thread
    act->m_osObject = true; // enable the thread event-loop
    while (act->m_osObject) {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
    remove_(act);
    Q_ALLEGE(tx_queue_delete(&act->m_eQueue) == TX_SUCCESS); // cleanup queue
    Q_ALLEGE(tx_thread_delete(&act->m_thread) == TX_SUCCESS);// cleanup thread
}
//............................................................................
extern "C" void thread_function(ULONG thread_input) { // ThreadX signature
    QF::thread_(reinterpret_cast<QActive *>(thread_input));
}
//............................................................................
void QActive::start(uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t stkSize,
                    QEvt const *ie)
{

    // allege that the ThreadX queue is created successfully
    Q_ALLEGE(tx_queue_create(&m_eQueue,
                 "Q",
                 TX_1_ULONG,
                 static_cast<VOID *>(qSto),
                 static_cast<ULONG>(qLen * sizeof(ULONG)))
             == TX_SUCCESS);

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object
    init(ie);       // execute initial transition

    QS_FLUSH();     // flush the trace buffer to the host

    // convert QF priority to the ThreadX priority
    UINT tx_prio = QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - prio;

    Q_ALLEGE(tx_thread_create(&m_thread,    // ThreadX thread control block
                 "AO",                      // thread name
                 &thread_function,          // thread function
                 reinterpret_cast<ULONG>(this), // thread input
                 stkSto,                    // stack start
                 stkSize,                   // stack size in bytes
                 tx_prio,                   // ThreadX priority
                 tx_prio,   //preemption threshold disabled (same as priority)
                 TX_NO_TIME_SLICE,
                 TX_AUTO_START)
             == TX_SUCCESS);

    m_osObject = true; // indicate that the thread is running
}
//............................................................................
void QActive::stop(void) {
    m_osObject = false; // stop the thread loop
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
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    uint_fast16_t nFree =
        static_cast<uint_fast16_t>(m_eQueue.tx_queue_available_storage);

    if (nFree > margin) {
        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::priv_.aoObjFilter, this)
            QS_TIME_();              // timestamp
            QS_OBJ_(sender);         // the sender object
            QS_SIG_(e->sig);         // the signal of the event
            QS_OBJ_(this);           // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries available
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free entries (unknown)
        QS_END_NOCRIT_()

        // is it a pool event?
        if (QF_EVT_POOL_ID_(e) != static_cast<uint8_t>(0)) {
            QF_EVT_REF_CTR_INC_(e);  // increment the reference counter
        }

        QEvt const *ep = const_cast<QEvt const *>(e);
        Q_ALLEGE(tx_queue_send(&m_eQueue, static_cast<VOID *>(&ep), TX_NO_WAIT)
                 == TX_SUCCESS);
        status = true;
    }
    else {
        // can tolerate dropping evts?
        Q_ASSERT(margin != static_cast<uint_fast16_t>(0));

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT, QS::priv_.aoObjFilter, this)
            QS_TIME_();              // timestamp
            QS_OBJ_(sender);         // the sender object
            QS_SIG_(e->sig);         // the signal of the event
            QS_OBJ_(this);           // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries available
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free entries (unknown)
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
        QS_TIME_();                  // timestamp
        QS_SIG_(e->sig);             // the signal of this event
        QS_OBJ_(this);               // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        // # free entries available
        QS_EQC_(static_cast<QEQueueCtr>(m_eQueue.tx_queue_available_storage));
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free entries (unknown)
    QS_END_NOCRIT_()

    // is it a pool event?
    if (QF_EVT_POOL_ID_(e) != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e);  // increment the reference counter
    }

    // LIFO posting must succeed, see NOTE1
    QEvt const *ep = const_cast<QEvt const *>(e);
    Q_ALLEGE(tx_queue_front_send(&m_eQueue, static_cast<VOID *>(&ep),
                                 TX_NO_WAIT) == TX_SUCCESS);

    QF_CRIT_EXIT_();
}
//............................................................................
QEvt const *QActive::get_(void) {
    QEvt const *e;
    Q_ALLEGE(tx_queue_receive(&m_eQueue, (VOID *)&e, TX_WAIT_FOREVER)
             == TX_SUCCESS);
    QS_CRIT_STAT_

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::priv_.aoObjFilter, this)
        QS_TIME_();                  // timestamp
        QS_SIG_(e->sig);             // the signal of this event
        QS_OBJ_(this);               // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free entries (unknown)
    QS_END_()

    return e;
}

} // namespace QP
