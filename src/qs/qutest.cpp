/// @file
/// @brief QF/C++ stub for QUTEST unit testing
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.5.0
/// Last updated on  2019-03-22
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
    QS_FUN_DICTIONARY(&QS::processTestEvts_);

    QS::onTestLoop(); // run the unit test
    QS::onCleanup();  // application cleanup
    return 0;         // return no error
}

//............................................................................
void QActive::start(uint_fast8_t const prio,
                    QEvt const *qSto[], uint_fast16_t const qLen,
                    void * const, uint_fast16_t const,
                    QEvt const * const ie)
{
    // priority must be in range
    Q_REQUIRE_ID(200, (static_cast<uint_fast8_t>(0) < prio)
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = static_cast<uint8_t>(prio); // set the QF prio of this AO

    QF::add_(this); // make QF aware of this AO

    this->init(ie); // take the top-most initial tran. (virtual)
    //QS_FLUSH();     // flush the trace buffer to the host
}
//............................................................................
#ifdef QF_ACTIVE_STOP
void QActive::stop(void) {
    unsubscribeAll(); // unsubscribe from all events
    QF::remove_(this); // remove this object from QF
}
#endif

//****************************************************************************
QActiveDummy::QActiveDummy(void)
  : QActive(static_cast<QStateHandler>(0))
{}
//............................................................................
void QActiveDummy::start(uint_fast8_t const prio,
        QEvt const * qSto[], uint_fast16_t const qLen,
        void * const stkSto, uint_fast16_t const stkSize,
        QEvt const * const ie)
{
    // priority must be in range
    // queue must NOT be provided
    // stack must NOT be provided
    // initialization event (ie) must NOT be provided
    Q_REQUIRE_ID(300, (static_cast<uint_fast8_t>(0) < prio)
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
        && (qSto == static_cast<QEvt const **>(0))
        && (qLen == static_cast<uint_fast16_t>(0))
        && (stkSto == static_cast<void *>(0))
        && (stkSize == static_cast<uint_fast16_t>(0))
        && (ie == static_cast<QEvt const *>(0)));

    m_prio = static_cast<uint8_t>(prio); // set the QF prio of this AO

    QF::add_(this); // make QF aware of this AO

    this->init(ie); // take the top-most initial tran. (virtual)
    //QS_FLUSH();     // flush the trace buffer to the host
}
//............................................................................
void QActiveDummy::init(QEvt const * const /*e*/) {
    QS_CRIT_STAT_
    QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.locFilter[QS::SM_OBJ], this)
        QS_OBJ_(this);        // this state machine object
        QS_FUN_(m_state.fun); // the source state
        QS_FUN_(m_temp.fun);  // the target of the initial transition
    QS_END_()
}
//............................................................................
void QActiveDummy::dispatch(QEvt const * const e) {
    QS_CRIT_STAT_
    QS_BEGIN_(QS_QEP_DISPATCH, QS::priv_.locFilter[QS::SM_OBJ], this)
        QS_TIME_();           // time stamp
        QS_SIG_(e->sig);      // the signal of the event
        QS_OBJ_(this);        // this state machine object
        QS_FUN_(m_state.fun); // the current state
    QS_END_()
}
//............................................................................
bool QActiveDummy::post_(QEvt const * const e,
                         uint_fast16_t const margin,
                         void const * const sender)
{
    bool status = true;
    QS_TEST_PROBE_DEF(&QActive::post_)

    // test-probe#1 for faking queue overflow
    QS_TEST_PROBE_ID(1,
        status = false;
        if (margin == QF_NO_MARGIN) {
            // fake assertion Mod=qf_actq,Loc=110
            Q_onAssert("qf_actq", 110);
        }
    )

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QS_BEGIN_NOCRIT_((status ? QS_QF_ACTIVE_POST_FIFO
                             : QS_QF_ACTIVE_POST_ATTEMPT),
                     QS::priv_.locFilter[QS::AO_OBJ], this)
        QS_TIME_();      // timestamp
        QS_OBJ_(sender); // the sender object
        QS_SIG_(e->sig); // the signal of the event
        QS_OBJ_(this);   // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_(static_cast<QEQueueCtr>(0)); // number of free entries
        QS_EQC_(static_cast<QEQueueCtr>(margin)); // margin requested
    QS_END_NOCRIT_()

    // callback to examine the posted event under the the same conditions
    // as producing the QS_QF_ACTIVE_POST_FIFO trace record, which are:
    // 1. the local AO-filter is not set (zero) OR
    // 2. the local AO-filter is set to this AO ('me')
    //
    if ((QS::priv_.locFilter[QS::AO_OBJ] == static_cast<QActive *>(0))
        || (QS::priv_.locFilter[QS::AO_OBJ] == this))
    {
        QS::onTestPost(sender, this, e, status);
    }

    QF_CRIT_EXIT_();

    // recycle the event immediately, because it was not really posted
    QF::gc(e);

    return status;
}
//............................................................................
void QActiveDummy::postLIFO(QEvt const * const e) {
    QS_TEST_PROBE_DEF(&QActive::postLIFO)

    // test-probe#1 for faking queue overflow
    QS_TEST_PROBE_ID(1,
        // fake assertion Mod=qf_actq,Loc=210
        Q_onAssert("qf_actq", 210);
        return;
    )

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    // is it a dynamic event?
    if (e->poolId_ != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO,
                     QS::priv_.locFilter[QS::AO_OBJ], this)
        QS_TIME_();      // timestamp
        QS_SIG_(e->sig); // the signal of this event
        QS_OBJ_(this);   // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_(static_cast<QEQueueCtr>(0)); // number of free entries
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min number of free entries
    QS_END_NOCRIT_()

    // callback to examine the posted event under the the same conditions
    // as producing the QS_QF_ACTIVE_POST_ATTEMPT trace record, which are:
    // 1. the local AO-filter is not set (zero) OR
    // 2. the local AO-filter is set to this AO ('me')
    //
    if ((QS::priv_.locFilter[QS::AO_OBJ] == static_cast<QActive *>(0))
        || (QS::priv_.locFilter[QS::AO_OBJ] == this))
    {
        QS::onTestPost(static_cast<QActive *>(0), this, e, true);
    }

    QF_CRIT_EXIT_();

    // recycle the event immediately, because it was not really posted
    QF::gc(e);
}

//****************************************************************************
void QS::processTestEvts_(void) {
    QS_TEST_PROBE_DEF(&QS::processTestEvts_)

    // return immediately (do nothing) for Test Probe != 0
    QS_TEST_PROBE(return;)

    while (rxPriv_.readySet.notEmpty()) {
        uint_fast8_t p = rxPriv_.readySet.findMax();
        QActive *a = QF::active_[p];

        // perform the run-to-completion (RTC) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and the "Vanialla" kernel asserts it.
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

//............................................................................
// The testing version of system tick processing performs as follows:
// 1. If the Current Time Event (TE) Object is defined and the TE is armed,
//    the TE is disarmed (if one-shot) and then posted to the recipient AO.
// 2. The linked-list of all armed Time Events is updated.
//
void QS::tickX_(uint_fast8_t const tickRate, void const * const sender) {
    QTimeEvt *t;
    QActive *act;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QTimeEvt *prev = &QF::timeEvtHead_[tickRate];

    QS_BEGIN_NOCRIT_(QS_QF_TICK, static_cast<void*>(0), static_cast<void*>(0))
        QS_TEC_(static_cast<QTimeEvtCtr>(++prev->m_ctr)); // tick ctr
        QS_U8_(static_cast<uint8_t>(tickRate));           // tick rate
    QS_END_NOCRIT_()

    // is current Time Event object provided?
    t = static_cast<QTimeEvt *>(QS::rxPriv_.currObj[QS::TE_OBJ]);
    if (t != static_cast<void *>(0)) {

        // the time event must be armed
        Q_ASSERT_ID(810, t->m_ctr != static_cast<QTimeEvtCtr>(0));

        act = (QActive *)t->m_act; // temp. for volatile

        // the recipient AO must be provided
        Q_ASSERT_ID(820, act != static_cast<QActive *>(0));

        // periodic time evt?
        if (t->m_interval != static_cast<QTimeEvtCtr>(0)) {
            t->m_ctr = t->m_interval; // rearm the time event
        }
        else { // one-shot time event: automatically disarm
            t->m_ctr = static_cast<QTimeEvtCtr>(0); // auto-disarm
            // mark as unlinked
            t->refCtr_ &=
                static_cast<uint8_t>(~static_cast<uint8_t>(TE_IS_LINKED));

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

        QF_CRIT_ENTRY_();
    }

    // update the linked list of time events
    for (;;) {
        t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == static_cast<QTimeEvt *>(0)) {

            // any new time events armed since the last run of QF::tickX_()?
            if (QF::timeEvtHead_[tickRate].m_act != static_cast<void *>(0)) {

                // sanity check
                Q_ASSERT_CRIT_(830, prev != static_cast<QTimeEvt *>(0));
                prev->m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
                QF::timeEvtHead_[tickRate].m_act = static_cast<void *>(0);
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
            prev = t; // advance to this time event
            QF_CRIT_EXIT_(); // exit crit. section to reduce latency

            // prevent merging critical sections, see NOTE1 below
            QF_CRIT_EXIT_NOP();
        }
        QF_CRIT_ENTRY_(); // re-enter crit. section to continue
    }

    QF_CRIT_EXIT_();
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

