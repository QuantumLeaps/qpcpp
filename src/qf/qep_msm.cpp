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
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {

Q_DEFINE_THIS_MODULE("qep_msm")

// maximum depth of entry levels in a MSM for tran. to history.
static constexpr std::int_fast8_t QMSM_MAX_ENTRY_DEPTH_ {4};

//! @cond INTERNAL

// top-state object for QMsm-style state machines
constexpr QP::QMState l_msm_top_s = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

} // unnamed namespace

#ifdef Q_SPY
// helper macro to trace state action (entry/exit)
#define QS_STATE_ACT_(rec_, state_)    \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_OBJ_PRE(this);              \
        QS_FUN_PRE(state_);            \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to top-most init
#define QS_TOP_INIT_(rec_, trg_)       \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_TIME_PRE();                 \
        QS_OBJ_PRE(this);              \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to trace transition segment
#define QS_TRAN_SEG_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_OBJ_PRE(this);              \
        QS_FUN_PRE(src_);              \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to trace transition begin/end
#define QS_TRAN0_(rec_, trg_)          \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_TIME_PRE();                 \
        QS_SIG_PRE(e->sig);            \
        QS_OBJ_PRE(this);              \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

// internal helper macro to trace regulsr transition
#define QS_TRAN_END_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_TIME_PRE();                 \
        QS_SIG_PRE(e->sig);            \
        QS_OBJ_PRE(this);              \
        QS_FUN_PRE(src_);              \
        QS_FUN_PRE(trg_);              \
    QS_END_PRE()                       \
    QS_CRIT_EXIT()

#else
#define QS_STATE_ACT_(rec_, state_)     (static_cast<void>(0))
#define QS_TOP_INIT_(rec_, trg_)        (static_cast<void>(0))
#define QS_TRAN_SEG_(rec_, src_, trg_)  (static_cast<void>(0))
#define QS_TRAN0_(rec_, trg_)           (static_cast<void>(0))
#define QS_TRAN_END_(rec_, src_, trg_)  (static_cast<void>(0))
#endif

//! @endcond

//============================================================================
namespace QP {

//............................................................................
QMsm::QMsm(QStateHandler const initial) noexcept
  : QAsm()
{
    m_state.obj = &l_msm_top_s; // the current state (top)
    m_temp.fun  = initial;      // the initial tran. handler
}

//............................................................................
void QMsm::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(200,
        (m_temp.fun != nullptr)
        && (m_state.obj == &l_msm_top_s));
    QF_CRIT_EXIT();

    // execute the top-most initial tran.
    QState r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    QF_CRIT_ENTRY();
    // the top-most initial tran. must be taken
    Q_ASSERT_INCRIT(210, r == Q_RET_TRAN_INIT);
    QF_CRIT_EXIT();

    QS_TRAN_SEG_(QS_QEP_STATE_INIT,
       m_state.obj->stateHandler, m_temp.tatbl->target->stateHandler);

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    while (r >= Q_RET_TRAN_INIT) {
        // execute the tran. table
        r = execTatbl_(m_temp.tatbl, qsId);
    }

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, m_state.obj->stateHandler);
}

//............................................................................
void QMsm::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QMState const *s = m_state.obj; // store the current state
    QMState const *t = s;

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300,
        (e != nullptr)
        && (s != nullptr));
    QF_CRIT_EXIT();

    QS_TRAN0_(QS_QEP_DISPATCH, s->stateHandler);

    // scan the state hierarchy up to the top state...
    QState r = Q_RET_SUPER;
    while (t != nullptr) {
        r = (*t->stateHandler)(this, e); // call state handler function

        if (r >= Q_RET_HANDLED) { // event handled? (the most frequent case)
            break; // done scanning the state hierarchy
        }
#ifdef Q_SPY
        if (r == Q_RET_UNHANDLED) { // event unhandled due to a guard?
            QS_CRIT_ENTRY();
            QS_BEGIN_PRE(QS_QEP_UNHANDLED, qsId)
                QS_SIG_PRE(e->sig);
                QS_OBJ_PRE(this);
                QS_FUN_PRE(t->stateHandler);
            QS_END_PRE()
            QS_CRIT_EXIT();
        }
#endif
        t = t->superstate; // advance to the superstate
    }

    if (r >= Q_RET_TRAN) { // any kind of tran. taken?
        QF_CRIT_ENTRY();
        // the tran. source state must not be NULL
        Q_ASSERT_INCRIT(330, t != nullptr);
        QF_CRIT_EXIT();

#ifdef Q_SPY
        QMState const * const ts = t; // tran. source for QS tracing
#endif // Q_SPY

        if (r == Q_RET_TRAN_HIST) { // was it tran. to history?
            QMState const * const hist = m_state.obj; // save history
            m_state.obj = s; // restore the original state

            QS_TRAN_SEG_(QS_QEP_TRAN_HIST,
                t->stateHandler, hist->stateHandler);

            // save the tran-action table before it gets clobbered
            QMTranActTable const *tatbl = m_temp.tatbl;
            exitToTranSource_(s, t, qsId);
            static_cast<void>(execTatbl_(tatbl, qsId));
            r = enterHistory_(hist, qsId);
            s = m_state.obj;
            t = s; // set target to the current state
        }

        while (r >= Q_RET_TRAN) {
            // save the tran-action table before it gets clobbered
            QMTranActTable const *tatbl = m_temp.tatbl;
            m_temp.obj = nullptr; // clear
            exitToTranSource_(s, t, qsId);
            r = execTatbl_(tatbl, qsId);
            s = m_state.obj;
            t = s; // set target to the current state
        }

        QS_TRAN_END_(QS_QEP_TRAN, ts->stateHandler, s->stateHandler);
    }
#ifdef Q_SPY
    else if (r == Q_RET_HANDLED) { // was the event handled?
        QF_CRIT_ENTRY();
        // internal tran. source can't be NULL
        Q_ASSERT_INCRIT(380, t != nullptr);
        QF_CRIT_EXIT();

        QS_TRAN0_(QS_QEP_INTERN_TRAN, t->stateHandler);
    }
    else if (t == nullptr) { // event bubbled to the 'top' state?
        QS_TRAN0_(QS_QEP_IGNORED, s->stateHandler);
    }
#endif // Q_SPY
    else {
        // empty
    }
}

//............................................................................
bool QMsm::isIn(QStateHandler const state) noexcept {
    bool inState = false; // assume that this SM is not in 'state'

    QMState const *s = m_state.obj;
    while (s != nullptr) {
        if (s->stateHandler == state) { // match found?
            inState = true;
            break;
        }
        s = s->superstate; // advance to the superstate
    }

    return inState;
}

//............................................................................
QMState const * QMsm::childStateObj(QMState const * const parent)
    const noexcept
{
    QMState const *s = m_state.obj; // start with current state
    QMState const *child = s;
    bool isFound = false; // assume the child NOT found

    while (s != nullptr) {
        if (s == parent) {
            isFound = true; // child is found
            break;
        }
        child = s;
        s = s->superstate;
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(590, isFound);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(isFound);
#endif

    return child; // return the child
}

//............................................................................
QState QMsm::execTatbl_(
    QMTranActTable const * const tatbl,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // precondition:
    // - the tran-action table pointer must not be NULL
    Q_REQUIRE_INCRIT(600, tatbl != nullptr);
    QF_CRIT_EXIT();

    QState r = Q_RET_NULL;
    QActionHandler const *a = &tatbl->act[0];
    while (*a != nullptr) {
        r = (*(*a))(this); // call the action through the 'a' pointer
        ++a;
#ifdef Q_SPY
        if (r == Q_RET_ENTRY) {
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, m_temp.obj->stateHandler);
        }
        else if (r == Q_RET_EXIT) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, m_temp.obj->stateHandler);
        }
        else if (r == Q_RET_TRAN_INIT) {
            QS_TRAN_SEG_(QS_QEP_STATE_INIT,
                tatbl->target->stateHandler,
                m_temp.tatbl->target->stateHandler);
        }
        else {
            // empty
        }
#endif // Q_SPY
    }
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(690, *a == nullptr);
    QF_CRIT_EXIT();

    m_state.obj = (r >= Q_RET_TRAN)
        ? m_temp.tatbl->target
        : tatbl->target;
    return r;
}

//............................................................................
void QMsm::exitToTranSource_(
    QMState const * const cs,
    QMState const * const ts,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif
    QS_CRIT_STAT

    // exit states from the current state to the tran. source state
    QMState const *s = cs;
    while (s != ts) {
        // exit action provided in state 's'?
        if (s->exitAction != nullptr) {
            // execute the exit action
            static_cast<void>((*s->exitAction)(this));

            QS_STATE_ACT_(QS_QEP_STATE_EXIT, m_temp.obj->stateHandler);
        }
        s = s->superstate; // advance to the superstate
    }
}

//............................................................................
QState QMsm::enterHistory_(
    QMState const * const hist,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    // record the entry path from current state to history
    QMState const *epath[QMSM_MAX_ENTRY_DEPTH_];
    QMState const *s = hist;
    std::int_fast8_t i = 0; // tran. entry path index & fixed upper loop bound
    while ((s != m_state.obj) && (i < QMSM_MAX_ENTRY_DEPTH_)) {
        if (s->entryAction != nullptr) {
            epath[i] = s;
            ++i;
        }
        s = s->superstate;
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(810, i <= QMSM_MAX_ENTRY_DEPTH_);
    QF_CRIT_EXIT();

    // retrace the entry path in reverse (desired) order...
    // NOTE: i the fixed upper loop bound
    for (i = i - 1; i >= 0; --i) {
        // run entry action in epath[i]
        static_cast<void>((*epath[i]->entryAction)(this));

        QS_STATE_ACT_(QS_QEP_STATE_ENTRY, epath[i]->stateHandler);
    }

    m_state.obj = hist; // set current state to the tran. target

    // initial tran. present?
    QState r = Q_RET_NULL;
    if (hist->initAction != nullptr) {
        r = (*hist->initAction)(this); // execute the tran. action

        QS_TRAN_SEG_(QS_QEP_STATE_INIT,
            hist->stateHandler, m_temp.tatbl->target->stateHandler);
    }
    return r;
}

//............................................................................
QMState const * QMsm::topQMState() const noexcept {
    return &l_msm_top_s;
}

} // namespace QP
