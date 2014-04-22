/// \file
/// \brief QP::QMsm::dispatch(), QP::QMsm::execTatbl_(),
/// QP::QMsm::exitToTranSource_(), and QP::QMsm::enterHistory_() definitions.
/// \ingroup qep
/// \cond
///***************************************************************************
/// Product: QEP/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-12
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#define QP_IMPL           // this is QP implementation
#include "qep_port.h"     // QEP port
#include "qep_pkg.h"      // QEP internal interface
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qmsm_dis")

//****************************************************************************
/// \description
/// Dispatches an event for processing to a meta state machine (MSM).
/// The processing of an event represents one run-to-completion (RTC) step.
///
/// \arguments
/// \arg[in] \c e  pointer to the event to be dispatched to the MSM
///
/// \note Must be called after QP::QMsm::init().
///
void QMsm::dispatch(QEvt const * const e) {
    QMState const *s = m_state.obj;  // store the current state
    QMState const *t = s;
    QState r = Q_RET_SUPER;
    QS_CRIT_STAT_

    /// \pre current state must be initialized
    Q_REQUIRE_ID(100, s != static_cast<QMState const *>(0));

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
            Q_ERROR_ID(110);
        }
    } while (t != static_cast<QMState const *>(0));

    // any kind of transition taken?
    if (r >= Q_RET_TRAN) {
#ifdef Q_SPY
        QMState const *ts = t; // transition source for QS tracing

        // the transition source state must not be NULL
        Q_ASSERT_ID(120, ts != static_cast<QMState const *>(0));
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

                QS_BEGIN_(QS_QEP_TRAN_XP, QS::priv_.smObjFilter, this)
                    QS_OBJ_(this);             // this state machine object
                    QS_FUN_(m_state.obj->stateHandler);   // source handler
                    QS_FUN_(tatbl->target->stateHandler); // target handler
                QS_END_()

                exitToTranSource_(s, t);
                (void)execTatbl_(tatbl);
                r = (*act)(this); // execute the XP action
            }
            else {
                // no other return value should be produced
                Q_ERROR_ID(130);
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
        Q_ASSERT_ID(140, t != static_cast<QMState const *>(0));

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
/// \description
/// Helper function to execute transition sequence in a tran-action table.
///
/// \arguments
/// \arg[in]  \c tatbl pointer to the transition-action table
///
/// \returns status of the last action from the transition-action table.
///
/// \note
/// This function is for internal use inside the QEP event processor and
/// should __not__ be called directly from the applications.
///
QState QMsm::execTatbl_(QMTranActTable const * const tatbl) {
    QActionHandler const *a;
    QState r = Q_RET_NULL;
    QS_CRIT_STAT_

    /// \pre the transition-action table pointer must not be NULL
    Q_REQUIRE_ID(200, tatbl != static_cast<QMTranActTable const *>(0));

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
/// \description
/// Helper function to exit the current state configuration to the
/// transition source, which is a hierarchical state machine might be a
/// superstate of the current state.
///
/// \arguments
/// \arg[in] \c s    pointer to the current state
/// \arg[in] \c ts   pointer to the transition source state
///
void QMsm::exitToTranSource_(QMState const *s,
                             QMState const * const ts)
{
    // exit states from the current state to the tran. source state
    while (s != ts) {
        // exit action provided in state 's'?
        if (s->exitAction != Q_ACTION_CAST(0)) {
            QState r = (*s->exitAction)(this); // execute the exit action

            //  is it a regular exit?
            if (r == Q_RET_EXIT) {
                QS_CRIT_STAT_

                QS_BEGIN_(QS_QEP_STATE_EXIT, QS::priv_.smObjFilter, this)
                    QS_OBJ_(this);            // this state machine object
                    QS_FUN_(s->stateHandler); // the exited state handler
                QS_END_()

                s = s->superstate; // advance to the superstate
            }
            // is it exit from a submachine?
            else if (r == Q_RET_SUPER_SUB) {
                // advance to the current host state of the submachie
                s = m_temp.obj;
            }
            else {
                Q_ERROR_ID(310);
            }
        }
        else {
            s = s->superstate; // advance to the superstate
        }
    }
}

//****************************************************************************
/// \description
/// Static helper function to execute the segment of transition to history
/// after entering the composite state and
///
/// \arguments
/// \arg[in] \c hist pointer to the history substate
///
/// \returns QP::Q_RET_INIT, if an initial transition has been executed in
/// the last entered state or QP::Q_RET_NULL if no such transition was taken.
///
QState QMsm::enterHistory_(QMState const * const hist) {
    QMState const *s;
    QMState const *ts = m_state.obj; // transition source
    QMState const *entry[MAX_ENTRY_DEPTH_];
    QState r;
    uint_fast8_t i = static_cast<uint_fast8_t>(0);  // entry path index
    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_TRAN_HIST, QS::priv_.smObjFilter, this)
        QS_OBJ_(this);               // this state machine object
        QS_FUN_(ts->stateHandler);   // source state handler
        QS_FUN_(hist->stateHandler); // target state handler
    QS_END_()

    for (s = hist; s != ts; s = s->superstate) {
        Q_ASSERT_ID(410, s != static_cast<QMState const *>(0));
        if (s->entryAction != static_cast<QActionHandler>(0)) {
            entry[i] = s;
            ++i;
            Q_ASSERT_ID(420, i <= static_cast<uint_fast8_t>(Q_DIM(entry)));
        }
    }

    // retrace the entry path in reverse (desired) order...
    while (i > static_cast<uint_fast8_t>(0)) {
        --i;
        r = (*entry[i]->entryAction)(this); // run entry action in entry[i]

        QS_BEGIN_(QS_QEP_STATE_ENTRY, QS::priv_.smObjFilter, this)
            QS_OBJ_(this);
            QS_FUN_(entry[i]->stateHandler); // entered state handler
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

} // namespace QP

