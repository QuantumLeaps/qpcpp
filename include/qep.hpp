//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QEP/C++ platform-independent public interface.
//!
#ifndef QEP_HPP
#define QEP_HPP

//============================================================================
//! The current QP version as a decimal constant XXYZ, where XX is a 2-digit
//! major version number, Y is a 1-digit minor version number, and Z is
//! a 1-digit release number.
#define QP_VERSION      701U

//! The current QP version number string of the form XX.Y.Z, where XX is
//! a 2-digit major version number, Y is a 1-digit minor version number,
//! and Z is a 1-digit release number.
#define QP_VERSION_STR  "7.0.1"

//! Encrypted  current QP release (7.0.1) and date (2022-06-30)
#define QP_RELEASE      0x7C7E85E2U

//============================================================================
#ifndef Q_SIGNAL_SIZE
    //! The size (in bytes) of the signal of an event. Valid values:
    //! 1U, 2U, or 4U; default 2U
    //!
    //! @description
    //! This macro can be defined in the QEP port file (qep_port.hpp) to
    //! configure the QP::QSignal type. When the macro is not defined, the
    //! default of 2 bytes is applied.
    #define Q_SIGNAL_SIZE 2U
#endif

//============================================================================
// Aliases for basic numerical types; MISRA-C++ 2008 rule 3-9-2(req).

//! alias for line numbers in assertions and return from QF::run()
using int_t = int;

//! alias for enumerations used for event signals
using enum_t = int;

//! alias for 32-bit IEEE 754 floating point numbers
//!
//! @note
//! QP does not use floating-point types anywhere in the internal
//! implementation, except in QS software tracing, where utilities for
//! output of floating-point numbers are provided for application-level
//! trace records.
//!
using float32_t = float;

//! alias for 64-bit IEEE 754 floating point numbers
//!
//! @note
//! QP does not use floating-point types anywhere in the internal
//! implementation, except in QS software tracing, where utilities for
//! output of floating-point numbers are provided for application-level
//! trace records.
//!
using float64_t = double;

//============================================================================
//! Perform downcast of an event onto a subclass of QEvt @p class_
//!
//! @description
//! This macro encapsulates the downcast of QEvt pointers, which violates
//! MISRA-C 2004 rule 11.4(advisory). This macro helps to localize this
//! deviation.
//!
#define Q_EVT_CAST(class_)   (static_cast<class_ const *>(e))

//! Perform cast from unsigned integer @p uint_ to pointer of type @p type_
//!
//! @description
//! This macro encapsulates the cast to (type_ *), which QP ports or
//! application might use to access embedded hardware registers.
//! Such uses can trigger PC-Lint "Note 923: cast from int to pointer"
//! and this macro helps to encapsulate this deviation.
//!
#define Q_UINT2PTR_CAST(type_, uint_)  (reinterpret_cast<type_ *>(uint_))

//! Initializer of static constant QEvt instances
//!
//! @description
//! This macro encapsulates the ugly casting of enumerated signals
//! to QSignal and constants for QEvt.poolID and QEvt.refCtr_.
//!
#define QEVT_INITIALIZER(sig_) { static_cast<QP::QSignal>(sig_), 0U, 0U }

//============================================================================
//! namespace associated with the QP/C++ framework
namespace QP {

    //! the current QP version number string based on QP_VERSION_STR
    constexpr char const versionStr[]{QP_VERSION_STR};

#if (Q_SIGNAL_SIZE == 1U)
    using QSignal = std::uint8_t;
#elif (Q_SIGNAL_SIZE == 2U)
    //! QSignal represents the signal of an event
    //!
    //! @description
    //! The relationship between an event and a signal is as follows. A signal
    //! in UML is the specification of an asynchronous stimulus that triggers
    //! reactions, and as such is an essential part of an event. (The signal
    //! conveys the type of the occurrence--what happened?) However, an event
    //! can also contain additional quantitative information about the
    //! occurrence in form of event parameters.
    using QSignal = std::uint16_t;
#elif (Q_SIGNAL_SIZE == 4U)
    using QSignal = std::uint32_t;
#else
    #error "Q_SIGNAL_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

#ifndef Q_EVT_CTOR // Is QEvt just a POD (Plain Old Datatype)?

    //! QEvt base class
    //!
    //! @description
    //! QEvt represents events without parameters and serves as the
    //! base class for derivation of events with parameters.
    //!
    //! @usage
    //! The following example illustrates how to add an event parameter by
    //! inheriting from the QEvt class.
    //! @include qep_qevt.cpp
    struct QEvt {
        QSignal sig;                   //!< signal of the event instance
        std::uint8_t poolId_;          //!< pool ID (0 for static event)
        std::uint8_t volatile refCtr_; //!< reference counter
    };

#else // QEvt is a full-blown class (not a POD)

    class QEvt {
    public:
        //! public constructor (overload for dynamic events)
        QEvt(QSignal const s) noexcept
          : sig(s)
          // poolId_/refCtr_ intentionally uninitialized
        {}

        //! public constructor (overload for static events)
        enum StaticEvt : std::uint8_t { STATIC_EVT };
        constexpr QEvt(QSignal const s, StaticEvt /*dummy*/) noexcept
          : sig(s),
            poolId_(0U),
            refCtr_(0U)
        {}

#ifdef Q_EVT_XTOR
        //! virtual destructor
        virtual ~QEvt() noexcept {}
#endif // Q_EVT_XTOR

    public:
        QSignal sig; //!< signal of the event instance

    private:
        std::uint8_t poolId_;          //!< pool ID (0 for static event)
        std::uint8_t volatile refCtr_; //!< reference counter

        friend class QF;
        friend class QS;
        friend class QActive;
        friend class QMActive;
        friend class QTimeEvt;
        friend class QEQueue;
        friend class QTicker;
        friend class QXThread;
        friend std::uint8_t QF_EVT_POOL_ID_ (QEvt const * const e) noexcept;
        friend std::uint8_t QF_EVT_REF_CTR_ (QEvt const * const e) noexcept;
        friend void QF_EVT_REF_CTR_INC_(QEvt const * const e) noexcept;
        friend void QF_EVT_REF_CTR_DEC_(QEvt const * const e) noexcept;
    };

#endif // Q_EVT_CTOR

// forward declarations...
struct QMState;
struct QMTranActTable;
class QXThread;

//! Type returned from state-handler functions
using QState = std::uint_fast8_t;

//! Pointer to state-handler function
using QStateHandler = QState (*)(void * const me, QEvt const * const e);

//! Pointer to an action-handler function
using QActionHandler = QState (*)(void * const me);

//! Pointer to a thread-handler function
using QXThreadHandler = void (*)(QXThread * const me);

//! Attribute of for the QHsm class (Hierarchical State Machine)
//!
//! @description
//! This union represents possible values stored in the 'state' and 'temp'
//! attributes of the QP::QHsm class.
union QHsmAttr {
    QStateHandler   fun;          //!< pointer to a state handler function
    QActionHandler  act;          //!< pointer to an action-handler function
    QXThreadHandler thr;          //!< pointer to an thread-handler function
    QMState         const *obj;   //!< pointer to QMState object
    QMTranActTable  const *tatbl; //!< transition-action table
};

//============================================================================
//! Hierarchical State Machine base class
//!
//! @description
//! QHsm represents a Hierarchical State Machine (HSM) with full support for
//! hierarchical nesting of states, entry/exit actions, and initial
//! transitions in any composite state.<br>
//!
//! QHsm is also the base class for the QMsm state machine, which provides
//! a superior efficiency, but requires the use of the QM modeling tool to
//! generate code.
//!
//! @note
//! QHsm is not intended to be instantiated directly, but rather serves as
//! the base class for derivation of state machines in the application code.
//!
//! @usage
//! The following example illustrates how to derive a state machine class
//! from QHsm.
//! @include qep_qhsm.cpp
//!
class QHsm {
    QHsmAttr m_state;  //!< current active state (state-variable)
    QHsmAttr m_temp;   //!< temporary: transition chain, target state, etc.

public:

#ifdef Q_HSM_XTOR // provide the destructor for the QHsm and its subclasses?
    //! virtual destructor
    virtual ~QHsm() noexcept {}
#endif

    //! executes the top-most initial transition in QP::QHsm
    //!
    //! @description
    //! Executes the top-most initial transition in a HSM.
    //!
    //! @param[in] e   pointer to an extra parameter (might be NULL)
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @note
    //! Must be called exactly __once__ before the QP::QHsm::dispatch().
    //!
    //! @tr{RQP103} @tr{RQP120I} @tr{RQP120D}
    //!
    virtual void init(void const * const e,
                      std::uint_fast8_t const qs_id);

    //! overloaded init(qs_id)
    virtual void init(std::uint_fast8_t const qs_id) {
        init(nullptr, qs_id);
    }

    //! Dispatches an event to QHsm
    //!
    //! @description
    //! Dispatches an event for processing to a hierarchical state machine.
    //! The processing of an event represents one run-to-completion (RTC) step.
    //!
    //! @param[in] e  pointer to the event to be dispatched to the HSM
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @note
    //! This state machine must be initialized by calling QP::QHsm::init()
    //! exactly **once** before calling QP::QHsm::dispatch().
    //!
    //! @tr{RQP103}
    //! @tr{RQP120A} @tr{RQP120B} @tr{RQP120C} @tr{RQP120D} @tr{RQP120E}
    //!
    virtual void dispatch(QEvt const * const e,
                          std::uint_fast8_t const qs_id);

    //! Tests if a given state is part of the current active state
    //! configuration
    //!
    //! @description
    //! Tests if a state machine derived from QHsm is-in a given state.
    //!
    //! @note
    //! For a HSM, to "be in a state" means also to be in a superstate of
    //! of the state.
    //!
    //! @param[in] s pointer to the state-handler function to be tested
    //!
    //! @returns
    //! 'true' if the HSM is in the @p state and 'false' otherwise
    //!
    //! @tr{RQP103}
    //! @tr{RQP120S}
    //!
    bool isIn(QStateHandler const s) noexcept;

    //! Obtain the current state (state handler function)
    //!
    //! @note used in the QM code generation
    QStateHandler state(void) const noexcept {
        return m_state.fun;
    }

    //! Obtain the current active child state of a given parent
    //!
    //! @note used in the QM code generation
    QStateHandler childState(QStateHandler const parent) noexcept;

    //! The top-state handler
    //!
    //! @description
    //! The QP::QHsm::top() state handler is the ultimate root of state
    //! hierarchy in all HSMs derived from QP::QHsm.
    //!
    //! @param[in] me pointer to the HSM instance
    //! @param[in] e  pointer to the event to be dispatched to the HSM
    //!
    //! @returns
    //! Always returns #Q_RET_IGNORED, which means that the top state ignores
    //! all events.
    //!
    //! @note
    //! The parameters to this state handler are not used. They are provided
    //! for conformance with the state-handler function signature
    //! QP::QStateHandler.
    //!
    //! @tr{RQP103} @tr{RQP120T}
    //!
    static QState top(void * const me, QEvt const * const e) noexcept;

protected:
    //! Protected constructor of QHsm
    //!
    //! @description
    //! Performs the first step of HSM initialization by assigning the initial
    //! pseudostate to the currently active state of the state machine.
    //!
    //! @param[in] initial pointer to the top-most initial state-handler
    //!                    function in the derived state machine
    //! @tr{RQP103}
    //!
    explicit QHsm(QStateHandler const initial) noexcept;

public:
// facilities for the QHsm implementation strategy...
    //! event passed to the superstate to handle
    static constexpr QState Q_RET_SUPER     {static_cast<QState>(0)};

    //! event passed to submachine superstate
    static constexpr QState Q_RET_SUPER_SUB {static_cast<QState>(1)};

    //! event unhandled due to a guard evaluating to 'false'
    static constexpr QState Q_RET_UNHANDLED {static_cast<QState>(2)};

    //! event handled (internal transition)
    static constexpr QState Q_RET_HANDLED   {static_cast<QState>(3)};

    //! event silently ignored (bubbled up to top)
    static constexpr QState Q_RET_IGNORED   {static_cast<QState>(4)};

    //! state entry action executed
    static constexpr QState Q_RET_ENTRY     {static_cast<QState>(5)};

    //! state exit  action executed
    static constexpr QState Q_RET_EXIT      {static_cast<QState>(6)};

    //! return value without any effect
    static constexpr QState Q_RET_NULL      {static_cast<QState>(7)};

    //! regular transition taken
    static constexpr QState Q_RET_TRAN      {static_cast<QState>(8)};

    //! initial transition taken
    static constexpr QState Q_RET_TRAN_INIT {static_cast<QState>(9)};

    //! entry-point transition into a submachine
    static constexpr QState Q_RET_TRAN_EP   {static_cast<QState>(10)};

    //! transition to history of a given state
    static constexpr QState Q_RET_TRAN_HIST {static_cast<QState>(11)};

    //! exit-point transition out of a submachine
    static constexpr QState Q_RET_TRAN_XP   {static_cast<QState>(12)};

protected:
    //! Helper function to specify a state transition
    QState tran(QStateHandler const target) noexcept {
        m_temp.fun = target;
        return Q_RET_TRAN;
    }

    //! Helper function to specify a transition to history
    QState tran_hist(QStateHandler const hist) noexcept {
        m_temp.fun = hist;
        return Q_RET_TRAN_HIST;
    }

    //! Helper function to specify the superstate of a given state
    QState super(QStateHandler const superstate) noexcept {
        m_temp.fun = superstate;
        return Q_RET_SUPER;
    }

    enum ReservedHsmSignals : QSignal {
        Q_ENTRY_SIG = 1,     //!< signal for entry actions
        Q_EXIT_SIG,          //!< signal for exit actions
        Q_INIT_SIG           //!< signal for nested initial transitions
    };

// protected facilities for the QMsm implementation strategy...
    //! Helper function to specify a regular state transition
    //! in a QM state-handler
    QState qm_tran(void const * const tatbl) noexcept {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN;
    }

    //! Helper function to specifiy a transition to history
    //! in  a QM state-handler
    QState qm_tran_hist(QMState const * const hist,
                        void const * const tatbl) noexcept
    {
        m_state.obj  = hist;
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_HIST;
    }

    //! Helper function to specify an initial state transition
    //! in a QM state-handler
    QState qm_tran_init(void const * const tatbl) noexcept {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_INIT;
    }

    //! Helper function to specify a transition to an entry point
    //! to a submachine state in a QM state-handler
    QState qm_tran_ep(void const * const tatbl) noexcept {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_EP;
    }

    //! Helper function to specify a transition to an exit point
    //! from a submachine state in a QM state-handler
    QState qm_tran_xp(QActionHandler const xp,
                      void const *const tatbl) noexcept
    {
        m_state.act = xp;
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_XP;
    }

#ifdef Q_SPY
    //! Helper function to specify a state entry in a QM state-handler
    QState qm_entry(QMState const * const s) noexcept {
        m_temp.obj = s;
        return Q_RET_ENTRY;
    }

    //! Helper function to specify a state exit in a QM state-handler
    QState qm_exit(QMState const * const s) noexcept {
        m_temp.obj = s;
        return Q_RET_EXIT;
    }

    //! Get the current state handler of the HSM
    virtual QStateHandler getStateHandler() noexcept;
#else
    //! Helper function to specify a state entry in a QM state-handler
    QState qm_entry(QMState const * const s) noexcept {
        (void)s;
        return Q_RET_ENTRY;
    }

    //! Helper function to specify a state exit in a QM state-handler
    QState qm_exit(QMState const * const s) noexcept {
        (void)s;
        return Q_RET_EXIT;
    }
#endif // Q_SPY

    //! Helper function to specify a submachine exit in a QM state-handler
    QState qm_sm_exit(QMState const * const s) noexcept {
        m_temp.obj = s;
        return Q_RET_EXIT;
    }

    //! Helper function to call in a QM state-handler when it passes
    //! the event to the host submachine state to handle an event.
    QState qm_super_sub(QMState const * const s) noexcept {
        m_temp.obj = s;
        return Q_RET_SUPER_SUB;
    }

private:
    //! maximum nesting depth of states in HSM
    static constexpr std::int_fast8_t MAX_NEST_DEPTH_{6};

    //! Helper function to take a transition in QP::QHsm
    //! @description
    //! helper function to execute transition sequence in a hierarchical state
    //! machine (HSM).
    //!
    //! @param[in,out] path array of pointers to state-handler functions
    //!                     to execute the entry actions
    //! @param[in]     qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @returns
    //! the depth of the entry path stored in the @p path parameter.
    //!
    //! @tr{RQP103}
    //! @tr{RQP120E} @tr{RQP120F}
    //!
    std::int_fast8_t hsm_tran(QStateHandler (&path)[MAX_NEST_DEPTH_],
                              std::uint_fast8_t const qs_id);

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
    friend class QHsmDummy;
    friend class QActiveDummy;
#endif // Q_UTEST
};

//============================================================================
//! QM State Machine implementation strategy
//!
//! @description
//! QP::QMsm (QM State Machine) provides a more efficient state machine
//! implementation strategy than QHsm, but requires the use of the QM
//! modeling tool, but are the fastest and need the least run-time
//! support (the smallest event-processor taking up the least code space).
//!
//! @note
//! QP::QMsm is not intended to be instantiated directly, but rather serves as
//! the base class for derivation of state machines in the application code.
//!
//! @usage
//! The following example illustrates how to derive a state machine class
//! from QP::QMsm:
//! @include qep_qmsm.cpp
//!
class QMsm : public QHsm {
public:
    //! Performs the second step of SM initialization by triggering
    //! the top-most initial transition.
    //!
    //! @description
    //! Executes the top-most initial transition in a MSM.
    //!
    //! @param[in] e   pointer to an extra parameter (might be nullptr)
    //! @param[in]     qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @attention
    //! QP::QMsm::init() must be called exactly __once__ before
    //! QP::QMsm::dispatch()
    //!
    void init(void const * const e,
              std::uint_fast8_t const qs_id) override;

    //! overloaded init(qs_id)
    //!
    //! @description
    //! Executes the top-most initial transition in a MSM (overloaded).
    //!
    //! @param[in]     qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @attention
    //! QP::QMsm::init() must be called exactly __once__ before
    //! QP::QMsm::dispatch()
    //!
    void init(std::uint_fast8_t const qs_id) override;

    //! Dispatches an event to a MSM
    //!
    //! @description
    //! Dispatches an event for processing to a meta state machine (MSM).
    //! The processing of an event represents one run-to-completion (RTC) step.
    //!
    //! @param[in] e  pointer to the event to be dispatched to the MSM
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @note
    //! Must be called after QP::QMsm::init().
    //!
    void dispatch(QEvt const * const e,
                  std::uint_fast8_t const qs_id) override;

    //! Tests if a given state is part of the active state configuration
    //!
    //! @description
    //! Tests if a state machine derived from QMsm is-in a given state.
    //!
    //! @note
    //! For a MSM, to "be-in" a state means also to "be-in" a superstate of
    //! of the state.
    //!
    //! @param[in] st  pointer to the QMState object that corresponds to the
    //!                tested state.
    //! @returns
    //! 'true' if the MSM is in the \c st and 'false' otherwise
    //!
    bool isInState(QMState const * const st) const noexcept;

    //! Return the current active state object (read only)
    QMState const *stateObj(void) const noexcept {
        return m_state.obj;
    }

    //! Obtain the current active child state of a given parent (read only)
    //!
    //! @description
    //! Finds the child state of the given @c parent, such that this child
    //! state is an ancestor of the currently active state. The main purpose
    //! of this function is to support **shallow history** transitions in
    //! state machines derived from QHsm.
    //!
    //! @param[in] parent pointer to the state-handler function
    //!
    //! @returns
    //! the child of a given @c parent state, which is an ancestor of the
    //! currently active state
    //!
    //! @note
    //! this function is designed to be called during state transitions, so it
    //! does not necessarily start in a stable state configuration.
    //! However, the function establishes stable state configuration upon exit.
    //!
    //! @tr{RQP103}
    //! @tr{RQP120H}
    //!
    QMState const *childStateObj(QMState const * const parent) const noexcept;

protected:
    //! Protected constructor
    //! @description
    //! Performs the first step of initialization by assigning the initial
    //! pseudostate to the currently active state of the state machine.
    //!
    //! @param[in] initial  the top-most initial transition for the MSM.
    //!
    //! @note
    //! The constructor is protected to prevent direct instantiating of the
    //! QP::QMsm objects. This class is intended for subclassing only.
    //!
    //! @sa
    //! The QP::QMsm example illustrates how to use the QMsm constructor
    //! in the constructor initializer list of the derived state machines.
    //!
    explicit QMsm(QStateHandler const initial) noexcept;

#ifdef Q_SPY
    //! Get the current state handler of the QMsm
    QStateHandler getStateHandler() noexcept override;
#endif

private:
    //! disallow inherited isIn() function in QP::QMsm and subclasses
    //! @sa QP::QMsm::isInState()
    bool isIn(QStateHandler const s) noexcept = delete;

    //! disallow inhertited state() function in QP::QMsm and subclasses
    //! @sa QP::QMsm::stateObj()
    QStateHandler state(void) const noexcept = delete;

    //! disallow inhertited childState() function in QP::QMsm and subclasses
    //! @sa QP::QMsm::childStateObj()
    QStateHandler childState(QStateHandler const parent) noexcept = delete;

    //! disallow inherited top() function in QP::QMsm and subclasses
    //! @sa QP::QMsm::msm_top_s
    static QState top(void * const me, QEvt const * const e) noexcept = delete;

    //! Internal helper function to execute a transition-action table
    //!
    //! @description
    //! Helper function to execute transition sequence in a tran-action table.
    //!
    //! @param[in] tatbl pointer to the transition-action table
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @returns
    //! the status of the last action from the transition-action table.
    //!
    //! @note
    //! This function is for internal use inside the QEP event processor and
    //! should __not__ be called directly from the applications.
    //!
    QState execTatbl_(QMTranActTable const * const tatbl,
                      std::uint_fast8_t const qs_id);

    //! Internal helper function to exit current state to transition source
    //!
    //! @description
    //! Helper function to exit the current state configuration to the
    //! transition source, which is a hierarchical state machine might be a
    //! superstate of the current state.
    //!
    //! @param[in] s    pointer to the current state
    //! @param[in] ts   pointer to the transition source state
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    void exitToTranSource_(QMState const *s,
                           QMState const * const ts,
                           std::uint_fast8_t const qs_id);

    //! Internal helper function to enter state history
    //!
    //! @description
    //! Static helper function to execute the segment of transition to history
    //! after entering the composite state and
    //!
    //! @param[in] hist pointer to the history substate
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @returns
    //! QP::Q_RET_INIT, if an initial transition has been executed in the last
    //! entered state or QP::Q_RET_NULL if no such transition was taken.
    //!
    QState enterHistory_(QMState const * const hist,
                         std::uint_fast8_t const qs_id);

    //! maximum depth of implemented entry levels for transitions to history
    static constexpr std::int_fast8_t MAX_ENTRY_DEPTH_ {4};

    //! the top state object for the QMsm
    static QMState const msm_top_s;

    friend class QMActive;
};

//! State object for the QP::QMsm class (QM State Machine).
//! @description
//! This class groups together the attributes of a QP::QMsm state, such as
//! the parent state (state nesting), the associated state handler function
//! and the exit action handler function. These attributes are used inside
//! the QP::QMsm::dispatch() and QP::QMsm::init() functions.
//!
//! @attention
//! The QP::QMState class is only intended for the QM code generator and
//! should not be used in hand-crafted code.
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

//============================================================================
//! Offset or the user signals
constexpr enum_t Q_USER_SIG {4};

} // namespace QP

//============================================================================
// Macros for coding QHsm-style state machines...

//! Macro to generate a declaration of a state-handler, state-caller and
//! a state-object for a given state in a subclass of QP::QHsm.
#define Q_STATE_DECL(state_)                                                 \
    QP::QState state_ ## _h(QP::QEvt const * const e);                       \
    static QP::QState state_(void * const me, QP::QEvt const * const e)

//! Macro to generate a definition of a state-handler for a given state
//! in a subclass of QP::QHsm.
#define Q_STATE_DEF(subclass_, state_)                                       \
    QP::QState subclass_::state_(void * const me, QP::QEvt const * const e) {\
        return static_cast<subclass_ *>(me)->state_ ## _h(e); }              \
    QP::QState subclass_::state_ ## _h(QP::QEvt const * const e)

//! Macro to specify that the event was handled
#define Q_HANDLED()           (Q_RET_HANDLED)

//! Macro to specify that the event was NOT handled
//! due to a guard condition evaluating to 'false'
#define Q_UNHANDLED()         (Q_RET_UNHANDLED)

//! Macro to perform casting to QStateHandler.
//! @description
//! This macro encapsulates the cast of a specific state handler function
//! pointer to QStateHandler, which violates MISRA-C 2004 rule 11.4(advisory).
//! This macro helps to localize this deviation.
#define Q_STATE_CAST(handler_) \
    (reinterpret_cast<QP::QStateHandler>(handler_))

//! Macro to perform casting to QActionHandler.
//! @description
//! This macro encapsulates the cast of a specific action handler function
//! pointer to QActionHandler, which violates MISRA-C2004 rule 11.4(advisory).
//! This macro helps to localize this deviation.
#define Q_ACTION_CAST(act_)   (reinterpret_cast<QP::QActionHandler>(act_))

//! Macro to provide strictly-typed zero-action to terminate action lists
//! in the transition-action-tables
#define Q_ACTION_NULL         (nullptr)

//============================================================================
// Macros for coding QMsm-style state machines...

//! Macro to generate a declaration of a state-handler, state-caller and
//! a state-object for a given state in a subclass of QP::QMsm.
#define QM_STATE_DECL(state_)                                                \
    QP::QState state_ ## _h(QP::QEvt const * const e);                       \
    static QP::QState state_(void * const me, QP::QEvt const * const e);     \
    static QP::QMState const state_ ## _s

//! Macro to generate a declaration of a state-handler, state-caller and
//! a state-object for a given *submachine* state in a subclass of QP::QMsm.
#define QM_SM_STATE_DECL(subm_, state_)                                      \
    QP::QState state_ ## _h(QP::QEvt const * const e);                       \
    static QP::QState state_(void * const me, QP::QEvt const * const e);     \
    static SM_ ## subm_ const state_ ## _s

//! Macro to generate a declaration of an action-handler and action-caller
//! in a subclass of QP::QMsm.
#define QM_ACTION_DECL(action_)                                              \
    QP::QState action_ ## _h(void);                                          \
    static QP::QState action_(void * const me)

//! Macro to generate a definition of a state-caller and state-handler
//! for a given state in a subclass of QP::QMsm.
#define QM_STATE_DEF(subclass_, state_)                                      \
    QP::QState subclass_::state_(void * const me, QP::QEvt const * const e) {\
        return static_cast<subclass_ *>(me)->state_ ## _h(e); }              \
    QP::QState subclass_::state_ ## _h(QP::QEvt const * const e)

//! Macro to generate a definition of an action-caller and action-handler
//! in a subclass of QP::QMsm.
#define QM_ACTION_DEF(subclass_, action_)                                    \
    QP::QState subclass_::action_(void * const me) {                         \
        return static_cast<subclass_ *>(me)->action_ ## _h(); }              \
    QP::QState subclass_::action_ ## _h(void)

//! Macro for a QM action-handler when it handles the event.
#define QM_HANDLED()          (Q_RET_HANDLED)

//! Macro for a QM action-handler when it does not handle the event
//! due to a guard condition evaluating to false.
#define QM_UNHANDLED()        (Q_RET_UNHANDLED)

//! Macro for a QM action-handler when it passes the event to the superstate
#define QM_SUPER()            (Q_RET_SUPER)

//! Macro to provide strictly-typed zero-state to use for submachines.
//! Applicable to suclasses of QP::QMsm.
#define QM_STATE_NULL         (nullptr)

#endif // QEP_HPP
