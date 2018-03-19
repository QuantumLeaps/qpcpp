/// @file
/// @brief QF/C++ stub for QUTEST unit testing
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.2.0
/// Last updated on  2018-03-16
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) 2002-2018 Quantum Leaps. All rights reserved.
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

// only build when Q_UTEST is defined
#ifdef Q_UTEST

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"       // QF package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#include "qs_port.h"      // include QS port

namespace QP {

Q_DEFINE_THIS_MODULE("qutest")

// Global objects ============================================================
uint8_t volatile QF_intNest;

// QF functions ==============================================================
void QF::init(void) {
    QF_maxPool_      = static_cast<uint_fast8_t>(0);
    QF_subscrList_   = static_cast<QSubscrList *>(0);
    QF_maxPubSignal_ = static_cast<enum_t>(0);
    QF_intNest       = static_cast<uint8_t>(0);

    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));
    bzero(&QS::rxPriv_.readySet,
          static_cast<uint_fast16_t>(sizeof(QS::rxPriv_.readySet)));
}
//............................................................................
void QF::stop(void) {
    QS::onReset();
}
//............................................................................
int_t QF::run(void) {
    // function dictionaries for the standard API
    QS_FUN_DICTIONARY(&QActive::post_);
    QS_FUN_DICTIONARY(&QActive::postLIFO);

    QS::onTestLoop();   // run the unit test
    QS::onCleanup();    // application cleanup
    return 0;           // return no error
}

//............................................................................
void QActive::start(uint_fast8_t const prio,
                    QEvt const *qSto[], uint_fast16_t const qLen,
                    void * const, uint_fast16_t const,
                    QEvt const * const ie)
{
    Q_REQUIRE_ID(500, ((uint_fast8_t)0 < prio) /* priority must be in range */
                 && (prio <= (uint_fast8_t)QF_MAX_ACTIVE));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = static_cast<uint8_t>(prio); // set the QF prio of this AO

    QF::add_(this);            // make QF aware of this AO

    this->init(ie); // take the top-most initial tran. (virtual)
    QS_FLUSH();     // flush the trace buffer to the host
}
//............................................................................
void QActive::stop(void) {
    QF::remove_(this); // remove this active object from the framework
}

//****************************************************************************
QTimeEvtCtr QTimeEvt::ctr(void) const {
    QTimeEvtCtr ret;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    ret = m_ctr;
    QF_CRIT_EXIT_();

    return ret;
}
//............................................................................
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

    // The reference counter attribute is not used in static events,
    // so for the Time Events it is reused to hold the tickRate in the
    // bits [0..6] and the linkedFlag in the MSB (bit [7]). The linkedFlag
    // is 0 for time events unlinked from any list and 1 otherwise.
    //
    refCtr_ = static_cast<uint8_t>(tickRate);
}
//............................................................................
void QTimeEvt::armX(QTimeEvtCtr const nTicks, QTimeEvtCtr const interval) {
    uint_fast8_t tickRate = static_cast<uint_fast8_t>(refCtr_)
                            & static_cast<uint_fast8_t>(0x7F);
    QTimeEvtCtr cntr = m_ctr;  // temporary to hold volatile
    QF_CRIT_STAT_

    /// @pre the host AO must be valid, time evnet must be disarmed,
    /// number of clock ticks cannot be zero, and the signal must be valid.
    ///
    Q_REQUIRE_ID(400, (m_act != static_cast<void *>(0))
                 && (cntr == static_cast<QTimeEvtCtr>(0))
                 && (nTicks != static_cast<QTimeEvtCtr>(0))
                 && (tickRate < static_cast<uint_fast8_t>(QF_MAX_TICK_RATE))
                 && (static_cast<enum_t>(sig) >= Q_USER_SIG));

    QF_CRIT_ENTRY_();
    m_ctr = nTicks;
    m_interval = interval;

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_ARM, QS::priv_.locFilter[QS::TE_OBJ], this)
        QS_TIME_();        // timestamp
        QS_OBJ_(this);     // this time event object
        QS_OBJ_(m_act);    // the active object
        QS_TEC_(nTicks);   // the number of ticks
        QS_TEC_(interval); // the interval
        QS_U8_(static_cast<uint8_t>(tickRate));  // tick rate
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
}
//............................................................................
bool QTimeEvt::disarm(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    bool wasArmed;

    // is the time event actually armed?
    if (m_ctr != static_cast<QTimeEvtCtr>(0)) {
        wasArmed = true;

        QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_DISARM,
                         QS::priv_.locFilter[QS::TE_OBJ], this)
            QS_TIME_();          // timestamp
            QS_OBJ_(this);       // this time event object
            QS_OBJ_(m_act);      // the target AO
            QS_TEC_(m_ctr);      // the number of ticks
            QS_TEC_(m_interval); // the interval
            // tick rate
            QS_U8_(static_cast<uint8_t>(refCtr_& static_cast<uint8_t>(0x7F)));
        QS_END_NOCRIT_()

        m_ctr = static_cast<QTimeEvtCtr>(0); // schedule removal from the list
    }
    else { // the time event was already not running
        wasArmed = false;

        QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_DISARM_ATTEMPT,
                         QS::priv_.locFilter[QS::TE_OBJ], this)
            QS_TIME_();          // timestamp
            QS_OBJ_(this);       // this time event object
            QS_OBJ_(m_act);      // the target AO
            // tick rate
            QS_U8_(static_cast<uint8_t>(refCtr_& static_cast<uint8_t>(0x7F)));
        QS_END_NOCRIT_()
    }
    QF_CRIT_EXIT_();
    return wasArmed;
}
//............................................................................
bool QTimeEvt::rearm(QTimeEvtCtr const nTicks) {
    uint_fast8_t tickRate = static_cast<uint_fast8_t>(refCtr_)
                            & static_cast<uint_fast8_t>(0x7F);
    QF_CRIT_STAT_

    /// @pre AO must be valid, tick rate must be in range, nTicks must not
    /// be zero, and the signal of this time event must be valid
    ///
    Q_REQUIRE_ID(600, (m_act != static_cast<void *>(0))
                 && (tickRate < static_cast<uint_fast8_t>(QF_MAX_TICK_RATE))
                 && (nTicks != static_cast<QTimeEvtCtr>(0))
                 && (static_cast<enum_t>(sig) >= Q_USER_SIG));

    QF_CRIT_ENTRY_();
    bool isArmed;

    // is the time evt not running?
    if (m_ctr == static_cast<QTimeEvtCtr>(0)) {
        isArmed = false;

        // is the time event unlinked?
        // NOTE: For a duration of a single clock tick of the specified
        // tick rate a time event can be disarmed and yet still linked into
        // the list, because unlinking is performed exclusively in the
        // QF::tickX() function.
        //
        if ((refCtr_ & static_cast<uint8_t>(0x80))
            == static_cast<uint8_t>(0))
        {
            refCtr_ |= static_cast<uint8_t>(0x80); // mark as linked
        }
    }
    // the time event is armed
    else {
        isArmed = true;
    }
    m_ctr = nTicks; // re-load the tick counter (shift the phasing)

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_REARM,
                     QS::priv_.locFilter[QS::TE_OBJ], this)
        QS_TIME_();          // timestamp
        QS_OBJ_(this);       // this time event object
        QS_OBJ_(m_act);      // the target AO
        QS_TEC_(m_ctr);      // the number of ticks
        QS_TEC_(m_interval); // the interval
        QS_U8_(static_cast<uint8_t>(tickRate)); // the tick rate
        if (isArmed) {
            QS_U8_(static_cast<uint8_t>(1)); // status: armed
        }
        else {
            QS_U8_(static_cast<uint8_t>(0)); // status: disarmed
        }
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
    return isArmed;
}
//............................................................................
void QF::tickX_(uint_fast8_t const tickRate, void const * const sender) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    if (QS::rxPriv_.currObj[QS::TE_OBJ] != static_cast<void *>(0)) {
        QTimeEvt *t = (QTimeEvt *)QS::rxPriv_.currObj[QS::TE_OBJ];
        QActive *act = (QActive *)t->m_act; // temp. for volatile
        if (t->m_interval == (QTimeEvtCtr)0) { // single-shot TE?
            t->m_ctr = (QTimeEvtCtr)0; // auto-disarm
            // mark as unlinked
            t->refCtr_ &= static_cast<uint8_t>(0x7F);

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
        QF_CRIT_EXIT_();
    }
}
//............................................................................
void QS::processTestEvts_(void) {
    while (rxPriv_.readySet.notEmpty()) {
        uint_fast8_t p = rxPriv_.readySet.findMax();
        QActive *a = QF::active_[p];

        // perform the run-to-completion (RTC) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and The "Vanialla" kernel asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QEvt const *e = a->get_();
        a->dispatch(e);
        QF::gc(e);

        if (a->m_eQueue.isEmpty()) { // empty queue?
            rxPriv_.readySet.remove(p);
        }
    }
}

} // namespace QP

//****************************************************************************
void Q_onAssert(char const * const module, int_t loc) {
    QS_BEGIN_NOCRIT_(QP::QS_ASSERT_FAIL,
                     static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_();
        QS_U16_(static_cast<uint16_t>(loc));
        QS_STR_((module != static_cast<char_t *>(0)) ? module : "?");
    QS_END_NOCRIT_()
    QP::QS::onFlush(); // flush the assertion record to the host
    QP::QS::onTestLoop(); // loop to wait for commands (typically reset)
    QP::QS::onReset(); // in case the QUTEST loop ever returns, reset manually
}

#endif // Q_UTEST

