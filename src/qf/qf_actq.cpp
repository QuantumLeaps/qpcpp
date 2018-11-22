/// @file
/// @brief QP::QActive native queue operations (based on QP::QEQueue)
///
/// @note
/// this source file is only included in the QF library when the native
/// QF active object queue is used (instead of a message queue of an RTOS).
///
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.3.6
/// Last updated on  2018-10-04
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
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
/// https://www.state-machine.com
/// mailto:info@state-machine.com
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

Q_DEFINE_THIS_MODULE("qf_actq")

//****************************************************************************
/// @description
/// Direct event posting is the simplest asynchronous communication method
/// available in QF.
///
/// @param[in,out] e      pointer to the event to be posted
/// @param[in]     margin number of required free slots in the queue
///                after posting the event. The special value QP::QF_NO_MARGIN
///                means that this function will assert if posting fails.
///
/// @returns
/// 'true' (success) if the posting succeeded (with the provided margin) and
/// 'false' (failure) when the posting fails.
///
/// @attention
/// Should be called only via the macro POST() or POST_X().
///
/// @note
/// The QP::QF_NO_MARGIN value of the @p margin argument is special and
/// denotes situation when the post() operation is assumed to succeed (event
/// delivery guarantee). An assertion fires, when the event cannot be
/// delivered in this case.
///
/// @usage
/// @include qf_post.cpp
///
/// @sa
/// QActive::postLIFO()
///
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin)
#else
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin,
                    void const * const sender)
#endif
{
    bool status;
    QF_CRIT_STAT_
    QS_TEST_PROBE_DEF(&QActive::post_)

    /// @pre event pointer must be valid
    Q_REQUIRE_ID(100, e != static_cast<QEvt const *>(0));

    QF_CRIT_ENTRY_();
    QEQueueCtr nFree = m_eQueue.m_nFree; // get volatile into the temporary

    // test-probe#1 for faking queue overflow
    QS_TEST_PROBE_ID(1,
        nFree = static_cast<QEQueueCtr>(0);
    )

    if (margin == QF_NO_MARGIN) {
        if (nFree > static_cast<QEQueueCtr>(0)) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_CRIT_(110); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post, but don't assert
    }

    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    if (status) { // can post the event?

        --nFree;  // one free entry just used up
        m_eQueue.m_nFree = nFree; // update the volatile
        if (m_eQueue.m_nMin > nFree) {
            m_eQueue.m_nMin = nFree; // update minimum so far
        }

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();               // timestamp
            QS_OBJ_(sender);          // the sender object
            QS_SIG_(e->sig);          // the signal of the event
            QS_OBJ_(this);            // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);           // number of free entries
            QS_EQC_(m_eQueue.m_nMin); // min number of free entries
        QS_END_NOCRIT_()

#ifdef Q_UTEST
        // callback to examine the posted event under the the same conditions
        // as producing the QS_QF_ACTIVE_POST_FIFO trace record, which are:
        // 1. the local AO-filter is not set (zero) OR
        // 2. the local AO-filter is set to this AO ('me')
        //
        if ((QS::priv_.locFilter[QS::AO_OBJ] == static_cast<QActive *>(0))
            || (QS::priv_.locFilter[QS::AO_OBJ] == this))
        {
            QS::onTestPost(sender, this, e, status);
        }
#endif
        // empty queue?
        if (m_eQueue.m_frontEvt == static_cast<QEvt const *>(0)) {
            m_eQueue.m_frontEvt = e;      // deliver event directly
            QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
        }
        // queue is not empty, insert event into the ring-buffer
        else {
            // insert event pointer e into the buffer (FIFO)
            QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_head) = e;

            // need to wrap head?
            if (m_eQueue.m_head == static_cast<QEQueueCtr>(0)) {
                m_eQueue.m_head = m_eQueue.m_end; // wrap around
            }
            --m_eQueue.m_head; // advance the head (counter clockwise)
        }

        QF_CRIT_EXIT_();
    }
    else { // cannot post the event

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();           // timestamp
            QS_OBJ_(sender);      // the sender object
            QS_SIG_(e->sig);      // the signal of the event
            QS_OBJ_(this);        // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);       // number of free entries
            QS_EQC_(static_cast<QEQueueCtr>(margin)); // margin requested
        QS_END_NOCRIT_()

#ifdef Q_UTEST
        // callback to examine the posted event under the the same conditions
        // as producing the QS_QF_ACTIVE_POST_ATTEMPT trace record, which are:
        // 1. the local AO-filter is not set (zero) OR
        // 2. the local AO-filter is set to this AO ('me')
        //
        if ((QS::priv_.locFilter[QS::AO_OBJ] == static_cast<QActive *>(0))
            || (QS::priv_.locFilter[QS::AO_OBJ] == this))
        {
            QS::onTestPost(sender, this, e, status);
        }
#endif

        QF_CRIT_EXIT_();

        QF::gc(e); // recycle the event to avoid a leak
    }

    return status;
}

//****************************************************************************
/// @description
/// posts an event to the event queue of the active object  using the
/// Last-In-First-Out (LIFO) policy.
///
/// @note
/// The LIFO policy should be used only for self-posting and with caution,
/// because it alters order of events in the queue.
///
/// @param[in]  e  pointer to the event to post to the queue
///
/// @sa
/// QActive::post_()
///
void QActive::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_
    QS_TEST_PROBE_DEF(&QActive::postLIFO)

    QF_CRIT_ENTRY_();
    QEQueueCtr nFree = m_eQueue.m_nFree;// tmp to avoid UB for volatile access

    QS_TEST_PROBE_ID(1,
        nFree = static_cast<QEQueueCtr>(0);
    )

    // the queue must be able to accept the event (cannot overflow)
    Q_ASSERT_CRIT_(210, nFree != static_cast<QEQueueCtr>(0));

    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    --nFree;  // one free entry just used up
    m_eQueue.m_nFree = nFree; // update the volatile
    if (m_eQueue.m_nMin > nFree) {
        m_eQueue.m_nMin = nFree; // update minimum so far
    }

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO,
                     QS::priv_.locFilter[QS::AO_OBJ], this)
        QS_TIME_();                      // timestamp
        QS_SIG_(e->sig);                 // the signal of this event
        QS_OBJ_(this);                   // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_(nFree);                  // number of free entries
        QS_EQC_(m_eQueue.m_nMin);        // min number of free entries
    QS_END_NOCRIT_()

#ifdef Q_UTEST
    // callback to examine the posted event under the the same conditions
    // as producing the QS_QF_ACTIVE_POST_ATTEMPT trace record, which are:
    // 1. the local AO-filter is not set (zero) OR
    // 2. the local AO-filter is set to this AO ('me')
    //
    if ((QS::priv_.locFilter[QS::AO_OBJ] == static_cast<QActive *>(0))
        || (QS::priv_.locFilter[QS::AO_OBJ] == this))
    {
        QS::onTestPost(static_cast<QActive *>(0), this, e, true);
    }
#endif

    QEvt const *frontEvt = m_eQueue.m_frontEvt;// read volatile into temporary
    m_eQueue.m_frontEvt = e; // deliver the event directly to the front

    // was the queue empty?
    if (frontEvt == static_cast<QEvt const *>(0)) {
        QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
    }
    // queue was not empty, leave the event in the ring-buffer
    else {
        ++m_eQueue.m_tail;
        if (m_eQueue.m_tail == m_eQueue.m_end) { // need to wrap the tail?
            m_eQueue.m_tail = static_cast<QEQueueCtr>(0); // wrap around
        }

        QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_tail) = frontEvt;
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// The behavior of this function depends on the kernel used in the QF port.
/// For built-in kernels (Vanilla or QK) the function can be called only when
/// the queue is not empty, so it doesn't block. For a blocking kernel/OS
/// the function can block and wait for delivery of an event.
///
/// @returns
/// A pointer to the received event. The returned pointer is guaranteed to be
/// valid (can't be NULL).
///
/// @note
/// This function is used internally by a QF port to extract events from
/// the event queue of an active object. This function depends on the event
/// queue implementation and is sometimes customized in the QF port
/// (file qf_port.h). Depending on the definition of the macro
/// QACTIVE_EQUEUE_WAIT_(), the function might block the calling thread when
/// no events are available.
///
QEvt const *QActive::get_(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QACTIVE_EQUEUE_WAIT_(this); // wait for event to arrive directly

    QEvt const *e = m_eQueue.m_frontEvt; // always remove evt from the front
    QEQueueCtr nFree = m_eQueue.m_nFree + static_cast<QEQueueCtr>(1);
    m_eQueue.m_nFree = nFree; // upate the number of free

    // any events in the ring buffer?
    if (nFree <= m_eQueue.m_end) {

        // remove event from the tail
        m_eQueue.m_frontEvt = QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_tail);
        if (m_eQueue.m_tail == static_cast<QEQueueCtr>(0)) { // need to wrap?
            m_eQueue.m_tail = m_eQueue.m_end; // wrap around
        }
        --m_eQueue.m_tail;

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_GET,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();                      // timestamp
            QS_SIG_(e->sig);                 // the signal of this event
            QS_OBJ_(this);                   // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);                  // number of free entries
        QS_END_NOCRIT_()
    }
    else {
        // the queue becomes empty
        m_eQueue.m_frontEvt = static_cast<QEvt const *>(0);

        // all entries in the queue must be free (+1 for fronEvt)
        Q_ASSERT_CRIT_(310, nFree ==
                            (m_eQueue.m_end + static_cast<QEQueueCtr>(1)));

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_GET_LAST,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();                      // timestamp
            QS_SIG_(e->sig);                 // the signal of this event
            QS_OBJ_(this);                   // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_END_NOCRIT_()
    }
    QF_CRIT_EXIT_();
    return e;
}

//****************************************************************************
/// @description
/// Queries the minimum of free ever present in the given event queue of
/// an active object with priority @p prio, since the active object
/// was started.
///
/// @note
/// QP::QF::getQueueMin() is available only when the native QF event
/// queue implementation is used. Requesting the queue minimum of an unused
/// priority level raises an assertion in the QF. (A priority level becomes
/// used in QF after the call to the QP::QF::add_() function.)
///
/// @param[in] prio  Priority of the active object, whose queue is queried
///
/// @returns
/// the minimum of free ever present in the given event queue of an active
/// object with priority @p prio, since the active object was started.
///
uint_fast16_t QF::getQueueMin(uint_fast8_t const prio) {

    Q_REQUIRE_ID(400, (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
                      && (active_[prio] != static_cast<QActive *>(0)));

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    uint_fast16_t min =
        static_cast<uint_fast16_t>(active_[prio]->m_eQueue.m_nMin);
    QF_CRIT_EXIT_();

    return min;
}

//****************************************************************************
QTicker::QTicker(uint_fast8_t const tickRate)
  : QActive(Q_STATE_CAST(0))
{
    // reuse m_head for tick-rate
    m_eQueue.m_head = static_cast<QEQueueCtr>(tickRate);
}
//............................................................................
void QTicker::init(QEvt const * const /*e*/) {
    m_eQueue.m_tail = static_cast<QEQueueCtr>(0);
}
//............................................................................
void QTicker::dispatch(QEvt const * const /*e*/) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    QEQueueCtr n = m_eQueue.m_tail; // # ticks since the last call
    m_eQueue.m_tail = static_cast<QEQueueCtr>(0); // clear the # ticks
    QF_CRIT_EXIT_();

    for (; n > static_cast<QEQueueCtr>(0); --n) {
        QF::TICK_X(static_cast<uint_fast8_t>(m_eQueue.m_head), this);
    }
}
//............................................................................
#ifndef Q_SPY
bool QTicker::post_(QEvt const * const /*e*/, uint_fast16_t const /*margin*/)
#else
bool QTicker::post_(QEvt const * const /*e*/, uint_fast16_t const /*margin*/,
                    void const * const sender)
#endif
{
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    if (m_eQueue.m_frontEvt == static_cast<QEvt const *>(0)) {

#ifdef Q_EVT_CTOR
        static QEvt const tickEvt(static_cast<QSignal>(0),
                                  QEvt::STATIC_EVT);
#else
        static QEvt const tickEvt = { static_cast<QSignal>(0),
                                      static_cast<uint8_t>(0),
                                      static_cast<uint8_t>(0) };
#endif // Q_EVT_CTOR

        m_eQueue.m_frontEvt = &tickEvt; // deliver event directly
        --m_eQueue.m_nFree; // one less free event

        QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
    }

    ++m_eQueue.m_tail; // account for one more tick event

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO,
                     QS::priv_.locFilter[QS::AO_OBJ], this)
        QS_TIME_();               // timestamp
        QS_OBJ_(sender);          // the sender object
        QS_SIG_(static_cast<QSignal>(0)); // the signal of the event
        QS_OBJ_(this);            // this active object
        QS_2U8_(static_cast<uint8_t>(0),
                static_cast<uint8_t>(0)); // pool Id & refCtr of the evt
        QS_EQC_(static_cast<uint8_t>(0)); // number of free entries
        QS_EQC_(static_cast<uint8_t>(0)); // min number of free entries
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();

    return true; // the event is always posted correctly
}

//****************************************************************************
void QTicker::postLIFO(QEvt const * const /*e*/) {
    Q_ERROR_ID(900); // operation not allowed
}

} // namespace QP

