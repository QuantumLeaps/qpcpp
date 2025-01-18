//============================================================================
// SafeQP/C++ Real-Time Embedded Framework (RTEF)
// Version 8.0.2
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: LicenseRef-QL-commercial
//
// This software is licensed under the terms of the Quantum Leaps commercial
// licenses. Please contact Quantum Leaps for more information about the
// available licensing options.
//
// RESTRICTIONS
// You may NOT :
// (a) redistribute, encumber, sell, rent, lease, sublicense, or otherwise
//     transfer rights in this software,
// (b) remove or alter any trademark, logo, copyright or other proprietary
//     notices, legends, symbols or labels present in this software,
// (c) plagiarize this software to sidestep the licensing obligations.
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
Q_DEFINE_THIS_MODULE("qf_time")
} // unnamed namespace

namespace QP {

//${QF::QTimeEvt} ............................................................
QTimeEvt QTimeEvt::timeEvtHead_[QF_MAX_TICK_RATE];

//............................................................................
QTimeEvt::QTimeEvt(
    QActive * const act,
    QSignal const sig,
    std::uint_fast8_t const tickRate) noexcept
 :  QEvt(sig),
    m_next(nullptr),
    m_act(act),
    m_ctr(0U),
    m_interval(0U),
    m_tickRate(0U),
    m_flags(0U)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300, (sig != 0U)
        && (tickRate < QF_MAX_TICK_RATE));
    QF_CRIT_EXIT();

    refCtr_ = 0U; // adjust from the QEvt(sig) ctor
}

//............................................................................
void QTimeEvt::armX(
    std::uint32_t const nTicks,
    std::uint32_t const interval) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // dynamic range checks
#if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(400, (nTicks < 0xFFU) && (interval < 0xFFU));
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(400, (nTicks < 0xFFFFU) && (interval < 0xFFFFU));
#endif

    QTimeEvtCtr const ctr = m_ctr;
    std::uint8_t const tickRate = m_tickRate;
#ifdef Q_SPY
    std::uint_fast8_t const qsId =
         static_cast<QActive const *>(m_act)->m_prio;
#endif // def Q_SPY

    Q_REQUIRE_INCRIT(410,
        (nTicks != 0U)
        && (ctr == 0U)
        && (m_act != nullptr)
        && (tickRate < QF_MAX_TICK_RATE));

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(ctr);
#endif // ndef Q_UNSAFE

    m_ctr = static_cast<QTimeEvtCtr>(nTicks);
    m_interval = static_cast<QTimeEvtCtr>(interval);

    // is the time event unlinked?
    // NOTE: For the duration of a single clock tick of the specified tick
    // rate a time event can be disarmed and yet still linked into the list
    // because un-linking is performed exclusively in the QTimeEvt::tick().
    if ((m_flags & QTE_FLAG_IS_LINKED) == 0U) {
        m_flags |= QTE_FLAG_IS_LINKED; // mark as linked

        // The time event is initially inserted into the separate
        // "freshly armed" list based on timeEvtHead_[tickRate].act.
        // Only later, inside QTimeEvt::tick(), the "freshly armed"
        // list is appended to the main list of armed time events based on
        // timeEvtHead_[tickRate].next. Again, this is to keep any
        // changes to the main list exclusively inside QTimeEvt::tick().
        m_next = timeEvtHead_[tickRate].toTimeEvt();
        timeEvtHead_[tickRate].m_act = this;
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_ARM, qsId)
        QS_TIME_PRE();        // timestamp
        QS_OBJ_PRE(this);     // this time event object
        QS_OBJ_PRE(m_act);    // the active object
        QS_TEC_PRE(nTicks);   // the # ticks
        QS_TEC_PRE(interval); // the interval
        QS_U8_PRE(tickRate);  // tick rate
    QS_END_PRE()

    QF_CRIT_EXIT();
}

//............................................................................
bool QTimeEvt::disarm() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QTimeEvtCtr const ctr = m_ctr;

#ifdef Q_SPY
    std::uint_fast8_t const qsId = static_cast<QActive *>(m_act)->m_prio;
#endif

    // was the time event actually armed?
    bool wasArmed = false;
    if (ctr != 0U) {
        wasArmed = true;
        m_flags |= QTE_FLAG_WAS_DISARMED;
        m_ctr = 0U; // schedule removal from the list

        QS_BEGIN_PRE(QS_QF_TIMEEVT_DISARM, qsId)
            QS_TIME_PRE();            // timestamp
            QS_OBJ_PRE(this);         // this time event object
            QS_OBJ_PRE(m_act);        // the target AO
            QS_TEC_PRE(ctr);          // the # ticks
            QS_TEC_PRE(m_interval);   // the interval
            QS_U8_PRE(m_tickRate);    // tick rate
        QS_END_PRE()
    }
    else { // the time event was already disarmed automatically
        m_flags &= static_cast<std::uint8_t>(~QTE_FLAG_WAS_DISARMED);

        QS_BEGIN_PRE(QS_QF_TIMEEVT_DISARM_ATTEMPT, qsId)
            QS_TIME_PRE();            // timestamp
            QS_OBJ_PRE(this);         // this time event object
            QS_OBJ_PRE(m_act);        // the target AO
            QS_U8_PRE(m_tickRate);    // tick rate
        QS_END_PRE()
    }

    QF_CRIT_EXIT();

    return wasArmed;
}

//............................................................................
bool QTimeEvt::rearm(std::uint32_t const nTicks) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // dynamic range checks
#if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFU);
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFFFU);
#endif

    std::uint8_t const tickRate = m_tickRate;
    QTimeEvtCtr const ctr = m_ctr;

    Q_REQUIRE_INCRIT(610,
        (nTicks != 0U)
        && (m_act != nullptr)
        && (tickRate < QF_MAX_TICK_RATE));

#ifdef Q_SPY
    std::uint_fast8_t const qsId = static_cast<QActive *>(m_act)->m_prio;
#endif

    m_ctr = static_cast<QTimeEvtCtr>(nTicks);

    // was the time evt not running?
    bool wasArmed = false;
    if (ctr == 0U) {
        // NOTE: For the duration of a single clock tick of the specified
        // tick rate a time event can be disarmed and yet still linked into
        // the list, because unlinking is performed exclusively in the
        // QTimeEvt::tick() function.

        // is the time event unlinked?
        if ((m_flags & QTE_FLAG_IS_LINKED) == 0U) {
            m_flags |= QTE_FLAG_IS_LINKED; // mark as linked

            // The time event is initially inserted into the separate
            // "freshly armed" list based on timeEvtHead_[tickRate].act.
            // Only later, inside QTimeEvt::tick(), the "freshly armed"
            // list is appended to the main list of armed time events based on
            // timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QTimeEvt::tick().
            m_next = timeEvtHead_[tickRate].toTimeEvt();
            timeEvtHead_[tickRate].m_act = this;
        }
    }
    else { // the time event was armed
        wasArmed = true;
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_REARM, qsId)
        QS_TIME_PRE();            // timestamp
        QS_OBJ_PRE(this);         // this time event object
        QS_OBJ_PRE(m_act);        // the target AO
        QS_TEC_PRE(nTicks);       // the # ticks
        QS_TEC_PRE(m_interval);   // the interval
        QS_2U8_PRE(tickRate, (wasArmed ? 1U : 0U));
    QS_END_PRE()

    QF_CRIT_EXIT();

    return wasArmed;
}

//............................................................................
bool QTimeEvt::wasDisarmed() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    bool const wasDisarmed = (m_flags & QTE_FLAG_WAS_DISARMED) != 0U;
    m_flags |= QTE_FLAG_WAS_DISARMED;

    QF_CRIT_EXIT();

    return wasDisarmed;
}

//............................................................................
void QTimeEvt::tick(
    std::uint_fast8_t const tickRate,
    void const * const sender) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(800, tickRate < Q_DIM(timeEvtHead_));

    QTimeEvt *prev = &timeEvtHead_[tickRate];

#ifdef Q_SPY
    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        prev->m_ctr = (prev->m_ctr + 1U);
        QS_TEC_PRE(prev->m_ctr); // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()
#endif // def Q_SPY

    // scan the linked-list of time events at this rate...
    while (true) {
        Q_ASSERT_INCRIT(810, prev != nullptr); // sanity check

        QTimeEvt *te = prev->m_next; // advance down the time evt. list

        if (te == nullptr) { // end of the list?
            // NO any new time events armed since the last QTimeEvt_tick_()?
            if (timeEvtHead_[tickRate].m_act == nullptr) {
                break; // terminate the while-loop
            }

            prev->m_next = timeEvtHead_[tickRate].toTimeEvt();
            timeEvtHead_[tickRate].m_act = nullptr;

            te = prev->m_next; // switch to the new list
        }

        // the time event 'te' must be valid
        Q_ASSERT_INCRIT(840, te != nullptr);

        QTimeEvtCtr ctr = te->m_ctr;

        if (ctr == 0U) { // time event scheduled for removal?
            prev->m_next = te->m_next;

            // mark time event 'te' as NOT linked
            te->m_flags &= static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);
            // do NOT advance the prev pointer
            QF_CRIT_EXIT(); // exit crit. section to reduce latency
        }
        else if (ctr == 1U) { // is time event about to expire?
            QActive * const act = te->toActive();
            prev = te->expire_(prev, act, tickRate);

#ifdef QXK_HPP_
            if (te->sig < Q_USER_SIG) {
                QXThread::timeout_(act);
                QF_CRIT_EXIT();
            }
            else {
                QF_CRIT_EXIT(); // exit crit. section before posting

                // act->POST() asserts if the queue overflows
                act->POST(te, sender);
            }
#else // not QXK
            QF_CRIT_EXIT(); // exit crit. section before posting

            // act->POST() asserts if the queue overflows
            act->POST(te, sender);
#endif
        }
        else { // time event keeps timing out
            --ctr; // decrement the tick counter
            te->m_ctr = ctr; // update the original

            prev = te; // advance to this time event
            QF_CRIT_EXIT(); // exit crit. section to reduce latency
        }
        QF_CRIT_ENTRY(); // re-enter crit. section to continue the loop
    }
    QF_CRIT_EXIT();
}

//............................................................................
bool QTimeEvt::noActive(std::uint_fast8_t const tickRate) noexcept {
    // NOTE: this function must be called *inside* critical section
    Q_REQUIRE_INCRIT(900, tickRate < QF_MAX_TICK_RATE);

    bool inactive = false;

    if (timeEvtHead_[tickRate].m_next != nullptr) {
        // empty
    }
    else if (timeEvtHead_[tickRate].m_act != nullptr) {
        // empty
    }
    else {
        inactive = true;
    }

    return inactive;
}

//............................................................................
QTimeEvt::QTimeEvt() noexcept
 :  QEvt(0U),
    m_next(nullptr),
    m_act(nullptr),
    m_ctr(0U),
    m_interval(0U),
    m_tickRate(0U),
    m_flags(0U)
{}

//............................................................................
QTimeEvt *QTimeEvt::expire_(
    QTimeEvt * const prev_link,
    QActive const * const act,
    std::uint_fast8_t const tickRate) noexcept
{
    // NOTE: this helper function is called *inside* critical section
#ifndef Q_SPY
    Q_UNUSED_PAR(act);
    Q_UNUSED_PAR(tickRate);
#endif

    QTimeEvt *prev = prev_link;
    if (m_interval != 0U) { // periodic time evt?
        m_ctr = m_interval; // rearm the time event
        prev = this; // advance to this time event
    }
    else { // one-shot time event: automatically disarm
        m_ctr = 0U;
        prev->m_next = m_next;

        // mark this time event as NOT linked
        m_flags &=
            static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);
        // do NOT advance the prev pointer

        QS_BEGIN_PRE(QS_QF_TIMEEVT_AUTO_DISARM, act->m_prio)
            QS_OBJ_PRE(this);     // this time event object
            QS_OBJ_PRE(act);      // the target AO
            QS_U8_PRE(tickRate);  // tick rate
        QS_END_PRE()
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_POST, act->m_prio)
        QS_TIME_PRE();            // timestamp
        QS_OBJ_PRE(this);         // the time event object
        QS_SIG_PRE(sig);          // signal of this time event
        QS_OBJ_PRE(act);          // the target AO
        QS_U8_PRE(tickRate);      // tick rate
    QS_END_PRE()

    return prev;
}

} // namespace QP
