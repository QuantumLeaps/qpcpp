/// @file
/// @brief QP::QEQueue implementation
/// @cond
///***************************************************************************
/// Last updated for version 5.8.1
/// Last updated on  2016-12-14
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"       // QF package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY


namespace QP {

Q_DEFINE_THIS_MODULE("qf_qeq")


//****************************************************************************
/// @description
/// Default constructor
///
QEQueue::QEQueue(void)
  : m_frontEvt(static_cast<QEvt const *>(0)),
    m_ring(static_cast<QEvt const **>(0)),
    m_end(static_cast<QEQueueCtr>(0)),
    m_head(static_cast<QEQueueCtr>(0)),
    m_tail(static_cast<QEQueueCtr>(0)),
    m_nFree(static_cast<QEQueueCtr>(0)),
    m_nMin(static_cast<QEQueueCtr>(0))
{}

//****************************************************************************
/// @description
/// Initialize the event queue by giving it the storage for the ring buffer.
///
/// @param[in] qSto an array of pointers to QP::QEvt to sereve as the
///                 ring buffer for the event queue
/// @param[in] qLen the length of the qSto[] buffer (in QP::QEvt pointers)
///
/// @note The actual capacity of the queue is qLen + 1, because of the extra
/// location forntEvt.
///
/// @note This function is also used to initialize the event queues of active
/// objects in the built-int QK and "Vanilla" kernels, as well as other
/// QP ports to OSes/RTOSes that do provide a suitable message queue.
///
void QEQueue::init(QEvt const *qSto[], uint_fast16_t const qLen) {
    m_frontEvt = static_cast<QEvt const *>(0); // no events in the queue
    m_ring     = &qSto[0];
    m_end      = static_cast<QEQueueCtr>(qLen);
    if (qLen > static_cast<uint_fast16_t>(0)) {
        m_head = static_cast<QEQueueCtr>(0);
        m_tail = static_cast<QEQueueCtr>(0);
    }
    m_nFree    = static_cast<QEQueueCtr>(
                 qLen + static_cast<uint_fast16_t>(1)); //+1 for frontEvt
    m_nMin     = m_nFree;

    QS_CRIT_STAT_
    QS_BEGIN_(QS_QF_EQUEUE_INIT, QS::priv_.eqObjFilter, this)
        QS_OBJ_(this);   // this QEQueue object
        QS_EQC_(m_end);  // the length of the queue
    QS_END_()
}

//****************************************************************************
/// @description
/// Post an event to the "raw" thread-safe event queue using the
/// First-In-First-Out (FIFO) order.
///
/// @param[in] e      pointer to the event to be posted to the queue
/// @param[in] margin number of unused slots in the queue that must
///                   be still available after posting the event
/// @note
/// The zero value of the @p margin parameter is special and denotes situation
/// when event posting is assumed to succeed (event delivery guarantee).
/// An assertion fires, when the event cannot be delivered in this case.
///
/// @returns 'true' (success) when the posting succeeded with the provided
/// margin and 'false' (failure) when the posting fails.
///
/// @note This function can be called from any task context or ISR context.
///
/// @sa QP::QEQueue::postLIFO(), QP::QEQueue::get()
///
bool QEQueue::post(QEvt const * const e, uint_fast16_t const margin) {
    bool status;
    QF_CRIT_STAT_

    /// @pre the event must be valid
    Q_REQUIRE_ID(200, e != static_cast<QEvt const *>(0));

    QF_CRIT_ENTRY_();
    QEQueueCtr nFree = m_nFree; // temporary to avoid UB for volatile access

    // required margin available?
    if (nFree > static_cast<QEQueueCtr>(margin)) {

        QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_POST_FIFO, QS::priv_.eqObjFilter, this)
            QS_TIME_();                      // timestamp
            QS_SIG_(e->sig);                 // the signal of this event
            QS_OBJ_(this);                   // this queue object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);                  // number of free entries
            QS_EQC_(m_nMin);                 // min number of free entries
        QS_END_NOCRIT_()

        // is it a dynamic event?
        if (e->poolId_ != static_cast<uint8_t>(0)) {
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        --nFree; // one free entry just used up
        m_nFree = nFree; // update the volatile
        if (m_nMin > nFree) {
            m_nMin = nFree; // update minimum so far
        }

        // is the queue empty?
        if (m_frontEvt == static_cast<QEvt const *>(0)) {
            m_frontEvt = e; // deliver event directly
        }
        // queue is not empty, leave event in the ring-buffer
        else {
            // insert event into the ring buffer (FIFO)
            QF_PTR_AT_(m_ring, m_head) = e; // insert e into buffer

            // need to wrap?
            if (m_head == static_cast<QEQueueCtr>(0)) {
                m_head = m_end; // wrap around
            }
            --m_head;
        }
        status = true; // event posted successfully
    }
    else {
        /// @note If the @p margin is zero, assert that the queue can accept
        /// the event. This is to support the "guaranteed event delivery"
        /// policy for most events posted within the framework.
        Q_ASSERT_ID(210, margin != static_cast<uint_fast16_t>(0));

        QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_POST_ATTEMPT, QS::priv_.eqObjFilter,
                         this)
            QS_TIME_();                      // timestamp
            QS_SIG_(e->sig);                 // the signal of this event
            QS_OBJ_(this);                   // this queue object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);                  // number of free entries
            QS_EQC_(static_cast<QEQueueCtr>(margin)); // margin requested
        QS_END_NOCRIT_()

        status = false; // event not posted
    }
    QF_CRIT_EXIT_();

    return status;
}

//****************************************************************************
/// @description
/// Post an event to the "raw" thread-safe event queue using the
/// Last-In-First-Out (LIFO) order.
///
/// @param[in] e   pointer to the event to be posted to the queue
///
/// @attention The LIFO policy should be used only with great __caution__,
/// because it alters the order of events in the queue.
///
/// @note This function can be called from any task context or ISR context.
///
/// @note this function is used for the "raw" thread-safe queues and __not__
/// for the queues of active objects.
///
/// @sa QP::QEQueue::post(), QP::QEQueue::get(), QP::QActive::defer()
///
void QEQueue::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QEQueueCtr nFree = m_nFree; // temporary to avoid UB for volatile access

    /// @pre the queue must be able to accept the event (cannot overflow)
    Q_REQUIRE_ID(300, nFree != static_cast<QEQueueCtr>(0));

    QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_POST_LIFO, QS::priv_.eqObjFilter, this)
        QS_TIME_();                      // timestamp
        QS_SIG_(e->sig);                 // the signal of this event
        QS_OBJ_(this);                   // this queue object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_(nFree);                  // number of free entries
        QS_EQC_(m_nMin);                 // min number of free entries
    QS_END_NOCRIT_()

    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    --nFree; // one free entry just used up
    m_nFree = nFree; // update the volatile
    if (m_nMin > nFree) {
        m_nMin = nFree; // update minimum so far
    }

    QEvt const *frontEvt = m_frontEvt; // read volatile into temporary
    m_frontEvt = e; // deliver event directly to the front of the queue

    // was the queue not empty?
    if (frontEvt != static_cast<QEvt const *>(0)) {
        ++m_tail;
        if (m_tail == m_end) { // need to wrap the tail?
            m_tail = static_cast<QEQueueCtr>(0); // wrap around
        }
        QF_PTR_AT_(m_ring, m_tail) = frontEvt; // buffer the old front evt
    }

    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// Retrieves an event from the front of the "raw" thread-safe queue and
/// returns a pointer to this event to the caller.
///
/// @returns pointer to event at the front of the queue, if the queue is
/// not empty and NULL if the queue is empty.
///
/// @note this function is used for the "raw" thread-safe queues and __not__
/// for the queues of active objects.
///
/// @sa QP::QEQueue::post(), QP::QEQueue::postLIFO(), QP::QActive::recall()
///
QEvt const *QEQueue::get(void) {
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    e = m_frontEvt;  // always remove the event from the front location

    // is the queue not empty?
    if (e != static_cast<QEvt const *>(0)) {
        QEQueueCtr nFree = m_nFree + static_cast<QEQueueCtr>(1);
        m_nFree = nFree;  // upate the number of free

        // any events in the the ring buffer?
        if (nFree <= m_end) {
            m_frontEvt = QF_PTR_AT_(m_ring, m_tail); // remove from the tail
            if (m_tail == static_cast<QEQueueCtr>(0)) { // need to wrap?
                m_tail = m_end; // wrap around
            }
            --m_tail;

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET, QS::priv_.eqObjFilter, this)
                QS_TIME_();              // timestamp
                QS_SIG_(e->sig);         // the signal of this event
                QS_OBJ_(this);           // this queue object
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
                QS_EQC_(nFree);          // number of free entries
            QS_END_NOCRIT_()
        }
        else {
            m_frontEvt = static_cast<QEvt const *>(0); // queue becomes empty

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_ID(410, nFree == (m_end + static_cast<QEQueueCtr>(1)));

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET_LAST, QS::priv_.eqObjFilter,
                             this)
                QS_TIME_();              // timestamp
                QS_SIG_(e->sig);         // the signal of this event
                QS_OBJ_(this);           // this queue object
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
            QS_END_NOCRIT_()
        }
    }
    QF_CRIT_EXIT_();
    return e;
}

} // namespace QP

