/// @file
/// @brief QXK/C++ extended (blocking) thread
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-18
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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

#ifndef QXTHREAD_HPP
#define QXTHREAD_HPP

namespace QP {

//! no-timeout sepcification when blocking on queues or semaphores
static constexpr std::uint_fast16_t QXTHREAD_NO_TIMEOUT = 0U;

//****************************************************************************
//! Extended (blocking) thread of the QXK preemptive kernel
/// @description
/// QP::QXThread represents the extended (blocking) thread of the QXK kernel.
/// Each blocking thread in the application must be represented by the
/// corresponding QP::QXThread instance
///
/// @note
/// Typically QP::QXThread is instantiated directly in the application code.
/// The customization of the thread occurs in the constructor, where you
/// provide the thred-handler function as the parameter.
///
/// @sa QP::QActive
///
/// @usage
/// The following example illustrates how to instantiate and use an extended
/// thread in your application.
/// @include qxk_thread.cpp
///
class QXThread : public QActive {
public:

    //! public constructor
    QXThread(QXThreadHandler const handler,
             std::uint_fast8_t const tickRate = 0U) noexcept;

    //! delay (block) the current extended thread for a specified # ticks
    static bool delay(std::uint_fast16_t const nTicks) noexcept;

    //! cancel the delay
    bool delayCancel(void) noexcept;

    //! obtain a message from the private message queue (block if no messages)
    static QEvt const *queueGet(
        std::uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT) noexcept;

    // virtual function overrides...
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
    //! with the framework.
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
    //! Posts an event @p e directly to the event queue of the extended
    //! thread @p me using the First-In-First-Out (FIFO) policy.
    bool post_(QEvt const * const e,
               std::uint_fast16_t const margin) noexcept override;
#else
    bool post_(QEvt const * const e, std::uint_fast16_t const margin,
               void const * const sender) noexcept override;
#endif

    //! Posts an event directly to the event queue of the active object
    //! using the Last-In-First-Out (LIFO) policy.
    void postLIFO(QEvt const * const e) noexcept override;

private:
    void block_(void) const noexcept;
    void unblock_(void) const noexcept;
    void teArm_(enum_t const sig, std::uint_fast16_t const nTicks) noexcept;
    bool teDisarm_(void) noexcept;

    // attributes...
    QTimeEvt m_timeEvt; //!< time event to handle blocking timeouts

    // friendships...
    friend class QXSemaphore;
    friend class QXMutex;
};

//****************************************************************************
//! Counting Semaphore of the QXK preemptive kernel
///
/// @description
/// QP::QXSemaphore is a blocking mechanism intended primarily for signaling
/// @ref QP::QXThread "extended threads". The semaphore is initialized with
/// the maximum count (see QP::QXSemaphore::init()), which allows you to
/// create a binary semaphore (when the maximum count is 1) and
/// counting semaphore when the maximum count is > 1.
///
/// @usage
/// The following example illustrates how to instantiate and use the semaphore
/// in your application.
/// @include qxk_sema.cpp
///
class QXSemaphore {
public:
    //! initialize the counting semaphore
    void init(std::uint_fast16_t const count,
              std::uint_fast16_t const max_count = 0xFFFFU) noexcept;

    //! wait (block) on the semaphore
    bool wait(std::uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT) noexcept;

    //! try wait on the semaphore (non-blocking)
    bool tryWait(void) noexcept;

    //! signal (unblock) the semaphore
    bool signal(void) noexcept;

private:
    QPSet m_waitSet; //!< set of extended threads waiting on this semaphore
    std::uint16_t volatile m_count;  //!< semaphore up-down counter
    std::uint16_t m_max_count; //!< maximum value of the semaphore counter
};

//****************************************************************************
//! Priority Ceiling Mutex the QXK preemptive kernel
///
/// @description
/// QP::QXMutex is a blocking mutual exclusion mechanism that can also apply
/// the **priority ceiling protocol** to avoid unbounded priority inversion
/// (if initialized with a non-zero ceiling priority, see QP::QXMutex::init()).
/// In that case, QP::QXMutex requires its own uinque QP priority level, which
/// cannot be used by any thread or any other QP::QXMutex.
/// If initialzied with zero ceiling priority, QP::QXMutex does **not** use
/// the priority ceiling protocol and does not require a unique QP priority
/// (see QP::QXMutex::init()).
/// QP::QXMutex is **recursive** (reentrant), which means that it can be
/// locked mutiliple times (up to 255 levels) by the *same* thread without
/// causing deadlock.
/// QP::QXMutex is primarily intended for the @ref QP::QXThread
/// "extened (blocking) threads", but can also be used by the @ref QP::QActive
/// "basic threads" through the non-blocking QP::QXMutex::tryLock() API.
///
/// @note
/// QP::QXMutex should be used in situations when at least one of the extended
/// threads contending for the mutex blocks while holding the mutex (between
/// the QP::QXMutex::lock() and QP::QXMutex::unlock() operations). If no
/// blocking is needed while holding the mutex, the more efficient
/// non-blocking mechanism of @ref QP::QXK::schedLock() "selective QXK
/// scheduler locking" should be used instead. @ref QP::QXK::schedLock()
/// "Selective scheduler locking" is available for both @ref QP::QActive
/// "basic threads" and @ref QP::QXThread "extended threads", so it is
/// applicable to situations where resources are shared among all
/// these threads.
///
/// @usage
/// The following example illustrates how to instantiate and use the mutex
/// in your application.
/// @include qxk_mutex.cpp
///
class QXMutex {
public:
    //! initialize the QXK priority-ceiling mutex QP::QXMutex
    void init(std::uint_fast8_t const ceiling) noexcept;

    //! lock the QXK priority-ceiling mutex QP::QXMutex
    bool lock(std::uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT) noexcept;

    //! try to lock the QXK priority-ceiling mutex QP::QXMutex
    bool tryLock(void) noexcept;

    //! unlock the QXK priority-ceiling mutex QP::QXMutex
    void unlock(void) noexcept;

private:
    QPSet m_waitSet; //!< set of extended-threads waiting on this mutex
    std::uint8_t volatile m_lockNest; //!< lock-nesting up-down counter
    std::uint8_t volatile m_holderPrio; //!< prio of the lock holder thread
    std::uint8_t m_ceiling; //!< prioirty ceiling of this mutex
};

} // namespace QP

#endif // QXTHREAD_HPP

