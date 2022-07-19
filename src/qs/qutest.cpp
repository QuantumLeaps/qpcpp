//$file${src::qs::qutest.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qs::qutest.cpp}
//
// This code has been generated by QM 5.2.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2023-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${src::qs::qutest.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @date Last updated on: 2022-06-30
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QUTest unit testing harness + QF/++ stub for QUTest

// only build when Q_UTEST is defined
#ifdef Q_UTEST

#define QP_IMPL         // this is QP implementation
#include "qf_port.hpp"  // QF port
#include "qf_pkg.hpp"   // QF package-scope internal interface
#include "qassert.h"    // QP embedded systems-friendly assertions
#include "qs_port.hpp"  // QS port
#include "qs_pkg.hpp"   // QS package-scope internal interface

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qutest")
} // unnamed namespace

//===========================================================================
// QUTest unit testing harness
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QS::QUTest} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QS {

//${QS::QUTest::QS::testData} ................................................
TestData testData;

//${QS::QUTest::QS::processTestEvts_} ........................................
void processTestEvts_() {
    QS_TEST_PROBE_DEF(&QS::processTestEvts_)

    // return immediately (do nothing) for Test Probe != 0
    QS_TEST_PROBE(return;)

    while (QF::readySet_.notEmpty()) {
        std::uint_fast8_t const p = QF::readySet_.findMax();
        QActive * const a = QActive::active_[p];

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
            QF::readySet_.rmove(p);
        }
    }
}

//${QS::QUTest::QS::test_pause_} .............................................
void test_pause_() {
    beginRec_(static_cast<std::uint_fast8_t>(QP::QS_TEST_PAUSED));
    endRec_();
    onTestLoop();
}

//${QS::QUTest::QS::getTestProbe_} ...........................................
std::uint32_t getTestProbe_(QP::QSpyFunPtr const api) noexcept {
    std::uint32_t data = 0U;
    for (std::uint8_t i = 0U; i < testData.tpNum; ++i) {
        if (testData.tpBuf[i].addr == reinterpret_cast<QSFun>(api)) {
            data = testData.tpBuf[i].data;

            QS_CRIT_STAT_
            QS_CRIT_E_();
            QS::beginRec_(static_cast<std::uint_fast8_t>(QS_TEST_PROBE_GET));
                QS_TIME_PRE_();    // timestamp
                QS_FUN_PRE_(api);  // the calling API
                QS_U32_PRE_(data); // the Test-Probe data
            QS::endRec_();
            QS_CRIT_X_();

            QS_REC_DONE(); // user callback (if defined)

            --testData.tpNum; // one less Test-Probe
            // move all remaining entries in the buffer up by one
            for (std::uint8_t j = i; j < testData.tpNum; ++j) {
                testData.tpBuf[j] = testData.tpBuf[j + 1U];
            }
            break; // we are done (Test-Probe retreived)
        }
    }
    return data;
}

} // namespace QS

//${QS::QUTest::QHsmDummy} ...................................................

//${QS::QUTest::QHsmDummy::QHsmDummy} ........................................
QHsmDummy::QHsmDummy()
: QHsm(nullptr)
{}

//${QS::QUTest::QHsmDummy::init} .............................................
void QHsmDummy::init(
    void const * const e,
    std::uint_fast8_t const qs_id)
{
    Q_UNUSED_PAR(e);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_STATE_INIT, qs_id)
        QS_OBJ_PRE_(this);        // this state machine object
        QS_FUN_PRE_(m_state.fun); // the source state
        QS_FUN_PRE_(m_temp.fun);  // the target of the initial transition
    QS_END_PRE_()
}

//${QS::QUTest::QHsmDummy::init} .............................................
void QHsmDummy::init(std::uint_fast8_t const qs_id) {
    QHsmDummy::init(nullptr, qs_id);
}

//${QS::QUTest::QHsmDummy::dispatch} .........................................
void QHsmDummy::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qs_id)
{
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_DISPATCH, qs_id)
        QS_TIME_PRE_();           // time stamp
        QS_SIG_PRE_(e->sig);      // the signal of the event
        QS_OBJ_PRE_(this);        // this state machine object
        QS_FUN_PRE_(m_state.fun); // the current state
    QS_END_PRE_()
}

//${QS::QUTest::QActiveDummy} ................................................

//${QS::QUTest::QActiveDummy::QActiveDummy} ..................................
QActiveDummy::QActiveDummy()
: QActive(nullptr)
{}

//${QS::QUTest::QActiveDummy::start} .........................................
void QActiveDummy::start(
    std::uint_fast8_t const prio,
    QEvt const * * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    // No special preconditions for checking parameters to allow starting
    // dummy AOs the exact same way as the real counterparts.
    static_cast<void>(qSto);    // unusuded parameter
    static_cast<void>(qLen);    // unusuded parameter
    static_cast<void>(stkSto);  // unusuded parameter
    static_cast<void>(stkSize); // unusuded parameter

    m_prio = static_cast<std::uint8_t>(prio); // set the QF prio of this AO

    register_(); // make QF aware of this AO

    QActiveDummy::init(par, m_prio); // take the top-most initial tran.
    //QS_FLUSH();
}

//${QS::QUTest::QActiveDummy::init} ..........................................
void QActiveDummy::init(
    void const * const e,
    std::uint_fast8_t const qs_id)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qs_id);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_STATE_INIT, m_prio)
        QS_OBJ_PRE_(this);        // this state machine object
        QS_FUN_PRE_(m_state.fun); // the source state
        QS_FUN_PRE_(m_temp.fun);  // the target of the initial transition
    QS_END_PRE_()
}

//${QS::QUTest::QActiveDummy::init} ..........................................
void QActiveDummy::init(std::uint_fast8_t const qs_id) {
    QActiveDummy::init(nullptr, qs_id);
}

//${QS::QUTest::QActiveDummy::dispatch} ......................................
void QActiveDummy::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qs_id)
{
    Q_UNUSED_PAR(qs_id);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_DISPATCH, m_prio)
        QS_TIME_PRE_();           // time stamp
        QS_SIG_PRE_(e->sig);      // the signal of the event
        QS_OBJ_PRE_(this);        // this state machine object
        QS_FUN_PRE_(m_state.fun); // the current state
    QS_END_PRE_()
}

//${QS::QUTest::QActiveDummy::post_} .........................................
bool QActiveDummy::post_(
    QEvt const * const e,
    std::uint_fast16_t const margin,
    void const * const sender) noexcept
{
    QS_TEST_PROBE_DEF(&QActive::post_)

    // test-probe#1 for faking queue overflow
    bool status = true;
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

    std::uint_fast8_t const rec =
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

//${QS::QUTest::QActiveDummy::postLIFO} ......................................
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

} // namespace QP
//$enddef${QS::QUTest} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//===========================================================================
// QF/++ stub for QUTest
//$define${QS::QUTest-stub} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QF {

//${QS::QUTest-stub::QF::init} ...............................................
void init() {
    //! Clear the internal QF variables, so that the framework can start
    //! correctly even if the startup code fails to clear the uninitialized
    //! data (as is required by the C++ Standard).
    QActive::subscrList_   = nullptr;
    QActive::maxPubSignal_ = 0;
    QF_intNest_ = 0U;
    QF_maxPool_ = 0U;

    bzero(&QActive::active_[0], sizeof(QActive::active_));
    bzero(&QF::readySet_, sizeof(QF::readySet_));
}

//${QS::QUTest-stub::QF::stop} ...............................................
void stop() {
    QS::onReset();
}

//${QS::QUTest-stub::QF::run} ................................................
int_t run() {
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

} // namespace QF
namespace QS {

//${QS::QUTest-stub::QS::onGetTime} ..........................................
QSTimeCtr onGetTime() {
    return (++testData.testTime);
}

} // namespace QS

//${QS::QUTest-stub::QActive} ................................................

//${QS::QUTest-stub::QActive::start} .........................................
void QActive::start(
    std::uint_fast8_t const prio,
    QEvt const * * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    static_cast<void>(stkSto);  // unused parameter
    static_cast<void>(stkSize); // unused parameter

    // priority must be in range
    Q_REQUIRE_ID(200, (0U < prio) && (prio <= QF_MAX_ACTIVE));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = static_cast<std::uint8_t>(prio); // set the QF prio of this AO

    register_(); // make QF aware of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    //QS_FLUSH(); // flush the trace buffer to the host
}

//${QS::QUTest-stub::QActive::stop} ..........................................
#ifdef QF_ACTIVE_STOP
void QActive::stop() {
    unsubscribeAll(); // unsubscribe from all events
    unregister_(); // remove this object from QF
}

#endif // def QF_ACTIVE_STOP

//${QS::QUTest-stub::QTimeEvt} ...............................................

//${QS::QUTest-stub::QTimeEvt::tick1_} .......................................
void QTimeEvt::tick1_(
    std::uint_fast8_t const tickRate,
    void const * const sender)
{
    // The testing version of system tick processing performs as follows:
    // 1. If the Current Time Event (TE) Object is defined and the TE is armed,
    //    the TE is disarmed (if one-shot) and then posted to the recipient AO.
    // 2. The linked-list of all armed Time Events is updated.
    //

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QTimeEvt *prev = &QTimeEvt::timeEvtHead_[tickRate];

    QS_BEGIN_NOCRIT_PRE_(QS_QF_TICK, 0U)
        prev->m_ctr = (prev->m_ctr + 1U);
        QS_TEC_PRE_(prev->m_ctr); // tick ctr
        QS_U8_PRE_(tickRate);     // tick rate
    QS_END_NOCRIT_PRE_()

    // is current Time Event object provided?
    QTimeEvt *t = static_cast<QTimeEvt *>(QS::rxPriv_.currObj[QS::TE_OBJ]);
    if (t != nullptr) {

        // the time event must be armed
        Q_ASSERT_ID(810, t->m_ctr != 0U);

        // temp. for volatile
        QActive * const act = static_cast<QActive *>(t->m_act);

        // the recipient AO must be provided
        Q_ASSERT_ID(820, act != nullptr);

        // periodic time evt?
        if (t->m_interval != 0U) {
            t->m_ctr = t->m_interval; // rearm the time event
        }
        else { // one-shot time event: automatically disarm
            t->m_ctr = 0U; // auto-disarm
            // mark time event 't' as NOT linked
            t->refCtr_ = static_cast<std::uint8_t>(t->refCtr_
                         & static_cast<std::uint8_t>(~TE_IS_LINKED));

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

            // any new time events armed since the last run of tickX_()?
            if (QTimeEvt::timeEvtHead_[tickRate].m_act != nullptr) {

                // sanity check
                Q_ASSERT_CRIT_(830, prev != nullptr);
                prev->m_next = QTimeEvt::timeEvtHead_[tickRate].toTimeEvt();
                QTimeEvt::timeEvtHead_[tickRate].m_act = nullptr;
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
            t->refCtr_ = static_cast<std::uint8_t>(t->refCtr_
                         & static_cast<std::uint8_t>(~TE_IS_LINKED));
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
//$enddef${QS::QUTest-stub} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//============================================================================
extern "C" {

Q_NORETURN Q_onAssert(char const * const module, int_t const location) {
    QS_BEGIN_NOCRIT_PRE_(QP::QS_ASSERT_FAIL, 0U)
        QS_TIME_PRE_();
        QS_U16_PRE_(location);
        QS_STR_PRE_((module != nullptr) ? module : "?");
    QS_END_NOCRIT_PRE_()

    QP::QS::onFlush();    // flush the assertion record to the host
    QP::QS::onTestLoop(); // loop to wait for commands (typically reset)
    QP::QS::onReset();    // in case the QUTest loop ever returns: reset
    for (;;) { // onReset() should not return, but to ensure no-return...
    }
}

} // extern "C"

#endif // def Q_UTEST
