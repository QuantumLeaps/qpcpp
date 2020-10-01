/// @file
/// @brief QF/C++ stub for QUTEST unit testing
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-21
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
/// along with this program. If not, see <www.gnu.org/licenses/>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

// only build when Q_UTEST is defined
#ifdef Q_UTEST

#define QP_IMPL         // this is QP implementation
#include "qf_port.hpp"  // QF port
#include "qf_pkg.hpp"   // QF package-scope internal interface
#include "qassert.h"    // QP embedded systems-friendly assertions
#include "qs_port.hpp"  // QS port
#include "qs_pkg.hpp"   // QS package-scope internal interface

namespace QP {

Q_DEFINE_THIS_MODULE("qutest")

// Global objects ============================================================
std::uint8_t volatile QF_intNest;

// QF functions ==============================================================
void QF::init(void) {
    /// Clear the internal QF variables, so that the framework can start
    /// correctly even if the startup code fails to clear the uninitialized
    /// data (as is required by the C++ Standard).
    QF_maxPool_      = 0U;
    QF_subscrList_   = nullptr;
    QF_maxPubSignal_ = 0;
    QF_intNest       = 0U;

    bzero(&active_[0], sizeof(active_));
    bzero(&QS::rxPriv_.readySet, sizeof(QS::rxPriv_.readySet));
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

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    QS::onTestLoop(); // run the unit test
    QS::onCleanup();  // application cleanup
    return 0;         // return no error
}

//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    static_cast<void>(stkSto);  // unused parameter
    static_cast<void>(stkSize); // unused parameter

    // priority must be in range
    Q_REQUIRE_ID(200, (0U < prio) && (prio <= QF_MAX_ACTIVE));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = static_cast<std::uint8_t>(prio); // set the QF prio of this AO

    QF::add_(this); // make QF aware of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    //QS_FLUSH(); // flush the trace buffer to the host
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
  : QActive(nullptr)
{}
//............................................................................
void QActiveDummy::start(std::uint_fast8_t const prio,
        QEvt const * * const qSto, std::uint_fast16_t const qLen,
        void * const stkSto, std::uint_fast16_t const stkSize,
        void const * const par)
{
    // No special preconditions for checking parameters to allow starting
    // dummy AOs the exact same way as the real counterparts.
    static_cast<void>(qSto);    // unusuded parameter
    static_cast<void>(qLen);    // unusuded parameter
    static_cast<void>(stkSto);  // unusuded parameter
    static_cast<void>(stkSize); // unusuded parameter

    m_prio = static_cast<std::uint8_t>(prio); // set the QF prio of this AO

    QF::add_(this); // make QF aware of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    //QS_FLUSH(); // flush the trace buffer to the host
}
//............................................................................
void QActiveDummy::init(void const * const e,
                        std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(e); // unused paramter
    static_cast<void>(qs_id); // unused paramter (if Q_SPY not defined)

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_STATE_INIT, qs_id)
        QS_OBJ_PRE_(this);        // this state machine object
        QS_FUN_PRE_(m_state.fun); // the source state
        QS_FUN_PRE_(m_temp.fun);  // the target of the initial transition
    QS_END_PRE_()
}
//............................................................................
void QActiveDummy::dispatch(QEvt const * const e,
                            std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(e); // unused paramter
    static_cast<void>(qs_id); // unused paramter (if Q_SPY not defined)

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_DISPATCH, qs_id)
        QS_TIME_PRE_();           // time stamp
        QS_SIG_PRE_(e->sig);      // the signal of the event
        QS_OBJ_PRE_(this);        // this state machine object
        QS_FUN_PRE_(m_state.fun); // the current state
    QS_END_PRE_()
}
//............................................................................
bool QActiveDummy::post_(QEvt const * const e,
                         std::uint_fast16_t const margin,
                         void const * const sender) noexcept
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
    QF_CRIT_E_();

    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    std::uint_fast8_t rec =
        (status ? static_cast<std::uint8_t>(QS_QF_ACTIVE_POST)
                : static_cast<std::uint8_t>(QS_QF_ACTIVE_POST_ATTEMPT));
    QS_BEGIN_NOCRIT_PRE_(rec, m_prio)
        QS_TIME_PRE_();      // timestamp
        QS_OBJ_PRE_(sender); // the sender object
        QS_SIG_PRE_(e->sig); // the signal of the event
        QS_OBJ_PRE_(this);   // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_PRE_(0U);     // number of free entries
        QS_EQC_PRE_(margin); // margin requested
    QS_END_NOCRIT_PRE_()

    // callback to examine the posted event under the same conditions
    // as producing the #QS_QF_ACTIVE_POST trace record, which are:
    // the local filter for this AO ('me->prio') is set
    //
    if ((QS::priv_.locFilter[m_prio >> 3U]
         & (1U << (m_prio & 7U))) != 0U)
    {
        QS::onTestPost(sender, this, e, status);
    }

    QF_CRIT_X_();

    // recycle the event immediately, because it was not really posted
    QF::gc(e);

    return status;
}
//............................................................................
void QActiveDummy::postLIFO(QEvt const * const e) noexcept {
    QS_TEST_PROBE_DEF(&QActive::postLIFO)

    // test-probe#1 for faking queue overflow
    QS_TEST_PROBE_ID(1,
        // fake assertion Mod=qf_actq,Loc=210
        Q_onAssert("qf_actq", 210);
    )

    QF_CRIT_STAT_
    QF_CRIT_E_();

    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE_();      // timestamp
        QS_SIG_PRE_(e->sig); // the signal of this event
        QS_OBJ_PRE_(this);   // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_PRE_(0U); // number of free entries
        QS_EQC_PRE_(0U); // min number of free entries
    QS_END_NOCRIT_PRE_()

    // callback to examine the posted event under the same conditions
    // as producing the #QS_QF_ACTIVE_POST trace record, which are:
    // the local filter for this AO ('me->prio') is set
    //
    if ((QS::priv_.locFilter[m_prio >> 3U]
         & (1U << (m_prio & 7U))) != 0U)
    {
        QS::onTestPost(nullptr, this, e, true);
    }

    QF_CRIT_X_();

    // recycle the event immediately, because it was not really posted
    QF::gc(e);
}

//****************************************************************************
void QS::processTestEvts_(void) {
    QS_TEST_PROBE_DEF(&QS::processTestEvts_)

    // return immediately (do nothing) for Test Probe != 0
    QS_TEST_PROBE(return;)

    while (rxPriv_.readySet.notEmpty()) {
        std::uint_fast8_t const p = rxPriv_.readySet.findMax();
        QActive * const a = QF::active_[p];

        // perform the run-to-completion (RTC) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and the "Vanialla" kernel asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QEvt const * const e = a->get_();
        a->dispatch(e, a->m_prio);
        QF::gc(e);

        if (a->m_eQueue.isEmpty()) { // empty queue?
            rxPriv_.readySet.rmove(p);
        }
    }
}

//............................................................................
// The testing version of system tick processing performs as follows:
// 1. If the Current Time Event (TE) Object is defined and the TE is armed,
//    the TE is disarmed (if one-shot) and then posted to the recipient AO.
// 2. The linked-list of all armed Time Events is updated.
//
void QS::tickX_(std::uint_fast8_t const tickRate,
                void const * const sender) noexcept
{
    QTimeEvt *t;
    QActive *act;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    QTimeEvt *prev = &QF::timeEvtHead_[tickRate];

    QS_BEGIN_NOCRIT_PRE_(QS_QF_TICK, 0U)
        ++prev->m_ctr;
        QS_TEC_PRE_(prev->m_ctr); // tick ctr
        QS_U8_PRE_(tickRate);     // tick rate
    QS_END_NOCRIT_PRE_()

    // is current Time Event object provided?
    t = static_cast<QTimeEvt *>(QS::rxPriv_.currObj[QS::TE_OBJ]);
    if (t != nullptr) {

        // the time event must be armed
        Q_ASSERT_ID(810, t->m_ctr != 0U);

        act = static_cast<QActive *>(t->m_act); // temp. for volatile

        // the recipient AO must be provided
        Q_ASSERT_ID(820, act != nullptr);

        // periodic time evt?
        if (t->m_interval != 0U) {
            t->m_ctr = t->m_interval; // rearm the time event
        }
        else { // one-shot time event: automatically disarm
            t->m_ctr = 0U; // auto-disarm
            // mark as unlinked
            t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);

            QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_AUTO_DISARM, act->m_prio)
                QS_OBJ_PRE_(t);       // this time event object
                QS_OBJ_PRE_(act);     // the target AO
                QS_U8_PRE_(tickRate); // tick rate
            QS_END_NOCRIT_PRE_()
        }

        QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_POST, act->m_prio)
            QS_TIME_PRE_();           // timestamp
            QS_OBJ_PRE_(t);           // the time event object
            QS_SIG_PRE_(t->sig);      // signal of this time event
            QS_OBJ_PRE_(act);         // the target AO
            QS_U8_PRE_(tickRate);     // tick rate
        QS_END_NOCRIT_PRE_()

        QF_CRIT_X_(); // exit crit. section before posting

        // asserts if queue overflows
        static_cast<void>(act->POST(t, sender));

        QF_CRIT_E_();
    }

    // update the linked list of time events
    for (;;) {
        t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == nullptr) {

            // any new time events armed since the last run of QF::tickX_()?
            if (QF::timeEvtHead_[tickRate].m_act != nullptr) {

                // sanity check
                Q_ASSERT_CRIT_(830, prev != nullptr);
                prev->m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
                QF::timeEvtHead_[tickRate].m_act = nullptr;
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
            prev = t; // advance to this time event
            QF_CRIT_X_(); // exit crit. section to reduce latency

            // prevent merging critical sections, see NOTE1 below
            QF_CRIT_EXIT_NOP();
        }
        QF_CRIT_E_(); // re-enter crit. section to continue
    }

    QF_CRIT_X_();
}

} // namespace QP

//****************************************************************************
extern "C" {

Q_NORETURN Q_onAssert(char_t const * const module, int_t const location) {
    QS_BEGIN_NOCRIT_PRE_(QP::QS_ASSERT_FAIL, 0U)
        QS_TIME_PRE_();
        QS_U16_PRE_(location);
        QS_STR_PRE_((module != nullptr) ? module : "?");
    QS_END_NOCRIT_PRE_()
    QP::QS::onFlush(); // flush the assertion record to the host
    QP::QS::onTestLoop(); // loop to wait for commands (typically reset)
    QP::QS::onReset(); // in case the QUTEST loop ever returns, reset manually
    for (;;) { // onReset() should not return, but to ensure no-return...
    }
}

} // extern "C"

#endif // Q_UTEST

