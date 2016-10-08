/// @file
/// @brief QEP/C++ platform-independent public interface.
/// @ingroup qep
/// @cond
///***************************************************************************
/// Last updated for version 5.7.3
/// Last updated on  2016-10-07
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

#ifndef qep_h
#define qep_h

//****************************************************************************
//! The current QP version as a decimal constant XYZ, where X is a 1-digit
// major version number, Y is a 1-digit minor version number, and Z is
// a 1-digit release number.
#define QP_VERSION      573

//! The current QP version number string of the form X.Y.Z, where X is
// a 1-digit major version number, Y is a 1-digit minor version number,
// and Z is a 1-digit release number.
#define QP_VERSION_STR  "5.7.3"

//! Tamperproof current QP release (5.7.3) and date (16-10-07)
#define QP_RELEASE      0xA00845D2U

//****************************************************************************
#ifndef Q_SIGNAL_SIZE
    //! The size (in bytes) of the signal of an event. Valid values:
    //! 1, 2, or 4; default 1
    /// @description
    /// This macro can be defined in the QEP port file (qep_port.h) to
    /// configure the QP::QSignal type. When the macro is not defined, the
    /// default of 1 byte is chosen.
    #define Q_SIGNAL_SIZE 2
#endif

//****************************************************************************
//! helper macro to calculate static dimension of a 1-dim array @p array_
#define Q_DIM(array_) (sizeof(array_) / sizeof((array_)[0]))

//****************************************************************************
// typedefs for basic numerical types; MISRA-C++ 2008 rule 3-9-2(req).

//! typedef for character strings.
/// @description
/// This typedef specifies character type for exclusive use in character
/// strings. Use of this type, rather than plain 'char', is in compliance
/// with the MISRA-C 2004 Rules 6.1(req), 6.3(adv).
typedef char char_t;

//! typedef for line numbers in assertions and return from QF_run()
typedef int int_t;

//! typedef for enumerations used for event signals
typedef int enum_t;

//! typedef for 32-bit IEEE 754 floating point numbers
/// @note
/// QP does not use floating-point types anywhere in the internal
/// implementation, except in QS software tracing, where utilities for
/// output of floating-point numbers are provided for application-level
/// trace records.
///
typedef float float32_t;

//! typedef for 64-bit IEEE 754 floating point numbers
/// @note
/// QP does not use floating-point types anywhere in the internal
/// implementation, except in QS software tracing, where utilities for
/// output of floating-point numbers are provided for application-level
/// trace records.
///
typedef double float64_t;


//! Perform downcast of an event onto a subclass of QEvt @p class_
/// @description
/// This macro encapsulates the downcast of QEvt pointers, which violates
/// MISRA-C 2004 rule 11.4(advisory). This macro helps to localize this
/// deviation.
///
#define Q_EVT_CAST(class_)   (static_cast<class_ const *>(e))

//! Perform cast from unsigned integer @p uint_ to pointer of type @p type_.
/// @description
/// This macro encapsulates the cast to (type_ *), which QP ports or
/// application might use to access embedded hardware registers.
/// Such uses can trigger PC-Lint "Note 923: cast from int to pointer"
/// and this macro helps to encapsulate this deviation.
///
#define Q_UINT2PTR_CAST(type_, uint_)  (reinterpret_cast<type_ *>(uint_))

//! Initializer of static constant QEvt instances
/// @description
/// This macro encapsulates the ugly casting of enumerated signals
/// to QSignal and constants for QEvt.poolID and QEvt.refCtr_.
///
#define QEVT_INITIALIZER(sig_) { static_cast<QP::QSignal>(sig_), \
    static_cast<uint8_t>(0), static_cast<uint8_t>(0) }


//****************************************************************************
namespace QP {

#if (Q_SIGNAL_SIZE == 1)
    typedef uint8_t QSignal;
#elif (Q_SIGNAL_SIZE == 2)
    //! QSignal represents the signal of an event.
    /// @description
    /// The relationship between an event and a signal is as follows. A signal
    /// in UML is the specification of an asynchronous stimulus that triggers
    /// reactions, and as such is an essential part of an event. (The signal
    /// conveys the type of the occurrence--what happened?) However, an event
    /// can also contain additional quantitative information about the
    /// occurrence in form of event parameters.
    typedef uint16_t QSignal;
#elif (Q_SIGNAL_SIZE == 4)
    typedef uint32_t QSignal;
#else
    #error "Q_SIGNAL_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

#ifdef Q_EVT_CTOR // Provide the constructor for the QEvt class?

    //************************************************************************
    class QEvt {
    public:
        QSignal sig; //!< signal of the event instance

        //! the constructor
        QEvt(QSignal const s) // poolId_/refCtr_ intentionally uninitialized
          : sig(s) {}

#ifdef Q_EVT_VIRTUAL
        // virtual destructor
        virtual ~QEvt() {}
#endif // Q_EVT_VIRTUAL

    private:
        uint8_t poolId_;          //!< pool ID (0 for static event)
        uint8_t volatile refCtr_; //!< reference counter

        friend class QF;
        friend class QMActive;
        friend class QActive;
        friend class QTimeEvt;
        friend class QEQueue;
        friend uint8_t QF_EVT_POOL_ID_ (QEvt const * const e);
        friend uint8_t QF_EVT_REF_CTR_ (QEvt const * const e);
        friend void QF_EVT_REF_CTR_INC_(QEvt const * const e);
        friend void QF_EVT_REF_CTR_DEC_(QEvt const * const e);
    };

#else // QEvt is a POD (Plain Old Datatype)

    //************************************************************************
    //! QEvt base class.
    /// @description
    /// QEvt represents events without parameters and serves as the
    /// base class for derivation of events with parameters.
    ///
    /// @usage
    /// The following example illustrates how to add an event parameter by
    /// inheriting from the QEvt class.
    /// @include qep_qevt.cpp
    struct QEvt {
        QSignal sig;              //!< signal of the event instance
        uint8_t poolId_;          //!< pool ID (0 for static event)
        uint8_t volatile refCtr_; //!< reference counter
    };

#endif // Q_EVT_CTOR


//! Type returned from state-handler functions
typedef uint_fast8_t QState;

//! pointer to state-handler function
typedef QState (*QStateHandler)(void * const me, QEvt const * const e);

//! pointer to an action-handler function
typedef QState (*QActionHandler)(void * const me);

//! State object for the QMsm class (Meta State Machine).
/// @description
/// This class groups together the attributes of a QMsm state, such as the
/// parent state (state nesting), the associated state handler function and
/// the exit action handler function. These attributes are used inside the
/// QP::QMsm::dispatch() and QP::QMsm::init() functions.
///
/// @attention
/// The QMStateObj class is only intended for the QM code generator and
/// should not be used in hand-crafted code.
struct QMState {
    QMState        const *superstate;   //!< superstate of this state
    QStateHandler  const stateHandler;  //!< state handler function
    QActionHandler const entryAction;   //!< entry action handler function
    QActionHandler const exitAction;    //!< exit action handler function
    QActionHandler const initAction;    //!< init action handler function
};

//! Transition-Action Table for the Meta State Machine.
struct QMTranActTable {
    QMState        const *target;
    QActionHandler const act[1];
};

//! Attribute of for the QMsm class (Meta State Machine).
/// @description
/// This union represents possible values stored in the 'state' and 'temp'
/// attributes of the QMsm class.
union QMAttr {
    QMState        const *obj;    //!< pointer to QMState object
    QMTranActTable const *tatbl;  //!< transition-action table
    QStateHandler  fun;           //!< pointer to a state handler function
    QActionHandler act;           //!< pointer to an action-handler function
};

//****************************************************************************

//! event passed to the superstate to handle
QState const Q_RET_SUPER     = static_cast<QState>(0);

//! event passed to submachine superstate
QState const Q_RET_SUPER_SUB = static_cast<QState>(1);

//! event unhandled due to a guard evaluating to FALSE
QState const Q_RET_UNHANDLED = static_cast<QState>(2);

//! event handled (internal transition)
QState const Q_RET_HANDLED   = static_cast<QState>(3);

//! event silently ignored (bubbled up to top)
QState const Q_RET_IGNORED   = static_cast<QState>(4);

//! state entry action executed
QState const Q_RET_ENTRY     = static_cast<QState>(5);

//! state exit  action executed
QState const Q_RET_EXIT      = static_cast<QState>(6);

//! return value without any effect
QState const Q_RET_NULL      = static_cast<QState>(7);

//! regular transition taken
QState const Q_RET_TRAN      = static_cast<QState>(8);

//! initial transition taken
QState const Q_RET_TRAN_INIT = static_cast<QState>(9);

//! event handled (transition to history)
QState const Q_RET_TRAN_HIST = static_cast<QState>(10);

//! entry-point transition into a submachine
QState const Q_RET_TRAN_EP   = static_cast<QState>(11);

//! exit-point transition out of a submachine
QState const Q_RET_TRAN_XP   = static_cast<QState>(12);


//****************************************************************************
//! Meta State Machine
/// @description
/// QMsm represents the most fundamental State Machine in QP. The application-
/// level state machines derived directly from QMsm typically require the use
/// of the QM modeling tool, but are the fastest and need the least run-time
/// support (the smallest event-processor taking up the least code space).@n
/// @n
/// QMsm is also the base class for the QFsm and QHsm state machines, which
/// can be coded and maintained by hand (as well as by QM), but aren't as fast
/// and requrie significantly more run-time code (0.5-1KB) to execute.
///
/// @note
/// QMsm is not intended to be instantiated directly, but rather serves as
/// the base class for derivation of state machines in the application code.
///
/// @usage
/// The following example illustrates how to derive a state machine class
/// from QMsm. Please note that the QMsm member 'super' is defined as the
/// _first_ member of the derived struct.
/// @include qep_qmsm.cpp
class QMsm {
    QMAttr m_state;  //!< current active state (state-variable)
    QMAttr m_temp;   //!< temporary: transition chain, target state, etc.

public:
    //! virtual destructor
    virtual ~QMsm();

    //! Executes the top-most initial transition in QP::QMsm.
    virtual void init(QEvt const * const e);
    virtual void init(void) { this->init(static_cast<QEvt const *>(0)); }

    //! Dispatches an event to QMsm
    virtual void dispatch(QEvt const * const e);

    //! Tests if a given state is part of the active state configuration
    bool isInState(QMState const *state) const;

    //! Return the current active state object (read only)
    QMState const *stateObj(void) const {
        return m_state.obj;
    }

    //! Obtain the current active child state of a given parent (read only)
    QMState const *childStateObj(QMState const * const parent) const;

protected:
    //! Protected constructor of QMsm.
    QMsm(QStateHandler const initial);

// protected facilities for coding MSMs...
    //! internal helper function to record a regular state transition
    QState qm_tran_(QMTranActTable const * const tatbl) {
        m_temp.tatbl = tatbl;
        return Q_RET_TRAN;
    }

    //! Internal helper function to record a regular state transition
    QState qm_tran_hist_(QMState const * const hist,
                         QMTranActTable const * const tatbl)
    {
        m_state.obj  = hist;
        m_temp.tatbl = tatbl;
        return Q_RET_TRAN_HIST;
    }

    //! Internal helper function to record an initial state transition
    QState qm_tran_init_(QMTranActTable const * const tatbl) {
        m_temp.tatbl = tatbl;
        return Q_RET_TRAN_INIT;
    }

    //! Internal helper function to record an transition to an entry point
    //! to a submachine state
    QState qm_tran_ep_(QMTranActTable const * const tatbl) {
        m_temp.tatbl = tatbl;
        return Q_RET_TRAN_EP;
    }

    //! Internal helper function to record an transition to an exit point
    //! from a submachine state
    QState qm_tran_xp_(QActionHandler const xp,
                       QMTranActTable const *const tatbl)
    {
        m_state.act = xp;
        m_temp.tatbl = tatbl;
        return Q_RET_TRAN_XP;
    }

    //! Internal helper function to record a state entry
    QState qm_entry_(QMState const * const state) {
        m_temp.obj  = state;
        return Q_RET_ENTRY;
    }

    //! Internal helper function to record a state exit
    QState qm_exit_(QMState const * const state) {
        m_temp.obj  = state;
        return Q_RET_EXIT;
    }

    //! Internal helper function to call in a QM action-handler when
    //! it passes the event to the host submachine state to handle an event.
    QState qm_super_sub_(QMState const * const state) {
        m_temp.obj  = state;
        return Q_RET_SUPER_SUB;
    }

    //! Internal helper function to call in a QM action-handler when
    //! it handles an event.
    static QState QM_HANDLED(void) {
        return Q_RET_HANDLED;
    }

    //! Macro to call in a QM action-handler when it does not handle
    //! an event due to a guard condition evaluating to false.
    static QState QM_UNHANDLED(void) {
        return Q_RET_UNHANDLED;
    }

    //! Internal helper function to call in a QM action-handler when
    //! it passes the event to the superstate for processing.
    static QState QM_SUPER(void) {
        return Q_RET_SUPER;
    }

public: // facilities for coding HSMs...
    //! internal helper function to specify the return of a state-handler
    //! when it handles the event.
    static QState Q_HANDLED(void) {
        return Q_RET_HANDLED;
    }

    //! internal helper function to specify the return of a state-handler
    //! function when it attempts to handle the event but a guard condition
    //! evaluates to false and there is no other explicit way of handling
    //! the event.
    static QState Q_UNHANDLED(void) {
        return Q_RET_UNHANDLED;
    }

    //! internal helper function to record a state transition
    QState tran_(QStateHandler const target) {
        m_temp.fun = target;
        return Q_RET_TRAN;
    }

    //! internal helper function to record a transition to history
    QState tran_hist_(QStateHandler const hist) {
        m_temp.fun = hist;
        return Q_RET_TRAN_HIST;
    }

    //! internal helper function to record the superstate
    QState super_(QStateHandler const superstate) {
        m_temp.fun = superstate;
        return Q_RET_SUPER;
    }

    enum ReservedHsmSignals {
        Q_ENTRY_SIG = 1,     //!< signal for entry actions
        Q_EXIT_SIG,          //!< signal for exit actions
        Q_INIT_SIG           //!< signal for nested initial transitions
    };

private:
    //! Internal helper function to execute a transition-action table
    QState execTatbl_(QMTranActTable const * const tatbl);

    //! Internal helper function to exit current state to transition source
    void exitToTranSource_(QMState const *s, QMState const *ts);

    //! Internal helper function to enter state history
    QState enterHistory_(QMState const * const hist);

    //! maximum depth of implemented entry levels for transitions to history
    static int_fast8_t const MAX_ENTRY_DEPTH_ = static_cast<int_fast8_t>(4);

    //! the top state object for the QMsm
    static QMState const msm_top_s;

    friend class QFsm;
    friend class QHsm;
    friend class QMActive;
    friend class QActive;
    friend class QXK;
    friend class QXThread;
    friend class QXMutex;
    friend class QXSemaphore;
};

//! Top-most state of QMSM is NULL
QMState * const QMsm_top = static_cast<QMState *>(0);

//****************************************************************************
//! Hierarchical State Machine base class
///
/// @description
/// QHsm represents a Hierarchical State Machine (HSM) with full support for
/// hierarchical nesting of states, entry/exit actions, and initial
/// transitions in any composite state. QHsm inherits QMsm without adding
/// new attributes, so it takes the same amount of RAM as QMsm.
///
/// @note
/// QHsm is not intended to be instantiated directly, but rather serves as
/// the base class for derivation of state machines in the application code.
///
/// @usage
/// The following example illustrates how to derive a state machine class
/// from QHsm.
/// @include qep_qhsm.cpp
class QHsm : public QMsm {
public:
    //! Performs the second step of FSM initialization by triggering
    /// the top-most initial transition.
    virtual void init(QEvt const * const e);
    virtual void init(void) { this->init(static_cast<QEvt const *>(0)); }

    //! Dispatches an event to a HSM
    virtual void dispatch(QEvt const * const e);

    //! Tests if a given state is part of the current active state
    //! configuration
    bool isIn(QStateHandler const s);

    //! the top-state.
    static QState top(void * const me, QEvt const * const e);

    //! Obtain the current active state (state handler function)
    QStateHandler state(void) const {
        return m_state.fun;
    }

    //! Obtain the current active child state of a given parent
    QStateHandler childState(QStateHandler const parent);

private:
    //! operation inherited from QMsm, but disallowed in QHsm
    bool isInState(QMState const *state) const;

    //! operation inherited from QMsm, but disallowed in QHsm
    QMState const *stateObj(void) const;

    //! operation inherited from QMsm, but disallowed in QHsm
    QMState const *childStateObj(QMState const * const parent) const;

protected:
    //! Protected constructor of a HSM
    QHsm(QStateHandler const initial);

private:
    enum {
        MAX_NEST_DEPTH_ = 6  //!< maximum nesting depth of states in HSM
    };

    //! internal helper function to take a transition
    int_fast8_t hsm_tran(QStateHandler (&path)[MAX_NEST_DEPTH_]);

    friend class QActive;
    friend class QMActive;
};

//! the current QP version number string based on QP_VERSION_STR
extern char_t const versionStr[6];

//****************************************************************************
//! Provides miscellaneous QEP services.
class QEP {
public:
    //! get the current QEP version number string of the form "X.Y.Z"
    static char_t const *getVersion(void) {
        return versionStr;
    }
};

//! Offset or the user signals
enum_t const Q_USER_SIG = static_cast<enum_t>(4);

} // namespace QP

//****************************************************************************

//! Perform cast to QStateHandler.
/// @description
/// This macro encapsulates the cast of a specific state handler function
/// pointer to QStateHandler, which violates MISRA-C 2004 rule 11.4(advisory).
/// This macro helps to localize this deviation.
#define Q_STATE_CAST(handler_) \
    (reinterpret_cast<QP::QStateHandler>(handler_))

//! Perform cast to QActionHandler.
/// @description
/// This macro encapsulates the cast of a specific action handler function
/// pointer to QActionHandler, which violates MISRA-C2004 rule 11.4(advisory).
/// This macro helps to localize this deviation.
#define Q_ACTION_CAST(action_) \
    (reinterpret_cast<QP::QActionHandler>(action_))

#ifdef Q_SPY
    //! Macro to call in a QM action-handler when it executes an entry
    //! action. Applicable only to QMSMs.
    #define QM_ENTRY(state_)  (me->qm_entry_((state_)))

    //! Macro to call in a QM action-handler when it executes an exit action.
    //! Applicable only to QMSMs.
    #define QM_EXIT(state_)   (me->qm_exit_((state_)))
#else
    #define QM_ENTRY(dummy)   (QP::Q_RET_ENTRY)
    #define QM_EXIT(dummy)    (QP::Q_RET_EXIT)
#endif

//! Macro to call in a QM state-handler when it executes a regular
//! transition. Applicable only to QMSMs.
#define QM_TRAN(tatbl_) \
    (me->qm_tran_(reinterpret_cast<QP::QMTranActTable const *>(tatbl_)))

//! Macro to call in a QM state-handler when it executes an initial
//! transition. Applicable only to QMSMs.
#define QM_TRAN_INIT(tatbl_) \
    (me->qm_tran_init_(reinterpret_cast<QP::QMTranActTable const *>(tatbl_)))

//! Macro to call in a QM state-handler when it executes a transition
//! to history. Applicable only to QMSMs.
#define QM_TRAN_HIST(history_, tatbl_) \
    (me->qm_tran_hist_((history_), \
     reinterpret_cast<QP::QMTranActTable const *>(tatbl_)))

//! Macro to call in a QM state-handler when it executes an initial
//! transition. Applicable only to QMSMs.
#define QM_TRAN_EP(tatbl_) \
    (me->qm_tran_ep_(reinterpret_cast<QP::QMTranActTable const *>(tatbl_)))

//! Macro to call in a QM state-handler when it executes a transition
//! to exit point. Applicable only to QMSMs.
#define QM_TRAN_XP(xp_, tatbl_) \
    (me->qm_tran_xp_((xp_), \
        reinterpret_cast<QP::QMTranActTable const *>(tatbl_)))

//! Designates the superstate of a given state in an MSM.
#define QM_SUPER_SUB(state_)   (me->qm_super_sub_((state_)))


//! Designates a target for an initial or regular transition.
//! Q_TRAN() can be used both in the FSMs and HSMs.
/// @usage
/// @include qep_qtran.cpp
#define Q_TRAN(target_)       (me->tran_(Q_STATE_CAST(target_)))

//! Designates a target for an initial or regular transition.
//! Q_TRAN() can be used both in the FSMs and HSMs.
/// @usage
/// @include qep_qtran.cpp
#define Q_TRAN_HIST(hist_)    (me->tran_hist_((hist_)))

//! Designates the superstate of a given state in an HSM.
/// @usage
/// @include qep_qhsm.cpp
#define Q_SUPER(state_)       (me->super_(Q_STATE_CAST(state_)))

#endif // qep_h
