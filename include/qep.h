//****************************************************************************
// Product: QEP/C++ platform-independent public interface
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Jan 06, 2014
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2014 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
#ifndef qep_h
#define qep_h

/// \file
/// \ingroup qep qf qk
/// \brief QEP/C++ platform-independent public interface.
///
/// This header file must be included directly or indirectly
/// in all modules (*.cpp files) that use QEP/C++.

#include "qevt.h"               // QEP event processor needs the QEvt facility

namespace QP {
                         /// \brief Type returned from state-handler functions
typedef uint8_t QState;
                                  /// \brief pointer to state-handler function
typedef QState (*QStateHandler)(void * const me, QEvt const * const e);

                              /// \brief pointer to an action-handler function
typedef QState (*QActionHandler)(void * const me);

/// \brief State object for the QMsm class (Meta State Machine).
///
/// This class groups together the attributes of a QMsm state, such as the
/// parent state (state nesting), the associated state handler function and
/// the exit action handler function. These attributes are used inside the
/// QMsm::dispatch() and QMsm::init() functions.
///
/// \note The QMStateObj class is only intended for the QM code generator
/// and should not be used in hand-crafted code.
///
struct QMState {
    QMState        const *parent;            ///< parent state (state nesting)
    QStateHandler  stateHandler;                  ///<  state handler function
    QActionHandler exitAction;               ///< exit action handler function
};

/// \brief Attribute of for the QMsm class (Meta State Machine).
///
/// This union represents possible values stored in the 'state' and 'temp'
/// attributes of the QMsm class.
///
union QMAttr {
    QMState        const *obj;                  ///< pointer to QMState object
    QActionHandler const *act;                   ///< array of action handlers
    QStateHandler  fun;               ///< pointer to a state handler function
};


//****************************************************************************

/// \brief event passed to the superstate to handle
QState const Q_RET_SUPER     = static_cast<QState>(0);

/// \brief event handled (internal transition)
QState const Q_RET_HANDLED   = static_cast<QState>(1);

/// \brief event unhandled due to a guard evaluting to FALSE
QState const Q_RET_UNHANDLED = static_cast<QState>(2);

/// \brief event ignored (bubbled up to top)
QState const Q_RET_IGNORED   = static_cast<QState>(3);

/// \brief regular transition taken
QState const Q_RET_TRAN      = static_cast<QState>(4);

/// \brief entry action executed
QState const Q_RET_ENTRY     = static_cast<QState>(5);

/// \brief exit action executed
QState const Q_RET_EXIT      = static_cast<QState>(6);

/// \brief initial transition taken
QState const Q_RET_INITIAL   = static_cast<QState>(7);

//****************************************************************************
/// \brief Meta State Machine
///
/// QMsm represents the most fundamental State Machine in QP. The application-
/// level state machines derived directly from QMsm typically require the use
/// of the QM modeling tool, but are the fastest and need the least run-time
/// support (the smallest event-processor taking up the least code space).
///
/// QMsm is also the base class for the QFsm and QHsm state machines, which
/// can be coded and maintained by hand (as well as by QM), but aren't as fast
/// and requrie significantly more run-time code (0.5-1KB) to execute.
///
/// \note QMsm is not intended to be instantiated directly, but rather serves
/// as the base class for derivation of state machines in the application
/// code.
///
/// The following example illustrates how to derive a state machine class
/// from QMsm. Please note that the QMsm member 'super' is defined as the
/// _first_ member of the derived struct.
/// \include qep_qmsm.cpp
///
/// \sa \ref derivation
///
class QMsm {
    QMAttr m_state;          ///< \brief current active state (state-variable)
    QMAttr m_temp; ///< \brief temporary: transition chain, target state, etc.

public:
    /// \brief virtual destructor
    virtual ~QMsm();

    /// \brief Performs the second step of QMsm initialization by triggering
    /// the top-most initial transition.
    ///
    /// The argument \a e is constant pointer to ::QEvt or a class
    /// derived from ::QEvt.
    ///
    /// \note Must be called only ONCE before QMsm::dispatch()
    virtual void init(QEvt const * const e);
    virtual void init(void) { this->init(static_cast<QEvt const *>(0)); }

    /// \brief Dispatches an event to QMsm
    ///
    /// Processes one event at a time in Run-to-Completion (RTC) fashion.
    /// The argument \a e is a constant pointer the ::QEvt or a class
    /// derived from ::QEvt.
    ///
    /// \note Must be called after QMsm::init().
    virtual void dispatch(QEvt const * const e);

protected:
    /// \brief Protected constructor of QMsm.
    ///
    /// Performs the first step of initialization by assigning the initial
    /// pseudostate to the currently active state of the state machine.
    ///
    /// \note The constructor is protected to prevent direct instantiating
    /// of QMsm objects. This class is intended for subclassing only.
    ///
    /// \sa The ::QMsm example illustrates how to use the QMsm constructor
    /// in the constructor initializer list of the derived state machines.
    QMsm(QStateHandler const initial) {
        m_state.obj = static_cast<QMState const *>(0);
        m_temp.fun  = initial;
    }

    /// \brief internal helper function to record a regular state transition
    QState qm_tran_(QMState const * const target,
                    QActionHandler const * const act)
    {
        m_state.obj = target;
        m_temp.act  = act;
        return static_cast<QState>(Q_RET_TRAN);
    }

    /// \brief internal helper function to record an initial state transition
    QState qm_initial_(QMState const * const target,
                       QActionHandler const * const act)
    {
        m_state.obj = target;
        m_temp.act  = act;
        return static_cast<QState>(Q_RET_INITIAL);
    }

    /// \brief internal helper function to record a state entry
    QState qm_entry_(QMState const * const state) {
        m_temp.obj  = state;
        return static_cast<QState>(Q_RET_ENTRY);
    }

    /// \brief internal helper function to record a state exit
    QState qm_exit_(QMState const * const state) {
        m_temp.obj  = state;
        return static_cast<QState>(Q_RET_EXIT);
    }

    static QActionHandler const s_emptyAction_[1];

private:
    /// \brief internal helper function to take a transition
    void msm_tran(QActionHandler const *a);

    friend class QFsm;
    friend class QHsm;
    friend class QMActive;
};

/// \brief Top-most state of QMSM is NULL
QMState * const QMsm_top = static_cast<QMState *>(0);

//****************************************************************************
/// \brief Hierarchical State Machine base class
///
/// QHsm represents a Hierarchical Finite State Machine (HSM). QHsm derives
/// from the ::QFsm class and extends the capabilities of a basic FSM
/// with state hierarchy.
///
/// \note QHsm is not intended to be instantiated directly, but rather serves
/// as the base class for derivation of state machines in the application
/// code.
///
/// The following example illustrates how to derive a state machine class
/// from QHsm.
/// \include qep_qhsm.cpp
class QHsm : public QMsm {
public:
    /// \brief Performs the second step of FSM initialization by triggering
    /// the top-most initial transition.
    virtual void init(QEvt const * const e);
    virtual void init(void) { this->init(static_cast<QEvt const *>(0)); }

    /// \brief Dispatches an event to a HSM
    virtual void dispatch(QEvt const * const e);

    /// \brief Tests if a given state is part of the current active state
    /// configuration
    ///
    /// \param state is a pointer to the state handler function, e.g.,
    /// &QCalc::on.
    bool isIn(QStateHandler const s);

    /// \brief Return the current active state (state handler function)
    QStateHandler state(void) const {
        return m_state.fun;
    }

protected:
    /// \brief Protected constructor of a HSM.
    QHsm(QStateHandler const initial);

    /// \brief the top-state.
    ///
    /// QHsm::top() is the ultimate root of state hierarchy in all HSMs
    /// derived from ::QHsm. This state handler always returns (QSTATE)0,
    /// which means that it "handles" all events.
    ///
    /// \sa Example of the QCalc::on() state handler.
    static QState top(void * const me, QEvt const * const e);

    /// \brief Inline function to specify the return of a state-handler
    /// when it handles the event.
    static QState Q_HANDLED(void) {
        return Q_RET_HANDLED;
    }

    /// \brief Macro to specify the return of a state-handler function when
    /// it attempts to handle the event but a guard condition evaluates to
    /// false and there is no other explicit way of handling the event.
    static QState Q_UNHANDLED(void) {
        return Q_RET_UNHANDLED;
    }

    /// \brief internal helper function to record a state transition
    QState tran_(QStateHandler const target) {
        m_temp.fun = target;
        return Q_RET_TRAN;
    }

    /// \brief internal helper function to record the superstate
    QState super_(QStateHandler const superstate) {
        m_temp.fun = superstate;
        return Q_RET_SUPER;
    }

    enum ReservedHsmSignals {
        Q_ENTRY_SIG = 1,                         ///< signal for entry actions
        Q_EXIT_SIG,                               ///< signal for exit actions
        Q_INIT_SIG                  ///< signal for nested initial transitions
    };

private:
    /// maximum depth of state nesting (including the top level), must be >= 3
    static int_t const MAX_NEST_DEPTH = static_cast<int_t>(6);

    /// \brief internal helper function to take a transition
    int_t hsm_tran(QStateHandler (&path)[MAX_NEST_DEPTH]);

    friend class QMActive;
};

//****************************************************************************
/// \brief Finite State Machine base class
///
/// QFsm represents a traditional non-hierarchical Finite State Machine (FSM)
/// without state hierarchy, but with entry/exit actions.
///
/// \note QFsm is not intended to be instantiated directly, but rather serves
/// as the base class for derivation of state machines in the application
/// code.
///
/// The following example illustrates how to derive a state machine class
/// from QFsm.
/// \include qep_qfsm.cpp
class QFsm : public QMsm {
public:
    /// \brief Performs the second step of FSM initialization by triggering
    /// the top-most initial transition.
    virtual void init(QEvt const * const e);

    /// \brief Overloaded init operation (no initialization event)
    virtual void init(void) { this->init(static_cast<QEvt const *>(0)); }

    /// \brief Dispatches an event to a FSM
    virtual void dispatch(QEvt const * const e);

protected:
    /// \brief Protected constructor of a FSM.
    ///
    /// Performs the first step of FSM initialization by assigning the
    /// initial pseudostate to the currently active state of the state
    /// machine.
    ///
    /// \note The constructor is protected to prevent direct instantiating
    /// of QFsm objects. This class is intended for subclassing only.
    ///
    /// \sa The ::QFsm example illustrates how to use the QHsm constructor
    /// in the constructor initializer list of the derived state machines.
    QFsm(QStateHandler const initial);

    /// \brief Inline function to specify the return of a state-handler
    /// when it handles the event.
    static QState Q_HANDLED(void) {
        return Q_RET_HANDLED;
    }

    /// \brief Macro to specify the return of a state-handler function when
    /// it attempts to handle the event but a guard condition evaluates to
    /// false and there is no other explicit way of handling the event.
    static QState Q_UNHANDLED(void) {
        return Q_RET_UNHANDLED;
    }

    /// \brief Macro to specify the return of a state-handler function when
    /// it ignored the event (didn't attempt to handle it in any way).
    static QState Q_IGNORED(void) {
        return Q_RET_IGNORED;
    }

    /// \brief internal helper function to record a state transition
    QState tran_(QStateHandler const target) {
        m_temp.fun = target;
        return Q_RET_TRAN;
    }

    enum ReservedFsmSignals {
        Q_ENTRY_SIG = 1,                         ///< signal for entry actions
        Q_EXIT_SIG                                ///< signal for exit actions
    };
};

//****************************************************************************
/// \brief Provides miscellaneous QEP services.
class QEP {
public:
    /// \brief get the current QEP version number string
    ///
    /// \return version of QEP as a constant 5-character string of the
    /// form X.Y.Z, where X is a 1-digit major version number, Y is a
    /// 1-digit minor version number, and Z is a 1-digit release number.
    static char_t const Q_ROM *getVersion(void) {
        return QP_VERSION_STR;
    }
};

/// Offset or the user signals
enum_t const Q_USER_SIG = static_cast<enum_t>(4);

}                                                              // namespace QP
//****************************************************************************

/// \brief Perform cast to QStateHandler.
///
/// This macro encapsulates the cast of a specific state handler function
/// pointer to QStateHandler, which violates MISRA-C 2004 rule 11.4(advisory).
/// This macro helps to localize this deviation.
///
#define Q_STATE_CAST(handler_) \
    (reinterpret_cast<QP::QStateHandler>(handler_))

/// \brief Perform cast to QActionHandler.
///
/// This macro encapsulates the cast of a specific action handler function
/// pointer to QActionHandler, which violates MISRA-C2004 rule 11.4(advisory).
/// This macro helps to localize this deviation.
///
#define Q_ACTION_CAST(action_) \
    (reinterpret_cast<QP::QActionHandler>(action_))

#ifdef Q_SPY
    /// \brief Macro to call in a QM action-handler when it executes
    /// an entry action. Applicable only to QMSMs.
    ///
    #define QM_ENTRY(state_)  (me->qm_entry_((state_)))

    /// \brief Macro to call in a QM action-handler when it executes
    /// an exit action. Applicable only to QMSMs.
    ///
    #define QM_EXIT(state_)   (me->qm_exit_((state_)))
#else
    #define QM_ENTRY(dummy)   (QP::Q_RET_ENTRY)
    #define QM_EXIT(dummy)    (QP::Q_RET_EXIT)
#endif

/// \brief Macro to call in a QM state-handler when it executes a regular
/// transition. Applicable only to QMSMs.
///
#define QM_TRAN(target_, act_) (me->qm_tran_((target_), (act_)))

/// \brief Macro to call in a QM state-handler when it executes an initial
/// transition. Applicable only to QMSMs.
///
#define QM_INITIAL(target_, act_) (me->qm_initial_((target_), (act_)))

/// \brief Macro to call in a QM action-handler when it handles an event.
/// Applicable only to QMSMs.
///
#define QM_HANDLED()          (QP::Q_RET_HANDLED)

/// \brief Macro to call in a QM action-handler when it handles an event.
/// Applicable only to QMSMs.
///
#define QM_UNHANDLED()        (QP::Q_RET_UNHANDLED)

/// \brief Macro to call in a QM action-handler when it handles an event.
/// Applicable only to QMSMs.
///
#define QM_SUPER()            (QP::Q_RET_SUPER)

/// \brief Designates a target for an initial or regular transition.
/// Q_TRAN() can be used both in the FSMs and HSMs.
///
/// \include qep_qtran.cpp
///
#define Q_TRAN(target_)       (me->tran_(Q_STATE_CAST(target_)))

/// \brief Designates the superstate of a given state in an HSM.
///
/// \include qep_qhsm.cpp
///
#define Q_SUPER(state_)       (me->super_(Q_STATE_CAST(state_)))

#endif                                                                // qep_h
