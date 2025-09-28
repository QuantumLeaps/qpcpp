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
#if (QF_MAX_TICK_RATE > 0U)

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_time")
} // unnamed namespace

namespace QP {

//............................................................................
std::array<QTimeEvt, QF_MAX_TICK_RATE> QTimeEvt_head_;

//............................................................................
QTimeEvt::QTimeEvt(
    QActive * const act,
    QSignal const sig,
    std::uint_fast8_t const tickRate) noexcept
 :  QEvt(sig),        // signal for this time event
    m_next(nullptr),  // empty next link
    m_act(act),       // associated AO
    m_ctr(0U),        // time event disarmed
    m_interval(0U),   // not periodic
    m_tickRate(static_cast<std::uint8_t>(tickRate)), // associated tickRate
    m_flags(0U)       // not armed
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the signal must be != 0, but other reserved signals are allowed
    Q_REQUIRE_INCRIT(300, sig != 0U);

    // the tick rate must be in the configured range
    Q_REQUIRE_INCRIT(310, tickRate < QF_MAX_TICK_RATE);
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

    // nTicks and interval parameters must fit in the configured dynamic range
#if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(400, nTicks   < 0xFFU);
    Q_REQUIRE_INCRIT(410, interval < 0xFFU);
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(400, nTicks   < 0xFFFFU);
    Q_REQUIRE_INCRIT(410, interval < 0xFFFFU);
#endif

#ifndef Q_UNSAFE
    QTimeEvtCtr const ctr = m_ctr;
#endif

    std::uint8_t const tickRate = m_tickRate;

    // nTicks must be != 0 for arming a time event
    Q_REQUIRE_INCRIT(440, nTicks != 0U);

    // the time event must not be already armed
    Q_REQUIRE_INCRIT(450, ctr == 0U);

    // the AO associated with this time event must be valid
    Q_REQUIRE_INCRIT(460, m_act != nullptr);

    // the tick rate of this time event must be in range
    Q_REQUIRE_INCRIT(470, tickRate < QF_MAX_TICK_RATE);

    m_ctr = static_cast<QTimeEvtCtr>(nTicks);
    m_interval = static_cast<QTimeEvtCtr>(interval);

    // is the time event unlinked?
    // NOTE: For the duration of a single clock tick of the specified tick
    // rate a time event can be disarmed and yet still linked into the list
    // because un-linking is performed exclusively in the QTimeEvt::tick().
    if ((m_flags & QTE_FLAG_IS_LINKED) == 0U) {
        // The time event is initially inserted into the separate
        // "freshly armed" list based on QTimeEvt_head_[tickRate].act.
        // Only later, inside QTimeEvt::tick(), the "freshly armed"
        // list is appended to the main list of armed time events based on
        // QTimeEvt_head_[tickRate].next. Again, this is to keep any
        // changes to the main list exclusively inside QTimeEvt::tick().

        m_flags |= QTE_FLAG_IS_LINKED; // mark as linked
        m_next = QTimeEvt_head_[tickRate].toTimeEvt();
        QTimeEvt_head_[tickRate].m_act = this;
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_ARM,
            static_cast<QActive const *>(m_act)->m_prio)
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

    QTimeEvtCtr const ctr = m_ctr; // get member into temporary

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

    // nTicks parameter must fit in the configured dynamic range
#if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFU);
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFFFU);
#endif

    std::uint8_t const tickRate = m_tickRate;
    QTimeEvtCtr const ctr = m_ctr;

    // nTicks must be != 0 for arming a time event
    Q_REQUIRE_INCRIT(610, nTicks != 0U);

    // the AO associated with this time event must be valid
    Q_REQUIRE_INCRIT(620, m_act != nullptr);

    // the tick rate of this time event must be in range
    Q_REQUIRE_INCRIT(630, tickRate < QF_MAX_TICK_RATE);

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
            // The time event is initially inserted into the separate
            // "freshly armed" list based on QTimeEvt_head_[tickRate].act.
            // Only later, inside QTimeEvt::tick(), the "freshly armed"
            // list is appended to the main list of armed time events based on
            // QTimeEvt_head_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QTimeEvt::tick().

            m_flags |= QTE_FLAG_IS_LINKED; // mark as linked
            m_next = QTimeEvt_head_[tickRate].toTimeEvt();
            QTimeEvt_head_[tickRate].m_act = this;
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

    // was this time-event disarmed automatically upon expiration?
    bool const wasDisarmed = (m_flags & QTE_FLAG_WAS_DISARMED) != 0U;

    m_flags |= QTE_FLAG_WAS_DISARMED; // mark as disarmed (SIDE EFFECT!)

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

    // the tick rate of this time event must be in range
    Q_REQUIRE_INCRIT(800, tickRate < QF_MAX_TICK_RATE);

    QTimeEvt *prev = &QTimeEvt_head_[tickRate];

#ifdef Q_SPY
    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        ++prev->m_ctr;
        QS_TEC_PRE(prev->m_ctr); // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()
#endif

    // scan the linked-list of time events at this tick rate...
    for (;;) {
        QTimeEvt *te = prev->m_next; // advance down the time evt. list

        if (te == nullptr) { // end of the list?
            // set 'te' to the the newly-armed linked list
            te = QTimeEvt_head_[tickRate].toTimeEvt();
            if (te == nullptr) { // no newly-armed time events?
                break; // terminate the loop
            }

            prev->m_next = te;
            QTimeEvt_head_[tickRate].m_act = nullptr;
        }

        QTimeEvtCtr ctr = te->m_ctr; // get member into temporary

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
            te->m_ctr = ctr; // update the member original

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

    bool const noActive = (QTimeEvt_head_[tickRate].m_next == nullptr);

    return noActive;
}

//............................................................................
// private default ctor
QTimeEvt::QTimeEvt() noexcept
  : QEvt(0U),          // signal for this time event
    m_next(nullptr),   // time event not linked
    m_act(nullptr),    // not associated with any AO
    m_ctr(0U),         // time event disarmed
    m_interval(0U),    // no periodic operation
    m_tickRate(0U),    // default tick rate
    m_flags(0U)        // not armed
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
        m_flags &= static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);
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

#endif // (QF_MAX_TICK_RATE > 0U)
