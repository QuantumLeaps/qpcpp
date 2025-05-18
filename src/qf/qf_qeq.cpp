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
  : m_frontEvt(nullptr),
    m_ring(nullptr),
    m_end(0U),
    m_head(0U),
    m_tail(0U),
    m_nFree(0U),
    m_nMin(0U)
{}
//............................................................................
void QEQueue::init(
    QEvt const * * const qSto,
    std::uint_fast16_t const qLen) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#if (QF_EQUEUE_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(100, qLen < 0xFFU);
#endif

    m_frontEvt = nullptr; // no events in the queue
    m_ring     = qSto;
    m_end      = static_cast<QEQueueCtr>(qLen);
    if (qLen > 0U) {
        m_head = 0U;
        m_tail = 0U;
    }
    m_nFree = static_cast<QEQueueCtr>(qLen + 1U); // +1 for frontEvt
    m_nMin      = m_nFree;

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

    Q_REQUIRE_INCRIT(200, e != nullptr);

    QEQueueCtr nFree = m_nFree; // get volatile into temporary

    bool status = (nFree > 0U);
    if (margin == QF::NO_MARGIN) { // no margin requested?
        // queue must not overflow
        Q_ASSERT_INCRIT(240, status);
    }
    else {
        status = (nFree > static_cast<QEQueueCtr>(margin));
    }

    if (status) {
        // is it a mutable event?
        if (e->poolNum_ != 0U) {
            Q_ASSERT_INCRIT(250, e->refCtr_ < (2U * QF_MAX_ACTIVE));
            QEvt_refCtr_inc_(e); // increment the reference counter
        }

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
            QEQueueCtr head = m_head; // get volatile into temporary
            m_ring[head] = e; // insert e into buffer

            if (head == 0U) { // need to wrap the head?
                head = m_end;
            }
            --head; // advance head (counter-clockwise)
            m_head = head; // update the original
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

    Q_REQUIRE_INCRIT(300, e != nullptr);

    QEQueueCtr nFree = m_nFree; // get volatile into temporary

    // must be able to LIFO-post the event
    Q_REQUIRE_INCRIT(330, nFree != 0U);

    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(340, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    --nFree; // one free entry just used up
    m_nFree = nFree; // update the original
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

    QEvt const * const frontEvt = m_frontEvt; // read into temporary
    m_frontEvt = e; // deliver the event directly to the front

    if (frontEvt != nullptr) { // was the queue NOT empty?
        QEQueueCtr tail = m_tail; // get volatile into temporary;
        ++tail;
        if (tail == m_end) { // need to wrap the tail?
            tail = 0U; // wrap around
        }
        m_tail = tail;
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
        QEQueueCtr nFree = m_nFree; // get volatile into temporary

        ++nFree; // one more free event in the queue
        m_nFree = nFree; // update the # free

        // any events in the ring buffer?
        if (nFree <= m_end) {
            // remove event from the tail
            QEQueueCtr tail = m_tail; // get volatile into temporary

            QEvt const * const frontEvt = m_ring[tail];
            Q_ASSERT_INCRIT(450, frontEvt != nullptr);

            QS_BEGIN_PRE(QS_QF_EQUEUE_GET, qsId)
                QS_TIME_PRE();      // timestamp
                QS_SIG_PRE(e->sig); // the signal of this event
                QS_OBJ_PRE(this);   // this queue object
                QS_2U8_PRE(e->poolNum_, e->refCtr_);
                QS_EQC_PRE(nFree);  // # free entries
            QS_END_PRE()

            m_frontEvt = frontEvt; // update the original

            if (tail == 0U) { // need to wrap the tail?
                tail = m_end;
            }
            --tail; // advance the tail (counter-clockwise)
            m_tail = tail; // update the original
        }
        else {
            m_frontEvt = nullptr; // queue becomes empty

            // all entries in the queue must be free (+1 for frontEvt)
            Q_INVARIANT_INCRIT(440, nFree == (m_end + 1U));

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

} // namespace QP
