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
#ifndef QP_HPP_
#define QP_HPP_

//============================================================================
#define QP_VERSION_STR "8.0.2"
#define QP_VERSION     802U
// <VER>=802 <DATE>=250120
#define QP_RELEASE     0x6AEAB45DU

//============================================================================
// default configuration settings
//! @cond INTERNAL

#ifndef Q_SIGNAL_SIZE
#define Q_SIGNAL_SIZE 2U
#endif

#ifndef QF_MAX_ACTIVE
#define QF_MAX_ACTIVE 32U
#endif

#if (QF_MAX_ACTIVE > 64U)
#error QF_MAX_ACTIVE exceeds the maximum of 64U;
#endif

#ifndef QF_MAX_TICK_RATE
#define QF_MAX_TICK_RATE 1U
#endif

#if (QF_MAX_TICK_RATE > 15U)
#error QF_MAX_TICK_RATE exceeds the maximum of 15U;
#endif

#ifndef QF_MAX_EPOOL
#define QF_MAX_EPOOL 3U
#endif

#if (QF_MAX_EPOOL > 15U)
#error QF_MAX_EPOOL exceeds the maximum of 15U;
#endif

#ifndef QF_TIMEEVT_CTR_SIZE
#define QF_TIMEEVT_CTR_SIZE 4U
#endif

#if (QF_TIMEEVT_CTR_SIZE > 4U)
#error QF_TIMEEVT_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U;
#endif

#ifndef QF_EVENT_SIZ_SIZE
#define QF_EVENT_SIZ_SIZE 2U
#endif

#if (QF_EVENT_SIZ_SIZE > 4U)
#error QF_EVENT_SIZ_SIZE defined incorrectly, expected 1U, 2U, or 4U;
#endif

//! @endcond

//============================================================================
// global types/utilities

using int_t  = int;

using enum_t  = int;
using float32_t  = float;
using float64_t  = double;

#define Q_UNUSED_PAR(par_)  (static_cast<void>(par_))
#define Q_DIM(array_)       (sizeof(array_) / sizeof((array_)[0U]))
#define Q_UINT2PTR_CAST(type_, uint_) (reinterpret_cast<type_ *>(uint_))

//============================================================================

namespace QP {

extern char const versionStr[24];

// QSignal type
#if (Q_SIGNAL_SIZE == 1U)
    using QSignal = std::uint8_t;
#elif (Q_SIGNAL_SIZE == 2U)
    using QSignal = std::uint16_t;
#elif (Q_SIGNAL_SIZE == 4U)
    using QSignal = std::uint32_t;
#endif

//============================================================================

class QEvt {
public:
    QSignal sig;
    std::uint8_t poolNum_;
    std::uint8_t volatile refCtr_;

public:
    enum DynEvt: std::uint8_t { DYNAMIC };

public:
    explicit constexpr QEvt(QSignal const s) noexcept
      : sig(s),
        poolNum_(0x00U),
        refCtr_(0xE0U)
    {}

    QEvt() = delete;
    void init() noexcept {
        // no event parameters to initialize
    }
    void init(DynEvt const dummy) noexcept {
        Q_UNUSED_PAR(dummy);
        // no event parameters to initialize
    }
}; // class QEvt

using QEvtPtr = QEvt const *;

//============================================================================
// QEP (hierarchical event processor) types

using QState = std::uint_fast8_t;

class QXThread; // forward declaration

using QStateHandler = QState (*)(void * const me, QEvt const * const e);
using QActionHandler = QState (*)(void * const me);
using QXThreadHandler = void (*)(QXThread * const me);

struct QMState {
    QMState const *superstate;
    QStateHandler const stateHandler;
    QActionHandler const entryAction;
    QActionHandler const exitAction;
    QActionHandler const initAction;
};

struct QMTranActTable {
    QMState const *target;
    QActionHandler const act[1];
};

union QAsmAttr {
    QStateHandler   fun;
    QActionHandler  act;
    QXThreadHandler thr;
    QMState         const *obj;
    QMTranActTable  const *tatbl;
};

constexpr enum_t  Q_USER_SIG {4};

//============================================================================
class QAsm {
protected:
    QAsmAttr m_state;
    QAsmAttr m_temp;

public:

    //! All possible return values from state-handlers
    //! NOTE: The ordering is important for algorithmic correctness.
    enum QStateRet : QState {
        // unhandled and need to "bubble up"
        Q_RET_SUPER,     //!< event passed to superstate to handle
        Q_RET_UNHANDLED, //!< event unhandled due to a guard

        // handled and do not need to "bubble up"
        Q_RET_HANDLED,   //!< event handled (internal transition)
        Q_RET_IGNORED,   //!< event silently ignored (bubbled up to top)

        // entry/exit
        Q_RET_ENTRY,     //!< state entry action executed
        Q_RET_EXIT,      //!< state exit  action executed

        // no side effects
        Q_RET_NULL,      //!< return value without any effect

        // transitions need to execute transition-action table in QP::QMsm
        Q_RET_TRAN,      //!< regular transition
        Q_RET_TRAN_INIT, //!< initial transition in a state

        // transitions that additionally clobber QHsm.m_state
        Q_RET_TRAN_HIST, //!< transition to history of a given state
    };

    //! Reserved signals by the QP-framework.
    enum ReservedSig : QSignal {
        Q_EMPTY_SIG,     //!< signal to execute the default case
        Q_ENTRY_SIG,     //!< signal for entry actions
        Q_EXIT_SIG,      //!< signal for exit actions
        Q_INIT_SIG       //!< signal for nested initial transitions
    };

protected:
    explicit QAsm() noexcept
      : m_state(),
        m_temp ()
    {}

public:

#ifdef Q_XTOR
    virtual ~QAsm() noexcept {
        // empty
    }
#endif // def Q_XTOR
    virtual void init(
        void const * const e,
        std::uint_fast8_t const qsId) = 0;
    virtual void init(std::uint_fast8_t const qsId) {
        this->init(nullptr, qsId);
    }
    virtual void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) = 0;
    virtual bool isIn(QStateHandler const state) noexcept {
        static_cast<void>(state);
        return false;
    }
    QStateHandler state() const noexcept {
        return m_state.fun;
    }
    QMState const * stateObj() const noexcept {
        return m_state.obj;
    }

#ifdef Q_SPY
    virtual QStateHandler getStateHandler() noexcept {
        return m_state.fun;
    }
#endif // def Q_SPY

    static QState top(
        void * const me,
        QEvt const * const e) noexcept
    {
        static_cast<void>(me);
        static_cast<void>(e);
        return Q_RET_IGNORED; // the top state ignores all events
    }

protected:
    QState tran(QStateHandler const target) noexcept {
        m_temp.fun = target;
        return Q_RET_TRAN;
    }
    QState tran_hist(QStateHandler const hist) noexcept {
        m_temp.fun = hist;
        return Q_RET_TRAN_HIST;
    }
    QState super(QStateHandler const superstate) noexcept {
        m_temp.fun = superstate;
        return Q_RET_SUPER;
    }
    QState qm_tran(void const * const tatbl) noexcept {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN;
    }
    QState qm_tran_init(void const * const tatbl) noexcept {
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_INIT;
    }
    QState qm_tran_hist(
        QMState const * const hist,
        void const * const tatbl) noexcept
    {
        m_state.obj  = hist;
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_HIST;
    }

#ifdef Q_SPY
    QState qm_entry(QMState const * const s) noexcept {
        m_temp.obj = s;
        return Q_RET_ENTRY;
    }
#endif // def Q_SPY

#ifndef Q_SPY
    QState qm_entry(QMState const * const s) noexcept {
        static_cast<void>(s); // unused parameter
        return Q_RET_ENTRY;
    }
#endif // ndef Q_SPY

#ifdef Q_SPY
    QState qm_exit(QMState const * const s) noexcept {
        m_temp.obj = s;
        return Q_RET_EXIT;
    }
#endif // def Q_SPY

#ifndef Q_SPY
    QState qm_exit(QMState const * const s) noexcept {
        static_cast<void>(s); // unused parameter
        return Q_RET_EXIT;
    }
#endif // ndef Q_SPY
}; // class QAsm

//============================================================================
class QHsm : public QP::QAsm {
protected:
    explicit QHsm(QStateHandler const initial) noexcept;

public:
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void init(std::uint_fast8_t const qsId) override {
        this->init(nullptr, qsId);
    }
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;
    bool isIn(QStateHandler const state) noexcept override;
    QStateHandler childState(QStateHandler const parent) noexcept;

#ifdef Q_SPY
    QStateHandler getStateHandler() noexcept override {
        return m_state.fun;
    }
#endif // def Q_SPY

private:
    std::int_fast8_t tran_simple_(
        QStateHandler * const path,
        std::uint_fast8_t const qsId);

    std::int_fast8_t tran_complex_(
        QStateHandler * const path,
        std::uint_fast8_t const qsId);

    void enter_target_(
        QStateHandler * const path,
        std::int_fast8_t const depth,
        std::uint_fast8_t const qsId);
}; // class QHsm

//============================================================================
class QMsm : public QP::QAsm {
protected:
    explicit QMsm(QStateHandler const initial) noexcept;

public:
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void init(std::uint_fast8_t const qsId) override {
        this->init(nullptr, qsId);
    }
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;

#ifdef Q_SPY
    QStateHandler getStateHandler() noexcept override {
        return m_state.obj->stateHandler;
    }
#endif // def Q_SPY
    bool isIn(QStateHandler const state) noexcept override;
    QMState const * childStateObj(QMState const * const parent) const noexcept;

private:
    QState execTatbl_(
        QMTranActTable const * const tatbl,
        std::uint_fast8_t const qsId);
    void exitToTranSource_(
        QMState const * const cs,
        QMState const * const ts,
        std::uint_fast8_t const qsId);
    QState enterHistory_(
        QMState const * const hist,
        std::uint_fast8_t const qsId);

public:
    QMState const * topQMState() const noexcept;
}; // class QMsm

} // namespace QP

//============================================================================
// QEP-macros

#define Q_STATE_DECL(state_) \
    QP::QState state_ ## _h(QP::QEvt const * const e); \
    static QP::QState state_(void * const me, QP::QEvt const * const e)

#define Q_STATE_DEF(subclass_, state_) \
    QP::QState subclass_::state_(void * const me, QP::QEvt const * const e) { \
        return static_cast<subclass_ *>(me)->state_ ## _h(e); } \
    QP::QState subclass_::state_ ## _h(QP::QEvt const * const e)

#define Q_HANDLED() (Q_RET_HANDLED)
#define Q_UNHANDLED() (Q_RET_UNHANDLED)

#define Q_EVT_CAST(subclass_) (static_cast<subclass_ const *>(e))
#define Q_STATE_CAST(handler_) (reinterpret_cast<QP::QStateHandler>(handler_))

#define QM_STATE_DECL(state_) \
    QP::QState state_ ## _h(QP::QEvt const * const e); \
    static QP::QState state_(void * const me, QP::QEvt const * const e); \
    static QP::QMState const state_ ## _s

#define QM_ACTION_DECL(action_) \
    QP::QState action_ ## _h(); \
    static QP::QState action_(void * const me)

#define QM_STATE_DEF(subclass_, state_) \
    QP::QState subclass_::state_(void * const me, QP::QEvt const * const e) {\
        return static_cast<subclass_ *>(me)->state_ ## _h(e); } \
    QP::QState subclass_::state_ ## _h(QP::QEvt const * const e)

#define QM_ACTION_DEF(subclass_, action_)  \
    QP::QState subclass_::action_(void * const me) { \
        return static_cast<subclass_ *>(me)->action_ ## _h(); } \
    QP::QState subclass_::action_ ## _h()

#define QM_HANDLED()   (Q_RET_HANDLED)
#define QM_UNHANDLED() (Q_RET_HANDLED)
#define QM_SUPER()     (Q_RET_SUPER)
#define QM_STATE_NULL  (nullptr)
#define Q_ACTION_NULL  (nullptr)

#ifdef Q_SPY
    #define INIT(qsId_)         init((qsId_))
    #define DISPATCH(e_, qsId_) dispatch((e_), (qsId_))
#else
    #define INIT(dummy)         init(0U)
    #define DISPATCH(e_, dummy) dispatch((e_), 0U)
#endif // ndef Q_SPY

//============================================================================
namespace QP {

using QPrioSpec = std::uint16_t;

#if (QF_TIMEEVT_CTR_SIZE == 1U)
    using QTimeEvtCtr = std::uint8_t;
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    using QTimeEvtCtr = std::uint16_t;
#elif (QF_TIMEEVT_CTR_SIZE == 4U)
    using QTimeEvtCtr = std::uint32_t;
#endif // (QF_TIMEEVT_CTR_SIZE == 4U)

#if (QF_MAX_ACTIVE <= 8U)
    using QPSetBits = std::uint8_t;
#elif (8U < QF_MAX_ACTIVE) && (QF_MAX_ACTIVE <= 16U)
    using QPSetBits = std::uint16_t;
#elif (16 < QF_MAX_ACTIVE)
    using QPSetBits = std::uint32_t;
#endif // (16 < QF_MAX_ACTIVE)

#ifndef QF_LOG2
    std::uint_fast8_t QF_LOG2(QP::QPSetBits const bitmask) noexcept;
#endif // ndef QF_LOG2

//============================================================================
class QPSet {
private:
    QPSetBits m_bits[((QF_MAX_ACTIVE + (8U*sizeof(QPSetBits))) - 1U)/(8U*sizeof(QPSetBits))];

public:
    void setEmpty() noexcept {
        m_bits[0] = 0U;
#if (QF_MAX_ACTIVE > 32)
        m_bits[1] = 0U;
#endif
    }
    bool isEmpty() const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
        return (m_bits[0] == 0U);
#else
        return (m_bits[0] == 0U) ? (m_bits[1] == 0U) : false;
#endif
    }
    bool notEmpty() const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
        return (m_bits[0] != 0U);
#else
        return (m_bits[0] != 0U) ? true : (m_bits[1] != 0U);
#endif
    }
    bool hasElement(std::uint_fast8_t const n) const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
        return (m_bits[0] & (static_cast<QPSetBits>(1U) << (n - 1U))) != 0U;
#else
        return (n <= 32U)
            ? ((m_bits[0] & (static_cast<QPSetBits>(1U) << (n - 1U)))  != 0U)
            : ((m_bits[1] & (static_cast<QPSetBits>(1U) << (n - 33U))) != 0U);
#endif
    }
    void insert(std::uint_fast8_t const n) noexcept {
#if (QF_MAX_ACTIVE <= 32U)
        m_bits[0] = (m_bits[0] | (static_cast<QPSetBits>(1U) << (n - 1U)));
#else
        if (n <= 32U) {
            m_bits[0] = (m_bits[0] | (static_cast<QPSetBits>(1U) << (n - 1U)));
        }
        else {
            m_bits[1] = (m_bits[1] | (static_cast<QPSetBits>(1U) << (n - 33U)));
        }
#endif
    }
    void remove(std::uint_fast8_t const n) noexcept {
#if (QF_MAX_ACTIVE <= 32U)
        m_bits[0] = (m_bits[0] & static_cast<QPSetBits>(~(1U << (n - 1U))));
#else
        if (n <= 32U) {
            (m_bits[0] = (m_bits[0] & ~(static_cast<QPSetBits>(1U) << (n - 1U))));
        }
        else {
            (m_bits[1] = (m_bits[1] & ~(static_cast<QPSetBits>(1U) << (n - 33U))));
        }
#endif
    }
    std::uint_fast8_t findMax() const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
        return QF_LOG2(m_bits[0]);
#else
        return (m_bits[1] != 0U)
            ? (QF_LOG2(m_bits[1]) + 32U)
            : (QF_LOG2(m_bits[0]));
#endif
    }

}; // class QPSet

//============================================================================
class QSubscrList {
private:
    QPSet m_set;

    // friends...
    friend class QActive;
}; // class QSubscrList

//============================================================================
class QPtrDis {
private:
    std::uintptr_t m_ptr_dis;

    // friends...
    friend class QTimeEvt;
    friend class QXThread;

public:
    QPtrDis(void const * const ptr = nullptr) noexcept;
}; // class QPtrDis

class QEQueue; // forward declaration


//============================================================================
class QActive : public QP::QAsm {
protected:
    std::uint8_t m_prio;
    std::uint8_t m_pthre;

#ifdef QACTIVE_THREAD_TYPE
    QACTIVE_THREAD_TYPE m_thread;
#endif // def QACTIVE_THREAD_TYPE

#ifdef QACTIVE_OS_OBJ_TYPE
    QACTIVE_OS_OBJ_TYPE m_osObject;
#endif // def QACTIVE_OS_OBJ_TYPE

#ifdef QACTIVE_EQUEUE_TYPE
    QACTIVE_EQUEUE_TYPE m_eQueue;
#endif // def QACTIVE_EQUEUE_TYPE

public:
    static QActive * registry_[QF_MAX_ACTIVE + 1U];
    static QSubscrList * subscrList_;
    static enum_t maxPubSignal_;

    // friends...
    friend class QTimeEvt;
    friend class QTicker;
    friend class QXThread;
    friend class QXMutex;
    friend class QXSemaphore;
    friend class QActiveDummy;
    friend class GuiQActive;
    friend class GuiQMActive;
    friend void schedLock();

protected:
    explicit QActive(QStateHandler const initial) noexcept
      : QAsm(),
        m_prio(0U),
        m_pthre(0U)
    {
        m_state.fun = Q_STATE_CAST(&top);
        m_temp.fun  = initial;
    }

public:
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override
    {
        reinterpret_cast<QHsm *>(this)->QHsm::init(e, qsId);
    }
    void init(std::uint_fast8_t const qsId) override {
        this->init(nullptr, qsId);
    }
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override
    {
        reinterpret_cast<QHsm *>(this)->QHsm::dispatch(e, qsId);
    }
    bool isIn(QStateHandler const state) noexcept override {
        return reinterpret_cast<QHsm *>(this)->QHsm::isIn(state);
    }
    QStateHandler childState(QStateHandler const parent) noexcept {
        return reinterpret_cast<QHsm *>(this)->QHsm::childState(parent);
    }
    void setAttr(
        std::uint32_t attr1,
        void const * attr2 = nullptr);
    void start(
        QPrioSpec const prioSpec,
        QEvtPtr * const qSto,
        std::uint_fast16_t const qLen,
        void * const stkSto,
        std::uint_fast16_t const stkSize,
        void const * const par);
    void start(
        QPrioSpec const prioSpec,
        QEvtPtr * const qSto,
        std::uint_fast16_t const qLen,
        void * const stkSto,
        std::uint_fast16_t const stkSize)
    {
        this->start(prioSpec, qSto, qLen, stkSto, stkSize, nullptr);
    }

#ifdef QACTIVE_CAN_STOP
    void stop();
#endif // def QACTIVE_CAN_STOP
    void register_() noexcept;
    void unregister_() noexcept;
    bool post_(
        QEvt const * const e,
        std::uint_fast16_t const margin,
        void const * const sender) noexcept;
    void postLIFO(QEvt const * const e) noexcept;
    QEvt const * get_() noexcept;
    static std::uint_fast16_t getQueueMin(std::uint_fast8_t const prio) noexcept;
    static void psInit(
        QSubscrList * const subscrSto,
        enum_t const maxSignal) noexcept;
    static void publish_(
        QEvt const * const e,
        void const * const sender,
        std::uint_fast8_t const qsId) noexcept;
    void subscribe(enum_t const sig) const noexcept;
    void unsubscribe(enum_t const sig) const noexcept;
    void unsubscribeAll() const noexcept;
    bool defer(
        QEQueue * const eq,
        QEvt const * const e) const noexcept;
    bool recall(QEQueue * const eq) noexcept;
    std::uint_fast16_t flushDeferred(
        QEQueue * const eq,
        std::uint_fast16_t const num = 0xFFFFU) const noexcept;
    std::uint_fast8_t getPrio() const noexcept {
        return static_cast<std::uint_fast8_t>(m_prio);
    }
    void setPrio(QPrioSpec const prio) noexcept {
        m_prio  = static_cast<std::uint8_t>(prio & 0xFFU);
        m_pthre = static_cast<std::uint8_t>(prio >> 8U);
    }
    std::uint_fast8_t getPThre() const noexcept {
        return static_cast<std::uint_fast8_t>(m_pthre);
    }

#ifdef QACTIVE_EQUEUE_TYPE
    QACTIVE_EQUEUE_TYPE const & getEQueue() const noexcept {
        return m_eQueue;
    }
#endif // def QACTIVE_EQUEUE_TYPE

#ifdef QACTIVE_OS_OBJ_TYPE
    QACTIVE_OS_OBJ_TYPE const & getOsObject() const noexcept {
        return m_osObject;
    }
#endif // def QACTIVE_OS_OBJ_TYPE

#ifdef QACTIVE_THREAD_TYPE
    QACTIVE_THREAD_TYPE const & getThread() const noexcept {
        return m_thread;
    }
#endif // def QACTIVE_THREAD_TYPE

#ifdef QACTIVE_THREAD_TYPE
    void setThread(QACTIVE_THREAD_TYPE const & thr) {
        m_thread = thr;
    }
#endif // def QACTIVE_THREAD_TYPE
    static void evtLoop_(QActive * act);

#ifdef QF_ISR_API
    virtual bool postFromISR(
        QEvt const * const e,
        std::uint_fast16_t const margin,
        void * par,
        void const * const sender) noexcept;
#endif // def QF_ISR_API

#ifdef QF_ISR_API
    static void publishFromISR(
        QEvt const * e,
        void * par,
        void const * sender) noexcept;
#endif // def QF_ISR_API

private:
    void postFIFO_(
        QEvt const * const e,
        void const * const sender);
}; // class QActive

//============================================================================
class QMActive : public QP::QActive {
protected:
    QMActive(QStateHandler const initial) noexcept;

public:
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override
    {
        reinterpret_cast<QMsm *>(this)->QMsm::init(e, qsId);
    }
    void init(std::uint_fast8_t const qsId) override {
        this->init(nullptr, qsId);
    }
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override
    {
        reinterpret_cast<QMsm *>(this)->QMsm::dispatch(e, qsId);
    }
    bool isIn(QStateHandler const state) noexcept override {
        return reinterpret_cast<QMsm *>(this)->QMsm::isIn(state);
    }

#ifdef Q_SPY
    QStateHandler getStateHandler() noexcept override {
        return reinterpret_cast<QMsm *>(this)->QMsm::getStateHandler();
    }
#endif // def Q_SPY
    QMState const * childStateObj(QMState const * const parent) const noexcept {
        return reinterpret_cast<QMsm const *>(this)
                   ->QMsm::childStateObj(parent);
    }
}; // class QMActive


//============================================================================
class QTimeEvt : public QP::QEvt {
private:
    QTimeEvt * volatile m_next;
    void * m_act;
    QTimeEvtCtr volatile m_ctr;
    QTimeEvtCtr m_interval;
    std::uint8_t m_tickRate;
    std::uint8_t m_flags;

public:
    static QTimeEvt timeEvtHead_[QF_MAX_TICK_RATE];

private:
    friend class QXThread;

public:
    QTimeEvt(
        QActive * const act,
        QSignal const sig,
        std::uint_fast8_t const tickRate = 0U) noexcept;
    void armX(
        std::uint32_t const nTicks,
        std::uint32_t const interval = 0U) noexcept;
    bool disarm() noexcept;
    bool rearm(std::uint32_t const nTicks) noexcept;
    bool wasDisarmed() noexcept;
    void const * getAct() const noexcept {
        return m_act;
    }
    QTimeEvtCtr getCtr() const noexcept {
        return m_ctr;
    }
    QTimeEvtCtr getInterval() const noexcept {
        return m_interval;
    }
    std::uint8_t getTickRate() const noexcept {
        return m_tickRate;
    }
    static void tick(
        std::uint_fast8_t const tickRate,
        void const * const sender) noexcept;

#ifdef Q_UTEST
    static void tick1_(
        std::uint_fast8_t const tickRate,
        void const * const sender);
#endif // def Q_UTEST

#ifdef QF_ISR_API
    static void tickFromISR(
        std::uint_fast8_t const tickRate,
        void * par,
        void const * sender) noexcept;
#endif // def QF_ISR_API
    static bool noActive(std::uint_fast8_t const tickRate) noexcept;
    QActive * toActive() noexcept {
        return static_cast<QActive *>(m_act);
    }
    QTimeEvt * toTimeEvt() noexcept {
        return static_cast<QTimeEvt *>(m_act);
    }

private:
    QTimeEvt() noexcept;
    QTimeEvt(QTimeEvt const & other) = delete;
    QTimeEvt & operator=(QTimeEvt const & other) = delete;
    QTimeEvt *expire_(
        QTimeEvt * const prev_link,
        QActive const * const act,
        std::uint_fast8_t const tickRate) noexcept;
}; // class QTimeEvt

//============================================================================
class QTicker : public QP::QActive {
public:
    explicit QTicker(std::uint_fast8_t const tickRate) noexcept;
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void init(std::uint_fast8_t const qsId) override {
        this->init(nullptr, qsId);
    }
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;
    void trig_(void const * const sender) noexcept;
}; // class QTicker

//============================================================================
namespace QF {

void init();
void stop();

int_t run();

void onStartup();
void onCleanup();

//! @deprecated
inline void psInit(
    QSubscrList * const subscrSto,
    enum_t const maxSignal) noexcept
{
    QActive::psInit(subscrSto, maxSignal);
}

//! @deprecated
inline void publish_(
    QEvt const * const e,
    void const * const sender,
    std::uint_fast8_t const qsId) noexcept
{
    QActive::publish_(e, sender, qsId);
}

//! @deprecated
inline void tick(
    std::uint_fast8_t const tickRate,
    void const * const sender) noexcept
{
    QTimeEvt::tick(tickRate, sender);
}

//! @deprecated
inline std::uint_fast16_t getQueueMin(std::uint_fast8_t const prio) noexcept {
    return QActive::getQueueMin(prio);
}

constexpr std::uint_fast16_t NO_MARGIN {0xFFFFU};

//============================================================================
// QF dynamic memory facilities
void poolInit(
    void * const poolSto,
    std::uint_fast32_t const poolSize,
    std::uint_fast16_t const evtSize) noexcept;

std::uint_fast16_t poolGetMaxBlockSize() noexcept;
std::uint_fast16_t getPoolMin(std::uint_fast8_t const poolNum) noexcept;
QEvt * newX_(
    std::uint_fast16_t const evtSize,
    std::uint_fast16_t const margin,
    enum_t const sig) noexcept;
void gc(QEvt const * const e) noexcept;
QEvt const * newRef_(
    QEvt const * const e,
    QEvt const * const evtRef) noexcept;

void deleteRef_(QEvt const * const evtRef) noexcept;

#ifndef QEVT_PAR_INIT
    template<class evtT_>
    inline evtT_ * q_new(enum_t const sig) {
        return static_cast<evtT_*>(
            QP::QF::newX_(sizeof(evtT_), QP::QF::NO_MARGIN, sig));
    }
    template<class evtT_>
    inline evtT_ * q_new_x(
        std::uint_fast16_t const margin,
        enum_t const sig)
    {
        return static_cast<evtT_*>(QP::QF::newX_(sizeof(evtT_), margin, sig));
    }
#else
    template<class evtT_, typename... Args>
    inline evtT_ * q_new(
        enum_t const sig,
        Args... args)
    {
        evtT_ *e = static_cast<evtT_*>(
            QP::QF::newX_(sizeof(evtT_), QP::QF::NO_MARGIN, sig));
        e->init(args...); // e cannot be nullptr
        return e;
    }
    template<class evtT_, typename... Args>
    inline evtT_ * q_new_x(
        std::uint_fast16_t const margin,
        enum_t const sig,
        Args... args)
    {
        evtT_ *e = static_cast<evtT_*>(QP::QF::newX_(sizeof(evtT_), margin, sig));
        if (e != nullptr) {
            e->init(args...);
        }
        return e;
    }
#endif // def QEVT_PAR_INIT

template<class evtT_>
inline void q_new_ref(
    QP::QEvt const * const e,
    evtT_ const *& evtRef)
{
    evtRef = static_cast<evtT_ const *>(QP::QF::newRef_(e, evtRef));
}

template<class evtT_>
inline void q_delete_ref(evtT_ const *& evtRef) {
    QP::QF::deleteRef_(evtRef);
    evtRef = nullptr;
}

#ifdef QF_ISR_API
QEvt * newXfromISR_(
    std::uint_fast16_t const evtSize,
    std::uint_fast16_t const margin,
    enum_t const sig) noexcept;
void gcFromISR(QEvt const * e) noexcept;
#endif // def QF_ISR_API

} // namespace QF
} // namespace QP

//============================================================================
extern "C" {

//${QF-extern-C::QF_onContextSw} .............................................
#ifdef QF_ON_CONTEXT_SW
void QF_onContextSw(
    QP::QActive * prev,
    QP::QActive * next);
#endif // def QF_ON_CONTEXT_SW
} // extern "C"

//============================================================================
// QF base facilities

#define Q_PRIO(prio_, pthre_) \
    (static_cast<QP::QPrioSpec>((prio_) | (pthre_) << 8U))

#ifndef QEVT_PAR_INIT
    #define Q_NEW(evtT_, sig_) (QP::QF::q_new<evtT_>((sig_)))
    #define Q_NEW_X(evtT_, margin_, sig_) (QP::QF::q_new_x<evtT_>((margin_), (sig_)))
#else
    #define Q_NEW(evtT_, sig_, ...) (QP::QF::q_new<evtT_>((sig_), __VA_ARGS__))
    #define Q_NEW_X(evtT_, margin_, sig_, ...) (QP::QF::q_new_x<evtT_>((margin_), (sig_), __VA_ARGS__))
#endif // QEVT_PAR_INIT

#define Q_NEW_REF(evtRef_, evtT_) (QP::QF::q_new_ref<evtT_>(e, (evtRef_)))
#define Q_DELETE_REF(evtRef_) do { \
    QP::QF::deleteRef_((evtRef_)); \
    (evtRef_) = nullptr; \
} while (false)

#ifdef Q_SPY
    #define PUBLISH(e_, sender_) \
        publish_((e_), (sender_), (sender_)->getPrio())
    #define POST(e_, sender_) post_((e_), QP::QF::NO_MARGIN, (sender_))
    #define POST_X(e_, margin_, sender_) \
        post_((e_), (margin_), (sender_))
    #define TICK_X(tickRate_, sender_) tick((tickRate_), (sender_))
    #define TRIG(sender_) trig_((sender_))
#else
    #define PUBLISH(e_, dummy) publish_((e_), nullptr, 0U)
    #define POST(e_, dummy) post_((e_), QP::QF::NO_MARGIN, nullptr)
    #define POST_X(e_, margin_, dummy) post_((e_), (margin_), nullptr)
    #define TICK_X(tickRate_, dummy) tick((tickRate_), nullptr)
    #define TRIG(sender_) trig_(nullptr)
#endif // ndef Q_SPY

#define TICK(sender_) TICK_X(0U, (sender_))

#ifndef QF_CRIT_EXIT_NOP
    #define QF_CRIT_EXIT_NOP() (static_cast<void>(0))
#endif // ndef QF_CRIT_EXIT_NOP

//============================================================================
// memory protection facilities

#ifdef QF_MEM_ISOLATE
    #error Memory isolation not supported in this QP edition, need SafeQP
#endif // def QF_MEM_ISOLATE

#endif // QP_HPP_
