/// @file
/// @brief QF/C++ time events and time management services
/// @ingroup qf
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

Q_DEFINE_THIS_MODULE("qf_time")

// Package-scope objects *****************************************************
QTimeEvt QF::timeEvtHead_[QF_MAX_TICK_RATE]; // heads of time event lists

#ifdef Q_SPY
//****************************************************************************
/// @description
/// This function must be called periodically from a time-tick ISR or from
/// a task so that QF can manage the timeout events assigned to the given
/// system clock tick rate.
///
/// @param[in] tickRate  system clock tick rate serviced in this call [1..15].
/// @param[in] sender    pointer to a sender object (used in QS only).
///
/// @note
/// this function should be called only via the macro TICK_X()
///
/// @note
/// the calls to QP::QF::tickX_() with different @p tickRate parameter can
/// preempt each other. For example, higher clock tick rates might be
/// serviced from interrupts while others from tasks (active objects).
///
/// @sa
/// QP::QTimeEvt.
///
void QF::tickX_(std::uint_fast8_t const tickRate,
                void const * const sender) noexcept
#else
void QF::tickX_(std::uint_fast8_t const tickRate) noexcept
#endif
{
    QTimeEvt *prev = &timeEvtHead_[tickRate];
    QF_CRIT_STAT_

    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_TICK, 0U)
        ++prev->m_ctr;
        QS_TEC_PRE_(prev->m_ctr); // tick ctr
        QS_U8_PRE_(tickRate);     // tick rate
    QS_END_NOCRIT_PRE_()

    // scan the linked-list of time events at this rate...
    for (;;) {
        QTimeEvt *t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == nullptr) {

            // any new time events armed since the last run of QF::tickX_()?
            if (timeEvtHead_[tickRate].m_act != nullptr) {

                // sanity check
                Q_ASSERT_CRIT_(110, prev != nullptr);
                prev->m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = nullptr;
                t = prev->m_next; // switch to the new list
            }
            else {
                break; // all currently armed time evts. processed
            }
        }

        // time event scheduled for removal?
        if (t->m_ctr == 0U) {
            prev->m_next = t->m_next;
            // mark time event 't' as NOT linked
            t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);
            // do NOT advance the prev pointer
            QF_CRIT_X_(); // exit crit. section to reduce latency

            // prevent merging critical sections, see NOTE1 below
            QF_CRIT_EXIT_NOP();
        }
        else {
            --t->m_ctr;

            // is time evt about to expire?
            if (t->m_ctr == 0U) {
                QActive * const act = t->toActive(); // temporary for volatile

                // periodic time evt?
                if (t->m_interval != 0U) {
                    t->m_ctr = t->m_interval; // rearm the time event
                    prev = t; // advance to this time event
                }
                // one-shot time event: automatically disarm
                else {
                    prev->m_next = t->m_next;

                    // mark time event 't' as NOT linked
                    t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);
                    // do NOT advance the prev pointer

                    QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_AUTO_DISARM,
                                         act->m_prio)
                        QS_OBJ_PRE_(t);       // this time event object
                        QS_OBJ_PRE_(act);     // the target AO
                        QS_U8_PRE_(tickRate); // tick rate
                    QS_END_NOCRIT_PRE_()
                }

                QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_POST, act->m_prio)
                    QS_TIME_PRE_();       // timestamp
                    QS_OBJ_PRE_(t);       // the time event object
                    QS_SIG_PRE_(t->sig);  // signal of this time event
                    QS_OBJ_PRE_(act);     // the target AO
                    QS_U8_PRE_(tickRate); // tick rate
                QS_END_NOCRIT_PRE_()

                QF_CRIT_X_(); // exit crit. section before posting

                // asserts if queue overflows
                static_cast<void>(act->POST(t, sender));
            }
            else {
                prev = t; // advance to this time event
                QF_CRIT_X_(); // exit crit. section to reduce latency

                // prevent merging critical sections, see NOTE1 below
                QF_CRIT_EXIT_NOP();
            }
        }
        QF_CRIT_E_(); // re-enter crit. section to continue
    }
    QF_CRIT_X_();
}

//****************************************************************************
// NOTE1:
// In some QF ports the critical section exit takes effect only on the next
// machine instruction. If this case, the next instruction is another entry
// to a critical section, the critical section won't be really exited, but
// rather the two adjacent critical sections would be merged.
//
// The QF_CRIT_EXIT_NOP() macro contains minimal code required
// to prevent such merging of critical sections in QF ports,
// in which it can occur.


//****************************************************************************
/// @description
/// Find out if any time events are armed at the given clock tick rate.
///
/// @param[in]  tickRate  system clock tick rate to find out about.
///
/// @returns
/// 'true' if no time events are armed at the given tick rate and
/// 'false' otherwise.
///
/// @note
/// This function should be called in critical section.
///
bool QF::noTimeEvtsActiveX(std::uint_fast8_t const tickRate) noexcept {
    bool inactive;
    if (timeEvtHead_[tickRate].m_next != nullptr) {
        inactive = false;
    }
    else if (timeEvtHead_[tickRate].m_act != nullptr) {
        inactive = false;
    }
    else {
        inactive = true;
    }
    return inactive;
}

//****************************************************************************
/// @description
/// When creating a time event, you must commit it to a specific active object
/// @p act, tick rate @p tickRate and event signal @p sgnl. You cannot change
/// these attributes later.
///
/// @param[in] act    pointer to the active object associated with this
///                   time event. The time event will post itself to this AO.
/// @param[in] sgnl   signal to associate with this time event.
/// @param[in] tickRate system tick rate to associate with this time event.
///
QTimeEvt::QTimeEvt(QActive * const act,
    enum_t const sgnl, std::uint_fast8_t const tickRate) noexcept
    :
#ifdef Q_EVT_CTOR
    QEvt(static_cast<QSignal>(sgnl), QEvt::STATIC_EVT),
#else
    QEvt(),
#endif
    m_next(nullptr),
    m_act(act),
    m_ctr(0U),
    m_interval(0U)
{
    /// @pre The signal must be valid and the tick rate in range
    Q_REQUIRE_ID(300, (sgnl >= Q_USER_SIG)
                      && (tickRate < QF_MAX_TICK_RATE));

#ifndef Q_EVT_CTOR
    sig = static_cast<QSignal>(sgnl); // set QEvt::sig of this time event
#endif

    // Setting the POOL_ID event attribute to zero is correct only for
    // events not allocated from event pools, which must be the case
    // for Time Events.
    //
    poolId_ = 0U;

    // The refCtr_ attribute is not used in time events, so it is
    // reused to hold the tickRate as well as other information
    //
    refCtr_ = static_cast<std::uint8_t>(tickRate);
}

//****************************************************************************
/// @note
/// private default ctor for internal use only
///
QTimeEvt::QTimeEvt() noexcept
    :
#ifdef Q_EVT_CTOR
    QEvt(0U, QEvt::STATIC_EVT),
#else
    QEvt(),
#endif // Q_EVT_CTOR
    m_next(nullptr),
    m_act(nullptr),
    m_ctr(0U),
    m_interval(0U)
{
#ifndef Q_EVT_CTOR
    sig = 0U;

    // Setting the POOL_ID event attribute to zero is correct only for
    // events not allocated from event pools, which must be the case
    // for Time Events.
    //
    poolId_ = 0U; // not from any event pool

    // The refCtr_ attribute is not used in time events, so it is
    // reused to hold the tickRate as well as other information
    //
    refCtr_ = 0U; // default rate 0

#endif  // Q_EVT_CTOR

}

//****************************************************************************
/// @description
/// Arms a time event to fire in a specified number of clock ticks and with
/// a specified interval. If the interval is zero, the time event is armed
/// for one shot ('one-shot' time event). The time event gets directly posted
/// (using the FIFO policy) into the event queue of the host active object.
///
/// @param[in] nTicks   number of clock ticks (at the associated rate)
///                     to rearm the time event with.
/// @param[in] interval interval (in clock ticks) for periodic time event.
///
/// @note
/// After posting, a one-shot time event gets automatically disarmed
/// while a periodic time event (interval != 0) is automatically re-armed.
///
/// @note
/// A time event can be disarmed at any time by calling
/// QP::QTimeEvt::disarm(). Also, a time event can be re-armed to fire in a
/// different number of clock ticks by calling the QP::QTimeEvt::rearm()
/// function.
///
/// @usage
/// The following example shows how to arm a one-shot time event from a state
/// machine of an active object:
/// @include qf_state.cpp
///
void QTimeEvt::armX(QTimeEvtCtr const nTicks,
                    QTimeEvtCtr const interval) noexcept
{
    std::uint8_t const tickRate = refCtr_ & TE_TICK_RATE;
    QTimeEvtCtr const ctr = m_ctr;  // temporary to hold volatile
    QF_CRIT_STAT_

    /// @pre the host AO must be valid, time evnet must be disarmed,
    /// number of clock ticks cannot be zero, and the signal must be valid.
    ///
    Q_REQUIRE_ID(400, (m_act != nullptr)
       && (ctr == 0U)
       && (nTicks != 0U)
       && (tickRate < static_cast<std::uint8_t>(QF_MAX_TICK_RATE))
       && (static_cast<enum_t>(sig) >= Q_USER_SIG));
#ifdef Q_NASSERT
    (void)ctr; // avoid compiler warning about unused variable
#endif

    QF_CRIT_E_();
    m_ctr = nTicks;
    m_interval = interval;

    // is the time event unlinked?
    // NOTE: For the duration of a single clock tick of the specified tick
    // rate a time event can be disarmed and yet still linked into the list,
    // because un-linking is performed exclusively in the QF_tickX() function.
    //
    if (static_cast<std::uint_fast8_t>(
           static_cast<std::uint_fast8_t>(refCtr_) & TE_IS_LINKED) == 0U)
    {
        refCtr_ |= TE_IS_LINKED; // mark as linked

        // The time event is initially inserted into the separate
        // "freshly armed" link list based on QF::timeEvtHead_[tickRate].act.
        // Only later, inside the QF::tickX() function, the "freshly armed"
        // list is appended to the main list of armed time events based on
        // QF::timeEvtHead_[tickRate].next. Again, this is to keep any
        // changes to the main list exclusively inside the QF::tickX()
        // function.
        //
        m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
        QF::timeEvtHead_[tickRate].m_act = this;
    }

#ifdef Q_SPY
    std::uint_fast8_t const qs_id = static_cast<QActive *>(m_act)->m_prio;
#endif
    QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_ARM, qs_id)
        QS_TIME_PRE_();        // timestamp
        QS_OBJ_PRE_(this);     // this time event object
        QS_OBJ_PRE_(m_act);    // the active object
        QS_TEC_PRE_(nTicks);   // the number of ticks
        QS_TEC_PRE_(interval); // the interval
        QS_U8_PRE_(tickRate);  // tick rate
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();
}

//****************************************************************************
/// @description
/// Disarm the time event so it can be safely reused.
///
/// @returns
/// 'true' if the time event was truly disarmed, that is, it was running.
/// The return of 'false' means that the time event was not truly disarmed,
/// because it was not running. The 'false' return is only possible for one-
///shot time events that have been automatically disarmed upon expiration.
/// In this case the 'false' return means that the time event has already
/// been posted or published and should be expected in the active object's
/// state machine.
///
/// @note
/// there is no harm in disarming an already disarmed time event
///
bool QTimeEvt::disarm(void) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();
    bool wasArmed;
#ifdef Q_SPY
    std::uint_fast8_t const qs_id = static_cast<QActive *>(m_act)->m_prio;
#endif

    // is the time event actually armed?
    if (m_ctr != 0U) {
        wasArmed = true;
        refCtr_ |= TE_WAS_DISARMED;

        QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_DISARM, qs_id)
            QS_TIME_PRE_();            // timestamp
            QS_OBJ_PRE_(this);         // this time event object
            QS_OBJ_PRE_(m_act);        // the target AO
            QS_TEC_PRE_(m_ctr);        // the number of ticks
            QS_TEC_PRE_(m_interval);   // the interval
            QS_U8_PRE_(refCtr_& TE_TICK_RATE);
        QS_END_NOCRIT_PRE_()

        m_ctr = 0U; // schedule removal from the list
    }
    else { // the time event was already disarmed automatically
        wasArmed = false;
        refCtr_ &= static_cast<std::uint8_t>(~TE_WAS_DISARMED);

        QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_DISARM_ATTEMPT, qs_id)
            QS_TIME_PRE_();            // timestamp
            QS_OBJ_PRE_(this);         // this time event object
            QS_OBJ_PRE_(m_act);        // the target AO
            QS_U8_PRE_(refCtr_& TE_TICK_RATE); // tick rate
        QS_END_NOCRIT_PRE_()

    }
    QF_CRIT_X_();
    return wasArmed;
}

//****************************************************************************
///
/// @description
/// Rearms a time event with a new number of clock ticks. This function can
/// be used to adjust the current period of a periodic time event or to
/// prevent a one-shot time event from expiring (e.g., a watchdog time event).
/// Rearming a periodic timer leaves the interval unchanged and is a
/// convenient method to adjust the phasing of a periodic time event.
///
/// @param[in] nTicks number of clock ticks (at the associated rate)
///                   to rearm the time event with.
///
/// @returns
/// 'true' if the time event was running as it was re-armed. The 'false'
/// return means that the time event was not truly rearmed because it was
/// not running. The 'false' return is only possible for one-shot time events
/// that have been automatically disarmed upon expiration. In this case the
/// 'false' return means that the time event has already been posted or
/// published and should be expected in the active object's state machine.
///
bool QTimeEvt::rearm(QTimeEvtCtr const nTicks) noexcept {
    std::uint8_t const tickRate = refCtr_ & TE_TICK_RATE;
    QF_CRIT_STAT_

    /// @pre AO must be valid, tick rate must be in range, nTicks must not
    /// be zero, and the signal of this time event must be valid
    ///
    Q_REQUIRE_ID(600, (m_act != nullptr)
        && (tickRate < static_cast<std::uint8_t>(QF_MAX_TICK_RATE))
        && (nTicks != 0U)
        && (static_cast<enum_t>(sig) >= Q_USER_SIG));

    QF_CRIT_E_();
    bool wasArmed;

    // is the time evt not running?
    if (m_ctr == 0U) {
        wasArmed = false;

        // is the time event unlinked?
        // NOTE: For a duration of a single clock tick of the specified
        // tick rate a time event can be disarmed and yet still linked into
        // the list, because unlinking is performed exclusively in the
        // QF::tickX() function.
        //
        if ((refCtr_ & TE_IS_LINKED) == 0U) {
            refCtr_ |= TE_IS_LINKED; // mark as linked

            // The time event is initially inserted into the separate
            // "freshly armed" list based on QF_timeEvtHead_[tickRate].act.
            // Only later, inside the QF_tickX() function, the "freshly armed"
            // list is appended to the main list of armed time events based on
            // QF_timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside the QF::tickX()
            // function.
            //
            m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
            QF::timeEvtHead_[tickRate].m_act = this;
        }
    }
    else { // the time event is being disarmed
        wasArmed = true;
    }
    m_ctr = nTicks; // re-load the tick counter (shift the phasing)

#ifdef Q_SPY
    std::uint_fast8_t const qs_id = static_cast<QActive *>(m_act)->m_prio;
#endif
    QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_REARM, qs_id)
        QS_TIME_PRE_();          // timestamp
        QS_OBJ_PRE_(this);       // this time event object
        QS_OBJ_PRE_(m_act);      // the target AO
        QS_TEC_PRE_(m_ctr);      // the number of ticks
        QS_TEC_PRE_(m_interval); // the interval
        QS_2U8_PRE_(tickRate, (wasArmed ? 1U : 0U));
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();
    return wasArmed;
}

//****************************************************************************
///
/// @description
/// Useful for checking whether a one-shot time event was disarmed in the
/// QTimeEvt_disarm() operation.
///
/// @returns
/// 'true' if the time event was truly disarmed in the last QTimeEvt::disarm()
/// operation. The 'false' return means that the time event was not truly
/// disarmed, because it was not running at that time. The 'false' return is
/// only possible for one-shot time events that have been automatically
/// disarmed upon expiration. In this case the 'false' return means that the
/// time event has already been posted or published and should be expected
/// in the active object's event queue.
///
/// @note
/// This function has a **side effect** of setting the "was disarmed" status,
/// which means that the second and subsequent times this function is called
/// the function will return 'true'.
///
bool QTimeEvt::wasDisarmed(void) noexcept {
    std::uint8_t const isDisarmed = refCtr_ & TE_WAS_DISARMED;
    refCtr_ |= TE_WAS_DISARMED; // set the flag
    return isDisarmed != 0U;
}

//****************************************************************************
/// @description
/// Useful for checking how many clock ticks (at the tick rate associated
/// with the time event) remain until the time event expires.
///
/// @returns
/// For an armed time event, the function returns the current value of the
/// down-counter of the given time event. If the time event is not armed,
/// the function returns 0.
///
/// @note
/// The function is thread-safe.
///
QTimeEvtCtr QTimeEvt::currCtr(void) const noexcept {
    QF_CRIT_STAT_

    QF_CRIT_E_();
    QTimeEvtCtr const ret = m_ctr;
    QF_CRIT_X_();

    return ret;
}

} // namespace QP

