/// @file
/// @brief QF/C++ port to ThreadX, all supported compilers
/// @cond
///**************************************************************************
/// Last updated for version 6.6.0
/// Last updated on  2019-09-12
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
/// <www.state-machine.com>
/// <info@state-machine.com>
///**************************************************************************
/// @endcond

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // include QS port
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------
static void thread_function(ULONG thread_input); // prototype

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
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
}
//............................................................................
static void thread_function(ULONG thread_input) { // ThreadX signature
    // run the active-object thread
    QF::thread_(reinterpret_cast<QActive *>(thread_input));
}
//............................................................................
void QActive::start(uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t stkSize,
                    void const * const par)
{
    // allege that the ThreadX queue is created successfully
    Q_ALLEGE_ID(210,
        tx_queue_create(&m_eQueue,
            const_cast<CHAR *>("AO"),
            TX_1_ULONG,
            static_cast<VOID *>(qSto),
            static_cast<ULONG>(qLen * sizeof(ULONG)))
        == TX_SUCCESS);

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object

    init(par);      // execute initial transition
    QS_FLUSH();     // flush the trace buffer to the host

    // convert QF priority to the ThreadX priority
    UINT tx_prio = QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - prio;

    Q_ALLEGE_ID(220,
        tx_thread_create(
            &m_thread, // ThreadX thread control block
            const_cast<CHAR *>("AO"), // thread name
            &thread_function, // thread function
            reinterpret_cast<ULONG>(this), // thread parameter
            stkSto,    // stack start
            stkSize,   // stack size in bytes
            tx_prio,   // ThreadX priority
            tx_prio,   // preemption threshold disabled (same as priority)
            TX_NO_TIME_SLICE,
            TX_AUTO_START)
        == TX_SUCCESS);
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

    if (margin == QF_NO_MARGIN) {
        if (nFree > static_cast<QEQueueCtr>(0)) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_ID(510); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();       // timestamp
            QS_OBJ_(sender);  // the sender object
            QS_SIG_(e->sig);  // the signal of the event
            QS_OBJ_(this);    // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        // is it a pool event?
        if (e->poolId_ != static_cast<uint8_t>(0)) {
            QF_EVT_REF_CTR_INC_(e);  // increment the reference counter
        }

        QF_CRIT_EXIT_();

        QEvt const *ep = const_cast<QEvt const *>(e);
        Q_ALLEGE_ID(520,
            tx_queue_send(&m_eQueue, static_cast<VOID *>(&ep), TX_NO_WAIT)
            == TX_SUCCESS);
    }
    else {

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT,
            QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();       // timestamp
            QS_OBJ_(sender);  // the sender object
            QS_SIG_(e->sig);  // the signal of the event
            QS_OBJ_(this);    // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        QF_CRIT_EXIT_();
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO,
                     QS::priv_.locFilter[QS::AO_OBJ], this)
        QS_TIME_();       // timestamp
        QS_SIG_(e->sig);  // the signal of this event
        QS_OBJ_(this);    // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        // # free entries available
        QS_EQC_(static_cast<QEQueueCtr>(m_eQueue.tx_queue_available_storage));
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free entries (unknown)
    QS_END_NOCRIT_()

    // is it a pool event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e);  // increment the reference counter
    }

    QF_CRIT_EXIT_();

    // LIFO posting must succeed, see NOTE1
    QEvt const *ep = const_cast<QEvt const *>(e);
    Q_ALLEGE_ID(610,
        tx_queue_front_send(&m_eQueue, static_cast<VOID *>(&ep), TX_NO_WAIT)
        == TX_SUCCESS);
}
//............................................................................
QEvt const *QActive::get_(void) {
    QEvt const *e;
    QS_CRIT_STAT_

    Q_ALLEGE_ID(710,
        tx_queue_receive(&m_eQueue, (VOID *)&e, TX_WAIT_FOREVER)
        == TX_SUCCESS);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::priv_.locFilter[QS::AO_OBJ], this)
        QS_TIME_();       // timestamp
        QS_SIG_(e->sig);  // the signal of this event
        QS_OBJ_(this);    // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        // min # free entries
        QS_EQC_(static_cast<QEQueueCtr>(m_eQueue.tx_queue_available_storage));
    QS_END_()

    return e;
}

//............................................................................
void QFSchedLock::lock(uint_fast8_t prio) {
    m_lockHolder = tx_thread_identify();

    /// @pre must be thread level, so current TX thread must be available
    Q_REQUIRE_ID(800, m_lockHolder != static_cast<TX_THREAD *>(0));

    UINT tx_err = tx_thread_preemption_change(m_lockHolder,
                     (QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - prio),
                     &m_prevThre);

    if (tx_err == TX_SUCCESS) {
        QS_CRIT_STAT_
        m_lockPrio = prio;

        QS_BEGIN_(QS_SCHED_LOCK, static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_(); // timestamp
            QS_2U8_(static_cast<uint8_t>(QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE
                                         - m_prevThre),
                    static_cast<uint8_t>(prio)); // new lock prio
        QS_END_()
    }
    else if (tx_err == TX_THRESH_ERROR) {
        // threshold was greater than (lower prio) than the current prio
        m_lockPrio = static_cast<uint_fast8_t>(0); // threshold not changed
    }
    else {
        /* no other errors are tolerated */
        Q_ERROR_ID(810);
    }
}

//............................................................................
void QFSchedLock::unlock(void) const {
    QS_CRIT_STAT_

    /// @pre the lock holder TX thread must be available
    Q_REQUIRE_ID(900, (m_lockHolder != static_cast<TX_THREAD *>(0))
                      && (m_lockPrio != static_cast<uint_fast8_t>(0)));

    QS_BEGIN_(QS_SCHED_UNLOCK,
              static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_(); // timestamp
        QS_2U8_(static_cast<uint8_t>(m_lockPrio), /* prev lock prio */
                static_cast<uint8_t>(QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE
                                     - m_prevThre)); // new lock prio
    QS_END_()

    // restore the preemption threshold of the lock holder
    UINT old_thre;
    Q_ALLEGE_ID(910, tx_thread_preemption_change(m_lockHolder,
                     m_prevThre,
                     &old_thre) == TX_SUCCESS);
}

} // namespace QP

