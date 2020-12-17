/// @file
/// @brief QP::QMsm implementation
/// @ingroup qep
/// @cond
///***************************************************************************
/// Last updated for version 6.9.2
/// Last updated on  2020-12-17
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#define QP_IMPL             // this is QP implementation
#include "qep_port.hpp"     // QEP port
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"        // QP embedded systems-friendly assertions

//! Internal macro to increment the given action table @p act_
/// @note Incrementing a pointer violates the MISRA-C 2004 Rule 17.4(req),
/// pointer arithmetic other than array indexing. Encapsulating this violation
/// in a macro allows to selectively suppress this specific deviation.
#define QEP_ACT_PTR_INC_(act_) (++(act_))

namespace QP {

Q_DEFINE_THIS_MODULE("qep_msm")

//****************************************************************************
QMState const QMsm::msm_top_s = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

//****************************************************************************
/// @description
/// Performs the first step of initialization by assigning the initial
/// pseudostate to the currently active state of the state machine.
///
/// @param[in] initial  the top-most initial transition for the MSM.
///
/// @note
/// The constructor is protected to prevent direct instantiating of the
/// QP::QMsm objects. This class is intended for subclassing only.
///
/// @sa
/// The QP::QMsm example illustrates how to use the QMsm constructor
/// in the constructor initializer list of the derived state machines.
///
QMsm::QMsm(QStateHandler const initial) noexcept
  : QHsm(initial)
{
    m_state.obj = &msm_top_s;
    m_temp.fun  = initial;
}

//****************************************************************************
/// @description
/// Executes the top-most initial transition in a MSM.
///
/// @param[in] e   pointer to an extra parameter (might be nullptr)
/// @param[in]     qs_id QS-id of this state machine (for QS local filter)
///
/// @attention
/// QP::QMsm::init() must be called exactly __once__ before
/// QP::QMsm::dispatch()
///
void QMsm::init(void const * const e, std::uint_fast8_t const qs_id) {
    QS_CRIT_STAT_

    /// @pre the top-most initial transition must be initialized, and the
    /// initial transition must not be taken yet.
    Q_REQUIRE_ID(200, (m_temp.fun != nullptr)
                      && (m_state.obj == &msm_top_s));

    // execute the top-most initial tran.
    QState r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    // initial tran. must be taken
    Q_ASSERT_ID(210, r == Q_RET_TRAN_INIT);

    QS_BEGIN_PRE_(QS_QEP_STATE_INIT, qs_id)
        QS_OBJ_PRE_(this);  // this state machine object
        QS_FUN_PRE_(m_state.obj->stateHandler);          // source handler
        QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target handler
    QS_END_PRE_()

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    do {
        r = execTatbl_(m_temp.tatbl, qs_id); // execute the tran-action table
    } while (r >= Q_RET_TRAN_INIT);

    QS_BEGIN_PRE_(QS_QEP_INIT_TRAN, qs_id)
        QS_TIME_PRE_();                         // time stamp
        QS_OBJ_PRE_(this);                      // this state machine object
        QS_FUN_PRE_(m_state.obj->stateHandler); // the new current state
    QS_END_PRE_()

    static_cast<void>(qs_id); // unused parameter (if Q_SPY not defined)
}

//****************************************************************************
/// @description
/// Dispatches an event for processing to a meta state machine (MSM).
/// The processing of an event represents one run-to-completion (RTC) step.
///
/// @param[in] e  pointer to the event to be dispatched to the MSM
/// @param[in] qs_id QS-id of this state machine (for QS local filter)
///
/// @note
/// Must be called after QP::QMsm::init().
///
void QMsm::dispatch(QEvt const * const e, std::uint_fast8_t const qs_id) {
    QMState const *s = m_state.obj;  // store the current state
    QMState const *t = s;
    QState r;
    QS_CRIT_STAT_

    /// @pre current state must be initialized
    Q_REQUIRE_ID(300, s != nullptr);

    QS_BEGIN_PRE_(QS_QEP_DISPATCH, qs_id)
        QS_TIME_PRE_();               // time stamp
        QS_SIG_PRE_(e->sig);          // the signal of the event
        QS_OBJ_PRE_(this);            // this state machine object
        QS_FUN_PRE_(s->stateHandler); // the current state handler
    QS_END_PRE_()

    // scan the state hierarchy up to the top state...
    do {
        r = (*t->stateHandler)(this, e); // call state handler function

        // event handled? (the most frequent case)
        if (r >= Q_RET_HANDLED) {
            break; // done scanning the state hierarchy
        }
        // event unhandled and passed to the superstate?
        else if (r == Q_RET_SUPER) {
            t = t->superstate; // advance to the superstate
        }
        // event unhandled and passed to a submachine superstate?
        else if (r == Q_RET_SUPER_SUB) {
            t = m_temp.obj; // current host state of the submachie
        }
        // event unhandled due to a guard?
        else if (r == Q_RET_UNHANDLED) {

            QS_BEGIN_PRE_(QS_QEP_UNHANDLED, qs_id)
                QS_SIG_PRE_(e->sig);    // the signal of the event
                QS_OBJ_PRE_(this);      // this state machine object
                QS_FUN_PRE_(t->stateHandler); // the current state
            QS_END_PRE_()

            t = t->superstate; // advance to the superstate
        }
        else {
            // no other return value should be produced
            Q_ERROR_ID(310);
        }
    } while (t != nullptr);

    // any kind of transition taken?
    if (r >= Q_RET_TRAN) {
#ifdef Q_SPY
        QMState const * const ts = t; // transition source for QS tracing

        // the transition source state must not be nullptr
        Q_ASSERT_ID(320, ts != nullptr);
#endif // Q_SPY

        do {
            // save the transition-action table before it gets clobbered
            QMTranActTable const * const tatbl = m_temp.tatbl;
            QHsmAttr tmp; // temporary to save intermediate values

            // was TRAN, TRAN_INIT, or TRAN_EP taken?
            if (r <= Q_RET_TRAN_EP) {
                exitToTranSource_(s, t, qs_id);
                r = execTatbl_(tatbl, qs_id);
                s = m_state.obj;
            }
            // was a transition segment to history taken?
            else if (r == Q_RET_TRAN_HIST) {
                tmp.obj = m_state.obj; // save history
                m_state.obj = s; // restore the original state
                exitToTranSource_(s, t, qs_id);
                static_cast<void>(execTatbl_(tatbl, qs_id));
                r = enterHistory_(tmp.obj, qs_id);
                s = m_state.obj;
            }
            // was a transition segment to an exit point taken?
            else if (r == Q_RET_TRAN_XP) {
                tmp.act = m_state.act; // save XP action
                m_state.obj = s; // restore the original state
                r = (*tmp.act)(this); // execute the XP action
                if (r == Q_RET_TRAN) { // XP -> TRAN ?
#ifdef Q_SPY
                    tmp.tatbl = m_temp.tatbl; // save m_temp
#endif // Q_SPY
                    exitToTranSource_(s, t, qs_id);
                    // take the tran-to-XP segment inside submachine
                    static_cast<void>(execTatbl_(tatbl, qs_id));
                    s = m_state.obj;
#ifdef Q_SPY
                    m_temp.tatbl = tmp.tatbl; // restore m_temp
#endif // Q_SPY
                }
                else if (r == Q_RET_TRAN_HIST) { // XP -> HIST ?
                    tmp.obj = m_state.obj; // save the history
                    m_state.obj = s; // restore the original state
#ifdef Q_SPY
                    s = m_temp.obj; // save m_temp
#endif // Q_SPY
                    exitToTranSource_(m_state.obj, t, qs_id);
                    // take the tran-to-XP segment inside submachine
                    static_cast<void>(execTatbl_(tatbl, qs_id));
#ifdef Q_SPY
                    m_temp.obj = s; // restore me->temp
#endif // Q_SPY
                    s = m_state.obj;
                    m_state.obj = tmp.obj; // restore the history
                }
                else {
                    // TRAN_XP must NOT be followed by any other tran type
                    Q_ASSERT_ID(330, r < Q_RET_TRAN);
                }
            }
            else {
                // no other return value should be produced
                Q_ERROR_ID(340);
            }

            t = s; // set target to the current state

        } while (r >= Q_RET_TRAN);

        QS_BEGIN_PRE_(QS_QEP_TRAN, qs_id)
            QS_TIME_PRE_();                // time stamp
            QS_SIG_PRE_(e->sig);           // the signal of the event
            QS_OBJ_PRE_(this);             // this state machine object
            QS_FUN_PRE_(ts->stateHandler); // the transition source
            QS_FUN_PRE_(s->stateHandler);  // the new active state
        QS_END_PRE_()
    }

#ifdef Q_SPY
    // was the event handled?
    else if (r == Q_RET_HANDLED) {
        // internal tran. source can't be nullptr
        Q_ASSERT_ID(340, t != nullptr);

        QS_BEGIN_PRE_(QS_QEP_INTERN_TRAN, qs_id)
            QS_TIME_PRE_();               // time stamp
            QS_SIG_PRE_(e->sig);          // the signal of the event
            QS_OBJ_PRE_(this);            // this state machine object
            QS_FUN_PRE_(t->stateHandler); // the source state
        QS_END_PRE_()

    }
    // event bubbled to the 'top' state?
    else if (t == nullptr) {

        QS_BEGIN_PRE_(QS_QEP_IGNORED, qs_id)
            QS_TIME_PRE_();               // time stamp
            QS_SIG_PRE_(e->sig);          // the signal of the event
            QS_OBJ_PRE_(this);            // this state machine object
            QS_FUN_PRE_(s->stateHandler); // the current state
        QS_END_PRE_()

    }
#endif // Q_SPY

    else {
        // empty
    }

    static_cast<void>(qs_id); // unused parameter (if Q_SPY not defined)
}

//****************************************************************************
#ifdef Q_SPY
    /// @description
    /// Helper function to get the current state handler of QMsm.
    ///
    QStateHandler QMsm::getStateHandler() noexcept {
        return m_state.obj->stateHandler;
    }
#endif

//****************************************************************************
/// @description
/// Helper function to execute transition sequence in a tran-action table.
///
/// @param[in] tatbl pointer to the transition-action table
/// @param[in] qs_id QS-id of this state machine (for QS local filter)
///
/// @returns
/// the status of the last action from the transition-action table.
///
/// @note
/// This function is for internal use inside the QEP event processor and
/// should __not__ be called directly from the applications.
///
QState QMsm::execTatbl_(QMTranActTable const * const tatbl,
                        std::uint_fast8_t const qs_id)
{
    QActionHandler const *a;
    QState r = Q_RET_NULL;
    QS_CRIT_STAT_

    /// @pre the transition-action table pointer must not be nullptr
    Q_REQUIRE_ID(400, tatbl != nullptr);

    for (a = &tatbl->act[0]; *a != nullptr; QEP_ACT_PTR_INC_(a)) {
        r = (*(*a))(this); // call the action through the 'a' pointer
#ifdef Q_SPY
        if (r == Q_RET_ENTRY) {

            QS_BEGIN_PRE_(QS_QEP_STATE_ENTRY, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(m_temp.obj->stateHandler); // entered state handler
            QS_END_PRE_()
        }
        else if (r == Q_RET_EXIT) {

            QS_BEGIN_PRE_(QS_QEP_STATE_EXIT, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(m_temp.obj->stateHandler); // exited state handler
            QS_END_PRE_()
        }
        else if (r == Q_RET_TRAN_INIT) {

            QS_BEGIN_PRE_(QS_QEP_STATE_INIT, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(tatbl->target->stateHandler);        // source
                QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target
            QS_END_PRE_()
        }
        else if (r == Q_RET_TRAN_EP) {

            QS_BEGIN_PRE_(QS_QEP_TRAN_EP, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(tatbl->target->stateHandler);        // source
                QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target
            QS_END_PRE_()
        }
        else if (r == Q_RET_TRAN_XP) {

            QS_BEGIN_PRE_(QS_QEP_TRAN_XP, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(tatbl->target->stateHandler);        // source
                QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target
            QS_END_PRE_()
        }
        else {
            // empty
        }
#endif // Q_SPY
    }

    static_cast<void>(qs_id); // unused parameter (if Q_SPY not defined)

    m_state.obj = (r >= Q_RET_TRAN)
        ? m_temp.tatbl->target
        : tatbl->target;
    return r;
}

//****************************************************************************
/// @description
/// Helper function to exit the current state configuration to the
/// transition source, which is a hierarchical state machine might be a
/// superstate of the current state.
///
/// @param[in] s    pointer to the current state
/// @param[in] ts   pointer to the transition source state
/// @param[in] qs_id QS-id of this state machine (for QS local filter)
///
void QMsm::exitToTranSource_(QMState const *s,
                             QMState const * const ts,
                             std::uint_fast8_t const qs_id)
{
    // exit states from the current state to the tran. source state
    while (s != ts) {
        // exit action provided in state 's'?
        if (s->exitAction != nullptr) {
            // execute the exit action
            static_cast<void>((*s->exitAction)(this));

            QS_CRIT_STAT_
            QS_BEGIN_PRE_(QS_QEP_STATE_EXIT, qs_id)
                QS_OBJ_PRE_(this);            // this state machine object
                QS_FUN_PRE_(s->stateHandler); // the exited state handler
            QS_END_PRE_()
        }

        s = s->superstate; // advance to the superstate

        // reached the top of a submachine?
        if (s == nullptr) {
            s = m_temp.obj; // the superstate from QM_SM_EXIT()
            Q_ASSERT_ID(510, s != nullptr);
        }
    }
    static_cast<void>(qs_id); // unused parameter (if Q_SPY not defined)
}

//****************************************************************************
/// @description
/// Static helper function to execute the segment of transition to history
/// after entering the composite state and
///
/// @param[in] hist pointer to the history substate
/// @param[in] qs_id QS-id of this state machine (for QS local filter)
///
/// @returns
/// QP::Q_RET_INIT, if an initial transition has been executed in the last
/// entered state or QP::Q_RET_NULL if no such transition was taken.
///
QState QMsm::enterHistory_(QMState const * const hist,
                           std::uint_fast8_t const qs_id)
{
    QMState const *s = hist;
    QMState const *ts = m_state.obj; // transition source
    QMState const *epath[MAX_ENTRY_DEPTH_];
    QState r;
    std::uint_fast8_t i = 0U;  // entry path index
    QS_CRIT_STAT_

    QS_BEGIN_PRE_(QS_QEP_TRAN_HIST, qs_id)
        QS_OBJ_PRE_(this);               // this state machine object
        QS_FUN_PRE_(ts->stateHandler);   // source state handler
        QS_FUN_PRE_(hist->stateHandler); // target state handler
    QS_END_PRE_()

    while (s != ts) {
        if (s->entryAction != nullptr) {
            epath[i] = s;
            ++i;
            Q_ASSERT_ID(620,
                        i <= static_cast<std::uint_fast8_t>(Q_DIM(epath)));
        }
        s = s->superstate;
        if (s == nullptr) {
            ts = s; // force exit from the for-loop
        }
    }

    // retrace the entry path in reverse (desired) order...
    while (i > 0U) {
        --i;
        // run entry action in epath[i]
        static_cast<void>((*epath[i]->entryAction)(this));

        QS_BEGIN_PRE_(QS_QEP_STATE_ENTRY, qs_id)
            QS_OBJ_PRE_(this);
            QS_FUN_PRE_(epath[i]->stateHandler); // entered state handler
        QS_END_PRE_()
    }

    m_state.obj = hist; // set current state to the transition target

    // initial tran. present?
    if (hist->initAction != nullptr) {
        r = (*hist->initAction)(this); // execute the transition action
    }
    else {
        r = Q_RET_NULL;
    }

    static_cast<void>(qs_id); // unused parameter (if Q_SPY not defined)
    return r;
}

//****************************************************************************
/// @description
/// Tests if a state machine derived from QMsm is-in a given state.
///
/// @note
/// For a MSM, to "be-in" a state means also to "be-in" a superstate of
/// of the state.
///
/// @param[in] st  pointer to the QMState object that corresponds to the
///                tested state.
/// @returns
/// 'true' if the MSM is in the \c st and 'false' otherwise
///
bool QMsm::isInState(QMState const * const st) const noexcept {
    bool inState = false; // assume that this MSM is not in 'state'

    for (QMState const *s = m_state.obj;
         s != nullptr;
         s = s->superstate)
    {
        if (s == st) {
            inState = true; // match found, return 'true'
            break;
        }
    }
    return inState;
}

//****************************************************************************
///
/// @description
/// Finds the child state of the given @c parent, such that this child state
/// is an ancestor of the currently active state. The main purpose of this
/// function is to support **shallow history** transitions in state machines
/// derived from QMsm.
///
/// @param[in] parent pointer to the state-handler object
///
/// @returns
/// the child of a given @c parent state, which is an ancestor of the
/// currently active state. For the corner case when the currently active
/// state is the given @c parent state, function returns the @c parent state.
///
QMState const *QMsm::childStateObj(QMState const * const parent)
    const noexcept
{
    QMState const *child = m_state.obj;
    bool isFound = false; // start with the child not found

    for (QMState const *s = m_state.obj;
         s != nullptr;
         s = s->superstate)
    {
        if (s == parent) {
            isFound = true; // child is found
            break;
        }
        else {
            child = s;
        }
    }

    /// @post the child must be found
    Q_ENSURE_ID(810, isFound);
#ifdef Q_NASSERT
    // avoid compiler warning about unused variable
    static_cast<void>(isFound);
#endif

    return child; // return the child
}

} // namespace QP

