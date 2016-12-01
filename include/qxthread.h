/// @file
/// @brief QXK/C++ extended (blocking) thread
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 5.8.0
/// Last updated on  2016-11-19
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps. All rights reserved.
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qxthread_h
#define qxthread_h

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
/// The following example illustrates how to instantiate a "naked" thread
/// in your application.
/// @include qf_qxthread.cpp
///
class QXThread : public QActive {
public:

    //! public constructor
    QXThread(QXThreadHandler const handler, uint_fast8_t const tickRate);

    //! delay (block) the current extended thread for a specified # ticks
    static bool delay(uint_fast16_t const nTicks,
                      uint_fast8_t const tickRate);

    //! cancel the delay
    bool delayCancel(void);

    //! obtain a message from the private message queue (block if no messages)
    static QEvt const *queueGet(uint_fast16_t const nTicks,
                                uint_fast8_t const tickRate);

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

private:
    void block_(void) const;
    void unblock_(void) const;
    void teArm_(enum_t const sig, uint_fast16_t const nTicks,
                uint_fast8_t const tickRate);
    bool teDisarm_(void);

    // attributes...
    QTimeEvt m_timeEvt;

    // friendships...
    friend class QXSemaphore;
};

//! no-timeout sepcification when blocking on queues or semaphores
#define QXTHREAD_NO_TIMEOUT  (static_cast<uint_fast16_t>(0))

//****************************************************************************
//! Counting Semaphore of the QXK preemptive kernel
class QXSemaphore {
public:
    //! initialize the counting semaphore
    void init(uint_fast16_t const count);

    //! signal (unblock) the semaphore
    void signal(void);

    //! wait (block) on the semaphore
    bool wait(uint_fast16_t const nTicks,
              uint_fast8_t const tickRate);

private:
    uint_fast16_t m_count;
    QPSet m_waitSet; //!< set of extended threads waiting on this semaphore
};

} // namespace QP

#endif // qxthread_h

