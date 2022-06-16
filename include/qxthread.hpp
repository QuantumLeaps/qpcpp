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
//! @brief QXK/C++ extended (blocking) thread

#ifndef QXTHREAD_HPP
#define QXTHREAD_HPP

namespace QP {

//! no-timeout specification when blocking on queues or semaphores
static constexpr std::uint_fast16_t QXTHREAD_NO_TIMEOUT = 0U;

//============================================================================
//! Extended (blocking) thread of the QXK preemptive kernel
//!
//! @description
//! QP::QXThread represents the extended (blocking) thread of the QXK kernel.
//! Each blocking thread in the application must be represented by the
//! corresponding QP::QXThread instance
//!
//! @note
//! Typically QP::QXThread is instantiated directly in the application code.
//! The customization of the thread occurs in the constructor, where you
//! provide the thread-handler function as the parameter.
//!
//! @sa QP::QActive
//!
//! @usage
//! The following example illustrates how to instantiate and use an extended
//! thread in your application.
//! @include qxk_thread.cpp
//!
class QXThread : public QActive {
public:

    //! public constructor
    //!
    //! @description
    //! Performs the first step of QXThread initialization by assigning the
    //! thread-handler function and the tick rate at which it will handle
    //! the timeouts.
    //!
    //! @param[in]     handler  the thread-handler function
    //! @param[in]     tickRate the ticking rate associated with this thread
    //!                for timeouts in this thread (see QXThread::delay() and
    //!                TICK_X())
    //!
    //! @note
    //! Must be called only ONCE before QXThread::start().
    //!
    QXThread(QXThreadHandler const handler,
             std::uint_fast8_t const tickRate = 0U) noexcept;

    //! delay (block) the current extended thread for a specified # ticks
    //!
    //! @description
    //! Blocking delay for the number of clock tick at the associated
    //! tick rate.
    //!
    //! @param[in]  nTicks    number of clock ticks (at the associated rate)
    //!                       to wait for the event to arrive.
    //! @note
    //! For the delay to work, the TICK_X() macro needs to be called
    //! periodically at the associated clock tick rate.
    //!
    //! @sa #QXThread
    //! @sa TICK_X()
    //!
    static bool delay(std::uint_fast16_t const nTicks) noexcept;

    //! cancel the delay
    //!
    //! @description
    //! Cancel the blocking delay and cause return from the QXThread::delay()
    //! function.
    //!
    //! @returns
    //! "true" if the thread was actually blocked on QXThread::delay() and
    //! "false" otherwise.
    //!
    bool delayCancel(void) noexcept;

    //! Get a message from the private message queue (block if no messages)
    //!
    //! @description
    //! The QXThread::queueGet() operation allows the calling extended thread
    //! to receive QP events (see QP::QEvt) directly into its own built-in
    //! event queue from an ISR, basic thread (AO), or another extended thread.
    //!
    //! If QXThread::queueGet() is called when no events are present in the
    //! thread's private event queue, the operation blocks the current
    //! extended thread until either an event is received, or a user-specified
    //! timeout expires.
    //!
    //! @param[in]  nTicks  number of clock ticks (at the associated rate)
    //!                     to wait for the event to arrive. The value of
    //!                     QXTHREAD_NO_TIMEOUT indicates that no timeout
    //!                     will occur and the queue will block indefinitely.
    //! @returns
    //! A pointer to the event. If the pointer is not nullptr, the event
    //! was delivered. Otherwise the event pointer of nullptr indicates that
    //! the queue has timed out.
    //!
    static QEvt const *queueGet(
        std::uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT) noexcept;

    //! virtual function overrides...
    //! Executes the top-most initial transition in HSM
    void init(void const * const e,
              std::uint_fast8_t const qs_id) noexcept override;
    void init(std::uint_fast8_t const qs_id) noexcept override {
        this->init(nullptr, qs_id);
    }

    //! Dispatches an event to HSM
    void dispatch(QEvt const * const e,
                  std::uint_fast8_t const qs_id) noexcept override;

    //! Starts execution of an extended thread and registers the thread
    //! with the framework
    //!
    //! @description
    //! Starts execution of an extended thread and registers it with the
    //! framework. The extended thread becomes ready-to-run immediately and
    //! is scheduled if the QXK is already running.
    //!
    //! @param[in]     prio    priority at which to start the extended thread
    //! @param[in]     qSto    pointer to the storage for the ring buffer of
    //!                        the event queue. This cold be NULL, if this
    //!                        extended thread does not use the built-in
    //!                        event queue.
    //! @param[in]     qLen    length of the event queue [in events],
    //!                        or zero if queue not used
    //! @param[in]     stkSto  pointer to the stack storage (must be provided)
    //! @param[in]     stkSize stack size [in bytes] (must not be zero)
    //! @param[in]     par     pointer to an extra parameter (might be NULL)
    //!
    //! @usage
    //! The following example shows starting an extended thread:
    //! @include qxk_start.cpp
    //!
    void start(std::uint_fast8_t const prio,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize,
               void const * const par) override;

    //! Overloaded start function (no initialization event)
    void start(std::uint_fast8_t const prio,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize) override
    {
        this->start(prio, qSto, qLen, stkSto, stkSize, nullptr);
    }

#ifndef Q_SPY
    //! Posts an event `e` directly to the event queue of the extended
    //! thread using the First-In-First-Out (FIFO) policy
    //!
    //! @description
    //! Extended threads can be configured (in QXThread::start()) to have
    //! a private event queue. In that case, QP events (see QP::QEvt) can
    //! be asynchronously posted or published to the extended thread.
    //! The thread can wait (and block) on its queue and then it can
    //! process the delivered event.
    //!
    //! @param[in] e      pointer to the event to be posted
    //! @param[in] margin number of required free slots in the queue
    //!                   after posting the event. The special value
    //!                   QP::QF_NO_MARGIN means that this function will
    //!                   assert if posting fails.
    //! @param[in] sender pointer to a sender object (used in QS only)
    //!
    //! @returns
    //! 'true' (success) if the posting succeeded (with the provided margin)
    //! and 'false' (failure) when the posting fails.
    //!
    //! @attention
    //! Should be called only via the macro POST() or POST_X().
    //!
    //! @note
    //! The QP::QF_NO_MARGIN value of the `margin` parameter is special and
    //! denotes situation when the post() operation is assumed to succeed
    //! (event delivery guarantee). An assertion fires, when the event cannot
    //! be delivered in this case.
    //!
    bool post_(QEvt const * const e,
               std::uint_fast16_t const margin) noexcept override;
#else
    bool post_(QEvt const * const e, std::uint_fast16_t const margin,
               void const * const sender) noexcept override;
#endif

    //! Posts an event directly to the event queue of the extended thread
    //! using the Last-In-First-Out (LIFO) policy
    //!
    //! @description
    //! Last-In-First-Out (LIFO) policy is not supported for extened threads.
    //!
    //! @param[in]  e  pointer to the event to post to the queue
    //!
    //! @sa
    //! QXThread::post_(),
    //! QActive::postLIFO_()
    //!
    void postLIFO(QEvt const * const e) noexcept override;

private:
    //! Block the extended thread
    //!
    //! @description
    //! Internal implementation of blocking the given extended thread.
    //!
    //! @note
    //! Must be called from within a critical section
    //!
    void block_(void) const noexcept;

    //! Unblock the extended thread
    //!
    //! @description
    //! Internal implementation of unblocking the given extended thread.
    //!
    //! @note
    //! must be called from within a critical section
    //!
    void unblock_(void) const noexcept;

    //! Arm the private time event
    //!
    //! @description
    //! Internal implementation of arming the private time event for
    //! a given timeout at a given system tick rate.
    //!
    //! @note
    //! Must be called from within a critical section
    //!
    void teArm_(enum_t const sig, std::uint_fast16_t const nTicks) noexcept;

    //! Disarm the private time event
    //!
    //! @description
    //! Internal implementation of disarming the private time event.
    //!
    //! @note
    //! Must be called from within a critical section
    //!
    bool teDisarm_(void) noexcept;

    // attributes...
    QTimeEvt m_timeEvt; //!< time event to handle blocking timeouts

    // friendships...
    friend class QXSemaphore;
    friend class QXMutex;
};

//============================================================================
//! Counting Semaphore of the QXK preemptive kernel
//!
//! @description
//! QP::QXSemaphore is a blocking mechanism intended primarily for signaling
//! @ref QP::QXThread "extended threads". The semaphore is initialized with
//! the maximum count (see QP::QXSemaphore::init()), which allows you to
//! create a binary semaphore (when the maximum count is 1) and
//! counting semaphore when the maximum count is > 1.
//!
//! @usage
//! The following example illustrates how to instantiate and use the semaphore
//! in your application.
//! @include qxk_sema.cpp
//!
class QXSemaphore {
public:
    //! initialize the counting semaphore
    //!
    //! @description
    //! Initializes a semaphore with the specified count and maximum count.
    //! If the semaphore is used for resource sharing, both the initial count
    //! and maximum count should be set to the number of identical resources
    //! guarded by the semaphore. If the semaphore is used as a signaling
    //! mechanism, the initial count should set to 0 and maximum count to 1
    //! (binary semaphore).
    //!
    //! @param[in]     count  initial value of the semaphore counter
    //! @param[in]     max_count  maximum value of the semaphore counter.
    //!                The purpose of the max_count is to limit the counter
    //!                so that the semaphore cannot unblock more times than
    //!                the maximum.
    //! @note
    //! QXSemaphore::init() must be called **before** the semaphore can be
    //! used (signaled or waited on).
    //!
    void init(std::uint_fast16_t const count,
              std::uint_fast16_t const max_count = 0xFFFFU) noexcept;

    //! wait (block) on the semaphore
    //!
    //! @description
    //! When an extended thread calls QXSemaphore::wait() and the value of the
    //! semaphore counter is greater than 0, QXSemaphore_wait() decrements the
    //! semaphore counter and returns (true) to its caller. However, if the
    //! value of the semaphore counter is 0, the function places the calling
    //! thread in the waiting list for the semaphore. The thread waits until
    //! the semaphore is signaled by calling QXSemaphore::signal(), or the
    //! specified timeout expires. If the semaphore is signaled before the
    //! timeout expires, QXK resumes the highest-priority extended thread
    //! waiting for the semaphore.
    //!
    //! @param[in]  nTicks    number of clock ticks (at the associated rate)
    //!                       to wait for the semaphore. The value of
    //!                       QXTHREAD_NO_TIMEOUT indicates that no timeout
    //!                       will occur and the semaphore will wait
    //!                       indefinitely.
    //! @returns
    //! true if the semaphore has been signaled, and false if the timeout
    //! occurred.
    //!
    //! @note
    //! Multiple extended threads can wait for a given semaphore.
    //!
    bool wait(std::uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT) noexcept;

    //! try wait on the semaphore (non-blocking)
    //!
    //! @description
    //! This operation checks if the semaphore counter is greater than 0,
    //! in which case the counter is decremented.
    //!
    //! @returns
    //! 'true' if the semaphore has count available and 'false' NOT available.
    //!
    //! @note
    //! This function can be called from any context, including ISRs and
    //! basic threads (active objects).
    //!
    bool tryWait(void) noexcept;

    //! signal (unblock) the semaphore
    //!
    //! @description
    //! If the semaphore counter value is 0 or more, it is incremented, and
    //! this function returns to its caller. If the extended threads are
    //! waiting for the semaphore to be signaled, QXSemaphore::signal()
    //! removes the highest-priority thread waiting for the semaphore from
    //! the waiting list and makes this thread ready-to-run. The QXK
    //! scheduler is then called to determine if the awakened thread is now
    //! the highest-priority thread that is ready-to-run.
    //!
    //! @returns
    //! 'true' when the semaphore gets signaled and 'false' when the
    //! semaphore count exceeded the maximum.
    //!
    //! @note
    //! A semaphore can be signaled from many places, including from ISRs, basic
    //! threads (AOs), and extended threads.
    //!
    bool signal(void) noexcept;

private:
    QPSet m_waitSet; //!< set of extended threads waiting on this semaphore
    std::uint16_t volatile m_count;  //!< semaphore up-down counter
    std::uint16_t m_max_count; //!< maximum value of the semaphore counter
};

//============================================================================
//! Priority Ceiling Mutex the QXK preemptive kernel
//!
//! @description
//! QP::QXMutex is a blocking mutual exclusion mechanism that can also apply
//! the **priority ceiling protocol** to avoid unbounded priority inversion
//! (if initialized with a non-zero ceiling priority, see QP::QXMutex::init()).
//! In that case, QP::QXMutex requires its own uinque QP priority level, which
//! cannot be used by any thread or any other QP::QXMutex.
//! If initialzied with zero ceiling priority, QP::QXMutex does **not** use
//! the priority ceiling protocol and does not require a unique QP priority
//! (see QP::QXMutex::init()).
//! QP::QXMutex is **recursive** (reentrant), which means that it can be
//! locked mutiliple times (up to 255 levels) by the *same* thread without
//! causing deadlock.
//! QP::QXMutex is primarily intended for the @ref QP::QXThread
//! "extened (blocking) threads", but can also be used by the @ref QP::QActive
//! "basic threads" through the non-blocking QP::QXMutex::tryLock() API.
//!
//! @note
//! QP::QXMutex should be used in situations when at least one of the extended
//! threads contending for the mutex blocks while holding the mutex (between
//! the QP::QXMutex::lock() and QP::QXMutex::unlock() operations). If no
//! blocking is needed while holding the mutex, the more efficient
//! non-blocking mechanism of @ref QP::QXK::schedLock() "selective QXK
//! scheduler locking" should be used instead. @ref QP::QXK::schedLock()
//! "Selective scheduler locking" is available for both @ref QP::QActive
//! "basic threads" and @ref QP::QXThread "extended threads", so it is
//! applicable to situations where resources are shared among all
//! these threads.
//!
//! @usage
//! The following example illustrates how to instantiate and use the mutex
//! in your application.
//! @include qxk_mutex.cpp
//!
class QXMutex {
public:
    //! initialize the QXK priority-ceiling mutex QP::QXMutex
    //!
    //! @description
    //! Initialize the QXK priority ceiling mutex.
    //!
    //! @param[in]  ceiling    the ceiling-priotity of this mutex or zero.
    //!
    //! @note
    //! `ceiling == 0` means that the priority-ceiling protocol shall **not**
    //! be used by this mutex. Such mutex will **not** change (boost) the
    //! priority of the holding thread.
    //!
    //! @note
    //! `ceiling > 0` means that the priority-ceiling protocol shall be used
    //! by this mutex. Such mutex __will__ boost the priority of the holding
    //! thread to the `ceiling` level for as long as the thread holds this
    //! mutex.
    //!
    //! @attention
    //! When the priority-ceiling protocol is used (`ceiling > 0`), the
    //! `ceiling` priority must be unused by any other thread or mutex.
    //! Also, the `ceiling` priority must be higher than priority of any
    //! thread that uses this mutex.
    //!
    //! @usage
    //! @include qxk_mutex.cpp
    //!
    void init(std::uint_fast8_t const ceiling) noexcept;

    //! lock the QXK priority-ceiling mutex QP::QXMutex
    //!
    //! @description
    //! Lock the QXK priority ceiling mutex QP::QXMutex.
    //!
    //! @param[in]  nTicks    number of clock ticks (at the associated rate)
    //!                       to wait for the semaphore. The value of
    //!                       #QXTHREAD_NO_TIMEOUT indicates that no timeout
    //!                       will occur and the semaphore will wait
    //!                       indefinitely.
    //! @returns
    //! 'true' if the mutex has been acquired and 'false' if a timeout
    //! occurred.
    //!
    //! @note
    //! The mutex locks are allowed to nest, meaning that the same extended
    //! thread can lock the same mutex multiple times (< 255). However,
    //! each call to QXMutex::lock() must be balanced by the matching call to
    //! QXMutex::unlock().
    //!
    //! @usage
    //! @include qxk_mutex.cpp
    //!
    bool lock(std::uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT) noexcept;

    //! try to lock the QXK priority-ceiling mutex QP::QXMutex
    //!
    //! @description
    //! Try to lock the QXK priority ceiling mutex QP::QXMutex.
    //!
    //! @returns
    //! 'true' if the mutex was successfully locked and 'false' if the mutex
    //! was unavailable and was NOT locked.
    //!
    //! @note
    //! This function **can** be called from both basic threads (active
    //! objects) and extended threads.
    //!
    //! @note
    //! The mutex locks are allowed to nest, meaning that the same extended
    //! thread can lock the same mutex multiple times (< 255). However, each
    //! successful call to QXMutex::tryLock() must be balanced by the
    //! matching call to QXMutex::unlock().
    //!
    bool tryLock(void) noexcept;

    //! unlock the QXK priority-ceiling mutex QP::QXMutex
    //!
    //! @description
    //! Unlock the QXK priority ceiling mutex.
    //!
    //! @note
    //! This function **can** be called from both basic threads (active
    //! objects) and extended threads.
    //!
    //! @note
    //! The mutex locks are allowed to nest, meaning that the same extended
    //! thread can lock the same mutex multiple times (< 255). However, each
    //! call to QXMutex::lock() or a *successful* call to QXMutex::tryLock()
    //! must be balanced by the matching call to QXMutex::unlock().
    //!
    //! @usage
    //! @include qxk_mutex.cpp
    //!
    void unlock(void) noexcept;

private:
    QPSet m_waitSet; //!< set of extended-threads waiting on this mutex
    std::uint8_t volatile m_lockNest; //!< lock-nesting up-down counter
    std::uint8_t volatile m_holderPrio; //!< prio of the lock holder thread
    std::uint8_t m_ceiling; //!< priority ceiling of this mutex
};

} // namespace QP

#endif // QXTHREAD_HPP
