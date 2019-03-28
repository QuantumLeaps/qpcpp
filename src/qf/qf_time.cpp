/// @file
/// @brief QF/C++ time events and time management services
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.5.0
/// Last updated on  2019-03-09
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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

Q_DEFINE_THIS_MODULE("qf_time")

// Package-scope objects *****************************************************
QTimeEvt QF::timeEvtHead_[QF_MAX_TICK_RATE]; // heads of time event lists

//****************************************************************************
/// @description
/// This function must be called periodically from a time-tick ISR or from
/// a task so that QF can manage the timeout events assigned to the given
/// system clock tick rate.
///
/// @param[in] tickRate  system clock tick rate serviced in this call [1..15].
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
#ifndef Q_SPY
void QF::tickX_(uint_fast8_t const tickRate)
#else
void QF::tickX_(uint_fast8_t const tickRate, void const * const sender)
#endif
{
    QTimeEvt *prev = &timeEvtHead_[tickRate];
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_TICK, static_cast<void*>(0), static_cast<void*>(0))
        QS_TEC_(static_cast<QTimeEvtCtr>(++prev->m_ctr)); // tick ctr
        QS_U8_(static_cast<uint8_t>(tickRate));           // tick rate
    QS_END_NOCRIT_()

    // scan the linked-list of time events at this rate...
    for (;;) {
        QTimeEvt *t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == static_cast<QTimeEvt *>(0)) {

            // any new time events armed since the last run of QF::tickX_()?
            if (timeEvtHead_[tickRate].m_act != static_cast<void *>(0)) {

                // sanity check
                Q_ASSERT_CRIT_(110, prev != static_cast<QTimeEvt *>(0));
                prev->m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = static_cast<void *>(0);
                t = prev->m_next; // switch to the new list
            }
            else {
                break; // all currently armed time evts. processed
            }
        }

        // time event scheduled for removal?
        if (t->m_ctr == static_cast<QTimeEvtCtr>(0)) {
            prev->m_next = t->m_next;
            // mark time event 't' as NOT linked
            t->refCtr_ &= static_cast<uint8_t>(
                              ~static_cast<uint8_t>(TE_IS_LINKED));
            // do NOT advance the prev pointer
            QF_CRIT_EXIT_(); // exit crit. section to reduce latency

            // prevent merging critical sections, see NOTE1 below
            QF_CRIT_EXIT_NOP();
        }
        else {
            --t->m_ctr;

            // is time evt about to expire?
            if (t->m_ctr == static_cast<QTimeEvtCtr>(0)) {
                QActive *act = t->toActive(); // temporary for volatile

                // periodic time evt?
                if (t->m_interval != static_cast<QTimeEvtCtr>(0)) {
                    t->m_ctr = t->m_interval; // rearm the time event
                    prev = t; // advance to this time event
                }
                // one-shot time event: automatically disarm
                else {
                    prev->m_next = t->m_next;

                    // mark time event 't' as NOT linked
                    t->refCtr_ &= static_cast<uint8_t>(
                                      ~static_cast<uint8_t>(TE_IS_LINKED));
                    // do NOT advance the prev pointer

                    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_AUTO_DISARM,
                                     QS::priv_.locFilter[QS::TE_OBJ], t)
                        QS_OBJ_(t);        // this time event object
                        QS_OBJ_(act);      // the target AO
                        QS_U8_(static_cast<uint8_t>(tickRate)); // tick rate
                    QS_END_NOCRIT_()
                }

                QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_POST,
                                 QS::priv_.locFilter[QS::TE_OBJ], t)
                    QS_TIME_();            // timestamp
                    QS_OBJ_(t);            // the time event object
                    QS_SIG_(t->sig);       // signal of this time event
                    QS_OBJ_(act);          // the target AO
                    QS_U8_(static_cast<uint8_t>(tickRate)); // tick rate
                QS_END_NOCRIT_()

                QF_CRIT_EXIT_(); // exit crit. section before posting

                (void)act->POST(t, sender); // asserts if queue overflows
            }
            else {
                prev = t; // advance to this time event
                QF_CRIT_EXIT_(); // exit crit. section to reduce latency

                // prevent merging critical sections, see NOTE1 below
                QF_CRIT_EXIT_NOP();
            }
        }
        QF_CRIT_ENTRY_(); // re-enter crit. section to continue
    }
    QF_CRIT_EXIT_();
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
bool QF::noTimeEvtsActiveX(uint_fast8_t const tickRate) {
    bool inactive;
    if (timeEvtHead_[tickRate].m_next != static_cast<QTimeEvt *>(0)) {
        inactive = false;
    }
    else if (timeEvtHead_[tickRate].m_act != static_cast<void *>(0)) {
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
    enum_t const sgnl, uint_fast8_t const tickRate)
    :
#ifdef Q_EVT_CTOR
    QEvt(static_cast<QSignal>(sgnl)),
#endif
    m_next(static_cast<QTimeEvt *>(0)),
    m_act(act),
    m_ctr(static_cast<QTimeEvtCtr>(0)),
    m_interval(static_cast<QTimeEvtCtr>(0))
{
    /// @pre The signal must be valid and the tick rate in range
    Q_REQUIRE_ID(300, (sgnl >= Q_USER_SIG)
        && (tickRate < static_cast<uint_fast8_t>(QF_MAX_TICK_RATE)));

#ifndef Q_EVT_CTOR
    sig = static_cast<QSignal>(sgnl); // set QEvt::sig of this time event
#endif

    // Setting the POOL_ID event attribute to zero is correct only for
    // events not allocated from event pools, which must be the case
    // for Time Events.
    //
    poolId_ = static_cast<uint8_t>(0);

    // The refCtr_ attribute is not used in time events, so it is
    // reused to hold the tickRate as well as other information
    //
    refCtr_ = static_cast<uint8_t>(tickRate);
}

//****************************************************************************
/// @note
/// private default ctor for internal use only
///
QTimeEvt::QTimeEvt()
    :
#ifdef Q_EVT_CTOR
    QEvt(static_cast<QSignal>(0)),
#endif // Q_EVT_CTOR

    m_next(static_cast<QTimeEvt *>(0)),
    m_act(static_cast<QActive *>(0)),
    m_ctr(static_cast<QTimeEvtCtr>(0)),
    m_interval(static_cast<QTimeEvtCtr>(0))
{
#ifndef Q_EVT_CTOR
    sig = static_cast<QSignal>(0);
#endif  // Q_EVT_CTOR

    // Setting the POOL_ID event attribute to zero is correct only for
    // events not allocated from event pools, which must be the case
    // for Time Events.
    //
    poolId_ = static_cast<uint8_t>(0); // not from any event pool

    // The refCtr_ attribute is not used in time events, so it is
    // reused to hold the tickRate as well as other information
    //
    refCtr_ = static_cast<uint8_t>(0); // default rate 0
}

//****************************************************************************
/// @description
/// Arms a time event to fire in a specified number of clock ticks and with
/// a specified interval. If the interval is zero, the time event is armed for
/// one shot ('one-shot' time event). The time event gets directly posted
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
void QTimeEvt::armX(QTimeEvtCtr const nTicks, QTimeEvtCtr const interval) {
    uint_fast8_t tickRate = static_cast<uint_fast8_t>(refCtr_)
                            & static_cast<uint_fast8_t>(TE_TICK_RATE);
    QTimeEvtCtr ctr = m_ctr;  // temporary to hold volatile
    QF_CRIT_STAT_

    /// @pre the host AO must be valid, time evnet must be disarmed,
    /// number of clock ticks cannot be zero, and the signal must be valid.
    ///
    Q_REQUIRE_ID(400, (m_act != static_cast<void *>(0))
                 && (ctr == static_cast<QTimeEvtCtr>(0))
                 && (nTicks != static_cast<QTimeEvtCtr>(0))
                 && (tickRate < static_cast<uint_fast8_t>(QF_MAX_TICK_RATE))
                 && (static_cast<enum_t>(sig) >= Q_USER_SIG));
#ifdef Q_NASSERT
    (void)ctr; // avoid compiler warning about unused variable
#endif

    QF_CRIT_ENTRY_();
    m_ctr = nTicks;
    m_interval = interval;

    // is the time event unlinked?
    // NOTE: For the duration of a single clock tick of the specified tick
    // rate a time event can be disarmed and yet still linked into the list,
    // because un-linking is performed exclusively in the QF_tickX() function.
    //
    if ((refCtr_ & static_cast<uint8_t>(TE_IS_LINKED))
         == static_cast<uint8_t>(0))
    {
        refCtr_ |= static_cast<uint8_t>(TE_IS_LINKED);  // mark as linked

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

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_ARM, QS::priv_.locFilter[QS::TE_OBJ], this)
        QS_TIME_();        // timestamp
        QS_OBJ_(this);     // this time event object
        QS_OBJ_(m_act);    // the active object
        QS_TEC_(nTicks);   // the number of ticks
        QS_TEC_(interval); // the interval
        QS_U8_(static_cast<uint8_t>(tickRate)); // tick rate
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
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
bool QTimeEvt::disarm(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    bool wasArmed;

    // is the time event actually armed?
    if (m_ctr != static_cast<QTimeEvtCtr>(0)) {
        wasArmed = true;
        refCtr_ |= static_cast<uint8_t>(TE_WAS_DISARMED);

        QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_DISARM,
                         QS::priv_.locFilter[QS::TE_OBJ], this)
            QS_TIME_();            // timestamp
            QS_OBJ_(this);         // this time event object
            QS_OBJ_(m_act);        // the target AO
            QS_TEC_(m_ctr);        // the number of ticks
            QS_TEC_(m_interval);   // the interval
            QS_U8_(static_cast<uint8_t>(
                       refCtr_& static_cast<uint8_t>(TE_TICK_RATE)));
        QS_END_NOCRIT_()

        m_ctr = static_cast<QTimeEvtCtr>(0); // schedule removal from the list
    }
    else { // the time event was already disarmed automatically
        wasArmed = false;

        QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_DISARM_ATTEMPT,
                         QS::priv_.locFilter[QS::TE_OBJ], this)
            QS_TIME_();            // timestamp
            QS_OBJ_(this);         // this time event object
            QS_OBJ_(m_act);        // the target AO
            QS_U8_(static_cast<uint8_t>( // tick rate
                       refCtr_& static_cast<uint8_t>(TE_TICK_RATE)));
        QS_END_NOCRIT_()

    }
    QF_CRIT_EXIT_();
    return wasArmed;
}

//****************************************************************************
///
/// @description
/// Rearms a time event with a new number of clock ticks. This function can
/// be used to adjust the current period of a periodic time event or to
/// prevent a one-shot time event from expiring (e.g., a watchdog time event).
/// Rearming a periodic timer leaves the interval unchanged and is a convenient
/// method to adjust the phasing of a periodic time event.
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
bool QTimeEvt::rearm(QTimeEvtCtr const nTicks) {
    uint_fast8_t tickRate = static_cast<uint_fast8_t>(refCtr_)
                            & static_cast<uint_fast8_t>(TE_TICK_RATE);
    QF_CRIT_STAT_

    /// @pre AO must be valid, tick rate must be in range, nTicks must not
    /// be zero, and the signal of this time event must be valid
    ///
    Q_REQUIRE_ID(600, (m_act != static_cast<void *>(0))
                 && (tickRate < static_cast<uint_fast8_t>(QF_MAX_TICK_RATE))
                 && (nTicks != static_cast<QTimeEvtCtr>(0))
                 && (static_cast<enum_t>(sig) >= Q_USER_SIG));

    QF_CRIT_ENTRY_();
    bool wasArmed;

    // is the time evt not running?
    if (m_ctr == static_cast<QTimeEvtCtr>(0)) {
        wasArmed = false;

        // is the time event unlinked?
        // NOTE: For a duration of a single clock tick of the specified
        // tick rate a time event can be disarmed and yet still linked into
        // the list, because unlinking is performed exclusively in the
        // QF::tickX() function.
        //
        if ((refCtr_ & static_cast<uint8_t>(TE_IS_LINKED))
            == static_cast<uint8_t>(0))
        {
            refCtr_ |= static_cast<uint8_t>(TE_IS_LINKED); // mark as linked

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

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_REARM,
                     QS::priv_.locFilter[QS::TE_OBJ], this)
        QS_TIME_();          // timestamp
        QS_OBJ_(this);       // this time event object
        QS_OBJ_(m_act);      // the target AO
        QS_TEC_(m_ctr);      // the number of ticks
        QS_TEC_(m_interval); // the interval
        QS_2U8_(static_cast<uint8_t>(tickRate),
                (wasArmed ? static_cast<uint8_t>(1)
                            : static_cast<uint8_t>(0)));
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
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
/// only possible for one-shot time events that have been automatically disarmed
/// upon expiration. In this case the 'false' return means that the time event
/// has already been posted or published and should be expected in the active
/// object's event queue.
///
/// @note
/// This function has a **side effect** of setting the "was disarmed" status,
/// which means that the second and subsequent times this function is called
/// the function will return 'true'.
///
bool QTimeEvt::wasDisarmed(void) {
    uint8_t isDisarmed = (refCtr_ & static_cast<uint8_t>(TE_WAS_DISARMED));
    refCtr_ |= static_cast<uint8_t>(TE_WAS_DISARMED); // set the flag
    return isDisarmed != static_cast<uint8_t>(0);
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
QTimeEvtCtr QTimeEvt::currCtr(void) const {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QTimeEvtCtr ret = m_ctr;
    QF_CRIT_EXIT_();

    return ret;
}

} // namespace QP

