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

Q_DEFINE_THIS_MODULE("qep_hsm")

// maximum depth of state nesting in a QHsm (including the top level)
// must be >= 3
static constexpr std::int_fast8_t QHSM_MAX_NEST_DEPTH_ {6};

//! @cond INTERNAL

// immutable events corresponding to the reserved signals.
static QP::QEvt const l_reservedEvt_[4] {
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
#define QHSM_RESERVED_EVT_(state_, sig_) \
    ((*(state_))(this, &l_reservedEvt_[(sig_)]))

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

// internal helper macro to trace transition action
#define QS_TRAN_ACT_(rec_, state_)     \
    QS_CRIT_ENTRY();                   \
    QS_BEGIN_PRE((rec_), qsId)         \
        QS_SIG_PRE(e->sig);            \
        QS_OBJ_PRE(this);              \
        QS_FUN_PRE(state_);            \
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
#define QS_TRAN_ACT_(rec_, state_)      (static_cast<void>(0))
#define QS_TRAN0_(rec_, trg_)           (static_cast<void>(0))
#define QS_TRAN_END_(rec_, src_, trg_)  (static_cast<void>(0))
#endif

//! @endcond

namespace QP {

//............................................................................
QHsm::QHsm(QStateHandler const initial) noexcept
: QAsm()
{
    m_state.fun = &top;
    m_temp.fun  = initial;
}

//............................................................................
void QHsm::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
    QF_CRIT_STAT

    // produce QS dictionary for QP::QHsm::top()
#ifdef Q_SPY
    QS_CRIT_ENTRY();
    bool isDone = true;
    if ((QS::priv_.flags & 0x01U) == 0U) {
        QS::priv_.flags |= 0x01U;
        isDone = false;
    }
    QS_CRIT_EXIT();
    if (!isDone) {
        QS_FUN_DICTIONARY(&QP::QHsm::top);
    }
#else
    Q_UNUSED_PAR(qsId);
#endif // def Q_SPY

    QStateHandler t = m_state.fun;

    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(200,
        (m_temp.fun != nullptr)
        && (t == Q_STATE_CAST(&top)));
    QF_CRIT_EXIT();

    // execute the top-most initial tran.
    QState r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    QF_CRIT_ENTRY();
    // the top-most initial tran. must be taken
    Q_ASSERT_INCRIT(210, r == Q_RET_TRAN);
    QF_CRIT_EXIT();

    QS_TRAN_SEG_(QS_QEP_STATE_INIT, t, m_temp.fun);

    // drill down into the state hierarchy with initial transitions...
    QStateHandler path[QHSM_MAX_NEST_DEPTH_]; // tran. entry path array
    path[0] = m_temp.fun;
    static_cast<void>(QHSM_RESERVED_EVT_(m_temp.fun, Q_EMPTY_SIG));

    std::int_fast8_t ip = 1; // tran. entry path index (also the loop bound)
    for (; (m_temp.fun != t) && (ip < QHSM_MAX_NEST_DEPTH_); ++ip) {
        path[ip] = m_temp.fun;
        static_cast<void>(QHSM_RESERVED_EVT_(m_temp.fun, Q_EMPTY_SIG));
    }
    QF_CRIT_ENTRY();
    // must NOT be too many state nesting levels or "malformed" HSM
    Q_ASSERT_INCRIT(220, ip <= QHSM_MAX_NEST_DEPTH_);
    QF_CRIT_EXIT();

    m_temp.fun = path[0];
    enter_target_(&path[0], ip - 1, qsId);
    t = path[0];

    QS_TOP_INIT_(QS_QEP_INIT_TRAN, t);

    m_state.fun = t; // change the current active state
#ifdef Q_UNSAFE
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

    QStateHandler s = m_state.fun;
    QStateHandler t = s;
    QF_CRIT_STAT

    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300,
        (e != nullptr)
        && (s != nullptr));
    QF_CRIT_EXIT();

    QS_TRAN0_(QS_QEP_DISPATCH, s);

    QState r = Q_RET_SUPER;

    // process the event hierarchically...
    m_temp.fun = s;
    std::int_fast8_t ip = QHSM_MAX_NEST_DEPTH_;
    // NOTE: ip is the fixed loop upper bound
    for (; ip > 0; --ip) {
        s = m_temp.fun;
        r = (*s)(this, e); // invoke state handler s

        if (r == Q_RET_UNHANDLED) { // unhandled due to a guard?
            QS_TRAN_ACT_(QS_QEP_UNHANDLED, s);
            r = QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG); // superstate of s
        }
        if (r != Q_RET_SUPER) { // event NOT "bubbled up"
            break;
        }
    }
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(310, ip > 0);
    QF_CRIT_EXIT();

    if (r >= Q_RET_TRAN) { // tran. (regular or history) taken?
#ifdef Q_SPY
        if (r == Q_RET_TRAN_HIST) { // tran. to history?
            QS_TRAN_SEG_(QS_QEP_TRAN_HIST, s, m_temp.fun);
        }
#endif // Q_SPY

        QStateHandler path[QHSM_MAX_NEST_DEPTH_];
        path[0] = m_temp.fun; // tran. target
        path[1] = t; // current state
        path[2] = s; // tran. source

        // exit current state to tran. source s...
        while (t != s) {
            // exit from t
            if (QHSM_RESERVED_EVT_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_EXIT, t);
                // find superstate of t
                static_cast<void>(QHSM_RESERVED_EVT_(t, Q_EMPTY_SIG));
            }
            t = m_temp.fun;
        }

        // take the tran...
        ip = tran_simple_(&path[0], qsId);
        if (ip < -1) { // not a simple tran.?
            ip = tran_complex_(&path[0], qsId);
        }

        enter_target_(&path[0], ip, qsId);
        t = path[0];
        QS_TRAN_END_(QS_QEP_TRAN, s, t);
    }
#ifdef Q_SPY
    else if (r == Q_RET_HANDLED) {
        QS_TRAN0_(QS_QEP_INTERN_TRAN, s);
    }
    else {
        QS_TRAN0_(QS_QEP_IGNORED, m_state.fun);
    }
#endif // Q_SPY

    m_state.fun = t; // change the current active state
}

//............................................................................
bool QHsm::isIn(QStateHandler const state) noexcept {
    bool inState = false; // assume that this HSM is not in 'state'

    // scan the state hierarchy bottom-up
    QStateHandler s = m_state.fun;
    QState r = Q_RET_SUPER;
    while (r != Q_RET_IGNORED) {
        if (s == state) { // do the states match?
            inState = true;  // 'true' means that match found
            break; // break out of the for-loop
        }
        r = QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG);
        s = m_temp.fun;
    }
    return inState; // return the status
}

//............................................................................
QStateHandler QHsm::childState(QStateHandler const parent) noexcept {
#ifndef Q_UNSAFE
    bool isFound = false; // assume the child state NOT found
#endif

    QStateHandler child = m_state.fun; // start with current state
    m_temp.fun = child; // establish stable state configuration
    QState r = Q_RET_SUPER;
    while (r != Q_RET_IGNORED) {
        // have the parent of the current child?
        if (m_temp.fun == parent) {
#ifndef Q_UNSAFE
            isFound = true; // indicate that child state was found
#endif
            break;
        }
        child = m_temp.fun;
        r = QHSM_RESERVED_EVT_(child, Q_EMPTY_SIG);
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(590, isFound);
    QF_CRIT_EXIT();

    return child;
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

    QStateHandler t = path[0];
    QStateHandler const s = path[2];
    std::int_fast8_t ip = 0; // tran. entry path index
    QS_CRIT_STAT

    // (a) check source==target (tran. to self)...
    if (s == t) {
        // exit source s
        if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
        }
        ip = 0; // enter the target
    }
    else {
        // find superstate of target
        static_cast<void>(QHSM_RESERVED_EVT_(t, Q_EMPTY_SIG));

        t = m_temp.fun;

        // (b) check source==target->super...
        if (s == t) {
            ip = 0; // enter the target
        }
        else {
            // find superstate of src
            static_cast<void>(QHSM_RESERVED_EVT_(s, Q_EMPTY_SIG));

            // (c) check source->super==target->super...
            if (m_temp.fun == t) {
                // exit source s
                if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                }
                ip = 0; // enter the target
            }
            // (d) check source->super==target...
            else if (m_temp.fun == path[0]) {
                // exit source s
                if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
                }
                ip = -1; // do not enter the target
            }
            else {
                path[1] = t; // save the superstate of target
                ip = -2; // cause execution of complex tran.
            }
        }
    }
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

    // (e) check rest of source==target->super->super..
    // and store the entry path along the way
    std::int_fast8_t iq = 0; // indicate that LCA was found
    std::int_fast8_t ip = 1; // enter target and its superstate
    QStateHandler const s = path[2]; // source state
    QStateHandler t = m_temp.fun;  // source->super
    QF_CRIT_STAT

    // find target->super->super...
    // note: ip is the fixed upper loop bound
    QState r = QHSM_RESERVED_EVT_(path[1], Q_EMPTY_SIG);
    while ((r == Q_RET_SUPER) && (ip < (QHSM_MAX_NEST_DEPTH_ - 1))) {
        ++ip;
        path[ip] = m_temp.fun; // store the entry path
        if (m_temp.fun == s) { // is it the source?
            iq = 1; // indicate that the LCA found
            --ip;   // do not enter the source
            break;  // terminate the loop
        }
        r = QHSM_RESERVED_EVT_(m_temp.fun, Q_EMPTY_SIG);
    }
    QF_CRIT_ENTRY();
    Q_INVARIANT_INCRIT(711, ip < (QHSM_MAX_NEST_DEPTH_ - 1));
    QF_CRIT_EXIT();

    // the LCA not found yet?
    if (iq == 0) {

        // exit source s
#ifndef Q_SPY
        static_cast<void>(QHSM_RESERVED_EVT_(s, Q_EXIT_SIG));
#else
        if (QHSM_RESERVED_EVT_(s, Q_EXIT_SIG) == Q_RET_HANDLED) {
            QS_STATE_ACT_(QS_QEP_STATE_EXIT, s);
        }
#endif // def Q_SPY

        // (f) check the rest of
        // source->super == target->super->super...
        iq = ip;
        r = Q_RET_IGNORED; // indicate that the LCA NOT found
        // note: iq is the fixed upper loop bound
        do {
            if (t == path[iq]) { // is this the LCA?
                r = Q_RET_HANDLED; // indicate the LCA found
                ip = iq - 1; // do not enter the LCA
                break; // terminate the loop
            }
            --iq; // try lower superstate of target
        } while (iq >= 0);

        if (r != Q_RET_HANDLED) { // the LCA still not found?
            // (g) check each source->super->...
            // for each target->super...
            r = Q_RET_SUPER; // keep looping
            while (r != Q_RET_HANDLED) {
                // exit from t
                if (QHSM_RESERVED_EVT_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {
                    QS_STATE_ACT_(QS_QEP_STATE_EXIT, t);
                    // find superstate of t
                    static_cast<void>(QHSM_RESERVED_EVT_(t, Q_EMPTY_SIG));
                }
                t = m_temp.fun; // set to super of t
                iq = ip;
                do {
                    if (t == path[iq]) { // is this the LCA?
                        ip = iq - 1; // do not enter the LCA
                        r = Q_RET_HANDLED; // break outer loop
                        break;  // terminate the inner loop
                    }
                    --iq; // try lower superstate of target
                } while (iq >= 0);
            }
        }
    }
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

    QF_CRIT_STAT

    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(800, depth < QHSM_MAX_NEST_DEPTH_);
    QF_CRIT_EXIT();

    std::int_fast8_t ip = depth;
    // execute state entry actions in the desired order...
    // note: ip is the fixed upper loop bound
    for (; ip >= 0; --ip) {
        // enter path[ip]
        if (QHSM_RESERVED_EVT_(path[ip], Q_ENTRY_SIG)
            == Q_RET_HANDLED)
        {
            QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);
        }
    }
    QStateHandler t = path[0];
    m_temp.fun = t; // update the next state

    // drill into the target hierarchy...
    while (QHSM_RESERVED_EVT_(t, Q_INIT_SIG) == Q_RET_TRAN) {

        QS_TRAN_SEG_(QS_QEP_STATE_INIT, t, m_temp.fun);

        ip = 0;
        path[0] = m_temp.fun;

        // find superstate
        static_cast<void>(QHSM_RESERVED_EVT_(m_temp.fun, Q_EMPTY_SIG));

        // note: ip is the fixed upper loop bound
        while ((m_temp.fun != t) && (ip < (QHSM_MAX_NEST_DEPTH_ - 1))) {
            ++ip;
            path[ip] = m_temp.fun;
            // find superstate
            static_cast<void>(QHSM_RESERVED_EVT_(
                m_temp.fun, Q_EMPTY_SIG));
        }
        QF_CRIT_ENTRY();
        // too many state nesting levels or "malformed" HSM
        Q_INVARIANT_INCRIT(891, ip < QHSM_MAX_NEST_DEPTH_);
        QF_CRIT_EXIT();

        m_temp.fun = path[0];

        // retrace the entry path in reverse (correct) order...
        // note: ip is the fixed upper loop bound
        do {
            // enter path[ip]
            if (QHSM_RESERVED_EVT_(path[ip], Q_ENTRY_SIG) == Q_RET_HANDLED) {
                QS_STATE_ACT_(QS_QEP_STATE_ENTRY, path[ip]);
            }
            --ip;
        } while (ip >= 0);

        t = path[0]; // current state becomes the new source
    }
}

} // namespace QP
