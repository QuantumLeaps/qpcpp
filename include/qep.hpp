/// @file
/// @brief QEP/C++ platform-independent public interface.
/// @ingroup qep
/// @cond
///***************************************************************************
/// Last updated for version 6.6.0
/// Last updated on  2019-10-14
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
/// <www.state-machine.com>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef QEP_HPP
#define QEP_HPP

//****************************************************************************
//! The current QP version as a decimal constant XXYZ, where XX is a 2-digit
// major version number, Y is a 1-digit minor version number, and Z is
// a 1-digit release number.
#define QP_VERSION      660U

//! The current QP version number string of the form XX.Y.Z, where XX is
// a 2-digit major version number, Y is a 1-digit minor version number,
// and Z is a 1-digit release number.
#define QP_VERSION_STR  "6.6.0"

//! Encrypted  current QP release (6.6.0) and date (2019-10-31)
#define QP_RELEASE      0x8E22F8FBU


//****************************************************************************
#ifndef Q_SIGNAL_SIZE
    //! The size (in bytes) of the signal of an event. Valid values:
    //! 1, 2, or 4; default 1
    /// @description
    /// This macro can be defined in the QEP port file (qep_port.hpp) to
    /// configure the QP::QSignal type. When the macro is not defined, the
    /// default of 1 byte is chosen.
    #define Q_SIGNAL_SIZE 2
#endif

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
//! namespace associated with the QP/C++ framework
/// @ingroup qep qf qs qv qk qxk
namespace QP {

//! the current QP version number string based on QP_VERSION_STR
extern char_t const versionStr[7];

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
        //! public constructor (dynamic event)
        QEvt(QSignal const s) // poolId_/refCtr_ intentionally uninitialized
          : sig(s) {}

        enum StaticEvt { STATIC_EVT };

        //! public constructor (static event)
        QEvt(QSignal const s, StaticEvt /*dummy*/)
          : sig(s),
            poolId_(static_cast<uint8_t>(0)),
            refCtr_(static_cast<uint8_t>(0))
        {}

#ifdef Q_EVT_VIRTUAL
        //! virtual destructor
        virtual ~QEvt() {}
#endif // Q_EVT_VIRTUAL

    public:
        QSignal sig; //!< signal of the event instance

    private:
        uint8_t poolId_;          //!< pool ID (0 for static event)
        uint8_t volatile refCtr_; //!< reference counter

        friend class QF;
        friend class QActive;
        friend class QMActive;
        friend class QTimeEvt;
        friend class QEQueue;
        friend class QTicker;
        friend class QXThread;
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

// forward declarations...
struct QMState;
struct QMTranActTable;

//! Attribute of for the QHsm class (Hierarchical State Machine).
/// @description
/// This union represents possible values stored in the 'state' and 'temp'
/// attributes of the QHsm and QMsm classes.
union QHsmAttr {
    QStateHandler  fun;           //!< pointer to a state handler function
    QActionHandler act;           //!< pointer to an action-handler function
    QMState        const *obj;    //!< pointer to QMState object
    QMTranActTable const *tatbl;  //!< transition-action table
};


//****************************************************************************
//! Hierarchical State Machine base class
///
/// @description
/// QHsm represents a Hierarchical State Machine (HSM) with full support for
/// hierarchical nesting of states, entry/exit actions, and initial
/// transitions in any composite state. QHsm inherits QMsm without adding
/// new attributes, so it takes the same amount of RAM as QMsm.
/// @n
/// QHsm is also the base class for the QMsm state machine, which provides
/// a superior efficiency, but requries the use of the QM modeling tool to
/// generate code.
///
/// @note
/// QHsm is not intended to be instantiated directly, but rather serves as
/// the base class for derivation of state machines in the application code.
///
/// @usage
/// The following example illustrates how to derive a state machine class
/// from QHsm.
/// @include qep_qhsm.cpp
///
class QHsm {
    QHsmAttr m_state;  //!< current active state (state-variable)
    QHsmAttr m_temp;   //!< temporary: transition chain, target state, etc.

public:
    //! virtual destructor
    virtual ~QHsm();

    //! Executes the top-most initial transition in QP::QHsm
    virtual void init(void) { this->init(static_cast<void const *>(0)); }

    //! @overload init(void)
    virtual void init(void const * const par);

    //! Dispatches an event to QHsm
    virtual void dispatch(QEvt const * const e);

    //! Tests if a given state is part of the current active state
    //! configuration
    bool isIn(QStateHandler const s);

    //! the top-state.
    static QState top(void * const me, QEvt const * const e);

    //! Obtain the current state (state handler function)
    //! @note used in the QM code generation
    QStateHandler state(void) const {
        return m_state.fun;
    }

    //! Obtain the current active child state of a given parent
    //! @note used in the QM code generation
    QStateHandler childState(QStateHandler const parent);

protected:
    //! Protected constructor of QHsm.
    QHsm(QStateHandler const initial);

// protected facilities for the QHsm implementation strategy...

    //! event passed to the superstate to handle
    static QState const Q_RET_SUPER     = static_cast<QState>(0);

    //! event passed to submachine superstate
    static QState const Q_RET_SUPER_SUB = static_cast<QState>(1);

    //! event unhandled due to a guard evaluating to 'false'
    static QState const Q_RET_UNHANDLED = static_cast<QState>(2);

    //! event handled (internal transition)
    static QState const Q_RET_HANDLED   = static_cast<QState>(3);

    //! event silently ignored (bubbled up to top)
    static QState const Q_RET_IGNORED   = static_cast<QState>(4);

    //! state entry action executed
    static QState const Q_RET_ENTRY     = static_cast<QState>(5);

    //! state exit  action executed
    static QState const Q_RET_EXIT      = static_cast<QState>(6);

    //! return value without any effect
    static QState const Q_RET_NULL      = static_cast<QState>(7);

    //! regular transition taken
    static QState const Q_RET_TRAN      = static_cast<QState>(8);

    //! initial transition taken
    static QState const Q_RET_TRAN_INIT = static_cast<QState>(9);

    //! entry-point transition into a submachine
    static QState const Q_RET_TRAN_EP   = static_cast<QState>(10);

    //! transition to history of a given state
    static QState const Q_RET_TRAN_HIST = static_cast<QState>(11);

    //! exit-point transition out of a submachine
    static QState const Q_RET_TRAN_XP   = static_cast<QState>(12);

    //! Helper function to specify a state transition
    QState tran(QStateHandler const target) {
        m_temp.fun = target;
        return Q_RET_TRAN;
    }

    //! Helper function to specify a transition to history
    QState tran_hist(QStateHandler const hist) {
        m_temp.fun = hist;
        return Q_RET_TRAN_HIST;
    }

    //! Helper function to specify the superstate of a given state
    QState super(QStateHandler const superstate) {
        m_temp.fun = superstate;
        return Q_RET_SUPER;
    }

    enum ReservedHsmSignals {
        Q_ENTRY_SIG = 1,     //!< signal for entry actions
        Q_EXIT_SIG,          //!< signal for exit actions
        Q_INIT_SIG           //!< signal for nested initial transitions
    };

// protected facilities for the QMsm implementation strategy...
    //! Helper function to specify a regular state transition
    //! in a QM state-handler
    QState qm_tran(void const * const tatbl) {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN;
    }

    //! Helper function to specifiy a transition to history
    //! in  a QM state-handler
    QState qm_tran_hist(QMState const * const hist,
                        void const * const tatbl)
    {
        m_state.obj  = hist;
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_HIST;
    }

    //! Helper function to specify an initial state transition
    //! in a QM state-handler
    QState qm_tran_init(void const * const tatbl) {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_INIT;
    }

    //! Helper function to specify a transition to an entry point
    //! to a submachine state in a QM state-handler
    QState qm_tran_ep(void const * const tatbl) {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_EP;
    }

    //! Helper function to specify a transition to an exit point
    //! from a submachine state in a QM state-handler
    QState qm_tran_xp(QActionHandler const xp,
                      void const *const tatbl)
    {
        m_state.act = xp;
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_XP;
    }

#ifdef Q_SPY
    //! Helper function to specify a state entry in a QM state-handler
    QState qm_entry(QMState const * const s) {
        m_temp.obj = s;
        return Q_RET_ENTRY;
    }

    //! Helper function to specify a state exit in a QM state-handler
    QState qm_exit(QMState const * const s) {
        m_temp.obj = s;
        return Q_RET_EXIT;
    }
#else
    //! Helper function to specify a state entry in a QM state-handler
    QState qm_entry(QMState const * const) {
        return Q_RET_ENTRY;
    }

    //! Helper function to specify a state exit in a QM state-handler
    QState qm_exit(QMState const * const) {
        return Q_RET_EXIT;
    }
#endif

    //! Helper function to specify a submachine exit in a QM state-handler
    QState qm_sm_exit(QMState const * const s) {
        m_temp.obj = s;
        return Q_RET_EXIT;
    }

    //! Helper function to call in a QM state-handler when it passes
    //! the event to the host submachine state to handle an event.
    QState qm_super_sub(QMState const * const s) {
        m_temp.obj = s;
        return Q_RET_SUPER_SUB;
    }

private:
    enum {
        MAX_NEST_DEPTH_ = 6  //!< maximum nesting depth of states in HSM
    };

    //! internal helper function to take a transition in QP::QHsm
    int_fast8_t hsm_tran(QStateHandler (&path)[MAX_NEST_DEPTH_]);

    friend class QMsm;
    friend class QActive;
    friend class QMActive;
    friend class QF;
    friend class QS;
    friend class QXK;
    friend class QXThread;
    friend class QXMutex;
    friend class QXSemaphore;
#ifdef Q_UTEST
    friend class QActiveDummy;
#endif // Q_UTEST
};

//****************************************************************************
//! QM State Machine implementation strategy
/// @description
/// QMsm (QM State Machine) provides a more efficient state machine
/// implementation strategy than QHsm, but requires the use of the QM
/// modeling tool, but are the fastest and need the least run-time
/// support (the smallest event-processor taking up the least code space).
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
///
class QMsm : public QHsm {
public:
    //! Performs the second step of SM initialization by triggering
    /// the top-most initial transition.
    virtual void init(void const * const par);
    virtual void init(void) { this->init(static_cast<void const *>(0)); }

    //! Dispatches an event to a HSM
    virtual void dispatch(QEvt const * const e);

    //! Tests if a given state is part of the active state configuration
    bool isInState(QMState const *st) const;

    //! Return the current active state object (read only)
    QMState const *stateObj(void) const {
        return m_state.obj;
    }

    //! Obtain the current active child state of a given parent (read only)
    QMState const *childStateObj(QMState const * const parent) const;

protected:
    //! Protected constructor
    QMsm(QStateHandler const initial);

private:
    //! Internal helper function to execute a transition-action table
    QState execTatbl_(QMTranActTable const * const tatbl);

    //! Internal helper function to exit current state to transition source
    void exitToTranSource_(QMState const *s, QMState const * const ts);

    //! Internal helper function to enter state history
    QState enterHistory_(QMState const * const hist);

    //! maximum depth of implemented entry levels for transitions to history
    static int_fast8_t const MAX_ENTRY_DEPTH_ = static_cast<int_fast8_t>(4);

    //! the top state object for the QMsm
    static QMState const msm_top_s;

    friend class QMActive;
};

//! State object for the QP::QMsm class (QM State Machine).
/// @description
/// This class groups together the attributes of a QP::QMsm state, such as
/// the parent state (state nesting), the associated state handler function
/// and the exit action handler function. These attributes are used inside
/// the QP::QMsm::dispatch() and QP::QMsm::init() functions.
///
/// @attention
/// The QP::QMState class is only intended for the QM code generator and
/// should not be used in hand-crafted code.
struct QMState {
    QMState        const *superstate;   //!< superstate of this state
    QStateHandler  const stateHandler;  //!< state handler function
    QActionHandler const entryAction;   //!< entry action handler function
    QActionHandler const exitAction;    //!< exit action handler function
    QActionHandler const initAction;    //!< init action handler function
};

//! Transition-Action Table for the QP::QMsm State Machine.
struct QMTranActTable {
    QMState        const *target;
    QActionHandler const act[1];
};


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
// Macros for coding QHsm-style state machines...

//! Macro to generate a declaration of a state-handler, state-caller and
//! a state-object for a given state in a subclass of QP::QHsm.
#define Q_STATE_DECL(state_) \
    QP::QState state_ ## _h(QP::QEvt const * const e); \
    static QP::QState state_(void * const me, QP::QEvt const * const e)

//! Macro to generate a definition of a state-handler for a given state
//! in a subclass of QP::QHsm.
#define Q_STATE_DEF(subclass_, state_) \
    QP::QState subclass_::state_(void * const me, QP::QEvt const * const e) {\
        return static_cast<subclass_ *>(me)->state_ ## _h(e); } \
    QP::QState subclass_::state_ ## _h(QP::QEvt const * const e)

//! Macro to specify that the event was handled
#define Q_HANDLED()           (Q_RET_HANDLED)

//! Macro to specify that the event was NOT handled
//! due to a guard condition evaluating to 'false'
#define Q_UNHANDLED()         (Q_RET_UNHANDLED)

//! Macro to specify a transition (also used for initial transitions).
/// @usage
/// @include qep_qtran.cpp
#define Q_TRAN(target_)       (me->tran(Q_STATE_CAST(target_)))

//! Macro to specify a transition to history.
/// @usage
/// @include qep_qtran.cpp
#define Q_TRAN_HIST(hist_)    (me->tran_hist((hist_)))

//! Macro to specify the superstate of a given state.
/// @usage
/// @include qep_qhsm.cpp
#define Q_SUPER(state_)       (me->super(Q_STATE_CAST(state_)))

//! Macro to perform casting to QStateHandler.
/// @description
/// This macro encapsulates the cast of a specific state handler function
/// pointer to QStateHandler, which violates MISRA-C 2004 rule 11.4(advisory).
/// This macro helps to localize this deviation.
#define Q_STATE_CAST(handler_) \
    (reinterpret_cast<QP::QStateHandler>(handler_))

//! Macro to perform casting to QActionHandler.
/// @description
/// This macro encapsulates the cast of a specific action handler function
/// pointer to QActionHandler, which violates MISRA-C2004 rule 11.4(advisory).
/// This macro helps to localize this deviation.
#define Q_ACTION_CAST(act_)   (reinterpret_cast<QP::QActionHandler>(act_))

//! Macro to provide strictly-typed zero-action to terminate action lists
//! in the transition-action-tables
#define Q_ACTION_NULL         (static_cast<QP::QActionHandler>(0))


//****************************************************************************
// Macros for coding QMsm-style state machines...

//! Macro to generate a declaration of a state-handler, state-caller and
//! a state-object for a given state in a subclass of QP::QMsm.
#define QM_STATE_DECL(state_) \
    QP::QState state_ ## _h(QP::QEvt const * const e); \
    static QP::QState state_(void * const me, QP::QEvt const * const e); \
    static QP::QMState const state_ ## _s

//! Macro to generate a declaration of a state-handler, state-caller and
//! a state-object for a given *submachine* state in a subclass of QP::QMsm.
#define QM_SM_STATE_DECL(subm_, state_) \
    QP::QState state_ ## _h(QP::QEvt const * const e); \
    static QP::QState state_(void * const me, QP::QEvt const * const e); \
    static SM_ ## subm_ const state_ ## _s

//! Macro to generate a declaration of an action-handler and action-caller
//! in a subclass of QP::QMsm.
#define QM_ACTION_DECL(action_) \
    QP::QState action_ ## _h(void); \
    static QP::QState action_(void * const me)

//! Macro to generate a definition of a state-caller and state-handler
//! for a given state in a subclass of QP::QMsm.
#define QM_STATE_DEF(subclass_, state_) \
    QP::QState subclass_::state_(void * const me, QP::QEvt const * const e) {\
        return static_cast<subclass_ *>(me)->state_ ## _h(e); } \
    QP::QState subclass_::state_ ## _h(QP::QEvt const * const e)

//! Macro to generate a definition of an action-caller and action-handler
//! in a subclass of QP::QMsm.
#define QM_ACTION_DEF(subclass_, action_) \
    QP::QState subclass_::action_(void * const me) { \
        return static_cast<subclass_ *>(me)->action_ ## _h(); } \
    QP::QState subclass_::action_ ## _h(void)

//! Macro to call in a QM state entry-handler. Applicable only to QMSMs.
#define QM_ENTRY(state_)      (me->qm_entry((state_)))

//! Macro to call in a QM state exit-handler. Applicable only to QMSMs.
#define QM_EXIT(state_)       (me->qm_exit((state_)))

//! Macro for a QM action-handler when it handles the event.
#define QM_HANDLED()          (Q_RET_HANDLED)

//! Macro for a QM action-handler when it does not handle the event
//! due to a guard condition evaluating to false.
#define QM_UNHANDLED()        (Q_RET_UNHANDLED)

//! Macro for a QM action-handler when it passes the event to the superstate
//! for processing.
#define QM_SUPER()            (Q_RET_SUPER)

//! Macro to call in a QM submachine exit-handler. Applicable only to QMSMs.
#define QM_SM_EXIT(state_)    (me->qm_sm_exit((state_)))

//! Macro to call in a QM state-handler when it executes a transition.
//! Applicable only to suclasses of QP::QMsm.
#define QM_TRAN(tatbl_)       (me->qm_tran((tatbl_)))

//! Macro to call in a QM state-handler when it executes an initial
//! transition. Applicable to suclasses of QP::QMsm.
#define QM_TRAN_INIT(tatbl_)  (me->qm_tran_init((tatbl_)))

//! Macro to call in a QM state-handler when it executes a transition
//! to history. Applicable to suclasses of QP::QMsm.
#define QM_TRAN_HIST(history_, tatbl_) \
    (me->qm_tran_hist((history_), (tatbl_)))

//! Macro to call in a QM state-handler when it executes an initial
//! transition. Applicable to suclasses of QP::QMsm.
#define QM_TRAN_EP(tatbl_)    (me->qm_tran_ep((tatbl_)))

//! Macro to call in a QM state-handler when it executes a transition
//! to exit point. Applicable to suclasses of QP::QMsm.
#define QM_TRAN_XP(xp_, tatbl_) (me->qm_tran_xp((xp_), (tatbl_)))

//! Designates the superstate of a given state in a subclass of QP::QMsm.
#define QM_SUPER_SUB(state_)  (me->qm_super_sub((state_)))

//! Macro to provide strictly-typed zero-state to use for submachines.
//! Applicable to suclasses of QP::QMsm.
#define QM_STATE_NULL         (static_cast<QP::QMState const *>(0))

#endif // QEP_HPP

