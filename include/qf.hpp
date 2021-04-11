/// @file
/// @brief QF/C++ platform-independent public interface.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.9.3
/// Last updated on  2021-02-26
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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

#ifndef QF_HPP
#define QF_HPP

//****************************************************************************
#ifndef QPSET_HPP
#include "qpset.hpp"
#endif

#ifdef Q_EVT_CTOR
#include <new>  // for placement new
#endif // Q_EVT_CTOR

//****************************************************************************
// apply defaults for all undefined configuration parameters
//
#ifndef QF_EVENT_SIZ_SIZE
    //! Default value of the macro configurable value in qf_port.hpp
    #define QF_EVENT_SIZ_SIZE    2U
#endif

#ifndef QF_MAX_EPOOL
    //! Default value of the macro configurable value in qf_port.hpp
    #define QF_MAX_EPOOL         3U
#endif

#ifndef QF_MAX_TICK_RATE
    //! Default value of the macro configurable value in qf_port.hpp
    //! Valid values: [0U..15U]; default 1U
    #define QF_MAX_TICK_RATE     1U
#elif (QF_MAX_TICK_RATE > 15U)
    #error "QF_MAX_TICK_RATE exceeds the maximum of 15U"
#endif

#ifndef QF_TIMEEVT_CTR_SIZE
    //! macro to override the default QTimeEvtCtr size.
    //! Valid values 1U, 2U, or 4U; default 2U
    #define QF_TIMEEVT_CTR_SIZE  2U
#endif


//****************************************************************************
namespace QP {

#if (QF_EVENT_SIZ_SIZE == 1U)
    using QEvtSize = std::uint8_t;
#elif (QF_EVENT_SIZ_SIZE == 2U)
    //! The data type to store the block-size defined based on
    //! the macro #QF_EVENT_SIZ_SIZE.
    /// @description
    /// The dynamic range of this data type determines the maximum block
    /// size that can be managed by the pool.
    using QEvtSize = std::uint16_t;
#elif (QF_EVENT_SIZ_SIZE == 4U)
    using QEvtSize = std::uint32_t;
#else
    #error "QF_EVENT_SIZ_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

//****************************************************************************
#if (QF_TIMEEVT_CTR_SIZE == 1U)
    using QTimeEvtCtr = std::uint8_t;
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    //! type of the Time Event counter, which determines the dynamic
    //! range of the time delays measured in clock ticks.
    /// @description
    /// This alias is configurable via the preprocessor switch
    /// #QF_TIMEEVT_CTR_SIZE. The other possible values of this type are
    /// as follows: @n
    /// std::uint8_t when (QF_TIMEEVT_CTR_SIZE == 1U), and @n
    /// std::uint32_t when (QF_TIMEEVT_CTR_SIZE == 4U).
    using QTimeEvtCtr = std::uint16_t;
#elif (QF_TIMEEVT_CTR_SIZE == 4U)
    using QTimeEvtCtr = std::uint32_t;
#else
    #error "QF_TIMEEVT_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

class QEQueue; // forward declaration

//****************************************************************************
//! QActive active object (based on QP::QHsm implementation)
/// @description
/// Active objects in QP are encapsulated tasks (each embedding a state
/// machine and an event queue) that communicate with one another
/// asynchronously by sending and receiving events. Within an active object,
/// events are processed in a run-to-completion (RTC) fashion, while QF
/// encapsulates all the details of thread-safe event exchange and queuing.
/// @n@n
/// QP::QActive represents an active object that uses the QP::QHsm-style
/// implementation strategy for state machines. This strategy is tailored
/// to manual coding, but it is also supported by the QM modeling tool.
/// The resulting code is slower than in the QP::QMsm-style implementation
/// strategy.
///
/// @note
/// QP::QActive is not intended to be instantiated directly, but rather serves
/// as the base class for derivation of active objects in the applications.
///
/// @sa QP::QMActive
///
/// @usage
/// The following example illustrates how to derive an active object from
/// QP::QActive.
/// @include qf_qactive.cpp
///
class QActive : public QHsm {
public: // for access from extern "C" functions
#ifdef QF_EQUEUE_TYPE
    //! OS-dependent event-queue type.
    /// @description
    /// The type of the queue depends on the underlying operating system or
    /// a kernel. Many kernels support "message queues" that can be adapted
    /// to deliver QF events to the active object. Alternatively, QF provides
    /// a native event queue implementation that can be used as well.
    ///
    /// @note
    /// The native QF event queue is configured by defining the macro
    /// #QF_EQUEUE_TYPE as QP::QEQueue.
    QF_EQUEUE_TYPE m_eQueue;
#endif

#ifdef QF_OS_OBJECT_TYPE
    //! OS-dependent per-thread object.
    /// @description
    /// This data might be used in various ways, depending on the QF port.
    /// In some ports m_osObject is used to block the calling thread when
    /// the native QF queue is empty. In other QF ports the OS-dependent
    /// object might be used differently.
    QF_OS_OBJECT_TYPE m_osObject;
#endif

#ifdef QF_THREAD_TYPE
    //! OS-dependent representation of the thread of the active object.
    /// @description
    /// This data might be used in various ways, depending on the QF port.
    /// In some ports m_thread is used store the thread handle. In other ports
    /// m_thread can be a pointer to the Thread-Local-Storage (TLS).
    QF_THREAD_TYPE m_thread;
#endif

#ifdef QXK_HPP // QXK kernel used?
    //! QXK dynamic priority (1..#QF_MAX_ACTIVE) of AO/thread.
    std::uint8_t m_dynPrio;
#endif

    //! QF priority (1..#QF_MAX_ACTIVE) of this active object.
    std::uint8_t m_prio;

protected:
    //! protected constructor (abstract class)
    QActive(QStateHandler const initial) noexcept;

public:
    //! Starts execution of an active object and registers the object
    //! with the framework.
    virtual void start(std::uint_fast8_t const prio,
        QEvt const * * const qSto, std::uint_fast16_t const qLen,
        void * const stkSto, std::uint_fast16_t const stkSize,
        void const * const par);

    //! Overloaded start function (no initialization event)
    virtual void start(std::uint_fast8_t const prio,
         QEvt const * * const qSto, std::uint_fast16_t const qLen,
         void * const stkSto, std::uint_fast16_t const stkSize)
    {
        this->start(prio, qSto, qLen, stkSto, stkSize, nullptr);
    }

#ifdef QF_ACTIVE_STOP
    //! Stops execution of an active object and removes it from the
    //! framework's supervision.
    /// @attention
    /// QActive::stop() must be called only from the AO that is about
    /// to stop its execution. By that time, any pointers or references
    /// to the AO are considered invalid (dangling) and it becomes
    /// illegal for the rest of the application to post events to the AO.
    void stop(void);
#endif

#ifndef Q_SPY
    //! Posts an event @p e directly to the event queue of the active
    //! object @p me using the First-In-First-Out (FIFO) policy.
    virtual bool post_(QEvt const * const e,
                       std::uint_fast16_t const margin) noexcept;
#else
    virtual bool post_(QEvt const * const e, std::uint_fast16_t const margin,
                       void const * const sender) noexcept;
#endif

    //! Posts an event directly to the event queue of the active object
    //! using the Last-In-First-Out (LIFO) policy.
    virtual void postLIFO(QEvt const * const e) noexcept;

    //! Un-subscribes from the delivery of all signals to the active object.
    void unsubscribeAll(void) const noexcept;

    //! Subscribes for delivery of signal @p sig to the active object
    void subscribe(enum_t const sig) const noexcept;

    //! Un-subscribes from the delivery of signal @p sig to the active object.
    void unsubscribe(enum_t const sig) const noexcept;

    //! Defer an event to a given separate event queue.
    bool defer(QEQueue * const eq, QEvt const * const e) const noexcept;

    //! Recall a deferred event from a given event queue.
    bool recall(QEQueue * const eq) noexcept;

    //! Flush the specified deferred queue 'eq'.
    std::uint_fast16_t flushDeferred(QEQueue * const eq) const noexcept;

    //! Get the priority of the active object.
    std::uint_fast8_t getPrio(void) const noexcept {
        return static_cast<std::uint_fast8_t>(m_prio);
    }

    //! Set the priority of the active object.
    void setPrio(std::uint_fast8_t const prio) {
        m_prio = static_cast<std::uint8_t>(prio);
    }

    //! Generic setting of additional attributes (useful in QP ports)
    void setAttr(std::uint32_t attr1, void const *attr2 = nullptr);

#ifdef QF_OS_OBJECT_TYPE
    //! accessor to the OS-object for extern "C" functions, such as
    //! the QK or QXK schedulers
    QF_OS_OBJECT_TYPE &getOsObject(void) noexcept { return m_osObject; }
#endif

#ifdef QF_THREAD_TYPE
    //! accessor to the Thread for extern "C" functions, such as
    //! the QK or QXK schedulers
    QF_THREAD_TYPE &getThread(void) noexcept { return m_thread; }
#endif

    //! Get an event from the event queue of an active object.
    QEvt const *get_(void) noexcept;

// duplicated API to be used exclusively inside ISRs (useful in some QP ports)
#ifdef QF_ISR_API
#ifdef Q_SPY
    virtual bool postFromISR_(QEvt const * const e,
        std::uint_fast16_t const margin, void *par,
        void const * const sender) noexcept;
#else
    virtual bool postFromISR_(QEvt const * const e,
        std::uint_fast16_t const margin, void *par) noexcept;
#endif // Q_SPY
#endif // QF_ISR_API

// friendships...
private:
    friend class QF;
    friend class QTimeEvt;
    friend class QTicker;
#ifdef QK_HPP
    friend class QMutex;
#endif // QK_HPP
#ifdef QXK_HPP
    friend class QXK;
    friend class QXThread;
    friend class QXMutex;
    friend class QXSemaphore;
#endif // QXK_HPP
#ifdef Q_UTEST
    friend class QActiveDummy;
#endif // Q_UTEST
};

//****************************************************************************
//! QMActive active object (based on QP::QMsm implementation)
/// @description
/// QP::QMActive represents an active object that uses the QP::QMsm-style
/// state machine implementation strategy. This strategy requires the use of
/// the QM modeling tool to generate state machine code automatically, but
/// the code is faster than in the QP::QHsm-style implementation strategy
/// and needs less run-time support (smaller event-processor).
///
/// @note
/// QP::QMActive is not intended to be instantiated directly, but rather
/// serves as the base class for derivation of active objects in the
/// applications.
///
/// @sa QP::QActive
///
/// @usage
/// The following example illustrates how to derive an active object from
/// QP::QMActive.
/// @include qf_qmactive.cpp
///
class QMActive : public QActive {
public:
    // all the following operations delegate to the QHsm class...
    void init(void const * const e,
              std::uint_fast8_t const qs_id) override;
    void init(std::uint_fast8_t const qs_id) override;
    void dispatch(QEvt const * const e,
                  std::uint_fast8_t const qs_id) override;

#ifdef Q_SPY
    //! Get the current state handler of the QMsm
    QStateHandler getStateHandler() noexcept override;
#endif

    //! Tests if a given state is part of the active state configuration
    bool isInState(QMState const * const st) const noexcept;

    //! Return the current active state object (read only)
    QMState const *stateObj(void) const noexcept {
        return m_state.obj;
    }

    //! Obtain the current active child state of a given parent (read only)
    QMState const *childStateObj(QMState const * const parent) const noexcept;

protected:
    //! protected constructor (abstract class)
    QMActive(QStateHandler const initial) noexcept;

private:
    //! operations inherited from QP::QHsm, but disallowed in QP::QMActive
    using QHsm::isIn;
    using QHsm::state;
    using QHsm::childState;
};


//****************************************************************************
//! Time Event class
/// @description
/// Time events are special QF events equipped with the notion of time
/// passage. The basic usage model of the time events is as follows. An
/// active object allocates one or more QTimeEvt objects (provides the
/// storage for them). When the active object needs to arrange for a timeout,
/// it arms one of its time events to fire either just once (one-shot) or
/// periodically. Each time event times out independently from the others,
/// so a QF application can make multiple parallel timeout requests (from the
/// same or different active objects). When QF detects that the appropriate
/// moment has arrived, it inserts the time event directly into the
/// recipient's event queue. The recipient then processes the time event just
/// like any other event.@n
/// @n
/// Time events, as any other QF events derive from the QP::QEvt base
/// class. Typically, you will use a time event as-is, but you can also
/// further derive more specialized time events from it by adding some more
/// data members and/or specialized functions that operate on the specialized
/// time events.@n
/// @n
/// Internally, the armed time events are organized into a bi-directional
/// linked list. This linked list is scanned in every invocation of the
/// QP::QF::tickX_() function. Only armed (timing out) time events are in the
/// list, so only armed time events consume CPU cycles.
///
/// @note
/// QF manages the time events in the macro TICK_X(), which must be called
/// periodically, from the clock tick ISR or from the special QP::QTicker
/// active object.
///
/// @note
/// Even though QP::QTimeEvt is a subclass of QP::QEvt, QP::QTimeEvt instances
/// can NOT be allocated dynamically from event pools. In other words, it is
/// illegal to allocate QP::QTimeEvt instances with the Q_NEW() or Q_NEW_X()
/// macros.
///
class QTimeEvt : public QEvt {
private:

    //! link to the next time event in the list
    QTimeEvt * volatile m_next;

    //! the active object that receives the time events
    /// @description
    /// The m_act pointer is reused inside the QP implementation to hold
    /// the head of the list of newly armed time events.
    void * volatile m_act;

    //! the internal down-counter of the time event.
    /// @description
    /// The down-counter is decremented by 1 in every TICK_X()
    /// invocation. The time event fires (gets posted or published) when
    /// the down-counter reaches zero.
    QTimeEvtCtr volatile m_ctr;

    //! the interval for the periodic time event (zero for the one-shot
    //! time event).
    /// @description
    /// The value of the interval is re-loaded to the internal
    /// down-counter when the time event expires, so that the time event
    /// keeps timing out periodically.
    QTimeEvtCtr m_interval;

public:

    //! The Time Event constructor.
    QTimeEvt(QActive * const act, enum_t const sgnl,
             std::uint_fast8_t const tickRate = 0U) noexcept;

    //! Arm a time event (one shot or periodic) for event posting.
    void armX(QTimeEvtCtr const nTicks,
              QTimeEvtCtr const interval = 0U) noexcept;

    //! Disarm a time event.
    bool disarm(void) noexcept;

    //! Rearm a time event.
    bool rearm(QTimeEvtCtr const nTicks) noexcept;

    //! Check the "was disarmed" status of a time event.
    bool wasDisarmed(void) noexcept;

    //! Get the current value of the down-counter of a time event.
    QTimeEvtCtr currCtr(void) const noexcept;

private:
    //! private default constructor only for friends
    QTimeEvt(void) noexcept;

    //! private copy constructor to disallow copying of QTimeEvts
    QTimeEvt(QTimeEvt const &) = delete;

    //! private assignment operator to disallow assigning of QTimeEvts
    QTimeEvt & operator=(QTimeEvt const &) = delete;

    //! encapsulate the cast the m_act attribute to QActive*
    QActive  *toActive(void) noexcept {
        return static_cast<QActive *>(m_act);
    }

    //! encapsulate the cast the m_act attribute to QTimeEvt*
    QTimeEvt *toTimeEvt(void) noexcept {
        return static_cast<QTimeEvt *>(m_act);
    }

    friend class QF;
    friend class QS;
#ifdef QXK_HPP
    friend class QXThread;
    friend void QXK_activate_(void);
#endif // QXK_HPP
};


//****************************************************************************
//! Subscriber List
/// @description
/// This data type represents a set of active objects that subscribe to
/// a given signal. The set is represented as priority-set, where each
/// bit corresponds to the unique priority of an active object.
using QSubscrList = QPSet;


//****************************************************************************
//! QF services.
/// @description
/// This class groups together QF services. It has only static members and
/// should not be instantiated.
class QF {
public:

    //! get the current QF version number string of the form X.Y.Z
    static char_t const *getVersion(void) noexcept {
        return versionStr;
    }

    //! QF initialization.
    static void init(void);

    //! Publish-subscribe initialization.
    static void psInit(QSubscrList * const subscrSto,
                       enum_t const maxSignal) noexcept;

    //! Event pool initialization for dynamic allocation of events.
    static void poolInit(void * const poolSto,
                         std::uint_fast32_t const poolSize,
                         std::uint_fast16_t const evtSize) noexcept;

    //! Obtain the block size of any registered event pools
    static std::uint_fast16_t poolGetMaxBlockSize(void) noexcept;


    //! Transfers control to QF to run the application.
    static int_t run(void);

    //! Startup QF callback.
    static void onStartup(void);

    //! Cleanup QF callback.
    static void onCleanup(void);

    //! Function invoked by the application layer to stop the QF
    //! application and return control to the OS/Kernel.
    static void stop(void);

#ifndef Q_SPY
    static void publish_(QEvt const * const e) noexcept;
    static void tickX_(std::uint_fast8_t const tickRate) noexcept;
#else
    //! Publish event to the framework.
    static void publish_(QEvt const * const e,
                         void const * const sender,
                         std::uint_fast8_t const qs_id) noexcept;

    //! Processes all armed time events at every clock tick.
    static void tickX_(std::uint_fast8_t const tickRate,
                       void const * const sender) noexcept;
#endif // Q_SPY

    //! Returns true if all time events are inactive and false
    //! any time event is active.
    static bool noTimeEvtsActiveX(std::uint_fast8_t const tickRate) noexcept;

    //! This function returns the minimum of free entries of the given
    //! event pool.
    static std::uint_fast16_t getPoolMin(std::uint_fast8_t const poolId)
        noexcept;

    //! This function returns the minimum of free entries of the given
    //! event queue.
    static std::uint_fast16_t getQueueMin(std::uint_fast8_t const prio)
        noexcept;

    //! Internal QF implementation of creating new dynamic event.
    static QEvt *newX_(std::uint_fast16_t const evtSize,
                       std::uint_fast16_t const margin,
                       enum_t const sig) noexcept;

    //! Recycle a dynamic event.
    static void gc(QEvt const * const e) noexcept;

    //! Internal QF implementation of creating new event reference.
    static QEvt const *newRef_(QEvt const * const e,
                               QEvt const * const evtRef) noexcept;

    //! Internal QF implementation of deleting event reference.
    static void deleteRef_(QEvt const * const evtRef) noexcept;

    //! Remove the active object from the framework.
    static void remove_(QActive * const a) noexcept;

    //! array of registered active objects
    static QActive *active_[QF_MAX_ACTIVE + 1U];

    //! Thread routine for executing an active object @p act.
    static void thread_(QActive *act);

    //! Register an active object to be managed by the framework
    static void add_(QActive * const a) noexcept;

    //! Clear a specified region of memory to zero.
    static void bzero(void * const start,
                      std::uint_fast16_t const len) noexcept;

// API to be used exclusively inside ISRs (useful in some QP ports)
#ifdef QF_ISR_API
#ifdef Q_SPY
    static void publishFromISR_(QEvt const *e, void *par,
                                void const *sender) noexcept;
    static void tickXfromISR_(std::uint_fast8_t const tickRate, void *par,
                              void const * const sender) noexcept;
#else
    static void publishFromISR_(QEvt const *e, void *par) noexcept;
    static void tickXfromISR_(std::uint_fast8_t const tickRate,
                              void *par) noexcept;
#endif // Q_SPY

    static QEvt *newXfromISR_(std::uint_fast16_t const evtSize,
                              std::uint_fast16_t const margin,
                              enum_t const sig) noexcept;
    static void gcFromISR(QEvt const *e) noexcept;

#endif // QF_ISR_API

// to be used in QF ports only...
private:
    //! heads of linked lists of time events, one for every clock tick rate
    static QTimeEvt timeEvtHead_[QF_MAX_TICK_RATE];

    friend class QActive;
    friend class QTimeEvt;
    friend class QS;
#ifdef QXK_HPP
    friend class QXThread;
#endif // QXK_HPP
};

//! special value of margin that causes asserting failure in case
//! event allocation or event posting fails
std::uint_fast16_t const QF_NO_MARGIN = 0xFFFFU;


//****************************************************************************
//! Ticker Active Object class
/// @description
/// QP::QTicker is an efficient active object specialized to process
/// QF system clock tick at a specified tick frequency [0..#QF_MAX_TICK_RATE].
/// Placing system clock tick processing in an active object allows you
/// to remove the non-deterministic TICK_X() processing from the interrupt
/// level and move it into the thread-level, where you can prioritize it
/// as low as you wish.
///
class QTicker : public QActive {
public:
    explicit QTicker(std::uint_fast8_t const tickRate) noexcept; // ctor

    void init(void const * const e,
              std::uint_fast8_t const qs_id) noexcept override;
    void init(std::uint_fast8_t const qs_id) noexcept override {
        this->init(qs_id);
    }
    void dispatch(QEvt const * const e,
                  std::uint_fast8_t const qs_id) noexcept override;
#ifndef Q_SPY
    bool post_(QEvt const * const e,
               std::uint_fast16_t const margin) noexcept override;
#else
    bool post_(QEvt const * const e, std::uint_fast16_t const margin,
               void const * const sender) noexcept override;
#endif
    void postLIFO(QEvt const * const e) noexcept override;
};

} // namespace QP

//****************************************************************************
#ifndef QF_CRIT_EXIT_NOP
    //! No-operation for exiting a critical section
    /// @description
    /// In some QF ports the critical section exit takes effect only on the
    /// next machine instruction. If this next instruction is another entry
    /// to a critical section, the critical section won't be really exited,
    /// but rather the two adjecent critical sections would be merged.
    /// The #QF_CRIT_EXIT_NOP() macro contains minimal code required to
    /// prevent such merging of critical sections in such merging of
    /// critical sections in QF ports, in which it can occur.
    #define QF_CRIT_EXIT_NOP()   (static_cast<void>(0))
#endif

//****************************************************************************
// Provide the constructor for the QEvt class?
#ifdef Q_EVT_CTOR

    #define Q_NEW(evtT_, sig_, ...)                             \
        (new(QP::QF::newX_(sizeof(evtT_), QP::QF_NO_MARGIN, 0)) \
            evtT_((sig_),  ##__VA_ARGS__))

    #define Q_NEW_X(e_, evtT_, margin_, sig_, ...) do {        \
        (e_) = static_cast<evtT_ *>(                           \
                  QP::QF::newX_(sizeof(evtT_), (margin_), 0)); \
        if ((e_) != nullptr) {                                 \
            new((e_)) evtT_((sig_),  ##__VA_ARGS__);           \
        } \
     } while (false)

#else // QEvt is a POD (Plain Old Datatype)

    //! Allocate a dynamic event.
    /// @description
    /// The macro calls the internal QF function QP::QF::newX_() with
    /// margin == QP::QF_NO_MARGIN, which causes an assertion when the event
    /// cannot be successfully allocated.
    ///
    /// @param[in] evtT_ event type (class name) of the event to allocate
    /// @param[in] sig_  signal to assign to the newly allocated event
    ///
    /// @returns a valid event pointer cast to the type @p evtT_.
    ///
    /// @note
    /// If #Q_EVT_CTOR is defined, the Q_NEW() macro becomes variadic and
    /// takes all the arguments needed by the constructor of the event
    /// class being allocated. The constructor is then called by means
    /// of the placement-new operator.
    ///
    /// @usage
    /// The following example illustrates dynamic allocation of an event:
    /// @include qf_post.cpp
    #define Q_NEW(evtT_, sig_) (static_cast<evtT_ *>( \
         QP::QF::newX_(sizeof(evtT_), QP::QF_NO_MARGIN, (sig_))))

    //! Allocate a dynamic event (non-asserting version).
    /// @description
    /// This macro allocates a new event and sets the pointer @p e_, while
    /// leaving at least @p margin_ of events still available in the pool
    ///
    /// @param[out] e_     pointer to the newly allocated event
    /// @param[in] evtT_   event type (class name) of the event to allocate
    /// @param[in] margin_ number of events that must remain available
    ///                    in the given pool after this allocation. The
    ///                    special value QP::QF_NO_MARGIN causes asserting
    ///                    failure in case event allocation fails.
    /// @param[in] sig_    signal to assign to the newly allocated event
    ///
    /// @returns an event pointer cast to the type @p evtT_ or NULL if the
    /// event cannot be allocated with the specified @p margin.
    ///
    /// @note
    /// If #Q_EVT_CTOR is defined, the Q_NEW_X() macro becomes variadic and
    /// takes all the arguments needed by the constructor of the event
    /// class being allocated. The constructor is then called by means
    /// of the placement-new operator.
    ///
    /// @usage
    /// The following example illustrates dynamic allocation of an event:
    /// @include qf_postx.cpp
    #define Q_NEW_X(e_, evtT_, margin_, sig_)        \
        ((e_) = static_cast<evtT_ *>(QP::QF::newX_(  \
                    sizeof(evtT_), (margin_), (sig_))))
#endif

//! Create a new reference of the current event `e` */
/// @description
/// The current event processed by an active object is available only for
/// the duration of the run-to-completion (RTC) step. After that step, the
/// current event is no longer available and the framework might recycle
/// (garbage-collect) the event. The macro Q_NEW_REF() explicitly creates
/// a new reference to the current event that can be stored and used beyond
/// the current RTC step, until the reference is explicitly recycled by
/// means of the macro Q_DELETE_REF().
///
/// @param[in,out] evtRef_  event reference to create
/// @param[in]     evtT_    event type (class name) of the event refrence
///
/// @usage
/// The example **defer** in the directory `examples/win32/defer` illustrates
/// the use of Q_NEW_REF()
///
/// @sa Q_DELETE_REF()
///
#define Q_NEW_REF(evtRef_, evtT_)  \
    ((evtRef_) = static_cast<evtT_ const *>(QP::QF::newRef_(e, (evtRef_))))

//! Delete the event reference */
/// @description
/// Every event reference created with the macro Q_NEW_REF() needs to be
/// eventually deleted by means of the macro Q_DELETE_REF() to avoid leaking
/// the event.
///
/// @param[in,out] evtRef_  event reference to delete
///
/// @usage
/// The example **defer** in the directory `examples/win32/defer` illustrates
/// the use of Q_DELETE_REF()
///
/// @sa Q_NEW_REF()
///
#define Q_DELETE_REF(evtRef_) do { \
    QP::QF::deleteRef_((evtRef_)); \
    (evtRef_) = 0U;                \
} while (false)


//****************************************************************************
// QS software tracing integration, only if enabled
#ifdef Q_SPY

    //! Invoke the system clock tick processing QP::QF::tickX_().
    /// @description
    /// This macro is the recommended way of invoking clock tick processing,
    /// because it provides the vital information for software tracing and
    /// avoids any overhead when the tracing is disabled.
    ///
    /// @param[in] tickRate_ clock tick rate to be serviced through this call
    /// @param[in] sender_   pointer to the sender object. This parameter
    ///            is actually only used when QS software tracing is enabled
    ///            (macro #Q_SPY is defined)
    /// @note
    /// When QS software tracing is disabled, the macro calls QF_tickX_()
    /// without the @p sender parameter, so the overhead of passing this
    /// extra parameter is entirely avoided.
    ///
    /// @note
    /// The pointer to the sender object is not necessarily a pointer
    /// to an active object. In fact, when TICK_X() is called from
    /// an interrupt, you would create a unique object just to unambiguously
    /// identify the ISR as the sender of the time events.
    ///
    /// @sa QP::QF::tickX_()
    #define TICK_X(tickRate_, sender_) tickX_((tickRate_), (sender_))

    //! Invoke the event publishing facility QP::QF::publish_(). This macro
    /// @description
    /// This macro is the recommended way of publishing events, because it
    /// provides the vital information for software tracing and avoids any
    /// overhead when the tracing is disabled.
    ///
    /// @param[in] e_      pointer to the posted event
    /// @param[in] sender_ pointer to the sender object. This parameter is
    ///          actually only used when QS software tracing is enabled
    ///          (macro #Q_SPY is defined). When QS software tracing is
    ///          disabled, the macro calls QF_publish_() without the
    ///          @p sender_ parameter, so the overhead of passing this
    ///          extra parameter is entirely avoided.
    ///
    /// @note
    /// The pointer to the sender object is not necessarily a pointer
    /// to an active object. In fact, if QF_PUBLISH() is called from an
    /// interrupt or other context, you can create a unique object just to
    /// unambiguously identify the publisher of the event.
    ///
    /// @sa QP::QF::publish_()
    #define PUBLISH(e_, sender_) \
        publish_((e_), (sender_), (sender_)->getPrio())

    //! Invoke the direct event posting facility QP::QActive::post_().
    /// @description
    /// This macro asserts if the queue overflows and cannot accept the event.
    ///
    /// @param[in] e_      pointer to the event to post
    /// @param[in] sender_ pointer to the sender object.
    ///
    /// @note
    /// The @p sendedr_ parameter is actually only used when QS tracing
    /// is enabled (macro #Q_SPY is defined). When QS software tracing is
    /// disenabled, the POST() macro does not pass the @p sender_
    /// parameter, so the overhead of passing this extra parameter is entirely
    /// avoided.
    ///
    /// @note the pointer to the sender object is not necessarily a pointer
    /// to an active object. In fact, if POST() is called from an interrupt
    /// or other context, you can create a unique object just to
    /// unambiguously identify the sender of the event.
    ///
    /// @sa QP::QActive::post_()
    #define POST(e_, sender_) post_((e_), QP::QF_NO_MARGIN, (sender_))

    //! Invoke the direct event posting facility QP::QActive::post_()
    //! without delivery guarantee.
    /// @description
    /// This macro does not assert if the queue overflows and cannot accept
    /// the event with the specified margin of free slots remaining.
    ///
    /// @param[in]  e_      pointer to the event to post
    /// @param[in]  margin_ the minimum free slots in the queue, which
    ///                     must still be available after posting the event.
    ///                     The special value QP::QF_NO_MARGIN causes
    ///                     asserting failure in case event posting fails.
    /// @param[in]  sender_ pointer to the sender object.
    ///
    /// @returns
    /// 'true' if the posting succeeded, and 'false' if the posting
    /// failed due to insufficient margin of free entries available in
    /// the queue.
    ///
    /// @note
    /// The @p sender_ parameter is actually only used when QS tracing
    /// is enabled (macro #Q_SPY is defined). When QS software tracing is
    /// disabled, the POST_X() macro does not pass the @p sender_ parameter,
    /// so the overhead of passing this extra parameter is entirely avoided.
    ///
    /// @note
    /// The pointer to the sender object is not necessarily a pointer
    /// to an active object. In fact, if POST_X() is called from an
    /// interrupt or other context, you can create a unique object just to
    /// unambiguously identify the sender of the event.
    ///
    /// @usage
    /// @include qf_postx.cpp
    #define POST_X(e_, margin_, sender_) \
        post_((e_), (margin_), (sender_))

#else

    #define PUBLISH(e_, dummy_)         publish_((e_))
    #define POST(e_, dummy_)            post_((e_), QP::QF_NO_MARGIN)
    #define POST_X(e_, margin_, dummy_) post_((e_), (margin_))
    #define TICK_X(tickRate_, dummy_)   tickX_((tickRate_))

#endif // Q_SPY

//! Invoke the system clock tick processing for rate 0
/// @sa TICK_X()
#define TICK(sender_) TICK_X(0U, (sender_))

#endif // QF_HPP

