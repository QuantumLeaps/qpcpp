/// @file
/// @brief QF/C++ Publish-Subscribe services
/// definitions.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-04-29
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
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

Q_DEFINE_THIS_MODULE("qf_ps")

// Package-scope objects *****************************************************
QSubscrList *QF_subscrList_;
enum_t QF_maxSignal_;

//****************************************************************************
/// @description
/// This function initializes the publish-subscribe facilities of QF and must
/// be called exactly once before any subscriptions/publications occur in
/// the application.
///
/// @param[in] subscrSto pointer to the array of subscriber lists
/// @param[in] maxSignal the dimension of the subscriber array and at
///                      the same time the maximum signal that can be
///                      published or subscribed.
///
/// The array of subscriber-lists is indexed by signals and provides a mapping
/// between the signals and subscriber-lists. The subscriber-lists are
/// bitmasks of type QP::QSubscrList, each bit in the bit mask corresponding
/// to the unique priority of an active object. The size of the
/// QP::QSubscrList bitmask depends on the value of the #QF_MAX_ACTIVE macro.
///
/// @note The publish-subscribe facilities are optional, meaning that you
/// might choose not to use publish-subscribe. In that case calling
/// QF::psInit() and using up memory for the subscriber-lists is unnecessary.
///
/// @sa QP::QSubscrList
///
/// @usage
/// The following example shows the typical initialization sequence of QF:
/// @include qf_main.cpp
///
void QF::psInit(QSubscrList * const subscrSto, enum_t const maxSignal) {
    QF_subscrList_ = subscrSto;
    QF_maxSignal_  = maxSignal;

    // zero the subscriber list, so that the framework can start correctly
    // even if the startup code fails to clear the uninitialized data
    // (as is required by the C++ Standard)
    bzero(subscrSto,
             static_cast<uint_fast16_t>(static_cast<uint_fast16_t>(maxSignal)
              * static_cast<uint_fast16_t>(sizeof(QSubscrList))));
}

//****************************************************************************
/// @description
/// This function posts (using the FIFO policy) the event @p e to ALL
/// active objects that have subscribed to the signal @p e->sig.
/// This function is designed to be callable from any part of the system,
/// including ISRs, device drivers, and active objects.
///
/// @note
/// In the general case, event publishing requires multicasting the
/// event to multiple subscribers. This happens in the caller's thread with
/// the scheduler locked to prevent preemption during the multicasting
/// process. (Please note that the interrupts are not locked.)
///
/// @attention
/// This function should be called only via the macro PUBLISH()
///
#ifndef Q_SPY
void QF::publish_(QEvt const * const e) {
#else
void QF::publish_(QEvt const * const e, void const * const sender) {
#endif
    /// @pre the published signal must be within the configured range
    Q_REQUIRE_ID(100, static_cast<enum_t>(e->sig) < QF_maxSignal_);

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_PUBLISH,
        static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_();                      // the timestamp
        QS_OBJ_(sender);                 // the sender object
        QS_SIG_(e->sig);                 // the signal of the event
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
    QS_END_NOCRIT_()

    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter, NOTE01
    }
    QF_CRIT_EXIT_();

#if (QF_MAX_ACTIVE <= 8)
    uint8_t tmp = QF_PTR_AT_(QF_subscrList_, e->sig).m_bits[0];
    while (tmp != static_cast<uint8_t>(0)) {
        uint8_t p = QF_LOG2(tmp);
        tmp &= Q_ROM_BYTE(QF_invPwr2Lkup[p]); // clear the subscriber bit

        // the priority of the AO must be registered with the framework
        Q_ASSERT_ID(110, active_[p] != static_cast<QMActive *>(0));

        // POST() asserts internally if the queue overflows
        (void)active_[p]->POST(e, sender);
    }
#else
    uint8_t i = QF_SUBSCR_LIST_SIZE;

    // go through all bytes in the subscription list
    do {
        --i;
        uint8_t tmp = QF_PTR_AT_(QF_subscrList_, e->sig).m_bits[i];
        while (tmp != static_cast<uint8_t>(0)) {
            uint8_t p = QF_LOG2(tmp);
            tmp &= Q_ROM_BYTE(QF_invPwr2Lkup[p]); // clear the subscriber bit
            // adjust the priority
            p = static_cast<uint8_t>(p + static_cast<uint8_t>(i << 3));

            // the priority level be registered with the framework
            Q_ASSERT(active_[p] != static_cast<QMActive *>(0));

            // POST() asserts internally if the queue overflows
            (void)active_[p]->POST(e, sender);
        }
    } while (i != static_cast<uint8_t>(0));
#endif

    // run the garbage collector
    gc(e);

    // NOTE: QP::QF::publish_() increments the reference counter to prevent
    // premature recycling of the event while the multicasting is still
    // in progress. At the end of the function, the garbage collector step
    // decrements the reference counter and recycles the event if the
    // counter drops to zero. This covers the case when the event was
    // published without any subscribers.
}

//****************************************************************************/
/// @description
/// This function is part of the Publish-Subscribe event delivery mechanism
/// available in QF. Subscribing to an event means that the framework will
/// start posting all published events with a given signal @p sig to the
/// event queue of the active object.
///
/// @param[in] sig event signal to subscribe
///
/// The following example shows how the Table active object subscribes
/// to three signals in the initial transition:
/// @include qf_subscribe.c
///
/// @sa QP::QF::publish_(), QP::QMActive::unsubscribe(), and
/// QP::QMActive::unsubscribeAll()
///
void QMActive::subscribe(enum_t const sig) const {
    uint_fast8_t p = m_prio;
    Q_REQUIRE_ID(300, (Q_USER_SIG <= sig)
              && (sig < QF_maxSignal_)
              && (static_cast<uint_fast8_t>(0) < p)
              && (p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
              && (QF::active_[p] == this));

    uint_fast8_t const i =
         static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_div8Lkup[p]));

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_SUBSCRIBE, QS::priv_.aoObjFilter, this)
        QS_TIME_();    // timestamp
        QS_SIG_(sig);  // the signal of this event
        QS_OBJ_(this); // this active object
    QS_END_NOCRIT_()

    // set the priority bit
    QF_PTR_AT_(QF_subscrList_, sig).m_bits[i] |= Q_ROM_BYTE(QF_pwr2Lkup[p]);
    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// This function is part of the Publish-Subscribe event delivery mechanism
/// available in QF. Un-subscribing from an event means that the framework
/// will stop posting published events with a given signal @p sig to the
/// event queue of the active object.
///
/// @param[in] sig event signal to unsubscribe
///
/// @note Due to the latency of event queues, an active object should NOT
/// assume that a given signal @p sig will never be dispatched to the
/// state machine of the active object after un-subscribing from that signal.
/// The event might be already in the queue, or just about to be posted
/// and the un-subscribe operation will not flush such events.
///
/// @note Un-subscribing from a signal that has never been subscribed in the
/// first place is considered an error and QF will raise an assertion.
///
/// @sa QP::QF::publish_(), QP::QMActive::subscribe(), and
/// QP::QMActive::unsubscribeAll()
void QMActive::unsubscribe(enum_t const sig) const {
    uint_fast8_t p = m_prio;
    Q_REQUIRE_ID(400, (Q_USER_SIG <= sig)
                      && (sig < QF_maxSignal_)
                      && (static_cast<uint_fast8_t>(0) < p)
                      && (p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
                      && (QF::active_[p] == this));

    uint_fast8_t const i =
        static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_div8Lkup[p]));

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_UNSUBSCRIBE, QS::priv_.aoObjFilter, this)
        QS_TIME_();         // timestamp
        QS_SIG_(sig);       // the signal of this event
        QS_OBJ_(this);      // this active object
    QS_END_NOCRIT_()

    // clear the priority bit
    QF_PTR_AT_(QF_subscrList_,sig).m_bits[i] &= Q_ROM_BYTE(QF_invPwr2Lkup[p]);
    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// This function is part of the Publish-Subscribe event delivery mechanism
/// available in QF. Un-subscribing from all events means that the framework
/// will stop posting any published events to the event queue of the active
/// object.
///
/// @note Due to the latency of event queues, an active object should NOT
/// assume that no events will ever be dispatched to the state machine of
/// the active object after un-subscribing from all events.
/// The events might be already in the queue, or just about to be posted
/// and the un-subscribe operation will not flush such events. Also, the
/// alternative event-delivery mechanisms, such as direct event posting or
/// time events, can be still delivered to the event queue of the active
/// object.
///
/// @sa QP::QF::publish_(), QP::QMActive::subscribe(), and
/// QP::QMActive::unsubscribe()
///
void QMActive::unsubscribeAll(void) const {
    uint_fast8_t const p = m_prio;

    Q_REQUIRE_ID(500, (static_cast<uint_fast8_t>(0) < p)
                      && (p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
                      && (QF::active_[p] == this));

    uint_fast8_t const i =
        static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_div8Lkup[p]));

    enum_t sig;
    for (sig = Q_USER_SIG; sig < QF_maxSignal_; ++sig) {
        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();
        if ((QF_PTR_AT_(QF_subscrList_, sig).m_bits[i]
             & Q_ROM_BYTE(QF_pwr2Lkup[p])) != static_cast<uint8_t>(0))
        {

            QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_UNSUBSCRIBE,
                             QS::priv_.aoObjFilter, this)
                QS_TIME_();     // timestamp
                QS_SIG_(sig);   // the signal of this event
                QS_OBJ_(this);  // this active object
            QS_END_NOCRIT_()

            // clear the priority bit
            QF_PTR_AT_(QF_subscrList_, sig).m_bits[i] &=
                Q_ROM_BYTE(QF_invPwr2Lkup[p]);
        }
        QF_CRIT_EXIT_();
    }
}

} // namespace QP
