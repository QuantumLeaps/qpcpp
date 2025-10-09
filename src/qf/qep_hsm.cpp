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

Q_DEFINE_THIS_MODULE("qep_hsm")

// maximum depth of state nesting in a QHsm (including the top level)
// must be >= 3
static constexpr std::int_fast8_t QHSM_MAX_NEST_DEPTH_ {6};

//! @cond INTERNAL

// array of immutable events corresponding to the reserved signals
static constexpr std::array<QP::QEvt, 4> l_resEvt_ {
    QP::QEvt(static_cast<QP::QSignal>(QP::QHsm::Q_EMPTY_SIG)),
    QP::QEvt(static_cast<QP::QSignal>(QP::QHsm::Q_ENTRY_SIG)),
    QP::QEvt(static_cast<QP::QSignal>(QP::QHsm::Q_EXIT_SIG)),
    QP::QEvt(static_cast<QP::QSignal>(QP::QHsm::Q_INIT_SIG))
};

//! @endcond

} // unnamed namespace

//============================================================================
//! @cond INTERNAL

// internal helper macro to pass a reserved event into the state handler

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

// internal helper macro to trace transition action
#define QS_TRAN_ACT_(rec_, state_) \
    QS_CRIT_ENTRY();           \
    QS_BEGIN_PRE((rec_), qsId) \
        QS_SIG_PRE(e->sig);    \
        QS_OBJ_PRE(this);      \
        QS_FUN_PRE(state_);    \
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
#define QS_TRAN_ACT_(rec_, state_)      (static_cast<void>(0))
#define QS_TRAN0_(rec_, trg_)           (static_cast<void>(0))
#define QS_TRAN_END_(rec_, src_, trg_)  (static_cast<void>(0))
#endif

//! @endcond

//============================================================================
namespace QP {

//............................................................................
QHsm::QHsm(QStateHandler const initial) noexcept
  : QAsm()
{
    m_state.fun = &top; // the current state (top)
    m_temp.fun  = initial; // the initial tran. handler
}

//............................................................................
void QHsm::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
#ifdef Q_SPY
    // produce QS dictionary for QHsm_top handler...
    // NOTE: the QHsm_top dictionary needs to be produced only once
    // and not every time QHsm_init_() is called.
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    bool toDo = false; // assume that dictionary not produced
    if ((QS::priv_.flags & 0x01U) == 0U) { // dictionary not produced yet?
        QS::priv_.flags |= 0x01U; // mark the QHsm::top dictionary as produced
        toDo = true;
    }
    QS_CRIT_EXIT();
    if (toDo) { // need to produce the dictionary?
        QS_FUN_DICTIONARY(&QP::QHsm::top);
    }
#else
    Q_UNUSED_PAR(qsId);
#endif // Q_SPY

    QStateHandler const s = m_state.fun; // current state

    // current state must be initialized to QHsm_top in QHsm_ctor()
    Q_REQUIRE_LOCAL(200, s == Q_STATE_CAST(&top));

    // temp contains the top-most initial tran. handler, which must be valid
    Q_REQUIRE_LOCAL(210, m_temp.fun != nullptr);

    // execute the top-most initial tran.
    QState const r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    // the top-most initial tran. must be taken
    Q_ASSERT_LOCAL(240, r == Q_RET_TRAN);

    // the top-most initial tran. must set the target in temp
    Q_ASSERT_LOCAL(250, m_temp.fun != nullptr);

    QS_TRAN_SEG_(QS_QEP_STATE_INIT, s, m_temp.fun); // output QS record

    // drill down into the state hierarchy with initial transitions...
    std::array <QStateHandler, QHSM_MAX_NEST_DEPTH_> path; // entry path array

    std::int_fast8_t ip = -1; // path index & fixed loop bound (one below [0])
    do {
        ++ip;

        // the entry path index must not overflow the allocated array
        Q_INVARIANT_LOCAL(260, ip < QHSM_MAX_NEST_DEPTH_);

        // the initial tran. must set target in temp
        Q_ASSERT_LOCAL(270, m_temp.fun != nullptr);

        path[ip] = m_temp.fun; // store the entry path

        // find the superstate of 'm_temp.fun', ignore result
        static_cast<void>((*m_temp.fun)(this, &l_resEvt_[Q_EMPTY_SIG]));
    } while (m_temp.fun != s);

    // enter the target (possibly recursively) by initial trans.
    enter_target_(&path[0], ip, qsId);

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, path[0]); // output QS record

    m_state.fun = path[0]; // change the current active state
#ifndef Q_UNSAFE
    // establish stable state configuration
    m_temp.uint = dis_update<std::uintptr_t>(m_state.uint);
#else
    Q_UNUSED_PAR(r);
#endif
}

//............................................................................
void QHsm::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    // this state machine must be in a stable state configuration
    // NOTE: stable state configuration is established after every RTC step.
    Q_INVARIANT_INCRIT(300,
        dis_verify<std::uintptr_t>(m_state.uint, m_temp.uint));

    // the event to be dispatched must be valid
    Q_REQUIRE_LOCAL(310, e != nullptr);

    QStateHandler s = m_state.fun; // current state
    QS_CRIT_STAT
    QS_TRAN0_(QS_QEP_DISPATCH, s); // output QS record

    // process the event hierarchically...
    std::array <QStateHandler, QHSM_MAX_NEST_DEPTH_> path; // entry path array
    m_temp.fun = s;
    QState r; // state handler return value
    std::int_fast8_t ip = QHSM_MAX_NEST_DEPTH_;//path index & fixed loop bound
    do {
        --ip;

        // the entry path index must stay in range of the path[] array
        Q_INVARIANT_LOCAL(340, ip >= 0);

        s = m_temp.fun; // set s to the superstate set previously
        path[ip] = s; // store the path to potential tran. source

        r = (*s)(this, e); // try to handle event e in state s

        if (r == Q_RET_UNHANDLED) { // unhandled due to a guard?
            QS_TRAN_ACT_(QS_QEP_UNHANDLED, s); // output QS record

            // find the superstate of 's'
            r = (*s)(this, &l_resEvt_[Q_EMPTY_SIG]);
        }
    } while (r == Q_RET_SUPER); // loop as long as superstate returned

    if (r == Q_RET_IGNORED) { // was event e ignored?
        QS_TRAN0_(QS_QEP_IGNORED, m_state.fun); // output QS record
    }
    else if (r == Q_RET_HANDLED) { // did the last handler handle event e?
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s); // output QS record
    }
    else if ((r == Q_RET_TRAN) || (r == Q_RET_TRAN_HIST)) { // tran. taken?
        // tran. must set temp to the target state
        Q_ASSERT_LOCAL(350, m_temp.fun != nullptr);

#ifdef Q_SPY
        if (r == Q_RET_TRAN_HIST) { // tran. to history?
            QS_TRAN_SEG_(QS_QEP_TRAN_HIST, s, m_temp.fun); // output QS record
        }
#endif

        path[0] = m_temp.fun; // save tran. target in path[0]

        // exit current state to tran. source...
        for (std::int_fast8_t iq = QHSM_MAX_NEST_DEPTH_ - 1; iq > ip; --iq) {
            // exit from 'path[iq]'
            if ((*path[iq])(this, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_EXIT, path[iq]); //output QS record
            }
        }
        path[2] = s; // save tran. source

        // take the tran...
        ip = tran_simple_(&path[0], qsId); // try simple tran. first
        if (ip < -1) { // not a simple tran.?
            ip = tran_complex_(&path[0], qsId);
        }

        // enter the target (possibly recursively) by initial trans.
        enter_target_(&path[0], ip, qsId);
        QS_TRAN_END_(QS_QEP_TRAN, s, path[0]); // output QS record

        m_state.fun = path[0]; // change the current active state
    }
    else {
        Q_ERROR_LOCAL(360); // last state handler returned impossible value
    }

#ifndef Q_UNSAFE
    // establish stable state configuration
    m_temp.uint = dis_update<std::uintptr_t>(m_state.uint);
#endif
}

//............................................................................
//! @private @memberof QHsm
std::int_fast8_t QHsm::tran_simple_(
    QStateHandler * const path,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QStateHandler t = path[0];       // target
    QStateHandler const s = path[2]; // source
    std::int_fast8_t ip = 0; // assume to enter the target
    QS_CRIT_STAT

    // (a) check source==target (tran. to self)...
    if (s == t) {
        // exit source 's' (external tran. semantics)
        if ((*s)(this, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS record
        }
    }
    else { // not a tran. to self
        // find superstate of target in 't'
        QState const r = (*t)(this, &l_resEvt_[Q_EMPTY_SIG]);

        // state handler t must return the superstate for Q_EMPTY_SIG
        Q_ASSERT_LOCAL(440, r == Q_RET_SUPER);

        // state handler t must set temp to the superstate
        Q_ASSERT_LOCAL(450, m_temp.fun != nullptr);
#ifdef Q_UNSAFE
        Q_UNUSED_PAR(r);
#endif

        // (b) check source==target->super...
        t = m_temp.fun;
        if (s != t) {
            // find superstate of source 's', ignore the result
            static_cast<void>((*s)(this, &l_resEvt_[Q_EMPTY_SIG]));

            // (c) check source->super==target->super...
            if (m_temp.fun == t) {
                // exit source 's'
                if ((*s)(this, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS record
                }
            }
            // (d) check source->super==target...
            else if (m_temp.fun == path[0]) {
                // exit source 's'
                if ((*s)(this, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS record
                }
                ip = -1; // set entry path index not to enter the target
            }
            else {
                path[1] = t; // save the superstate of target
                ip = -2; // cause execution of QHsm::tran_complex_()
            }
        }
    }
    // return: # levels in path[] for QHsm_enter_target_()
    // or indication to call QHsm_tran_complex_()
    return ip;
}

//............................................................................
//! @private @memberof QHsm
std::int_fast8_t QHsm::tran_complex_(
    QStateHandler * const path,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QStateHandler s = path[2];           // tran. source
    QStateHandler const ss = m_temp.fun; // source->super
    m_temp.fun = path[1];                // target->super
    std::int_fast8_t iq = 0; // assume that LCA is NOT found
    QState r;

    // (e) check rest of source == target->super->super...
    // and store the target entry path along the way
    std::int_fast8_t ip = 0; // path index & fixed loop bound (one below [1])
    do {
        ++ip;

        // the entry path index must stay in range of the path[] array
        Q_INVARIANT_LOCAL(540, ip < QHSM_MAX_NEST_DEPTH_);

        path[ip] = m_temp.fun; // store temp in the entry path array

        // find superstate of 'm_temp.fun'
        r = (*m_temp.fun)(this, &l_resEvt_[Q_EMPTY_SIG]);
        if (m_temp.fun == s) { // is temp the LCA?
            iq = 1; // indicate that LCA is found
            break;
        }
    } while (r == Q_RET_SUPER); // loop as long as superstate reached

    if (iq == 0) { // the LCA not found yet?
        QS_CRIT_STAT

#ifndef Q_SPY
        // exit the source 's', ignore the result
        static_cast<void>((*s)(this, &l_resEvt_[Q_EXIT_SIG]));
#else
        // exit the source 's'
        if ((*s)(this, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s); // output QS trace
        }
#endif // def Q_SPY

        // (f) check the rest of
        // source->super... == target->super->super...
        s = ss; // source->super
        r = Q_RET_IGNORED; // assume that the LCA NOT found
        iq = ip; // outside for(;;) to comply with MC:2023 R14.2
        for (; iq >= 0; --iq) {
            if (s == path[iq]) { // is this the LCA?
                r = Q_RET_HANDLED; // indicate the LCA found
                ip = iq - 1; // do not enter the LCA
                break;
            }
        }

        if (r != Q_RET_HANDLED) { // the LCA still not found?
            // (g) check each source->super->...
            // for each target->super...
            do {
                // exit from 's'
                if ((*s)(this, &l_resEvt_[Q_EXIT_SIG]) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                    // find superstate of 's', ignore the result
                    static_cast<void>((*s)(this, &l_resEvt_[Q_EMPTY_SIG]));
                }
                s = m_temp.fun; // set to super of s
                // NOTE: loop bounded per invariant:540
                iq = ip; // outside for(;;) to comply with MC:2023 R14.2
                for (; iq >= 0; --iq) {
                    if (s == path[iq]) { // is this the LCA?
                        ip = iq - 1; // indicate not to enter the LCA
                        r = Q_RET_HANDLED; // cause break from outer loop
                        break;
                    }
                }
            } while (r != Q_RET_HANDLED);
        }
    }
    // # levels in path[] for QHsm_enter_target_()
    return ip;
}

//............................................................................
//! @private @memberof QHsm
void QHsm::enter_target_(
    QStateHandler * const path,
    std::int_fast8_t const depth,
    std::uint_fast8_t const qsId)
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QS_CRIT_STAT
    std::int_fast8_t ip = depth;
    // execute entry actions from LCA to tran target...
    for (; ip >= 0; --ip) {
        // enter 'path[ip]'
        if ((*path[ip])(this, &l_resEvt_[Q_ENTRY_SIG]) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]); // output QS trace
        }
    }
    QStateHandler t = path[0]; // tran. target

    // drill into the target hierarchy with nested initial trans...

    // take initial tran in 't'
    while ((*t)(this, &l_resEvt_[Q_INIT_SIG]) == Q_RET_TRAN) { // tran. taken?
        // temp holds the target of initial tran. and must be valid
        Q_ASSERT_LOCAL(650, m_temp.fun != nullptr);

        QS_TRAN_SEG_(QS_QEP_STATE_INIT, t, m_temp.fun); // output QS record

        // find superstate of initial tran. target...
        ip = -1; // entry path index and fixed loop bound (one below [0])
        do {
            ++ip;
            // the entry path index must stay in range of the path[] array
            Q_INVARIANT_LOCAL(660, ip < QHSM_MAX_NEST_DEPTH_);

            path[ip] = m_temp.fun; // store the entry path

            // find superstate of 'm_temp.fun'
            QState const r = (*m_temp.fun)(this, &l_resEvt_[Q_EMPTY_SIG]);

            // the temp superstate handler must return superstate
            Q_ASSERT_LOCAL(680, r == Q_RET_SUPER);
#ifdef Q_UNSAFE
            Q_UNUSED_PAR(r);
#endif
        } while (m_temp.fun != t); // loop as long as tran.target not reached

        // retrace the entry path in reverse (correct) order...
        for (; ip >= 0; --ip) {
            // enter 'path[ip]'
            if ((*path[ip])(this, &l_resEvt_[Q_ENTRY_SIG]) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);//output QS record
            }
        }
        t = path[0]; // tran. target becomes the new source
    }
}

//............................................................................
bool QHsm::isIn(QStateHandler const stateHndl) noexcept {
    // this state machine must be in a stable state configuration
    // NOTE: stable state configuration is established after every RTC step.
    Q_INVARIANT_LOCAL(700,
        dis_verify<std::uintptr_t>(m_state.uint, m_temp.uint));

    bool inState = false; // assume that this HSM is NOT in 'stateHndl'

    // scan the state hierarchy bottom-up
    QStateHandler s = m_state.fun;
    QState r;
    do {
        if (s == stateHndl) { // do the states match?
            inState = true;  // 'true' means that match found
            break; // break out of the for-loop
        }

        // find superstate of 's'
        r = (*s)(this, &l_resEvt_[Q_EMPTY_SIG]);
        s = m_temp.fun;
    } while (r == Q_RET_SUPER);

#ifndef Q_UNSAFE
    // restore the invariant (stable state configuration)
    m_temp.uint = dis_update<std::uintptr_t>(m_state.uint);
#endif
    return inState; // return the status
}

//............................................................................
QStateHandler QHsm::childState(QStateHandler const parentHndl) noexcept {
#ifndef Q_UNSAFE
    bool isFound = false; // assume the child state NOT found
#endif

    QStateHandler child = m_state.fun; // start with current state
    m_temp.fun = child; // establish stable state configuration
    QState r = Q_RET_SUPER;
    do {
        // have the parent of the current child?
        if (m_temp.fun == parentHndl) {
#ifndef Q_UNSAFE
            isFound = true; // indicate that child state was found
#endif
            break;
        }
        child = m_temp.fun;

        // find superstate of 'child'
        r = (*child)(this, &l_resEvt_[Q_EMPTY_SIG]);
    } while (r == Q_RET_SUPER);
    Q_ENSURE_LOCAL(890, isFound);

    return child;
}

//............................................................................
QStateHandler QHsm::getStateHandler() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return m_state.fun; // public "getter" to the state handler (function)
}

} // namespace QP
