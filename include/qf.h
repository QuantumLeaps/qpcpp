/// @file
/// @brief QF/C++ platform-independent public interface.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.5.0
/// Last updated on  2019-03-21
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qf_h
#define qf_h

//****************************************************************************
#ifndef qpset_h
#include "qpset.h"
#endif

#ifdef Q_EVT_CTOR
#include <new>  // for placement new
#endif // Q_EVT_CTOR

//****************************************************************************
// apply defaults for all undefined configuration parameters
//
#ifndef QF_EVENT_SIZ_SIZE
    //! Default value of the macro configurable value in qf_port.h
    #define QF_EVENT_SIZ_SIZE    2
#endif

#ifndef QF_MAX_EPOOL
    //! Default value of the macro configurable value in qf_port.h
    #define QF_MAX_EPOOL         3
#endif

#ifndef QF_MAX_TICK_RATE
    //! Default value of the macro configurable value in qf_port.h
    //! Valid values: [0..15]; default 1
    #define QF_MAX_TICK_RATE     1
#elif (QF_MAX_TICK_RATE > 15)
    #error "QF_MAX_TICK_RATE exceeds the maximum of 15"
#endif

#ifndef QF_TIMEEVT_CTR_SIZE
    //! macro to override the default QTimeEvtCtr size.
    //! Valid values 1, 2, or 4; default 2
    #define QF_TIMEEVT_CTR_SIZE  2
#endif


//****************************************************************************
namespace QP {

#if (QF_EVENT_SIZ_SIZE == 1)
    typedef uint8_t QEvtSize;
#elif (QF_EVENT_SIZ_SIZE == 2)
    //! The data type to store the block-size defined based on
    //! the macro #QF_EVENT_SIZ_SIZE.
    /// @description
    /// The dynamic range of this data type determines the maximum block
    /// size that can be managed by the pool.
    typedef uint16_t QEvtSize;
#elif (QF_EVENT_SIZ_SIZE == 4)
    typedef uint32_t QEvtSize;
#else
    #error "QF_EVENT_SIZ_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

//****************************************************************************
#if (QF_TIMEEVT_CTR_SIZE == 1)
    typedef uint8_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 2)
    //! type of the Time Event counter, which determines the dynamic
    //! range of the time delays measured in clock ticks.
    /// @description
    /// This typedef is configurable via the preprocessor switch
    /// #QF_TIMEEVT_CTR_SIZE. The other possible values of this type are
    /// as follows: @n
    /// uint8_t when (QF_TIMEEVT_CTR_SIZE == 1), and @n
    /// uint32_t when (QF_TIMEEVT_CTR_SIZE == 4).
    typedef uint16_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 4)
    typedef uint32_t QTimeEvtCtr;
#else
    #error "QF_TIMEEVT_CTR_SIZE defined incorrectly, expected 1, 2, or 4"
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

    //! QF priority (1..#QF_MAX_ACTIVE) of this active object.
    uint8_t m_prio;

#ifdef qxk_h // QXK kernel used?
    //! QF start priority (1..#QF_MAX_ACTIVE) of this active object.
    uint8_t m_startPrio;
#endif

protected:
    //! protected constructor (abstract class)
    QActive(QStateHandler const initial);

public:
    //! Starts execution of an active object and registers the object
    //! with the framework.
    virtual void start(uint_fast8_t const prio,
                       QEvt const *qSto[], uint_fast16_t const qLen,
                       void * const stkSto, uint_fast16_t const stkSize,
                       QEvt const * const ie);

    //! Overloaded start function (no initialization event)
    virtual void start(uint_fast8_t const prio,
                       QEvt const *qSto[], uint_fast16_t const qLen,
                       void * const stkSto, uint_fast16_t const stkSize)
    {
        this->start(prio, qSto, qLen, stkSto, stkSize,
                    static_cast<QEvt const *>(0));
    }

#ifdef QF_ACTIVE_STOP
    //! Stops execution of an active object and removes it from the
    //! framework's supervision.
    void stop(void);
#endif

#ifndef Q_SPY
    //! Posts an event @p e directly to the event queue of the active
    //! object @p me using the First-In-First-Out (FIFO) policy.
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin);
#else
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin,
                       void const * const sender);
#endif

    //! Posts an event directly to the event queue of the active object
    //! using the Last-In-First-Out (LIFO) policy.
    virtual void postLIFO(QEvt const * const e);

    //! Un-subscribes from the delivery of all signals to the active object.
    void unsubscribeAll(void) const;

    //! Subscribes for delivery of signal @p sig to the active object
    void subscribe(enum_t const sig) const;

    //! Un-subscribes from the delivery of signal @p sig to the active object.
    void unsubscribe(enum_t const sig) const;

    //! Defer an event to a given separate event queue.
    bool defer(QEQueue * const eq, QEvt const * const e) const;

    //! Recall a deferred event from a given event queue.
    bool recall(QEQueue * const eq);

    //! Flush the specified deferred queue 'eq'.
    uint_fast16_t flushDeferred(QEQueue * const eq) const;

    //! Get the priority of the active object.
    uint_fast8_t getPrio(void) const {
        return static_cast<uint_fast8_t>(m_prio);
    }

    //! Set the priority of the active object.
    void setPrio(uint_fast8_t const prio) {
        m_prio = static_cast<uint8_t>(prio);
    }

    //! Generic setting of additional attributes (useful in QP ports)
    void setAttr(uint32_t attr1, void const *attr2 = static_cast<void *>(0));

#ifdef QF_OS_OBJECT_TYPE
    //! accessor to the OS-object for extern "C" functions, such as
    //! the QK scheduler
    QF_OS_OBJECT_TYPE &getOsObject(void) { return m_osObject; }
#endif

#ifdef QF_THREAD_TYPE
    //! accessor to the Thread for extern "C" functions, such as
    //! the QK scheduler
    QF_THREAD_TYPE &getThread(void) { return m_thread; }
#endif

    //! Get an event from the event queue of an active object.
    QEvt const *get_(void);

// duplicated API to be used exclusively inside ISRs (useful in some QP ports)
#ifdef QF_ISR_API
#ifdef Q_SPY
    virtual bool postFromISR_(QEvt const * const e,
                              uint_fast16_t const margin, void *par,
                              void const * const sender);
#else
    virtual bool postFromISR_(QEvt const * const e,
                              uint_fast16_t const margin, void *par);
#endif // Q_SPY
#endif // QF_ISR_API

// friendships...
private:
    friend class QF;
    friend class QTimeEvt;
    friend class QTicker;
#ifdef qk_h
    friend class QMutex;
#endif // qk_h
#ifdef qxk_h
    friend class QXK;
    friend class QXThread;
    friend class QXMutex;
    friend class QXSemaphore;
#endif // qxk_h
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
    virtual void init(QEvt const * const e);
    virtual void init(void);
    virtual void dispatch(QEvt const * const e);

    //! Tests if a given state is part of the active state configuration
    bool isInState(QMState const * const st) const;

    //! Return the current active state object (read only)
    QMState const *stateObj(void) const {
        return m_state.obj;
    }

    //! Obtain the current active child state of a given parent (read only)
    QMState const *childStateObj(QMState const * const parent) const;

protected:
    //! protected constructor (abstract class)
    QMActive(QStateHandler const initial);

private:
    //! operation inherited from QActive/QHsm, but disallowed in QP::QMActive
    bool isIn(QStateHandler const s);

    //! operation inherited from QActive/QHsm, but disallowed in QP::QMActive
    QStateHandler state(void) const;

    //! operation inherited from QActive/QHsm, but disallowed in QP::QMActive
    QStateHandler childState(QStateHandler const parent);
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
             uint_fast8_t const tickRate = static_cast<uint_fast8_t>(0));

    //! Arm a time event (one shot or periodic) for event posting.
    void armX(QTimeEvtCtr const nTicks,
              QTimeEvtCtr const interval = static_cast<QTimeEvtCtr>(0));

    //! Disarm a time event.
    bool disarm(void);

    //! Rearm a time event.
    bool rearm(QTimeEvtCtr const nTicks);

    //! Check the "was disarmed" status of a time event.
    bool wasDisarmed(void);

    //! Get the current value of the down-counter of a time event.
    QTimeEvtCtr currCtr(void) const;

#if (!defined QP_IMPL) && (QP_API_VERSION < 500)
    //! @deprecated TimeEvt ctor provided for backwards compatibility.
    /// @overload QTimeEvt::QTimeEvt(QActive * const, enum_t const,
    ///                              uint_fast8_t const)
    QTimeEvt(enum_t const sgnl) :
#ifdef Q_EVT_CTOR
        QEvt(static_cast<QSignal>(sgnl)),
#endif
        m_next(static_cast<QTimeEvt *>(0)),
        m_act(static_cast<void *>(0)),
        m_ctr(static_cast<QTimeEvtCtr>(0)),
        m_interval(static_cast<QTimeEvtCtr >(0))
    {
#ifndef Q_EVT_CTOR
        sig = static_cast<QSignal>(sgnl); // set QEvt::sig of this time event
#endif
        // time event must be static, see NOTE01
        poolId_ = static_cast<uint8_t>(0); // not from any event pool
        refCtr_ = static_cast<uint8_t>(0); // default rate 0, see NOTE02
    }

    //! @deprecated interface provided for backwards compatibility.
    void postIn(QActive * const act, QTimeEvtCtr const nTicks) {
        m_act = act;
        armX(nTicks, static_cast<QTimeEvtCtr>(0));
    }

    //! @deprecated interface provided for backwards compatibility.
    void postEvery(QActive * const act, QTimeEvtCtr const nTicks) {
        m_act = act;
        armX(nTicks, nTicks);
    }
#endif // (!defined QP_IMPL) && (QP_API_VERSION < 500)

private:
    //! private default constructor only for friends
    QTimeEvt(void);

    //! private copy constructor to disallow copying of QTimeEvts
    QTimeEvt(QTimeEvt const &);

    //! private assignment operator to disallow assigning of QTimeEvts
    QTimeEvt & operator=(QTimeEvt const &);

    //! encapsulate the cast the m_act attribute to QActive*
    QActive  *toActive(void)  { return static_cast<QActive  *>(m_act); }

    //! encapsulate the cast the m_act attribute to QTimeEvt*
    QTimeEvt *toTimeEvt(void) { return static_cast<QTimeEvt *>(m_act); }

    friend class QF;
    friend class QS;
#ifdef qxk_h
    friend class QXThread;
    friend void QXK_activate_(void);
#endif // qxk_h
};


//****************************************************************************
//! Subscriber List
/// @description
/// This data type represents a set of active objects that subscribe to
/// a given signal. The set is represented as priority-set, where each
/// bit corresponds to the unique priority of an active object.
typedef QPSet QSubscrList;


//****************************************************************************
//! QF services.
/// @description
/// This class groups together QF services. It has only static members and
/// should not be instantiated.
class QF {
public:

    //! get the current QF version number string of the form X.Y.Z
    static char_t const *getVersion(void) {
        return versionStr;
    }

    //! QF initialization.
    static void init(void);

    //! Publish-subscribe initialization.
    static void psInit(QSubscrList * const subscrSto,
                       enum_t const maxSignal);

    //! Event pool initialization for dynamic allocation of events.
    static void poolInit(void * const poolSto, uint_fast32_t const poolSize,
                         uint_fast16_t const evtSize);

    //! Obtain the block size of any registered event pools
    static uint_fast16_t poolGetMaxBlockSize(void);


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
    static void publish_(QEvt const *e);
    static void tickX_(uint_fast8_t const tickRate);
#else
    //! Publish event to the framework.
    static void publish_(QEvt const *e, void const *sender);

    //! Processes all armed time events at every clock tick.
    static void tickX_(uint_fast8_t const tickRate,
                       void const * const sender);
#endif // Q_SPY

    //! Returns true if all time events are inactive and false
    //! any time event is active.
    static bool noTimeEvtsActiveX(uint_fast8_t const tickRate);

    //! This function returns the minimum of free entries of the given
    //! event pool.
    static uint_fast16_t getPoolMin(uint_fast8_t const poolId);

    //! This function returns the minimum of free entries of the given
    //! event queue.
    static uint_fast16_t getQueueMin(uint_fast8_t const prio);

    //! Internal QF implementation of creating new dynamic event.
    static QEvt *newX_(uint_fast16_t const evtSize,
                       uint_fast16_t const margin, enum_t const sig);

    //! Recycle a dynamic event.
    static void gc(QEvt const *e);

    //! Internal QF implementation of creating new event reference.
    static QEvt const *newRef_(QEvt const * const e,
                               QEvt const * const evtRef);

    //! Internal QF implementation of deleting event reference.
    static void deleteRef_(QEvt const * const evtRef);

    //! Remove the active object from the framework.
    static void remove_(QActive * const a);

    //! array of registered active objects
    static QActive *active_[QF_MAX_ACTIVE + 1];

    //! Thread routine for executing an active object @p act.
    static void thread_(QActive *act);

    //! Register an active object to be managed by the framework
    static void add_(QActive * const a);

    //! Clear a specified region of memory to zero.
    static void bzero(void * const start, uint_fast16_t len);

// API to be used exclusively inside ISRs (useful in some QP ports)
#ifdef QF_ISR_API
#ifdef Q_SPY
    static void publishFromISR_(QEvt const *e, void *par,
                                void const *sender);
    static void tickXfromISR_(uint_fast8_t const tickRate, void *par,
                              void const * const sender);
#else
    static void publishFromISR_(QEvt const *e, void *par);
    static void tickXfromISR_(uint_fast8_t const tickRate, void *par);
#endif // Q_SPY

    static QEvt *newXfromISR_(uint_fast16_t const evtSize,
                              uint_fast16_t const margin, enum_t const sig);
    static void gcFromISR(QEvt const *e);

#endif // QF_ISR_API

// to be used in QF ports only...
private:
    //! heads of linked lists of time events, one for every clock tick rate
    static QTimeEvt timeEvtHead_[QF_MAX_TICK_RATE];

    friend class QActive;
    friend class QTimeEvt;
    friend class QS;
#ifdef qxk_h
    friend class QXThread;
#endif // qxk_h
};

//! special value of margin that causes asserting failure in case
//! event allocation or event posting fails
uint_fast16_t const QF_NO_MARGIN = static_cast<uint_fast16_t>(0xFFFF);


//****************************************************************************
//! Ticker Active Object class
/// @description
/// The QTicker is an efficient active object specialized to process
/// QF system clock tick at a specified tick frequency [0..QF_MAX_TICK_RATE].
/// Placing system clock tick processing in an active object allows you
/// to remove the non-deterministic QF::TICK_X() processing from the interrupt
/// level and move it into the thread-level, where you can prioritize it
/// as low as you wish.
///
class QTicker : public QActive {
public:
    QTicker(uint_fast8_t const tickRate); // ctor

    virtual void init(QEvt const * const e);
    virtual void init(void) { this->init(static_cast<QEvt const *>(0)); }
    virtual void dispatch(QEvt const * const e);
#ifndef Q_SPY
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin);
#else
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin,
                       void const * const sender);
#endif
    virtual void postLIFO(QEvt const * const e);
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
    #define QF_CRIT_EXIT_NOP()   ((void)0)
#endif

//****************************************************************************
// Provide the constructor for the QEvt class?
#ifdef Q_EVT_CTOR

    #define Q_NEW(evtT_, sig_, ...) \
        (new(QP::QF::newX_(static_cast<uint_fast16_t>(sizeof(evtT_)), \
                     QP::QF_NO_MARGIN, static_cast<enum_t>(0))) \
            evtT_((sig_),  ##__VA_ARGS__))

    #define Q_NEW_X(e_, evtT_, margin_, sig_, ...) do { \
        (e_) = static_cast<evtT_ *>(QP::QF::newX_(static_cast<uint_fast16_t>(\
                  sizeof(evtT_)), (margin_), static_cast<enum_t>(0))); \
        if ((e_) != static_cast<evtT_ *>(0)) { \
            new((e_)) evtT_((sig_),  ##__VA_ARGS__); \
        } \
     } while (0)

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
    #define Q_NEW(evtT_, sig_) \
        (static_cast<evtT_ *>(QP::QF::newX_( \
                static_cast<uint_fast16_t>(sizeof(evtT_)), \
                QP::QF_NO_MARGIN, (sig_))))

    //! Allocate a dynamic event (non-asserting version).
    /// @description
    /// This macro allocates a new event and sets the pointer @p e_, while
    /// leaving at least @p margin_ of events still available in the pool
    ///
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
    #define Q_NEW_X(e_, evtT_, margin_, sig_)  ((e_) = static_cast<evtT_ *>(\
        QP::QF::newX_(static_cast<uint_fast16_t>(sizeof(evtT_)),\
                      (margin_), (sig_))))
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
    (evtRef_) = 0; \
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
    /// @param[in] tickRate clock tick rate to be serviced through this call
    /// @param[in] sender   pointer to the sender object. This parameter
    ///            is actually only used when QS software tracing is enabled
    ///            (macro #Q_SPY is defined)
    /// @note
    /// When QS software tracing is disabled, the macro calls QF_tickX_()
    /// without the @p sender parameter, so the overhead of passing this
    /// extra parameter is entirely avoided.
    ///
    /// @note
    /// The pointer to the sender object is not necessarily a pointer
    /// to an active object. In fact, when #QF_TICK_X() is called from
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
    #define PUBLISH(e_, sender_)    publish_((e_), (sender_))

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
    /// disenabled, the QACTIVE_POST() macro does not pass the @p sender_
    /// parameter, so the overhead of passing this extra parameter is entirely
    /// avoided.
    ///
    /// @note the pointer to the sender object is not necessarily a pointer
    /// to an active object. In fact, if QP::QActive::post() is called from an
    /// interrupt or other context, you can create a unique object just to
    /// unambiguously identify the sender of the event.
    ///
    /// @sa QP::QActive::post_()
    #define POST(e_, sender_) \
        post_((e_), QP::QF_NO_MARGIN, (sender_))

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
    /// disabled, the QACTIVE_POST() macro does not pass the @p sender_
    /// parameter, so the overhead of passing this extra parameter is
    /// entirely avoided.
    ///
    /// @note
    /// The pointer to the sender object is not necessarily a pointer
    /// to an active object. In fact, if QP::QActive::post() is called from an
    /// interrupt or other context, you can create a unique object just to
    /// unambiguously identify the sender of the event.
    ///
    /// @usage
    /// @include qf_postx.cpp
    #define POST_X(e_, margin_, sender_) \
        post_((e_), (margin_), (sender_))

#else

    #define PUBLISH(e_, dummy_)  publish_((e_))
    #define POST(e_, dummy_)     post_((e_), QP::QF_NO_MARGIN)
    #define POST_X(e_, margin_, dummy_) post_((e_), (margin_))
    #define TICK_X(tickRate_, dummy_)   tickX_((tickRate_))

#endif // Q_SPY

//! Invoke the system clock tick processing for rate 0
/// @sa TICK_X()
#define TICK(sender_) TICK_X(static_cast<uint_fast8_t>(0), (sender_))

#endif // qf_h

