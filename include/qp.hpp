//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
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
#define QP_VERSION_STR  "8.1.1"
#define QP_VERSION      811U
// <VER>=810 <DATE>=251008
#define QP_RELEASE      0x6A6334D4U

//----------------------------------------------------------------------------
// default configuration settings
//! @cond INTERNAL

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

//----------------------------------------------------------------------------
// global types/utilities

using int_t  = int;

#define Q_UNUSED_PAR(par_)  (static_cast<void>(par_))
#define Q_DIM(array_)       (sizeof(array_) / sizeof((array_)[0U]))
#define Q_UINT2PTR_CAST(type_, uint_) (reinterpret_cast<type_ *>(uint_))

//============================================================================
namespace QP {

char const *version() noexcept;

using QSignal = std::uint16_t;

//----------------------------------------------------------------------------
class QEvt {
public:
    std::uint32_t sig         : 16;
    std::uint32_t poolNum_    :  8;
    std::uint32_t refCtr_     :  8;
    std::uint32_t filler_;

    enum DynEvt: std::uint8_t { DYNAMIC };

    explicit constexpr QEvt(QSignal const s) noexcept
      : sig(s)            // the provided event signal
        ,poolNum_(0x00U)  // no-pool event
        ,refCtr_ (0xE0U)  // special event "marker"
        ,filler_ (0xE0E0E0E0U) // the "filler" ensures the same QEvt size
                               // as in SafeQP/C++
    {}

    QEvt() // disallow the default ctor
        = delete;
    void init() const noexcept {
        // no event parameters to initialize
    }
    void init(DynEvt const dummy) const noexcept {
        Q_UNUSED_PAR(dummy);
        // no event parameters to initialize
    }
}; // class QEvt

using QEvtPtr = QEvt const *;

//----------------------------------------------------------------------------
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
    std::uintptr_t  uint;
};

constexpr QSignal Q_USER_SIG {4U};

//----------------------------------------------------------------------------
class QAsm {
public:
    QAsmAttr m_state;
    QAsmAttr m_temp;

    // All possible values returned from state/action handlers...
    // NOTE: The numerical order is important for algorithmic correctness.
    static constexpr QState Q_RET_SUPER     {0U};
    static constexpr QState Q_RET_UNHANDLED {1U};
    static constexpr QState Q_RET_HANDLED   {2U};
    static constexpr QState Q_RET_TRAN      {3U};
    static constexpr QState Q_RET_TRAN_HIST {4U};

    // used in QHsm only...
    static constexpr QState Q_RET_IGNORED   {5U};

    // used in QMsm only...
    static constexpr QState Q_RET_ENTRY     {6U};
    static constexpr QState Q_RET_EXIT      {7U};
    static constexpr QState Q_RET_TRAN_INIT {8U};


    // Reserved signals by the QP-framework (used in QHsm only)
    static constexpr QSignal Q_EMPTY_SIG    {0U};
    static constexpr QSignal Q_ENTRY_SIG    {1U};
    static constexpr QSignal Q_EXIT_SIG     {2U};
    static constexpr QSignal Q_INIT_SIG     {3U};

    static constexpr QState Q_HANDLED()     { // for coding QHsm state machines
        return Q_RET_HANDLED;   }
    static constexpr QState Q_UNHANDLED()   { // for coding QHsm state machines
        return Q_RET_UNHANDLED; }
    static constexpr QState QM_HANDLED()    { // for coding QMsm state machines
        return Q_RET_HANDLED;   }
    static constexpr QState QM_UNHANDLED()  { // for coding QMsm state machines
        return Q_RET_HANDLED;   }
    static constexpr QState QM_SUPER()      { // for coding QMsm state machines
        return Q_RET_SUPER;     }
    static constexpr QMState const *QM_STATE_NULL       { nullptr };
    static constexpr QActionHandler const Q_ACTION_NULL { nullptr };

#ifdef Q_XTOR
    virtual ~QAsm() noexcept {
        // empty
    }
#endif // def Q_XTOR
    virtual void init(
        void const * const e,
        std::uint_fast8_t const qsId) = 0;
    virtual void init(std::uint_fast8_t const qsId);
    virtual void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) = 0;
    virtual bool isIn(QStateHandler const stateHndl) = 0;
    virtual QStateHandler getStateHandler() const noexcept = 0;

    QStateHandler state() const noexcept {
        return m_state.fun; // public "getter" for the state handler
    }
    QMState const * stateObj() const noexcept {
        return m_state.obj; // public "getter" for the state object
    }

    static QState top(void * const me, QEvt const * const e) noexcept;

protected:
    explicit QAsm() noexcept;

    QState tran(QStateHandler const target) noexcept {
        // for coding QMsm state machines
        m_temp.fun = target;
        return Q_RET_TRAN;
    }
    QState tran_hist(QStateHandler const hist) noexcept {
        // for coding QMsm state machines
        m_temp.fun = hist;
        return Q_RET_TRAN_HIST;
    }
    QState super(QStateHandler const superstate) noexcept {
        // for coding QMsm state machines
        m_temp.fun = superstate;
        return Q_RET_SUPER;
    }
    QState qm_tran(void const * const tatbl) noexcept {
        // for coding QMsm state machines
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN;
    }
    QState qm_tran_init(void const * const tatbl) noexcept {
        // for coding QMsm state machines
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_INIT;
    }
    QState qm_tran_hist(
        QMState const * const hist,
        void const * const tatbl) noexcept
    {   // for coding QMsm state machines
        m_state.obj  = hist; // store the history in state
        m_temp.tatbl = static_cast<QP::QMTranActTable const *>(tatbl);
        return Q_RET_TRAN_HIST;
    }

#ifdef Q_SPY
    QState qm_entry(QMState const * const s) noexcept {
        // for coding QMsm state machines
        m_temp.obj = s;
        return Q_RET_ENTRY;
    }
#else
    QState qm_entry(QMState const * const s) const noexcept {
        // for coding QMsm state machines
        static_cast<void>(s); // unused parameter
        return Q_RET_ENTRY;
    }
#endif // Q_SPY

#ifdef Q_SPY
    QState qm_exit(QMState const * const s) noexcept {
        // for coding QMsm state machines
        m_temp.obj = s;
        return Q_RET_EXIT;
    }
#else
    QState qm_exit(QMState const * const s) const noexcept {
        // for coding QMsm state machines
        static_cast<void>(s); // unused parameter
        return Q_RET_EXIT;
    }
#endif // Q_SPY
}; // class QAsm

//----------------------------------------------------------------------------
class QHsm : public QP::QAsm {
protected:
    explicit QHsm(QStateHandler const initial) noexcept;

public:
    using QAsm::init;
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;
    bool isIn(QStateHandler const stateHndl) noexcept override;
    QStateHandler getStateHandler() const noexcept override;

    QStateHandler childState(QStateHandler const parentHndl) noexcept;

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

    // friends...
    friend class QS;
}; // class QHsm

//----------------------------------------------------------------------------
class QMsm : public QP::QAsm {
protected:
    explicit QMsm(QStateHandler const initial) noexcept;

public:
    using QAsm::init;
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;
    bool isIn(QStateHandler const stateHndl) noexcept override;
    QStateHandler getStateHandler() const noexcept override;

    QMState const * childStateObj(QMState const * const parentHndl)
        const noexcept;
    static QMState const * topQMState() noexcept;

private:
    QState execTatbl_(
        QMTranActTable const * const tatbl,
        std::uint_fast8_t const qsId);
    void exitToTranSource_(
        QMState const * const curr_state,
        QMState const * const tran_source,
        std::uint_fast8_t const qsId);
    QState enterHistory_(
        QMState const * const hist,
        std::uint_fast8_t const qsId);

    // friends...
    friend class QS;
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

class QEQueue; // forward declaration
class QActive; // forward declaration

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

//----------------------------------------------------------------------------
class QPSet {
private:
    QPSetBits m_bits0;
#if (QF_MAX_ACTIVE > 32U)
    QPSetBits m_bits1;
#endif

public:
    void setEmpty() noexcept;
    bool isEmpty() const noexcept;
    bool notEmpty() const noexcept;
    bool hasElement(std::uint_fast8_t const n) const noexcept;
    void insert(std::uint_fast8_t const n) noexcept;
    void remove(std::uint_fast8_t const n) noexcept;
    std::uint_fast8_t findMax() const noexcept;

    // friends...
    friend class QS;
}; // class QPSet

//----------------------------------------------------------------------------
class QSubscrList {
private:
    QPSet m_set;

    // friends...
    friend class QActive;
    friend class QS;
}; // class QSubscrList

//----------------------------------------------------------------------------
// declarations for friendship with the QActive class

extern "C" {
    std::uint_fast8_t QK_sched_() noexcept;
    std::uint_fast8_t QK_sched_act_(
        QP::QActive const * const act,
        std::uint_fast8_t const pthre_in) noexcept;
    void QK_activate_();

    std::uint_fast8_t QXK_sched_() noexcept;
    void QXK_contextSw_(QP::QActive * const next) noexcept;
    void QXK_threadExit_() noexcept;
    void QXK_activate_();
} // extern "C"

namespace QF {
    void init();
    void stop();
    int_t run();

    void onStartup();
    void onCleanup();

    constexpr std::uint_fast16_t NO_MARGIN {0xFFFFU};
} // namespace QF

namespace QXK {
    QP::QActive *current() noexcept;
} // namespace QXK

//----------------------------------------------------------------------------
class QActive : public QP::QAsm {
private:
    std::uint8_t m_prio;
    std::uint8_t m_pthre;

#ifdef QACTIVE_THREAD_TYPE
    QACTIVE_THREAD_TYPE m_thread;
#endif

#ifdef QACTIVE_OS_OBJ_TYPE
    QACTIVE_OS_OBJ_TYPE m_osObject;
#endif

#ifdef QACTIVE_EQUEUE_TYPE
    QACTIVE_EQUEUE_TYPE m_eQueue;
#endif

protected:
    explicit QActive(QStateHandler const initial) noexcept;

public:
    using QAsm::init;
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;
    bool isIn(QStateHandler const stateHndl) noexcept override;
    QStateHandler getStateHandler() const noexcept override;

    QStateHandler childState(QStateHandler const parentHandler) noexcept;
    void setAttr(
        std::uint32_t attr1,
        void const * attr2 = nullptr);
    void start(QPrioSpec const prioSpec,
        QEvtPtr * const qSto, std::uint_fast16_t const qLen,
        void * const stkSto, std::uint_fast16_t const stkSize,
        void const * const par = nullptr);

#ifdef QACTIVE_CAN_STOP
    void stop();
#endif // def QACTIVE_CAN_STOP
    void register_() noexcept;
    void unregister_() noexcept;
    void post_(QEvt const * const e,
        void const * const sender) noexcept
    {
        // delegate to postx_() with margin==QF::NO_MARGIN
        static_cast<void>(postx_(e, QF::NO_MARGIN, sender));
    }
    bool postx_(QEvt const * const e,
        std::uint_fast16_t const margin,
        void const * const sender) noexcept;
    void postLIFO(QEvt const * const e) noexcept;
    QEvt const * get_() noexcept;
    static std::uint16_t getQueueUse(
        std::uint_fast8_t const prio) noexcept;
    static std::uint16_t getQueueFree(
        std::uint_fast8_t const prio) noexcept;
    static std::uint16_t getQueueMin(
        std::uint_fast8_t const prio) noexcept;
    static void psInit(
        QSubscrList * const subscrSto,
        QSignal const maxSignal) noexcept;
    static void publish_(
        QEvt const * const e,
        void const * const sender,
        std::uint_fast8_t const qsId) noexcept;
    void subscribe(QSignal const sig) const noexcept;
    void unsubscribe(QSignal const sig) const noexcept;
    void unsubscribeAll() const noexcept;
    bool defer(
        QEQueue * const eq,
        QEvt const * const e) const noexcept;
    bool recall(QEQueue * const eq) noexcept;
    std::uint16_t flushDeferred(
        QEQueue * const eq,
        std::uint_fast16_t const num = 0xFFFFU) const noexcept;
    std::uint8_t getPrio() const noexcept {
        return m_prio; // public "getter" for the AO's prio
    }
    static void evtLoop_(QActive *act);
    static QActive *fromRegistry(std::uint_fast8_t const prio);

#ifdef QACTIVE_THREAD_TYPE
    QACTIVE_THREAD_TYPE const & getThread() const & {
        // ref-qualified reference (MISRA-C++:2023 Rule 6.8.4)
        return m_thread;
    }
    QACTIVE_THREAD_TYPE const & getThread() const &&
        // ref-qualified reference (MISRA-C++:2023 Rule 6.8.4)
        = delete;
    void setThread(QACTIVE_THREAD_TYPE const & thr) noexcept {
        m_thread = thr; // public "setter", useful for MPU applications
    }
#endif // QACTIVE_THREAD_TYPE

#ifdef QACTIVE_OS_OBJ_TYPE
    QACTIVE_OS_OBJ_TYPE const & getOsObject() const & {
        // ref-qualified reference (MISRA-C++:2023 Rule 6.8.4)
        return m_osObject;
    }
    QACTIVE_OS_OBJ_TYPE const & getOsObject() const &&
        // ref-qualified reference (MISRA-C++:2023 Rule 6.8.4)
        = delete;
#endif // QACTIVE_OS_OBJ_TYPE

#ifdef QF_ISR_API
    virtual bool postFromISR(
        QEvt const * const e,
        std::uint_fast16_t const margin,
        void * par,
        void const * const sender) noexcept;

    static void publishFromISR(
        QEvt const * e,
        void * par,
        void const * sender) noexcept;
#endif // QF_ISR_API

private:
    void postFIFO_(
        QEvt const * const e,
        void const * const sender);
    static void multicast_(
        QPSet * const subscrSet,
        QEvt const * const e,
        void const * const sender);

    // friends...
    friend class QTimeEvt;
    friend class QTicker;
    friend class QXThread;
    friend class QXMutex;
    friend class QXSemaphore;
    friend class QMActive;
    friend class QActiveDummy;
    friend class QS;

    friend void QF::init();
    friend void QF::stop();
    friend int_t QF::run();
    friend void QF::onStartup();
    friend void QF::onCleanup();

    friend std::uint_fast8_t QK_sched_() noexcept;
    friend std::uint_fast8_t QK_sched_act_(
        QP::QActive const * const act,
        std::uint_fast8_t const pthre_in) noexcept;
    friend void QK_activate_();

    friend std::uint_fast8_t QXK_sched_() noexcept;
    friend void QXK_contextSw_(QP::QActive * const next) noexcept;
    friend void QXK_threadExit_() noexcept;
    friend void QXK_activate_();
    friend QP::QActive *QXK::current() noexcept;

}; // class QActive

//----------------------------------------------------------------------------
class QMActive : public QP::QActive {
protected:
    explicit QMActive(QStateHandler const initial) noexcept;

public:
    using QActive::init;
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;
    bool isIn(QStateHandler const stateHndl) noexcept override;
    QStateHandler getStateHandler() const noexcept override;

    QMState const *childStateObj(QMState const * const parent) const noexcept;
}; // class QMActive

//----------------------------------------------------------------------------
#if (QF_MAX_TICK_RATE > 0U)

class QTimeEvt : public QP::QEvt {
private:
    QTimeEvt *m_next;
    void * m_act;
    QTimeEvtCtr m_ctr;
    QTimeEvtCtr m_interval;
    std::uint8_t m_tickRate;
    std::uint8_t m_flags;

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
    void const * getAct() const & {
        // ref-qualified reference (MISRA-C++:2023 Rule 6.8.4)
        return m_act;
    }
    void const * getAct() const &&
        // ref-qualified reference (MISRA-C++:2023 Rule 6.8.4)
        = delete;
    QTimeEvtCtr getCtr() const noexcept {
        return m_ctr; // public "getter" for the current time-evt count
    }
    QTimeEvtCtr getInterval() const noexcept {
        return m_interval; // public "getter" for the time-evt interval
    }
    std::uint8_t getTickRate() const noexcept {
        return m_tickRate;  // public "getter" for the time-evt tick rate
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
        // public "getter" for the AO associated with this time-evt
        return static_cast<QActive *>(m_act);
    }
    QTimeEvt * toTimeEvt() noexcept {
        // public "getter" for the AO associated with this time-evt
        // NOTE: used for the special time-evts in QTimeEvt_head_[] array
        return static_cast<QTimeEvt *>(m_act);
    }
    QTimeEvt() noexcept;

private:
    QTimeEvt *expire_(
        QTimeEvt * const prev_link,
        QActive const * const act,
        std::uint_fast8_t const tickRate) noexcept;

    // fiends...
    friend class QXThread;
    friend class QS;
}; // class QTimeEvt

//----------------------------------------------------------------------------
class QTicker : public QP::QActive {
public:
    explicit QTicker(std::uint8_t const tickRate) noexcept;
    using QActive::init;
    void init(
        void const * const e,
        std::uint_fast8_t const qsId) override;
    void dispatch(
        QEvt const * const e,
        std::uint_fast8_t const qsId) override;
    void trig_(void const * const sender) noexcept;
}; // class QTicker

#endif // (QF_MAX_TICK_RATE > 0U)

//----------------------------------------------------------------------------
namespace QF {

//! @deprecated
inline void psInit(    QSubscrList * const subscrSto,
    QSignal const maxSignal) noexcept
{
    // use QActive::psInit() instead of the deprecated QF::psInit()
    QActive::psInit(subscrSto, maxSignal);
}

//! @deprecated
inline void publish_(QEvt const * const e,
    void const * const sender, std::uint_fast8_t const qsId) noexcept
{
    // use  QTimeEvt::tick() instead of the deprecated QF::tick()
    QActive::publish_(e, sender, qsId);
}

//! @deprecated
static inline std::uint_fast16_t getQueueMin(
    std::uint_fast8_t const prio) noexcept
{
    // use QActive::getQueueMin() instead of the deprecated QF::getQueueMin()
    return QActive::getQueueMin(prio);
}

#if (QF_MAX_TICK_RATE > 0U)
//! @deprecated
inline void tick(
    std::uint_fast8_t const tickRate,
    void const * const sender) noexcept
{
    // use  QTimeEvt::tick() instead of the deprecated QF::tick()
    QTimeEvt::tick(tickRate, sender);
}
#endif // (QF_MAX_TICK_RATE > 0U)

//----------------------------------------------------------------------------
// QF dynamic memory facilities
void poolInit(
    void * const poolSto,
    std::uint_fast32_t const poolSize,
    std::uint_fast16_t const evtSize) noexcept;

std::uint16_t poolGetMaxBlockSize() noexcept;
std::uint16_t getPoolUse(std::uint_fast8_t const poolNum) noexcept;
std::uint16_t getPoolFree(std::uint_fast8_t const poolNum) noexcept;
std::uint16_t getPoolMin(std::uint_fast8_t const poolNum) noexcept;
QEvt * newX_(
    std::uint_fast16_t const evtSize,
    std::uint_fast16_t const margin,
    QSignal const sig) noexcept;
void gc(QEvt const * const e) noexcept;
QEvt const * newRef_(
    QEvt const * const e,
    QEvt const * const evtRef) noexcept;

void deleteRef_(QEvt const * const evtRef) noexcept;

#ifndef QEVT_PAR_INIT
    template<class evtT_>
    inline evtT_ * q_new(QSignal const sig) {
        // allocate a dynamic (mutable) event with NO_MARGIN
        // NOTE: the returned event ptr is guaranteed NOT to be nullptr
        return static_cast<evtT_*>(
            QP::QF::newX_(sizeof(evtT_), QP::QF::NO_MARGIN, sig));
    }
    template<class evtT_>
    inline evtT_ * q_new_x(std::uint_fast16_t const margin,
        QSignal const sig)
    {
        // allocate a dynamic (mutable) event with a provided margin
        // NOTE: the returned event ptr is MIGHT be nullptr
        return static_cast<evtT_*>(QP::QF::newX_(sizeof(evtT_), margin, sig));
    }
#else
    template<class evtT_, typename... Args>
    inline evtT_ * q_new(QSignal const sig, Args... args) {
        // allocate a dynamic (mutable) event with NO_MARGIN
        // NOTE: the returned event ptr is guaranteed NOT to be nullptr
        evtT_ *e = static_cast<evtT_*>(
            QP::QF::newX_(sizeof(evtT_), QP::QF::NO_MARGIN, sig));
        e->init(args...); // immediately initialize the event (RAII)
        return e;
    }
    template<class evtT_, typename... Args>
    inline evtT_ * q_new_x(std::uint_fast16_t const margin,
        QSignal const sig, Args... args)
    {
        // allocate a dynamic (mutable) event with a provided margin
        // NOTE: the event allocation is MIGHT fail
        evtT_ *e =
            static_cast<evtT_*>(QP::QF::newX_(sizeof(evtT_), margin, sig));
        if (e != nullptr) { // was the allocation successfull?
            e->init(args...); // immediately initialize the event (RAII)
        }
        // NOTE: the returned event ptr is MIGHT be nullptr
        return e;
    }
#endif // QEVT_PAR_INIT

template<class evtT_>
inline void q_new_ref(
    QP::QEvt const * const e,
    evtT_ const *& evtRef)
{
    // set the const event reference (must NOT be nullptr)
    evtRef = static_cast<evtT_ const *>(QP::QF::newRef_(e, evtRef));
}

template<class evtT_>
inline void q_delete_ref(evtT_ const *& evtRef) {
    QP::QF::deleteRef_(evtRef);
    evtRef = nullptr; // invalidate the associated event reference
}

#ifdef QF_ISR_API
QEvt * newXfromISR_(
    std::uint_fast16_t const evtSize,
    std::uint_fast16_t const margin,
    QSignal const sig) noexcept;
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

//----------------------------------------------------------------------------
// QF base facilities

#define Q_PRIO(prio_, pthre_) \
    (static_cast<QP::QPrioSpec>((prio_) | (pthre_) << 8U))

#ifndef QEVT_PAR_INIT
    #define Q_NEW(evtT_, sig_)    (QP::QF::q_new<evtT_>((sig_)))
    #define Q_NEW_X(evtT_, margin_, sig_) \
        (QP::QF::q_new_x<evtT_>((margin_), (sig_)))
#else
    #define Q_NEW(evtT_, sig_, ...) \
        (QP::QF::q_new<evtT_>((sig_), __VA_ARGS__))
    #define Q_NEW_X(evtT_, margin_, sig_, ...) \
        (QP::QF::q_new_x<evtT_>((margin_), (sig_), __VA_ARGS__))
#endif // QEVT_PAR_INIT

#define Q_NEW_REF(evtRef_, evtT_) (QP::QF::q_new_ref<evtT_>(e, (evtRef_)))
#define Q_DELETE_REF(evtRef_) do { \
    QP::QF::deleteRef_((evtRef_)); \
    (evtRef_) = nullptr; \
} while (false)

#ifdef Q_SPY
    #define PUBLISH(e_, sender_) \
        publish_((e_), (sender_), (sender_)->getPrio())
    #define POST(e_, sender_) post_((e_), (sender_))
    #define POST_X(e_, margin_, sender_) \
        postx_((e_), (margin_), (sender_))
    #define TICK_X(tickRate_, sender_) tick((tickRate_), (sender_))
    #define TRIG(sender_) trig_((sender_))
#else
    #define PUBLISH(e_, dummy) publish_((e_), nullptr, 0U)
    #define POST(e_, dummy) post_((e_), nullptr)
    #define POST_X(e_, margin_, dummy) postx_((e_), (margin_), nullptr)
    #define TICK_X(tickRate_, dummy) tick((tickRate_), nullptr)
    #define TRIG(sender_) trig_(nullptr)
#endif // ndef Q_SPY

#define TICK(sender_) TICK_X(0U, (sender_))

#ifndef QF_CRIT_EXIT_NOP
    #define QF_CRIT_EXIT_NOP() (static_cast<void>(0))
#endif // ndef QF_CRIT_EXIT_NOP

//----------------------------------------------------------------------------
// memory protection facilities

#ifdef QF_MEM_ISOLATE
    #error Memory isolation not supported in this QP edition, need SafeQP
#endif // def QF_MEM_ISOLATE

#endif // QP_HPP_
