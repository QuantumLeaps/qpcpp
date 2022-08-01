//! @file
//! @brief Various macros for configuring and porting QP/C++

//! The preprocessor switch to disable checking assertions
//!
//! @description
//! When defined, Q_NASSERT disables the following macros #Q_ASSERT,
//! #Q_REQUIRE, #Q_ENSURE, #Q_INVARIANT, #Q_ERROR as well as
//! #Q_ASSERT_ID, #Q_REQUIRE_ID, #Q_ENSURE_ID, #Q_INVARIANT_ID, and
//! #Q_ERROR_ID do NOT evaluate the test condition passed as the
//! argument to these macros.
//!
//! @note One notable exception is the macro #Q_ALLEGE, that still
//! evaluates the test condition, but does not report assertion
//! failures when the switch Q_NASSERT is defined.
#define Q_NASSERT

//! The maximum number of active objects in the application.
//!
//! @description
//! This macro *must* be defined in the QF port and should be in range
//! of 1U..64U, inclusive. The value of this macro determines the maximum
//! priority level of an active object in the system. Not all priority
//! levels must be used, but the maximum priority cannot exceed
//! #QF_MAX_ACTIVE.
//!
//! @note Once you choose a certain value of #QF_MAX_ACTIVE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MAX_ACTIVE              32U

//! The maximum number of clock tick rates in the application.
//!
//! @description
//! This macro can be defined in the QF ports and should be in range
//! of 1U..15U, inclusive. The value of this macro determines the maximum
//! number of clock tick rates for time events (::QTimeEvt).
//!
//! If the macro is not defined, the default value is 1U.
//!
//! @note Once you choose a certain value of #QF_MAX_TICK_RATE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.h header file and henceforth include
//! this header file in all builds.
#define QF_MAX_TICK_RATE            1U

//! The maximum number of event pools in the application.
//!
//! This macro should be defined in the QF port and should be in range
//! of 1..255, inclusive. The value of this macro determines the maximum
//! number of event pools in the system. Not all all these event pools must
//! actually be used by the applications, but the maximum number of pools
//! cannot exceed #QF_MAX_EPOOL.
//!
//! If the macro is not defined, the default value is 3
//!
//! @note Once you choose a certain value of #QF_MAX_EPOOL, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MAX_EPOOL                3U

//! The size (in bytes) of the event-size representation in the QF.
//! Valid values: 1U, 2U, or 4U; default 2U
//!
//! @description
//! This macro can be defined in the QF ports to configure the size
//! of the event-size.
//!
//! @note Once you choose a certain value of #QF_EVENT_SIZ_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_EVENT_SIZ_SIZE           2U

//! The size (in bytes) of the ring-buffer counters used in the native
//! QF event queue implementation. Valid values: 1U, 2U, or 4U; default 1U
//!
//! @description
//! This macro can be defined in the QF ports to configure the QP::QEQueueCtr
//! type. If the macro is not defined, the default of 1 byte will be chosen
//! in qequeue.hpp. The valid #QF_EQUEUE_CTR_SIZE values of 1U, 2U, or 4U,
//! correspond to QP::QEQueueCtr of uint8_t, uint16_t, and uint32_t,
//! respectively. The QP::QEQueueCtr data type determines the dynamic range
//! of numerical values of ring-buffer counters inside event queues, or,
//! in other words, the maximum number of events that the native QF event
//! queue can manage.
//! @sa QP::QEQueue
//!
//! @note Once you choose a certain value of #QF_EQUEUE_CTR_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_EQUEUE_CTR_SIZE          1U

//! The size (in bytes) of the block-size representation in the
//! native QF event pool. Valid values: 1U, 2U, or 4U; default 2U.
//! #QF_EVENT_SIZ_SIZE.
//!
//! @description
//! This macro can be defined in the QF ports to configure the QP::QMPoolSize
//! type. If the macro is not defined, the default of #QF_EVENT_SIZ_SIZE
//! will be chosen in qmpool.hpp, because the memory pool is primarily used for
//! implementing event pools.
//!
//! The valid #QF_MPOOL_SIZ_SIZE values of 1U, 2U, or 4U, correspond to
//! QP::QMPoolSize of uint8_t, uint16_t, and uint32_t, respectively. The
//! QP::QMPoolSize data type determines the dynamic range of block-sizes that
//! the native QP::QMPool can hanle.
//! @sa #QF_EVENT_SIZ_SIZE, QP::QMPool
//!
//! @note Once you choose a certain value of #QF_MPOOL_SIZ_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MPOOL_SIZ_SIZE           2U

//! The size (in bytes) of the block-counter representation in the
//! native QF event pool. Valid values: 1U, 2U, or 4U; default 2U.
//!
//! @description
//! This macro can be defined in the QF ports to configure the QP::QMPoolCtr
//! type. If the macro is not defined, the default of 2 bytes will be chosen
//! in qmpool.hpp. The valid #QF_MPOOL_CTR_SIZE values of 1, 2, or 4, correspond
//! to QP::QMPoolSize of uint8_t, uint16_t, and uint32_t, respectively. The
//! QP::QMPoolCtr data type determines the dynamic range of block-counters that
//! the native QP::QMPool can handle, or, in other words, the maximum number
//! of blocks that the native QF event pool can manage.
//! @sa QP::QMPool
//!
//! @note Once you choose a certain value of #QF_MPOOL_CTR_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MPOOL_CTR_SIZE           2U

//! The size (in bytes) of the time event -counter representation
//! in the QP::QTimeEvt class. Valid values: 1U, 2U, or 4U; default 2U.
//!
//! @description
//! This macro can be defined in the QF ports to configure the internal tick
//! counters of Time Events. If the macro is not defined, the default of 2
//! bytes will be chosen in qf.hpp. The valid #QF_TIMEEVT_CTR_SIZE values of
//! 1U, 2U, or 4U, correspond to tick counters of uint8_t, uint16_t, and
//! uint32_t, respectively. The tick counter representation determines the
//! dynamic range of time delays that a Time Event can handle.
//! @sa QP::QTimeEvt
//!
//! @note Once you choose a certain value of #QF_TIMEEVT_CTR_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_TIMEEVT_CTR_SIZE         2U

//! Size (in bytes) of the QS time stamp
//!
//! @description
//! This macro can be defined in the QS port file (qs_port.hpp) to configure
//! the QP::QSTimeCtr type. Valid values 1U, 2U, 4U. Default 4U.
#define QS_TIME_SIZE                4U

//! Define the interrupt disabling policy.
//!
//! @description
//! This macro encapsulates platform-specific way of disabling interrupts
//! from C++ for a given CPU and compiler.
//!
//! @note the #QF_INT_DISABLE macro should always be used in pair with the
//! macro #QF_INT_ENABLE.
//!
#define QF_INT_DISABLE()            intDisable()

//! Define the interrupt enabling policy.
//!
//! @description
//! This macro encapsulates platform-specific way of enabling interrupts
//! from "C" for a given CPU and compiler.
//!
//! @note the #QF_INT_DISABLE macro should always be used in pair with the
//! macro #QF_INT_ENABLE.
//!
#define QF_INT_ENABLE()             intEnable()

//! Define the type of the critical section status.
//!
//! @description
//! Defining this macro configures the "saving and restoring critical section
//! status" policy. Conversely, if this macro is not defined, the simple
//! "unconditional critical section exit" is used.
//!
#define QF_CRIT_STAT_TYPE           unsigned

//! Define the critical section entry policy.
//!
//! This macro enters a critical section (often by means of disabling
//! interrupts). When the "saving and restoring critical section status"
//! policy is used, the macro sets the \a status_ argument to the critical
//! section status just before the entry. When the policy of "unconditional
//! critical section exit" is used, the macro does not use the \a status_
//! argument.
//!
//! @note the #QF_CRIT_ENTRY macro should always be used in pair with the
//! macro #QF_CRIT_EXIT.
//!
#define QF_CRIT_ENTRY(stat_)        ((stat_) = critEntry())

//! Define the critical section exit policy.
//!
//! @description
//! This macro enters a critical section (often by means of disabling
//! interrupts). When the "saving and restoring critical section status"
//! policy is used, the macro restores the critical section status from the
//! @a status_ argument. When the policy of "unconditional critical section
//! exit" is used, the macro does not use the \a status argument and
//! exits the critical section unconditionally (often by means of enabling
//! interrupts).
//!
//! @note the #QF_CRIT_ENTRY macro should always be used in pair with the
//! macro #QF_CRIT_EXIT.
//!
#define QF_CRIT_EXIT(stat_)         critExit(stat_)

//! Enable the QActive::stop() API in the QF port.
//!
//! @description
//! Defining this macro enables the QActive::stop() API in a given port.
//! This feature should be used with caution, as stopping and re-starting
//! active objects **cleanly** can be tricky.
//!
#define QF_ACTIVE_STOP

//! The preprocessor switch to activate the constructor in QP::QEvt.
//!
//! @description
//! When #Q_EVT_CTOR is defined (typically in the qep_port.hpp header file),
//! QP::QEvt becomes a class with constructor. More importantly, the
//! subclasses of QP::QEvt (custom events) can have non-default constructors.
//! These constructors are then called when events are created (e.g.,
//! with Q_NEW())
//!
//! @sa #Q_EVT_VIRTUAL
#define Q_EVT_CTOR

//! The preprocessor switch to activate the virtual destructor in QP::QEvt.
//!
//! @description
//! This macro only works when #Q_EVT_CTOR is also defined. When also
//! #Q_EVT_VIRTUAL is defined (typically in the qep_port.hpp header
//! file), QP::QEvt becomes a class with a constructor and a virtual
//! destructor. More importantly, the subclasses of QP::QEvt (custom events)
//! can have (virtual) destructors. These destructors are then invoked
//! before recycling the event with QP::QF::gc().
#define Q_EVT_VIRTUAL

//! The preprocessor switch to activate the QS software tracing
//! instrumentation in the code
//!
//! @description
//! When defined, Q_SPY activates the QS software tracing instrumentation.
//! When Q_SPY is not defined, the QS instrumentation in the code does
//! not generate any code.
#define Q_SPY

//! The preprocessor switch to activate the QUTest unit testing
//! instrumentation in the code
//!
//! @note
//! This macro requires that #Q_SPY be defined as well.
#define Q_UTEST

//! This macro defines the type of the thread handle used for AOs
#define QF_THREAD_TYPE         void*

//! This macro defines the type of the event-queue used for AOs
#define QF_EQUEUE_TYPE         QEQueue

//! This macro defines the type of the OS-Object used for blocking
//! the native ::QEQueue when the queue is empty
//!
//! @description
//! This macro is used when ::QEQueue is used as the event-queue for AOs
//! but also the AO queue must *block* when the queue is empty.
//! In that case, #QF_OS_OBJECT_TYPE specifies the blocking mechanism.
//! For examle, in the POSIX port, the blocking mechanism is a condition
//! variable.
//!
#define QF_OS_OBJECT_TYPE      pthread_cond_t

//! Platform-dependent macro defining how QF should block the calling
//! task when the QF native queue is empty
//!
//! @note
//! This is just an example of #QACTIVE_EQUEUE_WAIT_ for the QK-port
//! of QF. QK never activates a task that has no events to process, so in this
//! case the macro asserts that the queue is not empty. In other QF ports you
//! need to define the macro appropriately for the underlying kernel/OS you're
//! using.
#define QACTIVE_EQUEUE_WAIT_(me_) \
    Q_ASSERT((me_)->m_eQueue.m_frontEvt != nullptr)

//! Platform-dependent macro defining how QF should signal the
//! active object task that an event has just arrived.
//!
//! @description
//! The macro is necessary only when the native QF event queue is used.
//! The signaling of task involves unblocking the task if it is blocked.
//!
//! @note #QACTIVE_EQUEUE_SIGNAL_ is called from a critical section.
//! It might leave the critical section internally, but must restore
//! the critical section before exiting to the caller.
//!
//! @note This is just an example of #QACTIVE_EQUEUE_SIGNAL_ for the QK-port
//! of QF. In other QF ports you need to define the macro appropriately for
//! the underlying kernel/OS you're using.
#define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    QF::readySet_.insert((me_)->m_prio); \
    if (QF::intNest_ == 0U) {            \
        uint8_t p = QK_schedPrio_();     \
        if (p != 0U) {                   \
            QK_sched_(p);                \
        }                                \
    }                                    \
} while (false)

//! This macro defines the type of the event pool used in this QF port.
//!
//! @note
//! This is a specific implementation for the QK-port of QF.
//! In other QF ports you need to define the macro appropriately for
//! the underlying kernel/OS you're using.
#define QF_EPOOL_TYPE_              QMPool

//! This macro enables calling the QK context-switch callback
//! QK_onContextSw()
#define QK_ON_CONTEXT_SW

//! This macro enables calling the QXK context-switch callback
//! QXK_onContextSw()
#define QXK_ON_CONTEXT_SW

//! Platform-dependent macro defining the event pool initialization
//!
//! @note
//! This is a specific implementation for the QK-port of QF.
//! In other QF ports you need to define the macro appropriately for
//! the underlying kernel/OS you're using.
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (p_).init((poolSto_), (poolSize_), static_cast<QMPoolSize>(evtSize_))

//! Platform-dependent macro defining how QF should obtain the
//! event pool block-size
//!
//! @note
//! This is a specific implementation for the QK-port of QF.
//! In other QF ports you need to define the macro appropriately for
//! the underlying kernel/OS you're using.
#define QF_EPOOL_EVENT_SIZE_(p_) static_cast<uint32_t>((p_).getBlockSize())

//! Platform-dependent macro defining how QF should obtain an event
//! @a e_ from the event pool @a p_
//!
//! @note
//! This is a specific implementation for the QK-port of QF.
//! In other QF ports you need to define the macro appropriately for
//! the underlying kernel/OS you're using.
#define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
    ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))

//! Platform-dependent macro defining how QF should return an event
//! @a e_ to the event pool @a p_
//!
//! @note
//! This is a specific implementation for the QK-port of QF.
//! In other QF ports you need to define the macro appropriately for
//! the underlying kernel/OS you're using.
#define QF_EPOOL_PUT_(p_, e_, qs_id_)   ((p_).put((e_), (qs_id_)))

//! Macro that should be defined (typically on the compiler's command line)
//! in the Win32-GUI applications that use the @ref win32 or @ref win32-qv
//! ports.
#define WIN32_GUI
