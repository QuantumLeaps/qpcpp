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

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_qeq")
} // unnamed namespace

namespace QP {

//............................................................................
QEQueue::QEQueue() noexcept
  : m_frontEvt(nullptr),  // queue empty initially
    m_ring(nullptr),      // no queue buffer initially
    m_end(0U),            // end index 0 initially
    m_head(0U),           // head index 0 initially
    m_tail(0U),           // tail index 0 initially
    m_nFree(0U),          // no free events initially
    m_nMin(0U)            // all time minimum of free entries so far
{}
//............................................................................
void QEQueue::init(
    QEvt const * * const qSto,
    std::uint_fast16_t const qLen) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#if (QF_EQUEUE_CTR_SIZE == 1U)
    // the qLen paramter must not exceed the dynamic range of uint8_t
    Q_REQUIRE_INCRIT(10, qLen < 0xFFU);
#endif

    m_frontEvt = nullptr; // no events in the queue
    m_ring     = qSto;    // the beginning of the ring buffer
    m_end      = static_cast<QEQueueCtr>(qLen); // index of the last element
    if (qLen > 0U) { // queue buffer storage provided?
        m_head = 0U; // head index: for removing events
        m_tail = 0U; // tail index: for inserting events
    }
    m_nFree = static_cast<QEQueueCtr>(qLen + 1U); // +1 for frontEvt
    m_nMin  = m_nFree; // minimum so far

    QF_CRIT_EXIT();
}

//............................................................................
bool QEQueue::post(
    QEvt const * const e,
    std::uint_fast16_t const margin,
    std::uint_fast8_t const qsId) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the posted event must be valid
    Q_REQUIRE_INCRIT(100, e != nullptr);

    QEQueueCtr nFree = m_nFree; // get member into temporary

    bool status = (nFree > 0U);
    if (margin == QF::NO_MARGIN) { // no margin requested?
        // queue must not overflow
        Q_ASSERT_INCRIT(130, status);
    }
    else {
        status = (nFree > static_cast<QEQueueCtr>(margin));
    }

    if (status) { // can post the event?

#if (QF_MAX_EPOOL > 0U)
        if (e->poolNum_ != 0U) { // is it a mutable event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
#endif // (QF_MAX_EPOOL > 0U)

        --nFree; // one free entry just used up
        m_nFree = nFree; // update the original
        if (m_nMin > nFree) { // is this the new minimum?
            m_nMin = nFree; // update minimum so far
        }

#ifdef Q_SPY
        QS_BEGIN_PRE(QS_QF_EQUEUE_POST, qsId)
            QS_TIME_PRE();        // timestamp
            QS_SIG_PRE(e->sig);   // the signal of the event
            QS_OBJ_PRE(this);     // this queue object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);    // # free entries
            QS_EQC_PRE(m_nMin);   // min # free entries
        QS_END_PRE()
#endif // def Q_SPY

        if (m_frontEvt == nullptr) { // is the queue empty?
            m_frontEvt = e; // deliver event directly
        }
        else { // queue was not empty, insert event into the ring-buffer
            QEQueueCtr head = m_head; // get member into temporary
            m_ring[head] = e; // insert e into buffer

            if (head == 0U) { // need to wrap the head?
                head = m_end;
            }
            --head; // advance head (counter-clockwise)
            m_head = head; // update the member original
        }
    }
    else { // event cannot be posted
#ifdef Q_SPY
        QS_BEGIN_PRE(QS_QF_EQUEUE_POST_ATTEMPT, qsId)
            QS_TIME_PRE();       // timestamp
            QS_SIG_PRE(e->sig);  // the signal of this event
            QS_OBJ_PRE(this);    // this queue object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries
            QS_EQC_PRE(margin);  // margin requested
        QS_END_PRE()
#endif // def Q_SPY
    }

    QF_CRIT_EXIT();

    return status;
}

//............................................................................
void QEQueue::postLIFO(
    QEvt const * const e,
    std::uint_fast8_t const qsId) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // event e to be posted must be valid
    Q_REQUIRE_INCRIT(200, e != nullptr);

    QEQueueCtr nFree = m_nFree; // get member into temporary

    // must be able to LIFO-post the event
    Q_REQUIRE_INCRIT(230, nFree != 0U);

    if (e->poolNum_ != 0U) { // is it a mutable event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    --nFree; // one free entry just used up
    m_nFree = nFree; // update the member original
    if (m_nMin > nFree) { // is this the new minimum?
        m_nMin = nFree; // update minimum so far
    }

    QS_BEGIN_PRE(QS_QF_EQUEUE_POST_LIFO, qsId)
        QS_TIME_PRE();        // timestamp
        QS_SIG_PRE(e->sig);   // the signal of this event
        QS_OBJ_PRE(this);     // this queue object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(nFree);    // # free entries
        QS_EQC_PRE(m_nMin);   // min # free entries
    QS_END_PRE()

    QEvt const * const frontEvt = m_frontEvt; // get member into temporary
    m_frontEvt = e; // deliver the event directly to the front

    if (frontEvt != nullptr) { // was the queue NOT empty?
        QEQueueCtr tail = m_tail; // get member into temporary
        ++tail;
        if (tail == m_end) { // need to wrap the tail?
            tail = 0U; // wrap around
        }
        m_tail = tail; // update the member original
        m_ring[tail] = frontEvt;
    }

    QF_CRIT_EXIT();
}

//............................................................................
QEvt const * QEQueue::get(std::uint_fast8_t const qsId) noexcept {
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QEvt const * const e = m_frontEvt; // always remove evt from the front

    if (e != nullptr) { // was the queue not empty?
        QEQueueCtr nFree = m_nFree; // get member into temporary

        ++nFree; // one more free event in the queue
        m_nFree = nFree; // update the # free

        if (nFree <= m_end) { // any events in the ring buffer?
            // remove event from the tail
            QEQueueCtr tail = m_tail; // get member into temporary

            QEvt const * const frontEvt = m_ring[tail];

            // the queue must have at least one event (at the front)
            Q_ASSERT_INCRIT(350, frontEvt != nullptr);

            QS_BEGIN_PRE(QS_QF_EQUEUE_GET, qsId)
                QS_TIME_PRE();      // timestamp
                QS_SIG_PRE(e->sig); // the signal of this event
                QS_OBJ_PRE(this);   // this queue object
                QS_2U8_PRE(e->poolNum_, e->refCtr_);
                QS_EQC_PRE(nFree);  // # free entries
            QS_END_PRE()

            m_frontEvt = frontEvt; // update the member original

            if (tail == 0U) { // need to wrap the tail?
                tail = m_end; // wrap around
            }
            --tail; // advance the tail (counter-clockwise)
            m_tail = tail; // update the member original
        }
        else {
            m_frontEvt = nullptr; // queue becomes empty

            // all entries in the queue must be free (+1 for frontEvt)
            Q_INVARIANT_INCRIT(360, nFree == (m_end + 1U));

            QS_BEGIN_PRE(QS_QF_EQUEUE_GET_LAST, qsId)
                QS_TIME_PRE();      // timestamp
                QS_SIG_PRE(e->sig); // the signal of this event
                QS_OBJ_PRE(this);   // this queue object
                QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_END_PRE()
        }
    }

    QF_CRIT_EXIT();

    return e;
}

//............................................................................
std::uint16_t QEQueue::getUse() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    std::uint16_t nUse = 0U;
    if (m_frontEvt != nullptr) { // queue not empty?
        nUse = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(m_end) + 1U
            - static_cast<std::uint16_t>(m_nFree));
    }
    return nUse;
}
//............................................................................
std::uint16_t QEQueue::getFree() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return m_nFree;
}
//............................................................................
std::uint16_t QEQueue::getMin() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return m_nMin;
}
//............................................................................
bool QEQueue::isEmpty() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return m_frontEvt == nullptr;
}
//............................................................................
QEvt const *QEQueue::peekFront() const & {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return m_frontEvt;
}

} // namespace QP
