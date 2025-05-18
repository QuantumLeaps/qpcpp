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

// maximum depth of state nesting in a QMsm (including the top level)
static constexpr std::int_fast8_t QMSM_MAX_NEST_DEPTH_ {6};

// maximum length of transition-action array
static constexpr std::int_fast8_t QMSM_MAX_TRAN_LENGTH_ {2*QMSM_MAX_NEST_DEPTH_};

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
#define QS_STATE_ACT_(rec_, state_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_OBJ_PRE(this);      \
        QS_FUN_PRE(state_);    \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to top-most init
#define QS_TOP_INIT_(rec_, trg_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_TIME_PRE();         \
        QS_OBJ_PRE(this);      \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to trace transition segment
#define QS_TRAN_SEG_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_OBJ_PRE(this);      \
        QS_FUN_PRE(src_);      \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to trace transition begin/end
#define QS_TRAN0_(rec_, trg_)  \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_TIME_PRE();         \
        QS_SIG_PRE(e->sig);    \
        QS_OBJ_PRE(this);      \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
    QS_CRIT_EXIT()

// internal helper macro to trace regular transition
#define QS_TRAN_END_(rec_, src_, trg_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_TIME_PRE();         \
        QS_SIG_PRE(e->sig);    \
        QS_OBJ_PRE(this);      \
        QS_FUN_PRE(src_);      \
        QS_FUN_PRE(trg_);      \
    QS_END_PRE()               \
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

    Q_REQUIRE_LOCAL(200, m_temp.fun != nullptr);
    Q_REQUIRE_LOCAL(210, m_state.obj == &l_msm_top_s);

    // execute the top-most initial tran.
    QState r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    // the top-most initial tran. must be taken
    Q_ASSERT_LOCAL(240, r == Q_RET_TRAN_INIT);
    Q_ASSERT_LOCAL(250, m_temp.tatbl != nullptr);

    QS_CRIT_STAT
    QS_TRAN_SEG_(QS_QEP_STATE_INIT,
       m_state.obj->stateHandler, m_temp.tatbl->target->stateHandler);

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    do {
        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(280, lbound >= 0);
        r = execTatbl_(m_temp.tatbl, qsId);
    } while (r >= Q_RET_TRAN_INIT);

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, m_state.obj->stateHandler);

#ifndef Q_UNSAFE
    // establish stable state configuration
    m_temp.uint = static_cast<std::uintptr_t>(~m_state.uint);
#endif
}

//............................................................................
void QMsm::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    Q_INVARIANT_LOCAL(300,
        m_state.uint == static_cast<std::uintptr_t>(~m_temp.uint));

    Q_REQUIRE_LOCAL(310, e != nullptr);

    QMState const *s = m_state.obj; // the current state
    QMState const * const t = s; // store the current state for later
    QS_CRIT_STAT
    QS_TRAN0_(QS_QEP_DISPATCH, s->stateHandler);

    // scan the state hierarchy up to the top state...
    QState r = Q_RET_SUPER;
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    do {
         --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(340, lbound >= 0);

        r = (*s->stateHandler)(this, e); // call state handler function
        if (r >= Q_RET_HANDLED) { // event handled? (the most frequent case)
            break; // done scanning the state hierarchy
        }
#ifdef Q_SPY
        if (r == Q_RET_UNHANDLED) { // event unhandled due to a guard?
            QS_CRIT_ENTRY();
            QS_BEGIN_PRE(QS_QEP_UNHANDLED, qsId)
                QS_SIG_PRE(e->sig);
                QS_OBJ_PRE(this);
                QS_FUN_PRE(s->stateHandler);
            QS_END_PRE()
            QS_CRIT_EXIT();
        }
#endif
        s = s->superstate; // advance to the superstate
    } while (s != nullptr);

    if (s == nullptr) { // event bubbled to the 'top' state?
#ifdef Q_SPY
        QS_TRAN0_(QS_QEP_IGNORED, t->stateHandler);
#endif // Q_SPY
    }
    else if (r >= Q_RET_TRAN) { // any kind of tran. taken?
#ifdef Q_SPY
        QMState const * const ts = s; // tran. source for QS tracing
#endif // Q_SPY

        if (r == Q_RET_TRAN) {
            struct QMTranActTable const * const tatbl = m_temp.tatbl;
            exitToTranSource_(t, s, qsId);
            r = execTatbl_(tatbl, qsId);
#ifdef Q_SPY
            s = m_state.obj;
#endif // Q_SPY
        }
        else if (r == Q_RET_TRAN_HIST) { // was it tran. to history?
            QMState const * const hist = m_state.obj; // save history
            m_state.obj = t; // restore the original state

            QS_TRAN_SEG_(QS_QEP_TRAN_HIST,
                s->stateHandler, hist->stateHandler);

            // save the tran-action table before it gets clobbered
            struct QMTranActTable const * const tatbl = m_temp.tatbl;
            exitToTranSource_(t, s, qsId);
            static_cast<void>(execTatbl_(tatbl, qsId));
            r = enterHistory_(hist, qsId);
#ifdef Q_SPY
            s = m_state.obj;
#endif // Q_SPY
        }
        else {
            // empty
        }

        lbound = QMSM_MAX_NEST_DEPTH_;
        while (r == Q_RET_TRAN_INIT) { // initial tran. in the target?

            r = execTatbl_(m_temp.tatbl, qsId);
#ifdef Q_SPY
            s = m_state.obj;
#endif // Q_SPY

             --lbound; // fixed loop bound
            Q_INVARIANT_LOCAL(360, lbound >= 0);
        }

        QS_TRAN_END_(QS_QEP_TRAN, ts->stateHandler, s->stateHandler);
    }
#ifdef Q_SPY
    else if (r == Q_RET_HANDLED) { // was the event handled?
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s->stateHandler);
    }
#endif // Q_SPY
    else {
        // empty
    }

#ifndef Q_UNSAFE
    // establish stable state configuration
    m_temp.uint = static_cast<std::uintptr_t>(~m_state.uint);
#endif
}

//............................................................................
QState QMsm::execTatbl_(
    QMTranActTable const * const tatbl,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    Q_REQUIRE_LOCAL(400, tatbl != nullptr);

    QS_CRIT_STAT
    QState r = Q_RET_NULL;
    QActionHandler const *a = &tatbl->act[0];
    std::int_fast8_t lbound = QMSM_MAX_TRAN_LENGTH_;
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

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(480, lbound >= 0);
    }

    m_state.obj = (r >= Q_RET_TRAN)
        ? m_temp.tatbl->target
        : tatbl->target;
    return r;
}

//............................................................................
void QMsm::exitToTranSource_(
    QMState const * const curr_state,
    QMState const * const tran_source,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif
    QS_CRIT_STAT

    // exit states from the current state to the tran. source state
    QMState const *s = curr_state;
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != tran_source) {
        if (s->exitAction != nullptr) { // exit action provided?
            static_cast<void>((*s->exitAction)(this));
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, m_temp.obj->stateHandler);
        }
        s = s->superstate; // advance to the superstate

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(580, lbound >= 0);
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
    QMState const *epath[QMSM_MAX_NEST_DEPTH_];
    QMState const *s = hist;
    std::int_fast8_t i = -1; // entry path index (one below [0])
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != m_state.obj) {
        if (s->entryAction != nullptr) {
            ++i;
            Q_INVARIANT_LOCAL(610, i < QMSM_MAX_NEST_DEPTH_);
            epath[i] = s;
        }
        s = s->superstate;

        --lbound; // fixed loop bound
        Q_INVARIANT_LOCAL(620, lbound >= 0);
    }

    QS_CRIT_STAT
    // retrace the entry path in reverse (desired) order...
    // NOTE: i the fixed loop bound
    for (; i >= 0; --i) {
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

//............................................................................
bool QMsm::isIn(QStateHandler const stateHndl) noexcept {
    bool inState = false; // assume that this SM is not in 'state'
    QMState const *s = m_state.obj;
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != nullptr) {
        if (s->stateHandler == stateHndl) { // match found?
            inState = true;
            break;
        }
        s = s->superstate; // advance to the superstate

         --lbound; // fixed loop bound
         Q_INVARIANT_LOCAL(740, lbound >= 0);
    }

    return inState;
}

//............................................................................
QMState const * QMsm::childStateObj(QMState const * const parentHndl)
    const noexcept
{
    QMState const *s = m_state.obj; // start with current state
    QMState const *child = s;
    bool isFound = false; // assume the child NOT found
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_;
    while (s != nullptr) {
        if (s == parentHndl) {
            isFound = true; // child is found
            break;
        }
        child = s;
        s = s->superstate;

         --lbound; // fixed loop bound
         Q_INVARIANT_LOCAL(840, lbound >= 0);
    }
    Q_ENSURE_LOCAL(890, isFound);

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(isFound);
#endif

    return child;
}

} // namespace QP
