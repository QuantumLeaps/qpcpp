/// @file
/// @brief QP::QEQueue implementation
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-17
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

namespace QP {

Q_DEFINE_THIS_MODULE("qf_qeq")


//****************************************************************************
/// @description
/// Default constructor
///
QEQueue::QEQueue(void) noexcept
  : m_frontEvt(nullptr),
    m_ring(nullptr),
    m_end(0U),
    m_head(0U),
    m_tail(0U),
    m_nFree(0U),
    m_nMin(0U)
{}

//****************************************************************************
/// @description
/// Initialize the event queue by giving it the storage for the ring buffer.
///
/// @param[in] qSto an array of pointers to QP::QEvt to sereve as the
///                 ring buffer for the event queue
/// @param[in] qLen the length of the qSto[] buffer (in QP::QEvt pointers)
///
/// @note
/// The actual capacity of the queue is qLen + 1, because of the extra
/// location forntEvt.
///
/// @note This function is also used to initialize the event queues of active
/// objects in the built-int QV, QK and QXK kernels, as well as other
/// QP ports to OSes/RTOSes that do provide a suitable message queue.
///
void QEQueue::init(QEvt const *qSto[],
                   std::uint_fast16_t const qLen) noexcept
{
    m_frontEvt = nullptr; // no events in the queue
    m_ring     = &qSto[0];
    m_end      = static_cast<QEQueueCtr>(qLen);
    if (qLen > 0U) {
        m_head = 0U;
        m_tail = 0U;
    }
    m_nFree    = static_cast<QEQueueCtr>(qLen + 1U); //+1 for frontEvt
    m_nMin     = m_nFree;
}

//****************************************************************************
/// @description
/// Post an event to the "raw" thread-safe event queue using the
/// First-In-First-Out (FIFO) order.
///
/// @param[in] e      pointer to the event to be posted to the queue
/// @param[in] margin number of required free slots in the queue after
///                   posting the event. The special value QP::QF_NO_MARGIN
///                   means that this function will assert if posting
/// @param[in] qs_id  QS-id of this state machine (for QS local filter)
///
/// @note
/// The QP::QF_NO_MARGIN value of the @p margin argument is special and
/// denotes situation when the post() operation is assumed to succeed (event
/// delivery guarantee). An assertion fires, when the event cannot be
/// delivered in this case.
///
/// @returns 'true' (success) when the posting succeeded with the provided
/// margin and 'false' (failure) when the posting fails.
///
/// @note This function can be called from any task context or ISR context.
///
/// @sa QP::QEQueue::postLIFO(), QP::QEQueue::get()
///
bool QEQueue::post(QEvt const * const e,
                   std::uint_fast16_t const margin,
                   std::uint_fast8_t const qs_id) noexcept
{
    bool status;
    QF_CRIT_STAT_

    /// @pre event must be valid
    Q_REQUIRE_ID(200, e != nullptr);

    QF_CRIT_E_();
    QEQueueCtr nFree = m_nFree; // temporary to avoid UB for volatile access

    // margin available?
    if (((margin == QF_NO_MARGIN) && (nFree > 0U))
        || (nFree > static_cast<QEQueueCtr>(margin)))
    {
        // is it a dynamic event?
        if (e->poolId_ != 0U) {
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        --nFree; // one free entry just used up
        m_nFree = nFree; // update the volatile
        if (m_nMin > nFree) {
            m_nMin = nFree; // update minimum so far
        }

        QS_BEGIN_NOCRIT_PRE_(QS_QF_EQUEUE_POST, qs_id)
            QS_TIME_PRE_();                     // timestamp
            QS_SIG_PRE_(e->sig);                // the signal of this event
            QS_OBJ_PRE_(this);                  // this queue object
            QS_2U8_PRE_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
            QS_EQC_PRE_(nFree);                 // number of free entries
            QS_EQC_PRE_(m_nMin);                // min number of free entries
        QS_END_NOCRIT_PRE_()

        // is the queue empty?
        if (m_frontEvt == nullptr) {
            m_frontEvt = e; // deliver event directly
        }
        // queue is not empty, leave event in the ring-buffer
        else {
            // insert event into the ring buffer (FIFO)
            QF_PTR_AT_(m_ring, m_head) = e; // insert e into buffer

            // need to wrap?
            if (m_head == 0U) {
                m_head = m_end; // wrap around
            }
            --m_head;
        }
        status = true; // event posted successfully
    }
    else {
        /// @note assert if event cannot be posted and dropping events is
        /// not acceptable
        Q_ASSERT_CRIT_(210, margin != QF_NO_MARGIN);

        QS_BEGIN_NOCRIT_PRE_(QS_QF_EQUEUE_POST_ATTEMPT, qs_id)
            QS_TIME_PRE_();        // timestamp
            QS_SIG_PRE_(e->sig);   // the signal of this event
            QS_OBJ_PRE_(this);     // this queue object
            QS_2U8_PRE_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
            QS_EQC_PRE_(nFree);    // number of free entries
            QS_EQC_PRE_(margin);   // margin requested
        QS_END_NOCRIT_PRE_()

        status = false; // event not posted
    }
    QF_CRIT_X_();

    static_cast<void>(qs_id); // unused parameter, if Q_SPY not defined

    return status;
}

//****************************************************************************
/// @description
/// Post an event to the "raw" thread-safe event queue using the
/// Last-In-First-Out (LIFO) order.
///
/// @param[in] e     pointer to the event to be posted to the queue
/// @param[in] qs_id QS-id of this state machine (for QS local filter)
///
/// @attention
/// The LIFO policy should be used only with great __caution__,
/// because it alters the order of events in the queue.
///
/// @note
/// This function can be called from any task context or ISR context.
///
/// @note
/// This function is used for the "raw" thread-safe queues and __not__
/// for the queues of active objects.
///
/// @sa
/// QP::QEQueue::post(), QP::QEQueue::get(), QP::QActive::defer()
///
void QEQueue::postLIFO(QEvt const * const e,
                       std::uint_fast8_t const qs_id) noexcept
{
    QF_CRIT_STAT_

    QF_CRIT_E_();
    QEQueueCtr nFree = m_nFree; // temporary to avoid UB for volatile access

    /// @pre the queue must be able to accept the event (cannot overflow)
    Q_REQUIRE_CRIT_(300, nFree != 0U);

    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    --nFree; // one free entry just used up
    m_nFree = nFree; // update the volatile
    if (m_nMin > nFree) {
        m_nMin = nFree; // update minimum so far
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_EQUEUE_POST_LIFO, qs_id)
        QS_TIME_PRE_();                      // timestamp
        QS_SIG_PRE_(e->sig);                 // the signal of this event
        QS_OBJ_PRE_(this);                   // this queue object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_PRE_(nFree);                  // number of free entries
        QS_EQC_PRE_(m_nMin);                 // min number of free entries
    QS_END_NOCRIT_PRE_()

    QEvt const * const frontEvt = m_frontEvt; // read volatile into temporary
    m_frontEvt = e; // deliver event directly to the front of the queue

    // was the queue not empty?
    if (frontEvt != nullptr) {
        ++m_tail;
        if (m_tail == m_end) { // need to wrap the tail?
            m_tail = 0U; // wrap around
        }
        QF_PTR_AT_(m_ring, m_tail) = frontEvt; // buffer the old front evt
    }

    static_cast<void>(qs_id); // unused parameter, if Q_SPY not defined

    QF_CRIT_X_();
}

//****************************************************************************
/// @description
/// Retrieves an event from the front of the "raw" thread-safe queue and
/// returns a pointer to this event to the caller.
///
/// @param[in] qs_id QS-id of this state machine (for QS local filter)
///
/// @returns
/// pointer to event at the front of the queue, if the queue is
/// not empty and NULL if the queue is empty.
///
/// @note
/// this function is used for the "raw" thread-safe queues and __not__
/// for the queues of active objects.
///
/// @sa
/// QP::QEQueue::post(), QP::QEQueue::postLIFO(), QP::QActive::recall()
///
QEvt const *QEQueue::get(std::uint_fast8_t const qs_id) noexcept {
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    e = m_frontEvt;  // always remove the event from the front location

    // is the queue not empty?
    if (e != nullptr) {
        QEQueueCtr const nFree = m_nFree + 1U;
        m_nFree = nFree;  // upate the number of free

        // any events in the the ring buffer?
        if (nFree <= m_end) {
            m_frontEvt = QF_PTR_AT_(m_ring, m_tail); // remove from the tail
            if (m_tail == 0U) { // need to wrap?
                m_tail = m_end; // wrap around
            }
            --m_tail;

            QS_BEGIN_NOCRIT_PRE_(QS_QF_EQUEUE_GET, qs_id)
                QS_TIME_PRE_();              // timestamp
                QS_SIG_PRE_(e->sig);         // the signal of this event
                QS_OBJ_PRE_(this);           // this queue object
                QS_2U8_PRE_(e->poolId_, e->refCtr_);
                QS_EQC_PRE_(nFree);          // # free entries
            QS_END_NOCRIT_PRE_()
        }
        else {
            m_frontEvt = nullptr; // queue becomes empty

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_CRIT_(410, nFree == (m_end + 1U));

            QS_BEGIN_NOCRIT_PRE_(QS_QF_EQUEUE_GET_LAST, qs_id)
                QS_TIME_PRE_();              // timestamp
                QS_SIG_PRE_(e->sig);         // the signal of this event
                QS_OBJ_PRE_(this);           // this queue object
                QS_2U8_PRE_(e->poolId_, e->refCtr_);
            QS_END_NOCRIT_PRE_()
        }
    }
    QF_CRIT_X_();

    static_cast<void>(qs_id); // unused parameter, if Q_SPY not defined

    return e;
}

} // namespace QP

