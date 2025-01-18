//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Version 8.0.2
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
    m_nFree = static_cast<QEQueueCtr>(qLen + 1U); //+1 for frontEvt
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

    QEQueueCtr tmp = m_nFree; // get volatile into temporary

    // can the queue accept the event?
    bool status = ((margin == QF::NO_MARGIN) && (tmp > 0U))
        || (tmp > static_cast<QEQueueCtr>(margin));
    if (status) {
        // is it a mutable event?
        if (e->poolNum_ != 0U) {
            Q_ASSERT_INCRIT(205, e->refCtr_ < (2U * QF_MAX_ACTIVE));
            QEvt_refCtr_inc_(e); // increment the reference counter
        }

        --tmp; // one free entry just used up

        m_nFree = tmp; // update the original
        if (m_nMin > tmp) { // is this the new minimum?
            m_nMin = tmp; // update minimum so far
        }

#ifdef Q_SPY
        QS_BEGIN_PRE(QS_QF_EQUEUE_POST, qsId)
            QS_TIME_PRE();        // timestamp
            QS_SIG_PRE(e->sig);   // the signal of the event
            QS_OBJ_PRE(this);     // this queue object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(tmp);      // # free entries
            QS_EQC_PRE(m_nMin);   // min # free entries
        QS_END_PRE()
#endif // def Q_SPY

        if (m_frontEvt == nullptr) { // is the queue empty?
            m_frontEvt = e; // deliver event directly
        }
        else { // queue was not empty, insert event into the ring-buffer
            tmp = m_head; // get volatile into temporary
            m_ring[tmp] = e; // insert e into buffer

            if (tmp == 0U) { // need to wrap the head?
                tmp = m_end;
            }
            --tmp; // advance head (counter-clockwise)

            m_head = tmp; // update the original
        }
    }
    else { // event cannot be posted
        // dropping events must be acceptable
        Q_ASSERT_INCRIT(230, margin != QF::NO_MARGIN);

#ifdef Q_SPY
        QS_BEGIN_PRE(QS_QF_EQUEUE_POST_ATTEMPT, qsId)
            QS_TIME_PRE();        // timestamp
            QS_SIG_PRE(e->sig);   // the signal of this event
            QS_OBJ_PRE(this);     // this queue object
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(tmp);      // # free entries
            QS_EQC_PRE(margin);   // margin requested
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

    QEQueueCtr tmp = m_nFree; // get volatile into temporary

    // must be able to LIFO-post the event
    Q_REQUIRE_INCRIT(310, tmp != 0U);

    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(305, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    --tmp; // one free entry just used up

    m_nFree = tmp; // update the original
    if (m_nMin > tmp) { // is this the new minimum?
        m_nMin = tmp; // update minimum so far
    }

    QS_BEGIN_PRE(QS_QF_EQUEUE_POST_LIFO, qsId)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this queue object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(tmp);     // # free entries
        QS_EQC_PRE(m_nMin);  // min # free entries
    QS_END_PRE()

    QEvt const * const frontEvt = m_frontEvt; // read into temporary
    m_frontEvt = e; // deliver the event directly to the front

    if (frontEvt != nullptr) { // was the queue NOT empty?
        tmp = m_tail; // get volatile into temporary;
        ++tmp;
        if (tmp == m_end) { // need to wrap the tail?
            tmp = 0U; // wrap around
        }
        m_tail = tmp;
        m_ring[tmp] = frontEvt;
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
        QEQueueCtr tmp = m_nFree; // get volatile into temporary

        ++tmp; // one more free event in the queue

        m_nFree = tmp; // update the # free

        // any events in the ring buffer?
        if (tmp <= m_end) {

            QS_BEGIN_PRE(QS_QF_EQUEUE_GET, qsId)
                QS_TIME_PRE();      // timestamp
                QS_SIG_PRE(e->sig); // the signal of this event
                QS_OBJ_PRE(this);   // this queue object
                QS_2U8_PRE(e->poolNum_, e->refCtr_);
                QS_EQC_PRE(tmp);    // # free entries
            QS_END_PRE()

            tmp = m_tail; // get volatile into temporary
            QEvt const * const frontEvt = m_ring[tmp];

            Q_ASSERT_INCRIT(420, frontEvt != nullptr);

            m_frontEvt = frontEvt; // update the original

            if (tmp == 0U) { // need to wrap the tail?
                tmp = m_end;
            }
            --tmp; // advance the tail (counter-clockwise)
            m_tail = tmp; // update the original
        }
        else {
            m_frontEvt = nullptr; // queue becomes empty

            // all entries in the queue must be free (+1 for frontEvt)
            Q_INVARIANT_INCRIT(440, tmp == (m_end + 1U));

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
