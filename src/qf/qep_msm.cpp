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

// unnamed namespace for local definitions with internal linkage
namespace {

Q_DEFINE_THIS_MODULE("qep_msm")

// maximum depth of state nesting in a QMsm (including the top level)
static constexpr std::size_t QMSM_MAX_NEST_DEPTH_ {6U};

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

    // current state must be initialized to &l_msm_top_s in QMsm_ctor()
    Q_REQUIRE_LOCAL(200, m_state.obj == &l_msm_top_s);

    // temp contains the top-most initial tran. handler, which must be valid
    Q_REQUIRE_LOCAL(210, m_temp.fun != nullptr);

    // execute the top-most initial tran.
    QState r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    // the top-most initial tran. must be taken
    Q_ASSERT_LOCAL(240, r == Q_RET_TRAN_INIT);

    // the top-most initial tran. must set the tran-action table in temp
    Q_ASSERT_LOCAL(250, m_temp.tatbl != nullptr);

    QS_CRIT_STAT
    QS_TRAN_SEG_(QS_QEP_STATE_INIT,
       m_state.obj->stateHandler, m_temp.tatbl->target->stateHandler);

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    do {
        r = execTatbl_(m_temp.tatbl, qsId);
    } while (r == Q_RET_TRAN_INIT);

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, m_state.obj->stateHandler);

#ifndef Q_UNSAFE
    // establish stable state configuration at the end of RTC step
    m_temp.uint = dis_update<std::uintptr_t>(m_state.uint);
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

    // this state machine must be in a stable state configuration
    // NOTE: stable state configuration is established after every RTC step.
    Q_INVARIANT_LOCAL(300,
        dis_verify<std::uintptr_t>(m_state.uint, m_temp.uint));

    // the event to be dispatched must be valid
    Q_REQUIRE_LOCAL(310, e != nullptr);

    QMState const *s = m_state.obj; // the current state
    QMState const * const t = s; // store the current state for later
    QS_CRIT_STAT
    QS_TRAN0_(QS_QEP_DISPATCH, s->stateHandler);

    // scan the state hierarchy up to the top state...
    QState r;
    do {
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
#endif
    }
    else if (r == Q_RET_HANDLED) { // was the event e handled?
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s->stateHandler); // output QS record
    }
    else if ((r == Q_RET_TRAN) || (r == Q_RET_TRAN_HIST)) { //any tran. taken?
#ifdef Q_SPY
        QMState const * const ts = s; // tran. source for QS tracing
#endif // Q_SPY

        if (r == Q_RET_TRAN) { // tran. taken?
            struct QMTranActTable const * const tatbl = m_temp.tatbl;
            exitToTranSource_(t, s, qsId);
            r = execTatbl_(tatbl, qsId);
        }
        else { // must be tran. to history
            QMState const * const hist = m_state.obj; // save history
            m_state.obj = t; // restore the original state

            QS_TRAN_SEG_(QS_QEP_TRAN_HIST,
                s->stateHandler, hist->stateHandler);

            // save the tran-action table before it gets clobbered
            struct QMTranActTable const * const tatbl = m_temp.tatbl;
            exitToTranSource_(t, s, qsId);
            static_cast<void>(execTatbl_(tatbl, qsId));
            r = enterHistory_(hist, qsId);
        }
#ifdef Q_SPY
        s = m_state.obj;
#endif

        while (r == Q_RET_TRAN_INIT) { // initial tran. in the target?

            r = execTatbl_(m_temp.tatbl, qsId);
#ifdef Q_SPY
            s = m_state.obj;
#endif
        }

        QS_TRAN_END_(QS_QEP_TRAN, ts->stateHandler, s->stateHandler);
    }
    else {
        Q_ERROR_LOCAL(360); // last action handler returned impossible value
    }

#ifndef Q_UNSAFE
    // establish stable state configuration at the end of RTC step
    m_temp.uint = dis_update<std::uintptr_t>(m_state.uint);
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

    // the tran-action table parameter must be valid
    Q_REQUIRE_LOCAL(400, tatbl != nullptr);

    QS_CRIT_STAT
    QState r = Q_RET_SUPER;
    QActionHandler const *a = &tatbl->act[0];
    while (*a != nullptr) { // not the end of the tran-action table?
        r = (*(*a))(this); // call the action through the 'a' pointer
        ++a; // advance to the next entry in the tran-action table

        if (r == Q_RET_ENTRY) { // was the action a state entry?
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, m_temp.obj->stateHandler);
        }
        else if (r == Q_RET_EXIT) { // was the action a state exit?
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, m_temp.obj->stateHandler);
        }
        else if (r == Q_RET_TRAN_INIT) { // was the action a state init?
            QS_TRAN_SEG_(QS_QEP_STATE_INIT,
                tatbl->target->stateHandler,
                m_temp.tatbl->target->stateHandler);
        }
        else {
            // the last action handler returned impossible value (corrupt SM?)
            Q_ERROR_LOCAL(460);
        }
    }

    m_state.obj = tatbl->target; // set new current state
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
    // NOTE: the following loop does not need the fixed loop bound check
    // because the path from the current state to the tran.source has
    // been already checked in the invariant 340.
    QMState const *s = curr_state;
    while (s != tran_source) {
        if (s->exitAction != nullptr) { // exit action provided?
            // exit state s, ignore the result
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
    std::array<QMState const *, QMSM_MAX_NEST_DEPTH_> path;
    QMState const *s = hist;
    std::size_t ip = 0U; // entry path index
    while (s != m_state.obj) {
        if (s->entryAction != nullptr) { // does s have an entry action?
            Q_INVARIANT_LOCAL(610, ip < QMSM_MAX_NEST_DEPTH_);
            path[ip] = s;
            ++ip;
        }
        s = s->superstate;
    }

    QS_CRIT_STAT
    // retrace the entry path in reverse (desired) order...
    // NOTE: i is the fixed loop bound already checked in invariant 610
    while (ip > 0U) {
        --ip;
        // enter the state in path[ip], ignore the result
        static_cast<void>((*path[ip]->entryAction)(this));
        QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]->stateHandler);
    }

    m_state.obj = hist; // set current state to the tran. target

    // initial tran. present?
    QState r = Q_RET_SUPER;
    if (hist->initAction != nullptr) { // init. action provided?
        r = (*hist->initAction)(this); // execute the init. action
        QS_TRAN_SEG_(QS_QEP_STATE_INIT,
            hist->stateHandler, m_temp.tatbl->target->stateHandler);
    }
    return r; // inform the caller if the init action was taken
}

//............................................................................
QMState const * QMsm::topQMState() noexcept {
    // return the top state (object pointer)
    return &l_msm_top_s;
}
//............................................................................
bool QMsm::isIn(QStateHandler const stateHndl) noexcept {
    bool inState = false; // assume that this SM is not in 'state'
    QMState const *s = m_state.obj;
    while (s != nullptr) {
        if (s->stateHandler == stateHndl) { // match found?
            inState = true;
            break;
        }
        s = s->superstate; // advance to the superstate
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
    while (s != nullptr) { // top of state hierarchy not reached yet?
        if (s == parentHndl) {
            isFound = true; // child is found
            break;
        }
        child = s;
        s = s->superstate;
    }
    // the child state must be found, or the state machine is corrupt
    Q_ENSURE_LOCAL(890, isFound);

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(isFound);
#endif

    return child;
}
//............................................................................
QStateHandler QMsm::getStateHandler() const noexcept {
    // return the current state handler (function pointer)
    return m_state.obj->stateHandler;
}

} // namespace QP
