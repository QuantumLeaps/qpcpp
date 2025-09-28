//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

//============================================================================
// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_actq")
} // unnamed namespace

namespace QP {

//............................................................................
bool QActive::postx_(
    QEvt const * const e,
    std::uint_fast16_t const margin,
    void const * const sender) noexcept
{
#ifdef Q_UTEST // test?
#if (Q_UTEST != 0) // testing QP-stub?
    if (m_temp.fun == Q_STATE_CAST(0)) { // QActiveDummy?
        return static_cast<QActiveDummy *>(this)->fakePost(e, margin, sender);
    }
#endif // (Q_UTEST != 0)
#endif // def Q_UTEST

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the event to post must not be NULL
    Q_REQUIRE_INCRIT(100, e != nullptr);

    QEQueueCtr const nFree = m_eQueue.m_nFree; // get member into temporary

    bool status = (nFree > 0U);
    if (margin == QF::NO_MARGIN) { // no margin requested?
        // queue must not overflow
        Q_ASSERT_INCRIT(130, status);
    }
    else {
        status = (nFree > static_cast<QEQueueCtr>(margin));
    }

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    if (status) { // can post the event?
        postFIFO_(e, sender);
#ifdef Q_UTEST
        if (QS_LOC_CHECK_(m_prio)) {
            QF_CRIT_EXIT();
            QS::onTestPost(sender, this, e, true); // QUTest callback
            QF_CRIT_ENTRY();
        }
#endif // def Q_UTEST
        QF_CRIT_EXIT();
    }
    else { // event cannot be posted
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries
            QS_EQC_PRE(margin);  // margin requested
        QS_END_PRE()

#ifdef Q_UTEST
        if (QS_LOC_CHECK_(m_prio)) {
            QF_CRIT_EXIT();
            QS::onTestPost(sender, this, e, status);
            QF_CRIT_ENTRY();
        }
#endif // def Q_USTEST

        QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
        QF::gc(e); // recycle the event to avoid a leak
#endif // (QF_MAX_EPOOL > 0U)
    }

    return status;
}

//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
#ifdef Q_UTEST // test?
#if (Q_UTEST != 0) // testing QP-stub?
    if (m_temp.fun == Q_STATE_CAST(0)) { // QActiveDummy?
        static_cast<QActiveDummy *>(this)->QActiveDummy::fakePostLIFO(e);
        return;
    }
#endif // (Q_UTEST != 0)
#endif // def Q_UTEST

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the event to post must be be valid (which includes not NULL)
    Q_REQUIRE_INCRIT(200, e != nullptr);

    QEQueueCtr nFree = m_eQueue.m_nFree; // get member into temporary

    // the queue must NOT overflow for the LIFO posting policy.
    Q_REQUIRE_INCRIT(230, nFree != 0U);

    if (e->poolNum_ != 0U) { // is it a mutable event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    --nFree; // one free entry just used up
    m_eQueue.m_nFree = nFree; // update the original
    if (m_eQueue.m_nMin > nFree) {
        m_eQueue.m_nMin = nFree; // update minimum so far
    }

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(nFree);   // # free entries
        QS_EQC_PRE(m_eQueue.m_nMin); // min # free entries
    QS_END_PRE()

#ifdef Q_UTEST
    // callback to examine the posted event under the same conditions
    // as producing the #QS_QF_ACTIVE_POST trace record, which are:
    // the local filter for this AO ('m_prio') is set
    if (QS_LOC_CHECK_(m_prio)) {
        QF_CRIT_EXIT();
        QS::onTestPost(nullptr, this, e, true);
        QF_CRIT_ENTRY();
    }
#endif // def Q_UTEST

    QEvt const * const frontEvt = m_eQueue.m_frontEvt;
    m_eQueue.m_frontEvt = e; // deliver the event directly to the front

    if (frontEvt != nullptr) { // was the queue NOT empty?
        QEQueueCtr tail = m_eQueue.m_tail; // get member into temporary
        ++tail;
        if (tail == m_eQueue.m_end) { // need to wrap the tail?
            tail = 0U; // wrap around
        }
        m_eQueue.m_tail = tail;
        m_eQueue.m_ring[tail] = frontEvt;
    }
    else { // queue was empty
        QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
    }

    QF_CRIT_EXIT();
}

//............................................................................
QEvt const * QActive::get_() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // wait for event to arrive directly (depends on QP port)
    // NOTE: might use assertion-IDs 400-409
    QACTIVE_EQUEUE_WAIT_(this);

    // always remove event from the front
    QEvt const * const e = m_eQueue.m_frontEvt;

    // the queue must NOT be empty
    Q_REQUIRE_INCRIT(310, e != nullptr); // queue must NOT be empty

    QEQueueCtr nFree = m_eQueue.m_nFree; // get member into temporary

    ++nFree; // one more free event in the queue
    m_eQueue.m_nFree = nFree; // update the # free

    if (nFree <= m_eQueue.m_end) { // any events in the ring buffer?

        // remove event from the tail
        QEQueueCtr tail = m_eQueue.m_tail; // get member into temporary
        QEvt const * const frontEvt = m_eQueue.m_ring[tail];

        // the event queue must not be empty (frontEvt != NULL)
        Q_ASSERT_INCRIT(350, frontEvt != nullptr);

        QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_SIG_PRE(e->sig);  // the signal of this event
            QS_OBJ_PRE(this);    // this active object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries
        QS_END_PRE()

        m_eQueue.m_frontEvt = frontEvt; // update the original

        if (tail == 0U) { // need to wrap the tail?
            tail = m_eQueue.m_end;
        }
        --tail; // advance the tail (counter-clockwise)
        m_eQueue.m_tail = tail; // update the original
    }
    else {
        m_eQueue.m_frontEvt = nullptr; // queue becomes empty

        // all entries in the queue must be free (+1 for fronEvt)
        Q_ASSERT_INCRIT(360, nFree == (m_eQueue.m_end + 1U));

        QS_BEGIN_PRE(QS_QF_ACTIVE_GET_LAST, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_SIG_PRE(e->sig);  // the signal of this event
            QS_OBJ_PRE(this);    // this active object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_END_PRE()
    }

    QF_CRIT_EXIT();

    return e;
}

//............................................................................
void QActive::postFIFO_(
    QEvt const * const e,
    void const * const sender)
{
    // NOTE: this helper function is called *inside* critical section
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QEQueueCtr nFree = m_eQueue.m_nFree; // get member into temporary

    --nFree; // one free entry just used up
    m_eQueue.m_nFree = nFree; // update the original
    if (m_eQueue.m_nMin > nFree) {
        m_eQueue.m_nMin = nFree; // update minimum so far
    }

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
        QS_TIME_PRE();        // timestamp
        QS_OBJ_PRE(sender);   // the sender object
        QS_SIG_PRE(e->sig);   // the signal of the event
        QS_OBJ_PRE(this);     // this active object (recipient)
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(nFree);    // # free entries
        QS_EQC_PRE(m_eQueue.m_nMin); // min # free entries
    QS_END_PRE()

    if (m_eQueue.m_frontEvt == nullptr) { // is the queue empty?
        m_eQueue.m_frontEvt = e; // deliver event directly

#ifdef QXK_HPP_
        if (m_state.act == nullptr) { // extended thread?
            QXTHREAD_EQUEUE_SIGNAL_(this); // signal eXtended Thread
        }
        else { // basic thread (AO)
            QACTIVE_EQUEUE_SIGNAL_(this); // signal the Active Object
        }
#else
        QACTIVE_EQUEUE_SIGNAL_(this); // signal the Active Object
#endif // def QXK_HPP_
    }
    else { // queue was not empty, insert event into the ring-buffer
        QEQueueCtr head = m_eQueue.m_head; // get member into temporary
        m_eQueue.m_ring[head] = e; // insert e into buffer

        if (head == 0U) { // need to wrap the head?
            head = m_eQueue.m_end;
        }
        --head; // advance the head (counter-clockwise)

        m_eQueue.m_head = head; // update the original
    }
}

//............................................................................
std::uint16_t QActive::getQueueUse(std::uint_fast8_t const prio) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // prio must be in range. prio==0 is OK (special case)
    Q_REQUIRE_INCRIT(500, prio <= QF_MAX_ACTIVE);

    std::uint16_t nUse = 0U;

    if (prio > 0U) {
        QActive const * const a = QActive_registry_[prio];
        // the AO must be registered (started)
        Q_REQUIRE_INCRIT(510, a != nullptr);

        // NOTE: QEQueue_getUse() does NOT apply crit.sect. internally
        nUse = a->m_eQueue.getUse();
    }
    else { // special case of prio==0U: use of all AO event queues
        for (std::uint_fast8_t p = QF_MAX_ACTIVE; p > 0U; --p) {
            QActive const * const a = QActive_registry_[p];
            if (a != nullptr) { // is the AO registered?
                // NOTE: QEQueue::getUse() does NOT apply crit.sect. internally
                nUse += a->m_eQueue.getUse();
            }
        }
    }

    QF_CRIT_EXIT();

    return nUse;
}

//............................................................................
std::uint16_t QActive::getQueueFree(std::uint_fast8_t const prio) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(600, (0U < prio) && (prio <= QF_MAX_ACTIVE));

    QActive const * const a = QActive_registry_[prio];
    // the AO must be registered (started)
    Q_REQUIRE_INCRIT(610, a != nullptr);

    // NOTE: critical section prevents asynchronous change of the free count
    std::uint16_t const nFree =
        static_cast<std::uint16_t>(a->m_eQueue.m_nFree);
    QF_CRIT_EXIT();

    return nFree;
}

//............................................................................
std::uint16_t QActive::getQueueMin(std::uint_fast8_t const prio) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the queried prio. must be in range (excluding the idle thread)
    Q_REQUIRE_INCRIT(700, (0U < prio) && (prio <= QF_MAX_ACTIVE));

    QActive const * const a = QActive_registry_[prio];
    // the AO must be registered (started)
    Q_REQUIRE_INCRIT(710, a != nullptr);

    // NOTE: critical section prevents asynchronous change of the min count
    std::uint16_t const nMin = static_cast<std::uint16_t>(a->m_eQueue.m_nMin);

    QF_CRIT_EXIT();

    return nMin;
}

//============================================================================
#if (QF_MAX_TICK_RATE > 0U)

//............................................................................
QTicker::QTicker(std::uint8_t const tickRate) noexcept
  : QActive(nullptr)
{
    // reuse m_head for tick-rate
    m_eQueue.m_head = static_cast<QEQueueCtr>(tickRate);
}

//............................................................................
void QTicker::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qsId);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // instead of the top-most initial transition, QTicker initializes
    // the super.eQueue member inherited from QActive and reused
    // to count the number of tick events posted to this QTicker.
    // see also: QTicker_trig_()
    m_eQueue.m_tail = 0U;
    QF_CRIT_EXIT();
}

//............................................................................
void QTicker::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qsId);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // get members into temporaries
    QEQueueCtr nTicks = m_eQueue.m_tail;
    QEQueueCtr const tickRate = m_eQueue.m_head;

    // QTicker_dispatch_() shall be called only when it has tick events
    Q_REQUIRE_INCRIT(800, nTicks > 0U);

    m_eQueue.m_tail = 0U; // clear # ticks

    QF_CRIT_EXIT();

    // instead of dispatching the event, QTicker calls QTimeEvt_tick_()
    // processing for the number of times indicated in eQueue.tail.
    for (; nTicks > 0U; --nTicks) {
        QTimeEvt::tick(static_cast<std::uint_fast8_t>(tickRate), this);
    }
}

//............................................................................
void QTicker::trig_(void const * const sender) noexcept {
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    // static immutable (const) event to post to the QTicker AO
    static constexpr QEvt tickEvt(0U);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QEQueueCtr nTicks = m_eQueue.m_tail; // get member into temporary

    if (nTicks == 0U) { // no ticks accumulated yet?
        // when no ticks accumuilated, m_eQueue.m_fronEvt must be NULL
        Q_REQUIRE_INCRIT(930, m_eQueue.m_frontEvt == nullptr);

        m_eQueue.m_frontEvt = &tickEvt; // deliver event directly
        m_eQueue.m_nFree = 0U;

        QACTIVE_EQUEUE_SIGNAL_(this); // signal the event queue
    }
    else { // some tick have accumulated (and were not processed yet)
        // when some ticks accumuilated, eQueue.fronEvt must be &tickEvt
        Q_REQUIRE_INCRIT(940,m_eQueue.m_frontEvt == &tickEvt);

        // the nTicks counter must accept one more count without overflowing
        Q_REQUIRE_INCRIT(950, nTicks < 0xFFU);
    }

    ++nTicks; // account for one more tick event
    m_eQueue.m_tail = nTicks; // update the original

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
        QS_TIME_PRE();      // timestamp
        QS_OBJ_PRE(sender); // the sender object
        QS_SIG_PRE(0U);     // the signal of the event
        QS_OBJ_PRE(this);   // this active object
        QS_2U8_PRE(0U, 0U); // poolNum & refCtr
        QS_EQC_PRE(0U);     // # free entries
        QS_EQC_PRE(0U);     // min # free entries
    QS_END_PRE()

    QF_CRIT_EXIT();
}

#endif // (QF_MAX_TICK_RATE > 0U)

} // namespace QP
