//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2021-12-23
//! @version Last updated for: @ref qpcpp_7_0_0
//!
//! @file
//! @brief QP::QActive native queue operations (based on QP::QEQueue)
//!
//! @note
//! this source file is only included in the QF library when the native
//! QF active object queue is used (instead of a message queue of an RTOS).
//!

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

// unnamed namespace for local definitions with internal linkage
namespace {

Q_DEFINE_THIS_MODULE("qf_actq")

} // unnamed namespace

namespace QP {

#ifdef Q_SPY
//============================================================================
//! @description
//! Direct event posting is the simplest asynchronous communication method
//! available in QF.
//!
//! @param[in,out] e      pointer to the event to be posted
//! @param[in]     margin number of required free slots in the queue
//!                after posting the event. The special value QP::QF_NO_MARGIN
//!                means that this function will assert if posting fails.
//! @param[in]     sender pointer to a sender object (used in QS only)
//!
//! @returns
//! 'true' (success) if the posting succeeded (with the provided margin) and
//! 'false' (failure) when the posting fails.
//!
//! @attention
//! Should be called only via the macro POST() or POST_X().
//!
//! @note
//! The QP::QF_NO_MARGIN value of the @p margin argument is special and
//! denotes situation when the post() operation is assumed to succeed (event
//! delivery guarantee). An assertion fires, when the event cannot be
//! delivered in this case.
//!
//! @usage
//! @include qf_post.cpp
//!
//! @sa
//! QActive::postLIFO()
//!
bool QActive::post_(QEvt const * const e,
                    std::uint_fast16_t const margin,
                    void const * const sender) noexcept
#else
bool QActive::post_(QEvt const * const e,
                    std::uint_fast16_t const margin) noexcept
#endif
{
    //! @pre event pointer must be valid
    Q_REQUIRE_ID(100, e != nullptr);

    QF_CRIT_STAT_
    QF_CRIT_E_();
    QEQueueCtr nFree = m_eQueue.m_nFree; // get volatile into the temporary

    // test-probe#1 for faking queue overflow
    QS_TEST_PROBE_DEF(&QActive::post_)
    QS_TEST_PROBE_ID(1,
        nFree = 0U;
    )

    bool status;
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
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
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    if (status) { // can post the event?

        --nFree;  // one free entry just used up
        m_eQueue.m_nFree = nFree; // update the volatile
        if (m_eQueue.m_nMin > nFree) {
            m_eQueue.m_nMin = nFree; // update minimum so far
        }

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();               // timestamp
            QS_OBJ_PRE_(sender);          // the sender object
            QS_SIG_PRE_(e->sig);          // the signal of the event
            QS_OBJ_PRE_(this);            // this active object
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool-Id & ref-ctr
            QS_EQC_PRE_(nFree);           // number of free entries
            QS_EQC_PRE_(m_eQueue.m_nMin); // min number of free entries
        QS_END_NOCRIT_PRE_()

#ifdef Q_UTEST
        // callback to examine the posted event under the same conditions
        // as producing the #QS_QF_ACTIVE_POST trace record, which are:
        // the local filter for this AO ('me->prio') is set
        //
        if (QS_LOC_CHECK_(m_prio)) {
            QS::onTestPost(sender, this, e, status);
        }
#endif
        // empty queue?
        if (m_eQueue.m_frontEvt == nullptr) {
            m_eQueue.m_frontEvt = e;      // deliver event directly
            QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
        }
        // queue is not empty, insert event into the ring-buffer
        else {
            // insert event pointer e into the buffer (FIFO)
            m_eQueue.m_ring[m_eQueue.m_head] = e;

            // need to wrap head?
            if (m_eQueue.m_head == 0U) {
                m_eQueue.m_head = m_eQueue.m_end; // wrap around
            }
            --m_eQueue.m_head; // advance the head (counter clockwise)
        }

        QF_CRIT_X_();
    }
    else { // cannot post the event

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();           // timestamp
            QS_OBJ_PRE_(sender);      // the sender object
            QS_SIG_PRE_(e->sig);      // the signal of the event
            QS_OBJ_PRE_(this);        // this active object
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool-Id & ref-ctr
            QS_EQC_PRE_(nFree);       // number of free entries
            QS_EQC_PRE_(margin);      // margin requested
        QS_END_NOCRIT_PRE_()

#ifdef Q_UTEST
        // callback to examine the posted event under the same conditions
        // as producing the #QS_QF_ACTIVE_POST trace record, which are:
        // the local filter for this AO ('me->prio') is set
        //
        if (QS_LOC_CHECK_(m_prio)) {
            QS::onTestPost(sender, this, e, status);
        }
#endif

        QF_CRIT_X_();

        QF::gc(e); // recycle the event to avoid a leak
    }

    return status;
}

//============================================================================
//! @description
//! posts an event to the event queue of the active object  using the
//! Last-In-First-Out (LIFO) policy.
//!
//! @note
//! The LIFO policy should be used only for self-posting and with caution,
//! because it alters order of events in the queue.
//!
//! @param[in]  e  pointer to the event to post to the queue
//!
//! @sa
//! QActive::post_()
//!
void QActive::postLIFO(QEvt const * const e) noexcept {

    QF_CRIT_STAT_
    QF_CRIT_E_();
    QEQueueCtr nFree = m_eQueue.m_nFree;// tmp to avoid UB for volatile access

    QS_TEST_PROBE_DEF(&QActive::postLIFO)
    QS_TEST_PROBE_ID(1,
        nFree = 0U;
    )

    // the queue must be able to accept the event (cannot overflow)
    Q_ASSERT_CRIT_(210, nFree != 0U);

    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    --nFree;  // one free entry just used up
    m_eQueue.m_nFree = nFree; // update the volatile
    if (m_eQueue.m_nMin > nFree) {
        m_eQueue.m_nMin = nFree; // update minimum so far
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE_();                      // timestamp
        QS_SIG_PRE_(e->sig);                 // the signal of this event
        QS_OBJ_PRE_(this);                   // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool-Id & ref-ctr
        QS_EQC_PRE_(nFree);                  // number of free entries
        QS_EQC_PRE_(m_eQueue.m_nMin);        // min number of free entries
    QS_END_NOCRIT_PRE_()

#ifdef Q_UTEST
    // callback to examine the posted event under the same conditions
    // as producing the #QS_QF_ACTIVE_POST trace record, which are:
    // the local filter for this AO ('me->prio') is set
    //
    if (QS_LOC_CHECK_(m_prio)) {
        QS::onTestPost(nullptr, this, e, true);
    }
#endif

    // read volatile into temporary
    QEvt const * const frontEvt = m_eQueue.m_frontEvt;
    m_eQueue.m_frontEvt = e; // deliver the event directly to the front

    // was the queue empty?
    if (frontEvt == nullptr) {
        QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
    }
    // queue was not empty, leave the event in the ring-buffer
    else {
        ++m_eQueue.m_tail;
        if (m_eQueue.m_tail == m_eQueue.m_end) { // need to wrap the tail?
            m_eQueue.m_tail = 0U; // wrap around
        }

        m_eQueue.m_ring[m_eQueue.m_tail] = frontEvt;
    }
    QF_CRIT_X_();
}

//============================================================================
//! @description
//! The behavior of this function depends on the kernel used in the QF port.
//! For built-in kernels (Vanilla or QK) the function can be called only when
//! the queue is not empty, so it doesn't block. For a blocking kernel/OS
//! the function can block and wait for delivery of an event.
//!
//! @returns
//! A pointer to the received event. The returned pointer is guaranteed to be
//! valid (can't be NULL).
//!
//! @note
//! This function is used internally by a QF port to extract events from
//! the event queue of an active object. This function depends on the event
//! queue implementation and is sometimes customized in the QF port
//! (file qf_port.hpp). Depending on the definition of the macro
//! QACTIVE_EQUEUE_WAIT_(), the function might block the calling thread when
//! no events are available.
//!
QEvt const *QActive::get_(void) noexcept {

    QF_CRIT_STAT_
    QF_CRIT_E_();
    QACTIVE_EQUEUE_WAIT_(this); // wait for event to arrive directly

    // always remove evt from the front
    QEvt const * const e = m_eQueue.m_frontEvt;
    QEQueueCtr const nFree = m_eQueue.m_nFree + 1U;
    m_eQueue.m_nFree = nFree; // upate the number of free

    // any events in the ring buffer?
    if (nFree <= m_eQueue.m_end) {

        // remove event from the tail
        m_eQueue.m_frontEvt = m_eQueue.m_ring[m_eQueue.m_tail];
        if (m_eQueue.m_tail == 0U) { // need to wrap?
            m_eQueue.m_tail = m_eQueue.m_end; // wrap around
        }
        --m_eQueue.m_tail;

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_GET, m_prio)
            QS_TIME_PRE_();                      // timestamp
            QS_SIG_PRE_(e->sig);                 // the signal of this event
            QS_OBJ_PRE_(this);                   // this active object
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool-Id & ref-ctr
            QS_EQC_PRE_(nFree);                  // number of free entries
        QS_END_NOCRIT_PRE_()
    }
    else {
        // the queue becomes empty
        m_eQueue.m_frontEvt = nullptr;

        // all entries in the queue must be free (+1 for fronEvt)
        Q_ASSERT_CRIT_(310, nFree == (m_eQueue.m_end + 1U));

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_GET_LAST, m_prio)
            QS_TIME_PRE_();                      // timestamp
            QS_SIG_PRE_(e->sig);                 // the signal of this event
            QS_OBJ_PRE_(this);                   // this active object
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool-Id & ref-ctr
        QS_END_NOCRIT_PRE_()
    }
    QF_CRIT_X_();
    return e;
}

//============================================================================
//! @description
//! Queries the minimum of free ever present in the given event queue of
//! an active object with priority @p prio, since the active object
//! was started.
//!
//! @note
//! QP::QF::getQueueMin() is available only when the native QF event
//! queue implementation is used. Requesting the queue minimum of an unused
//! priority level raises an assertion in the QF. (A priority level becomes
//! used in QF after the call to the QP::QF::add_() function.)
//!
//! @param[in] prio  Priority of the active object, whose queue is queried
//!
//! @returns
//! the minimum of free ever present in the given event queue of an active
//! object with priority @p prio, since the active object was started.
//!
std::uint_fast16_t QF::getQueueMin(std::uint_fast8_t const prio) noexcept {

    Q_REQUIRE_ID(400, (prio <= QF_MAX_ACTIVE)
                      && (active_[prio] != nullptr));

    QF_CRIT_STAT_
    QF_CRIT_E_();
    std::uint_fast16_t const min =
        static_cast<std::uint_fast16_t>(active_[prio]->m_eQueue.m_nMin);
    QF_CRIT_X_();

    return min;
}

//============================================================================
QTicker::QTicker(std::uint_fast8_t const tickRate) noexcept
  : QActive(nullptr)
{
    // reuse m_head for tick-rate
    m_eQueue.m_head = static_cast<QEQueueCtr>(tickRate);
}
//............................................................................
void QTicker::init(void const * const e,
                   std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(e); // unused parameter
    static_cast<void>(qs_id); // unused parameter
    m_eQueue.m_tail = 0U;
}
//............................................................................
void QTicker::dispatch(QEvt const * const e,
                   std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(e); // unused parameter
    static_cast<void>(qs_id); // unused parameter

    QF_CRIT_STAT_
    QF_CRIT_E_();
    QEQueueCtr nTicks = m_eQueue.m_tail; // # ticks since the last call
    m_eQueue.m_tail = 0U; // clear the # ticks
    QF_CRIT_X_();

    for (; nTicks > 0U; --nTicks) {
        QF::TICK_X(static_cast<std::uint_fast8_t>(m_eQueue.m_head), this);
    }
}
//............................................................................
void QTicker::init(std::uint_fast8_t const qs_id) noexcept {
    QTicker::init(nullptr, qs_id);
}
//............................................................................
#ifdef Q_SPY
//============================================================================
//! @sa
//! QActive::post_()
//!
bool QTicker::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
#else
bool QTicker::post_(QEvt const * const e, std::uint_fast16_t const margin)
    noexcept
#endif
{
    static_cast<void>(e);      // unused parameter
    static_cast<void>(margin); // unused parameter

    QF_CRIT_STAT_
    QF_CRIT_E_();
    if (m_eQueue.m_frontEvt == nullptr) {

#ifdef Q_EVT_CTOR
        static QEvt const tickEvt(0U, QEvt::STATIC_EVT);
#else
        static QEvt const tickEvt = { 0U, 0U, 0U };
#endif // Q_EVT_CTOR

        m_eQueue.m_frontEvt = &tickEvt; // deliver event directly
        --m_eQueue.m_nFree; // one less free event

        QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
    }

    ++m_eQueue.m_tail; // account for one more tick event

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
        QS_TIME_PRE_();      // timestamp
        QS_OBJ_PRE_(sender); // the sender object
        QS_SIG_PRE_(0U);     // the signal of the event
        QS_OBJ_PRE_(this);   // this active object
        QS_2U8_PRE_(0U, 0U); // pool-Id & ref-ctr
        QS_EQC_PRE_(0U);     // number of free entries
        QS_EQC_PRE_(0U);     // min number of free entries
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();

    return true; // the event is always posted correctly
}

//============================================================================
void QTicker::postLIFO(QEvt const * const e) noexcept {
    static_cast<void>(e); // unused parameter
    Q_ERROR_ID(900); // operation not allowed
}

} // namespace QP
