/// @file
/// @brief QXK/C++ extended (blocking) thread
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.5.0
/// Last updated on  2019-03-24
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2002-2019 Quantum Leaps. All rights reserved.
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

#ifndef qxthread_h
#define qxthread_h

//! no-timeout sepcification when blocking on queues or semaphores
#define QXTHREAD_NO_TIMEOUT  (static_cast<uint_fast16_t>(0))

namespace QP {

class QXThread; // forward declaration

//! Thread handler pointer-to-function
typedef void (*QXThreadHandler)(QXThread * const me);

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
             uint_fast8_t const tickRate = static_cast<uint_fast8_t>(0));

    //! delay (block) the current extended thread for a specified # ticks
    static bool delay(uint_fast16_t const nTicks);

    //! cancel the delay
    bool delayCancel(void);

    //! obtain a message from the private message queue (block if no messages)
    static QEvt const *queueGet(
        uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT);

    // virtual function overrides...
    //! Executes the top-most initial transition in QP::QMsm.
    virtual void init(QEvt const * const e);
    virtual void init(void) { this->init(static_cast<QEvt const *>(0)); }

    //! Dispatches an event to QMsm
    virtual void dispatch(QEvt const * const e);

    //! Starts execution of an extended thread and registers the thread
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

#ifndef Q_SPY
    //! Posts an event @p e directly to the event queue of the extended
    //! thread @p me using the First-In-First-Out (FIFO) policy.
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin);
#else
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin,
                       void const * const sender);
#endif

    //! Posts an event directly to the event queue of the active object
    //! using the Last-In-First-Out (LIFO) policy.
    virtual void postLIFO(QEvt const * const e);

    //! get the blocking object for this thread (NULL if not blocked)
    bool isBlockedOn(void const * const obj
                     = static_cast<void const *>(0)) const
    {
        return reinterpret_cast<void const *>(m_temp.obj) == obj;
    }

private:
    void block_(void) const;
    void unblock_(void) const;
    void teArm_(enum_t const sig, uint_fast16_t const nTicks);
    bool teDisarm_(void);

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
    void init(uint_fast16_t const count,
              uint_fast16_t const max_count
                  = static_cast<uint_fast16_t>(0xFFFF));

    //! wait (block) on the semaphore
    bool wait(uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT);

    //! try wait on the semaphore (non-blocking)
    bool tryWait(void);

    //! signal (unblock) the semaphore
    bool signal(void);

private:
    QPSet m_waitSet; //!< set of extended threads waiting on this semaphore
    uint16_t volatile m_count;  //!< semaphore up-down counter
    uint16_t m_max_count; //!< maximum value of the semaphore counter
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
    void init(uint_fast8_t const ceiling);

    //! lock the QXK priority-ceiling mutex QP::QXMutex
    bool lock(uint_fast16_t const nTicks = QXTHREAD_NO_TIMEOUT);

    //! try to lock the QXK priority-ceiling mutex QP::QXMutex
    bool tryLock(void);

    //! unlock the QXK priority-ceiling mutex QP::QXMutex
    void unlock(void);

private:
    QPSet m_waitSet; //!< set of extended-threads waiting on this mutex
    uint8_t volatile m_lockNest; //!< lock-nesting up-down counter
    uint8_t volatile m_holderPrio; //!< priority of the lock holder thread
    uint8_t m_ceiling; //< prioirty ceiling of this mutex
};

} // namespace QP

#endif // qxthread_h

