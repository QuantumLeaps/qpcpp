/// @file
/// @brief QP::QMsm implementation
/// @ingroup qep
/// @cond
///***************************************************************************
/// Last updated for version 5.8.0
/// Last updated on  2016-11-21
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps. All rights reserved.
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qep_port.h"     // QEP port
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"      // QP embedded systems-friendly assertions

//! Internal macro to increment the given action table @p act_
/// @note Incrementing a pointer violates the MISRA-C 2004 Rule 17.4(req),
/// pointer arithmetic other than array indexing. Encapsulating this violation
/// in a macro allows to selectively suppress this specific deviation.
#define QEP_ACT_PTR_INC_(act_) (++(act_))

namespace QP {

Q_DEFINE_THIS_MODULE("qep_msm")


//****************************************************************************
char_t const versionStr[6] = QP_VERSION_STR;

//****************************************************************************
QMState const QMsm::msm_top_s = {
    static_cast<QMState const *>(0),
    Q_STATE_CAST(0),
    Q_ACTION_CAST(0),
    Q_ACTION_CAST(0),
    Q_ACTION_CAST(0)
};

//****************************************************************************
/// @description
/// Performs the first step of initialization by assigning the initial
/// pseudostate to the currently active state of the state machine.
///
/// @param[in] initial  the top-most initial transition for the MSM.
///
/// @note The constructor is protected to prevent direct instantiating
/// of the QP::QMsm objects. This class is intended for subclassing only.
///
/// @sa The QP::QMsm example illustrates how to use the QMsm constructor
/// in the constructor initializer list of the derived state machines.
///
QMsm::QMsm(QStateHandler const initial)
  : QHsm(initial)
{
    m_state.obj = &msm_top_s;
    m_temp.fun  = initial;
}

//****************************************************************************
/// @description
/// Executes the top-most initial transition in a MSM.
///
/// @param[in] e  a constant pointer to QP::QEvt or a subclass of QP::QEvt
///
/// @attention
/// QP::QMsm::init() must be called exactly __once__ before
/// QP::QMsm::dispatch()
///
void QMsm::init(QEvt const * const e) {
    QS_CRIT_STAT_

    /// @pre the top-most initial transition must be initialized, and the
    /// initial transition must not be taken yet.
    Q_REQUIRE_ID(200, (m_temp.fun != Q_STATE_CAST(0))
                      && (m_state.obj == &msm_top_s));

    QState r = (*m_temp.fun)(this, e); // execute the top-most initial tran.

    // initial tran. must be taken
    Q_ASSERT_ID(210, r == Q_RET_TRAN_INIT);

    QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
        QS_OBJ_(this);  // this state machine object
        QS_FUN_(m_state.obj->stateHandler);          // source state handler
        QS_FUN_(m_temp.tatbl->target->stateHandler); // target state handler
    QS_END_()

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    do {
        r = execTatbl_(m_temp.tatbl); // execute the transition-action table
    } while (r >= Q_RET_TRAN_INIT);

    QS_BEGIN_(QS_QEP_INIT_TRAN, QS::priv_.smObjFilter, this)
        QS_TIME_();                         // time stamp
        QS_OBJ_(this);                      // this state machine object
        QS_FUN_(m_state.obj->stateHandler); // the new current state
    QS_END_()
}

//****************************************************************************
/// @description
/// Dispatches an event for processing to a meta state machine (MSM).
/// The processing of an event represents one run-to-completion (RTC) step.
///
/// @param[in] e  pointer to the event to be dispatched to the MSM
///
/// @note Must be called after QP::QMsm::init().
///
void QMsm::dispatch(QEvt const * const e) {
    QMState const *s = m_state.obj;  // store the current state
    QMState const *t = s;
    QState r = Q_RET_SUPER;
    QS_CRIT_STAT_

    /// @pre current state must be initialized
    Q_REQUIRE_ID(300, s != static_cast<QMState const *>(0));

    QS_BEGIN_(QS_QEP_DISPATCH, QS::priv_.smObjFilter, this)
        QS_TIME_();                  // time stamp
        QS_SIG_(e->sig);             // the signal of the event
        QS_OBJ_(this);               // this state machine object
        QS_FUN_(s->stateHandler);    // the current state handler
    QS_END_()

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

            QS_BEGIN_(QS_QEP_UNHANDLED, QS::priv_.smObjFilter, this)
                QS_SIG_(e->sig);    // the signal of the event
                QS_OBJ_(this);      // this state machine object
                QS_FUN_(t->stateHandler); // the current state
            QS_END_()

            t = t->superstate; // advance to the superstate
        }
        else {
            // no other return value should be produced
            Q_ERROR_ID(310);
        }
    } while (t != static_cast<QMState const *>(0));

    // any kind of transition taken?
    if (r >= Q_RET_TRAN) {
#ifdef Q_SPY
        QMState const *ts = t; // transition source for QS tracing

        // the transition source state must not be NULL
        Q_ASSERT_ID(320, ts != static_cast<QMState const *>(0));
#endif // Q_SPY

        do {
            // save the transition-action table before it gets clobbered
            QMTranActTable const *tatbl = m_temp.tatbl;

            // was a regular state transition segment taken?
            if (r == Q_RET_TRAN) {
                exitToTranSource_(s, t);
                r = execTatbl_(tatbl);
            }
            // was an initial transition segment taken?
            else if (r == Q_RET_TRAN_INIT) {
                r = execTatbl_(tatbl);
            }
            // was a transition segment to history taken?
            else if (r == Q_RET_TRAN_HIST) {
                QMState const *hist = m_state.obj; // save history
                m_state.obj = s; // restore the original state
                exitToTranSource_(s, t);
                (void)execTatbl_(tatbl);
                r = enterHistory_(hist);
            }
            // was a transition segment to an entry point taken?
            else if (r == Q_RET_TRAN_EP) {
                r = execTatbl_(tatbl);
            }
            // was a transition segment to an exit point taken?
            else if (r == Q_RET_TRAN_XP) {
                QActionHandler const act = m_state.act; // save XP action
                m_state.obj = s; // restore the original state

                r = (*act)(this); // execute the XP action
                if (r == Q_RET_TRAN) {
                    exitToTranSource_(s, t);
                    // take the tran-to-XP segment inside submachine
                    (void)execTatbl_(tatbl);
                    m_state.obj = s; // restore original state (history)
#ifdef Q_SPY
                    t = m_temp.tatbl->target; // store for tracing
#endif /* Q_SPY */
                    // take the XP-Segment from submachine-state
                    r = execTatbl_(m_temp.tatbl);

                    QS_BEGIN_(QS_QEP_TRAN_XP, QS::priv_.smObjFilter, this)
                        QS_OBJ_(this); // this state machine object
                        QS_FUN_(s);    // source handler
                        QS_FUN_(t);    // target handler
                    QS_END_()
                }
            }
            else {
                // no other return value should be produced
                Q_ERROR_ID(330);
            }
            s = m_state.obj;
            t = s;

        } while (r >= Q_RET_TRAN);

        QS_BEGIN_(QS_QEP_TRAN, QS::priv_.smObjFilter, this)
            QS_TIME_();                // time stamp
            QS_SIG_(e->sig);           // the signal of the event
            QS_OBJ_(this);             // this state machine object
            QS_FUN_(ts->stateHandler); // the transition source
            QS_FUN_(s->stateHandler);  // the new active state
        QS_END_()
    }

#ifdef Q_SPY
    // was the event handled?
    else if (r == Q_RET_HANDLED) {
        // internal tran. source can't be NULL
        Q_ASSERT_ID(340, t != static_cast<QMState const *>(0));

        QS_BEGIN_(QS_QEP_INTERN_TRAN, QS::priv_.smObjFilter, this)
            QS_TIME_();               // time stamp
            QS_SIG_(e->sig);          // the signal of the event
            QS_OBJ_(this);            // this state machine object
            QS_FUN_(t->stateHandler); // the source state
        QS_END_()

    }
    // event bubbled to the 'top' state?
    else if (t == static_cast<QMState const *>(0)) {

        QS_BEGIN_(QS_QEP_IGNORED, QS::priv_.smObjFilter, this)
            QS_TIME_();               // time stamp
            QS_SIG_(e->sig);          // the signal of the event
            QS_OBJ_(this);            // this state machine object
            QS_FUN_(s->stateHandler); // the current state
        QS_END_()

    }
#endif // Q_SPY

    else {
        // empty
    }
}

//****************************************************************************
/// @description
/// Helper function to execute transition sequence in a tran-action table.
///
/// @param[in] tatbl pointer to the transition-action table
///
/// @returns status of the last action from the transition-action table.
///
/// @note
/// This function is for internal use inside the QEP event processor and
/// should __not__ be called directly from the applications.
///
QState QMsm::execTatbl_(QMTranActTable const * const tatbl) {
    QActionHandler const *a;
    QState r = Q_RET_NULL;
    QS_CRIT_STAT_

    /// @pre the transition-action table pointer must not be NULL
    Q_REQUIRE_ID(400, tatbl != static_cast<QMTranActTable const *>(0));

    for (a = &tatbl->act[0]; *a != Q_ACTION_CAST(0); QEP_ACT_PTR_INC_(a)) {
        r = (*(*a))(this); // call the action through the 'a' pointer
#ifdef Q_SPY
        if (r == Q_RET_ENTRY) {

            QS_BEGIN_(QS_QEP_STATE_ENTRY, QS::priv_.smObjFilter, this)
                QS_OBJ_(this); // this state machine object
                QS_FUN_(m_temp.obj->stateHandler); // entered state handler
            QS_END_()
        }
        else if (r == Q_RET_EXIT) {

            QS_BEGIN_(QS_QEP_STATE_EXIT, QS::priv_.smObjFilter, this)
                QS_OBJ_(this); // this state machine object
                QS_FUN_(m_temp.obj->stateHandler); // exited state handler
            QS_END_()
        }
        else if (r == Q_RET_TRAN_INIT) {

            QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
                QS_OBJ_(this); // this state machine object
                QS_FUN_(tatbl->target->stateHandler);        // source
                QS_FUN_(m_temp.tatbl->target->stateHandler); // target
            QS_END_()
        }
        else if (r == Q_RET_TRAN_EP) {

            QS_BEGIN_(QS_QEP_TRAN_EP, QS::priv_.smObjFilter, this)
                QS_OBJ_(this); // this state machine object
                QS_FUN_(tatbl->target->stateHandler);        // source
                QS_FUN_(m_temp.tatbl->target->stateHandler); // target
            QS_END_()
        }
        else if (r == Q_RET_TRAN_XP) {

            QS_BEGIN_(QS_QEP_TRAN_XP, QS::priv_.smObjFilter, this)
                QS_OBJ_(this); // this state machine object
                QS_FUN_(tatbl->target->stateHandler);        // source
                QS_FUN_(m_temp.tatbl->target->stateHandler); // target
            QS_END_()
        }
        else {
            // empty
        }
#endif // Q_SPY
    }

    if (r >= Q_RET_TRAN_INIT) {
        m_state.obj = m_temp.tatbl->target; // the tran. target
    }
    else {
        m_state.obj = tatbl->target; // the tran. target
    }

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
///
void QMsm::exitToTranSource_(QMState const *s,
                             QMState const * const ts)
{
    // exit states from the current state to the tran. source state
    while (s != ts) {
        // exit action provided in state 's'?
        if (s->exitAction != Q_ACTION_CAST(0)) {
            (void)(*s->exitAction)(this); // execute the exit action

            QS_CRIT_STAT_
            QS_BEGIN_(QS_QEP_STATE_EXIT, QS::priv_.smObjFilter, this)
                QS_OBJ_(this);            // this state machine object
                QS_FUN_(s->stateHandler); // the exited state handler
            QS_END_()
        }

        s = s->superstate; // advance to the superstate

        // reached the top of a submachine?
        if (s == static_cast<QMState const *>(0)) {
            s = m_temp.obj; // the superstate from QM_SM_EXIT()
            Q_ASSERT_ID(510, s != static_cast<QMState const *>(0));
        }
    }
}

//****************************************************************************
/// @description
/// Static helper function to execute the segment of transition to history
/// after entering the composite state and
///
/// @param[in] hist pointer to the history substate
///
/// @returns QP::Q_RET_INIT, if an initial transition has been executed in
/// the last entered state or QP::Q_RET_NULL if no such transition was taken.
///
QState QMsm::enterHistory_(QMState const * const hist) {
    QMState const *s = hist;
    QMState const *ts = m_state.obj; // transition source
    QMState const *epath[MAX_ENTRY_DEPTH_];
    QState r;
    uint_fast8_t i = static_cast<uint_fast8_t>(0);  // entry path index
    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_TRAN_HIST, QS::priv_.smObjFilter, this)
        QS_OBJ_(this);               // this state machine object
        QS_FUN_(ts->stateHandler);   // source state handler
        QS_FUN_(hist->stateHandler); // target state handler
    QS_END_()

    while (s != ts) {
        if (s->entryAction != static_cast<QActionHandler>(0)) {
            epath[i] = s;
            ++i;
            Q_ASSERT_ID(620, i <= static_cast<uint_fast8_t>(Q_DIM(epath)));
        }
        s = s->superstate;
        if (s == 0) {
            ts = s; /* force exit from the for-loop */
        }
    }

    // retrace the entry path in reverse (desired) order...
    while (i > static_cast<uint_fast8_t>(0)) {
        --i;
        r = (*epath[i]->entryAction)(this); // run entry action in epath[i]

        QS_BEGIN_(QS_QEP_STATE_ENTRY, QS::priv_.smObjFilter, this)
            QS_OBJ_(this);
            QS_FUN_(epath[i]->stateHandler); // entered state handler
        QS_END_()
    }

    m_state.obj = hist; // set current state to the transition target

    // initial tran. present?
    if (hist->initAction != static_cast<QActionHandler>(0)) {
        r = (*hist->initAction)(this); // execute the transition action
    }
    else {
        r = Q_RET_NULL;
    }
    return r;
}

//****************************************************************************
/// \description
/// Tests if a state machine derived from QMsm is-in a given state.
///
/// \note For a MSM, to "be-in" a state means also to "be-in" a superstate of
/// of the state.
///
/// \arguments
/// \arg[in] \c st pointer to the QMState object that corresponds to the
///                tested state.
///
/// \returns 'true' if the MSM is in the \c st and 'false' otherwise
///
bool QMsm::isInState(QMState const * const st) const {
    bool inState = false; // assume that this MSM is not in 'state'

    for (QMState const *s = m_state.obj;
         s != static_cast<QMState const *>(0);
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
/// @returns the child of a given @c parent state, which is an ancestor of
/// the currently active state
///
QMState const *QMsm::childStateObj(QMState const * const parent) const {
    QMState const *child = m_state.obj;
    bool isConfirmed = false; // start with the child not confirmed
    QMState const *s;

    for (s = m_state.obj->superstate;
         s != static_cast<QMState const *>(0);
         s = s->superstate)
    {
        if (s == parent) {
            isConfirmed = true; // child is confirmed
            break;
        }
        else {
            child = s;
        }
    }

    /// @post the child must be confirmed
    Q_ENSURE_ID(810, isConfirmed != false);

    return child; // return the child
}

} // namespace QP


