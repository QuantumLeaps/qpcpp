//////////////////////////////////////////////////////////////////////////////
// Product: QEP/C++ platform-independent public interface
// Last Updated for Version: 4.5.04
// Date of the Last Update:  Jan 16, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
//////////////////////////////////////////////////////////////////////////////
#ifndef qep_h
#define qep_h

/// \file
/// \ingroup qep qf qk
/// \brief QEP/C++ platform-independent public interface.
///
/// This header file must be included directly or indirectly
/// in all modules (*.cpp files) that use QEP/C++.

#include "qevt.h"               // QEP event processor needs the QEvt facility

/// \brief Designates a target for an initial or regular transition.
/// Q_TRAN() can be used both in the FSMs and HSMs.
///
/// \include qep_qtran.c
///
#define Q_TRAN(target_)     (me->tran(Q_STATE_CAST(target_)))

/// \brief Designates the superstate of a given state in an HSM.
///
/// \include qep_qhsm.c
///
#define Q_SUPER(super_)     (me->super(Q_STATE_CAST(super_)))

/// \brief Perform cast to QStateHandler.
///
/// This macro encapsulates the cast of a specific state handler function
/// pointer to QStateHandler, which violates MISRA-C 2004 rule 11.4(advisory).
/// This macro helps to localize this deviation.
///
#define Q_STATE_CAST(handler_) \
    (reinterpret_cast<QP_ QStateHandler>(handler_))

/// \brief Perform downcast of an event onto a subclass of QEvt \a class_
///
/// This macro encapsulates the downcast of QEvt pointers, which violates
/// MISRA-C 2004 rule 11.4(advisory). This macro helps to localize this
/// deviation.
///
#define Q_EVT_CAST(class_)   (static_cast<class_ const *>(e))

//////////////////////////////////////////////////////////////////////////////
QP_BEGIN_

/// \brief Provides miscellaneous QEP services.
class QEP {
public:
    /// \brief get the current QEP version number string
    ///
    /// \return version of the QEP as a constant 6-character string of the
    /// form x.y.zz, where x is a 1-digit major version number, y is a
    /// 1-digit minor version number, and zz is a 2-digit release number.
    static char_t const Q_ROM * Q_ROM_VAR getVersion(void);
};

//////////////////////////////////////////////////////////////////////////////

                       /// \brief Type returned from  a state-handler function
typedef uint8_t QState;
                                  /// \brief pointer to state-handler function
typedef QState (*QStateHandler)(void * const me, QEvt const * const e);

//////////////////////////////////////////////////////////////////////////////
/// \brief Value returned by a state-handler function when it handles
/// the event.
QState const Q_RET_HANDLED = static_cast<QState>(0);

/// \brief Value returned by a non-hierarchical state-handler function when
/// it ignores (does not handle) the event.
QState const Q_RET_IGNORED = static_cast<QState>(1);

/// \brief Value returned by a state-handler function when it takes a
/// regular state transition.
QState const Q_RET_TRAN    = static_cast<QState>(2);

/// \brief Value returned by a state-handler function when it forwards
/// the event to the superstate to handle.
QState const Q_RET_SUPER   = static_cast<QState>(3);

/// \brief Value returned by a state-handler function when a guard
/// condition prevents it from handling the event.
///
QState const Q_RET_UNHANDLED = static_cast<QState>(4);


/// Offset or the user signals
enum_t const Q_USER_SIG    = static_cast<enum_t>(4);


//////////////////////////////////////////////////////////////////////////////
/// \brief Finite State Machine base class
///
/// QFsm represents a traditional non-hierarchical Finite State Machine (FSM)
/// without state hierarchy, but with entry/exit actions.
///
/// QFsm is also a base structure for the ::QHsm class.
///
/// \note QFsm is not intended to be instantiated directly, but rather serves
/// as the base class for derivation of state machines in the application
/// code.
///
/// The following example illustrates how to derive a state machine class
/// from QFsm.
/// \include qep_qfsm.cpp
class QFsm {
private:
    QStateHandler m_state;          ///< current active state (state-variable)
    QStateHandler m_temp;  ///< temporary state: target of tran. or superstate

public:
    /// \brief virtual destructor
    virtual ~QFsm();

    /// \brief Performs the second step of FSM initialization by triggering
    /// the top-most initial transition.
    ///
    /// The argument \a e is constant pointer to ::QEvt or a class
    /// derived from ::QEvt.
    ///
    /// \note Must be called only ONCE before QFsm::dispatch()
    ///
    /// The following example illustrates how to initialize a FSM, and
    /// dispatch events to it:
    /// \include qep_qfsm_use.cpp
    void init(QEvt const * const e = static_cast<QEvt const *>(0));

    /// \brief Dispatches an event to a FSM
    ///
    /// Processes one event at a time in Run-to-Completion (RTC) fashion.
    /// The argument \a e is a constant pointer the ::QEvt or a
    /// class derived from ::QEvt.
    ///
    /// \note Must be called after QFsm::init().
    ///
    /// \sa example for QFsm::init()
    void dispatch(QEvt const * const e);

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
    QFsm(QStateHandler const initial)
      : m_state(Q_STATE_CAST(0)),
        m_temp(initial)
    {}

    /// \brief Return the current active state.
    QStateHandler state(void) const {
        return m_state;
    }

    /// \brief Inline function to specify the return of a state-handler
    /// when it igonres (does not handle in any way) the event.
    static QState Q_IGNORED(void) { return Q_RET_IGNORED; }

    /// \brief Inline function to specify the return of a state-handler
    /// when it handles the event.
    ///
    /// \include qep_qhsm.cpp
    static QState Q_HANDLED(void) { return Q_RET_HANDLED; }

    /// \brief Macro to specify the return of a state-handler function when
    /// it attempts to handle the event but a guard condition evaluates to
    /// false and there is no other explicit way of handling the event.
    static QState Q_UNHANDLED(void) { return Q_RET_UNHANDLED; }

    /// \brief internal helper function to record a state transition
    QState tran(QStateHandler const target) {
        m_temp = target;
        return Q_RET_TRAN;
    }

    enum ReservedFsmSignals {
        Q_ENTRY_SIG = 1,                         ///< signal for entry actions
        Q_EXIT_SIG                                ///< signal for exit actions
    };
};

//////////////////////////////////////////////////////////////////////////////
/// \brief Hierarchical State Machine base class
///
/// QHsm represents a Hierarchical Finite State Machine (HSM). QHsm derives
/// from the ::QFsm class and extends the capabilities of a basic FSM
/// with state hierarchy.
///
/// \note QHsm is not intended to be instantiated directly, but rather serves
/// as the base structure for derivation of state machines in the application
/// code.
///
/// The following example illustrates how to derive a state machine class
/// from QHsm.
/// \include qep_qhsm.cpp
class QHsm {
private:
    QStateHandler m_state;          ///< current active state (state-variable)
    QStateHandler m_temp;  ///< temporary state: target of tran. or superstate

public:
    /// \brief virtual destructor
    virtual ~QHsm();

    /// \brief Performs the second step of HSM initialization by triggering
    /// the top-most initial transition.
    ///
    /// \param e constant pointer ::QEvt or a class derived from ::QEvt
    /// \note Must be called only ONCE before QHsm::dispatch()
    ///
    /// The following example illustrates how to initialize a HSM, and
    /// dispatch events to it:
    /// \include qep_qhsm_use.cpp
    void init(QEvt const * const e = static_cast<QEvt const *>(0));

    /// \brief Dispatches an event to a HSM
    ///
    /// Processes one event at a time in Run-to-Completion (RTC) fashion.
    /// The argument \a e is a constant pointer the ::QEvt or a
    /// class derived from ::QEvt.
    ///
    /// \note Must be called after QHsm::init().
    ///
    /// \sa example for QHsm::init()
    void dispatch(QEvt const * const e);

    /// \brief Tests if a given state is part of the current active state
    /// configuration
    ///
    /// \param state is a pointer to the state handler function, e.g.,
    /// &QCalc::on.
    bool isIn(QStateHandler const s);

protected:
    /// \brief Protected constructor of a HSM.
    ///
    /// Performs the first step of HSM initialization by assigning the
    /// initial pseudostate to the currently active state of the state
    /// machine.
    ///
    /// \note The constructor is protected to prevent direct instantiating
    /// of QHsm objects. This class is intended for subclassing only.
    ///
    /// \sa The ::QHsm example illustrates how to use the QHsm constructor
    /// in the constructor initializer list of the derived state machines.
    /// \sa QFsm::QFsm()
    QHsm(QStateHandler const initial)
      : m_state(Q_STATE_CAST(&QHsm::top)),
        m_temp(initial)
    {}

    /// \brief Return the current active state.
    QStateHandler state(void) const {
        return m_state;
    }

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
    ///
    /// \include qep_qhsm.cpp
    static QState Q_HANDLED(void) { return Q_RET_HANDLED; }

    /// \brief Macro to specify the return of a state-handler function when
    /// it attempts to handle the event but a guard condition evaluates to
    /// false and there is no other explicit way of handling the event.
    static QState Q_UNHANDLED(void) { return Q_RET_UNHANDLED; }

    /// \brief internal helper function to record a state transition
    QState tran(QStateHandler const target) {
        m_temp = target;
        return Q_RET_TRAN;
    }

    /// \brief internal helper function to record the superstate
    QState super(QStateHandler const superstate) {
        m_temp = superstate;
        return Q_RET_SUPER;
    }

    enum ReservedHsmSignals {
        Q_ENTRY_SIG = 1,                         ///< signal for entry actions
        Q_EXIT_SIG,                               ///< signal for exit actions
        Q_INIT_SIG                  ///< signal for nested initial transitions
    };
};

//////////////////////////////////////////////////////////////////////////////

QP_END_

#endif                                                                // qep_h
