/// @file
/// @brief QXK/C++ preemptive kernel core functions
/// public interface.
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.4.0
/// Last updated on  2019-02-10
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

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qxk_pkg.h"      // QXK package-scope internal interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef qxk_h
    #error "Source file included in a project NOT based on the QXK kernel"
#endif // qxk_h

Q_DEFINE_THIS_MODULE("qxk")

// Public-scope objects ******************************************************
extern "C" {
    QXK_Attr QXK_attr_;   // global attributes of the QXK kernel
} // extern "C"

namespace QP {

// Local-scope objects *******************************************************
class QXKIdleThread : public QActive {
public:
    QXKIdleThread() : QActive(Q_STATE_CAST(0))
    {}
};

static QXKIdleThread l_idleThread;


//****************************************************************************
/// @description
/// Initializes QF and must be called exactly once before any other QF
/// function. Typically, QF::init() is called from main() even before
/// initializing the Board Support Package (BSP).
///
/// @note QF::init() clears the internal QF variables, so that the framework
/// can start correctly even if the startup code fails to clear the
/// uninitialized data (as is required by the C++ Standard).
///
void QF::init(void) {
    QF_maxPool_      = static_cast<uint_fast8_t>(0);
    QF_subscrList_   = static_cast<QSubscrList *>(0);
    QF_maxPubSignal_ = static_cast<enum_t>(0);

    bzero(&timeEvtHead_[0], static_cast<uint_fast16_t>(sizeof(timeEvtHead_)));
    bzero(&active_[0],      static_cast<uint_fast16_t>(sizeof(active_)));
    bzero(&QXK_attr_,       static_cast<uint_fast16_t>(sizeof(QXK_attr_)));
    bzero(&l_idleThread,    static_cast<uint_fast16_t>(sizeof(l_idleThread)));

    // setup the QXK scheduler as initially locked and not running
    QXK_attr_.lockPrio = static_cast<uint8_t>(QF_MAX_ACTIVE + 1);

    // setup the QXK idle loop...
    active_[0] = &l_idleThread; // register the idle thread with QF
    QXK_attr_.idleThread = &l_idleThread; // save the idle thread ptr
    QXK_attr_.actPrio = static_cast<uint8_t>(0); // set idle thread prio

#ifdef QXK_INIT
    QXK_INIT(); // port-specific initialization of the QXK kernel
#endif
}

//****************************************************************************
/// @description
/// This function stops the QF application. After calling this function,
/// QF attempts to gracefully stop the application. This graceful shutdown
/// might take some time to complete. The typical use of this function is
/// for terminating the QF application to return back to the operating
/// system or for handling fatal errors that require shutting down
/// (and possibly re-setting) the system.
///
/// @sa QF::onCleanup()
///
void QF::stop(void) {
    onCleanup(); // application-specific cleanup callback
    // nothing else to do for the preemptive QXK kernel
}

//****************************************************************************
//! process all events posted during initialization
static void initial_events(void); // prototype
static void initial_events(void) {
    QXK_attr_.lockPrio = static_cast<uint8_t>(0); // unlock the scheduler

    // any active objects need to be scheduled before starting event loop?
    if (QXK_sched_() != static_cast<uint_fast8_t>(0)) {
        QXK_activate_(); // process all events produced so far
    }
}

//****************************************************************************
/// @description
/// QF::run() is typically called from main() after you initialize
/// the QF and start at least one active object with QActive::start().
///
/// @returns In QXK, the QF::run() function does not return.
///
int_t QF::run(void) {
    QF_INT_DISABLE();
    initial_events(); // process all events posted during initialization
    onStartup(); // application-specific startup callback
    QF_INT_ENABLE();

    // the QXK idle loop...
    for (;;) {
        QXK::onIdle(); // application-specific QXK idle callback
    }

#ifdef __GNUC__  // GNU compiler?
    return static_cast<int_t>(0);
#endif
}

//****************************************************************************
// @description
// Starts execution of the AO and registers the AO with the framework.
// Also takes the top-most initial transition in the AO's state machine.
// This initial transition is taken in the callee's thread of execution.
//
// @param[in] prio    priority at which to start the active object
// @param[in] qSto    pointer to the storage for the ring buffer of the
//                    event queue (used only with the built-in QP::QEQueue)
// @param[in] qLen    length of the event queue [events]
// @param[in] stkSto  pointer to the stack storage (used only when
//                    per-AO stack is needed)
// @param[in] stkSize stack size [bytes]
// @param[in] ie      pointer to the optional initialization event
//                    (might be NULL).
//
void QActive::start(uint_fast8_t const prio,
                     QEvt const *qSto[], uint_fast16_t const qLen,
                     void * const stkSto, uint_fast16_t const stkSize,
                     QEvt const * const ie)
{
    /// @pre AO cannot be started:
    /// - from an ISR;
    /// - the priority must be in range;
    /// - the stack storage must NOT be provided (because the QXK kernel does
    /// not need per-AO stacks).
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_())
        && (static_cast<uint_fast8_t>(0) < prio)
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
        && (stkSto == static_cast<void *>(0))
        && (stkSize == static_cast<uint_fast16_t>(0)));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_osObject = static_cast<void *>(0); // no private stack for AO
    m_prio = static_cast<uint8_t>(prio);      // set the QF prio of this AO
    m_startPrio = static_cast<uint8_t>(prio); // set start QF prio of this AO
    QF::add_(this);   // make QF aware of this AO

    this->init(ie); // take the top-most initial tran. (virtual)
    QS_FLUSH();     // flush the trace buffer to the host

    // see if this AO needs to be scheduled in case QXK is running
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    if (QXK_sched_() != static_cast<uint_fast8_t>(0)) { // activation needed?
        QXK_activate_();
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// This function locks the QXK scheduler to the specified ceiling.
///
/// @param[in]   ceiling    priority ceiling to which the QXK scheduler
///                         needs to be locked
///
/// @returns
/// The previous QXK Scheduler lock status, which is to be used to unlock
/// the scheduler by restoring its previous lock status in QXK::schedUnlock().
///
/// @note
/// QXK::schedLock() must be always followed by the corresponding
/// QXK::schedUnlock().
///
/// @sa QXK::schedUnlock()
///
/// @usage
/// The following example shows how to lock and unlock the QXK scheduler:
/// @include qxk_lock.cpp
///
QSchedStatus QXK::schedLock(uint_fast8_t const ceiling) {
    QSchedStatus stat;
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    /// @pre The QXK scheduler lock:
    /// - cannot be called from an ISR;
    Q_REQUIRE_ID(400, !QXK_ISR_CONTEXT_());

    // first store the previous lock prio if below the ceiling
    if (static_cast<uint_fast8_t>(QXK_attr_.lockPrio) < ceiling) {
        stat = (static_cast<QSchedStatus>(QXK_attr_.lockPrio) << 8);
        QXK_attr_.lockPrio = static_cast<uint8_t>(ceiling);

        QS_BEGIN_NOCRIT_(QS_SCHED_LOCK,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_(); // timestamp
            QS_2U8_(static_cast<uint8_t>(stat), /* the previous lock prio */
                    QXK_attr_.lockPrio); // new lock prio
        QS_END_NOCRIT_()

        // add the previous lock holder priority
        stat |= static_cast<QSchedStatus>(QXK_attr_.lockHolder);
        QXK_attr_.lockHolder =
            (QXK_attr_.curr != static_cast<QActive *>(0))
            ? QXK_attr_.curr->m_prio
            : static_cast<uint8_t>(0);
    }
    else {
       stat = static_cast<QSchedStatus>(0xFF);
    }
    QF_CRIT_EXIT_();

    return stat; // return the status to be saved in a stack variable
}

//****************************************************************************
///
/// @description
/// This function unlocks the QXK scheduler to the previous status.
///
/// @param[in]   stat       previous QXK Scheduler lock status returned from
///                         QXK::schedLock()
/// @note
/// QXK::schedUnlock() must always follow the corresponding
/// QXK::schedLock().
///
/// @sa QXK::schedLock()
///
/// @usage
/// The following example shows how to lock and unlock the QXK scheduler:
/// @include qxk_lock.cpp
///
void QXK::schedUnlock(QSchedStatus const stat) {
    // has the scheduler been actually locked by the last QXK_schedLock()?
    if (stat != static_cast<QSchedStatus>(0xFF)) {
        uint_fast8_t lockPrio = static_cast<uint_fast8_t>(QXK_attr_.lockPrio);
        uint_fast8_t prevPrio = static_cast<uint_fast8_t>(stat >> 8);
        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();

        /// @pre The scheduler cannot be unlocked:
        /// - from the ISR context; and
        /// - the current lock priority must be greater than the previous
        Q_REQUIRE_ID(500, (!QXK_ISR_CONTEXT_())
                          && (lockPrio > prevPrio));

        QS_BEGIN_NOCRIT_(QS_SCHED_UNLOCK,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_(); // timestamp
            QS_2U8_(static_cast<uint8_t>(lockPrio),/* prio before unlocking */
                    static_cast<uint8_t>(prevPrio));// prio after unlocking
        QS_END_NOCRIT_()

        // restore the previous lock priority and lock holder
        QXK_attr_.lockPrio   = static_cast<uint8_t>(prevPrio);
        QXK_attr_.lockHolder =
            static_cast<uint8_t>(stat & static_cast<QSchedStatus>(0xFF));

        // find the highest-prio thread ready to run
        if (QXK_sched_() != static_cast<uint_fast8_t>(0)) { // priority found?
            QXK_activate_(); // activate any unlocked basic threads
        }

        QF_CRIT_EXIT_();
    }
}

} // namespace QP


//============================================================================
extern "C" {

//****************************************************************************
/// @description
/// The QXK scheduler finds the priority of the highest-priority thread
/// that is ready to run.
///
/// @returns the 1-based priority of the the active object to run next,
/// or zero if no eligible active object is found.
///
/// @attention
/// QXK_sched_() must be always called with interrupts **disabled** and
/// returns with interrupts **disabled**.
///
uint_fast8_t QXK_sched_(void) {
    // find the highest-prio thread ready to run
    uint_fast8_t p = QXK_attr_.readySet.findMax();

    if (p <= static_cast<uint_fast8_t>(QXK_attr_.lockPrio)) { // below lock?
        p = static_cast<uint_fast8_t>(QXK_attr_.lockHolder);
        Q_ASSERT_ID(610, (p == static_cast<uint_fast8_t>(0))
                         || QXK_attr_.readySet.hasElement(p));
    }

    QP::QActive *next = QP::QF::active_[p];

    // the thread found must be registered in QF
    Q_ASSERT_ID(620, next != static_cast<QP::QActive *>(0));

    // is the current thread a basic-thread?
    if (QXK_attr_.curr == static_cast<QP::QActive *>(0)) {

        // is next a basic-thread?
        if (next->m_osObject == static_cast<void *>(0)) {
            if (p > static_cast<uint_fast8_t>(QXK_attr_.actPrio)) {
                QXK_attr_.next = next; // set the next AO to activate
            }
            else {
                QXK_attr_.next = static_cast<QP::QActive *>(0);
                p = static_cast<uint_fast8_t>(0); // no activation needed
            }
        }
        else {  // this is an extened-thread

            QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT,
                             QP::QS::priv_.locFilter[QP::QS::AO_OBJ],
                             next)
                QS_TIME_();         // timestamp
                QS_2U8_(static_cast<uint8_t>(p), /* prio of the next AO */
                        QXK_attr_.actPrio); // prio of the curr AO
            QS_END_NOCRIT_()

            QXK_attr_.next = next;
            p = static_cast<uint_fast8_t>(0); // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
    }
    else { // currently executing an extended-thread

        // is the next thread different from the current?
        if (next != QXK_attr_.curr) {

            QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT,
                             QP::QS::priv_.locFilter[QP::QS::AO_OBJ],
                             next)
                QS_TIME_(); // timestamp
                QS_2U8_(static_cast<uint8_t>(p), /* next prio */
                        QXK_attr_.curr->m_prio);
            QS_END_NOCRIT_()

            QXK_attr_.next = next;
            p = static_cast<uint_fast8_t>(0); // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
        else { // next is the same as current
            // no need to context-switch
            QXK_attr_.next = static_cast<QP::QActive *>(0);
            p = static_cast<uint_fast8_t>(0); // no activation needed
        }
    }
    return p;
}

//****************************************************************************
/// @attention
/// QXK_activate_() must be always called with interrupts **disabled** and
/// returns with interrupts **disabled**.
///
/// @note
/// The activate function might enable interrupts internally, but it always
/// returns with interrupts **disabled**.
///
void QXK_activate_(void) {
    uint_fast8_t pin = static_cast<uint_fast8_t>(QXK_attr_.actPrio);
    QP::QActive *a = QXK_attr_.next; // the next AO (basic-thread) to execute

    // QXK Context switch callback defined or QS tracing enabled?
#if (defined QXK_ON_CONTEXT_SW) || (defined Q_SPY)
    uint_fast8_t pprev = pin;
#endif // QXK_ON_CONTEXT_SW || Q_SPY

    // QXK_attr_.next must be valid
    Q_REQUIRE_ID(700, a != static_cast<QP::QActive *>(0));

    uint_fast8_t p = static_cast<uint_fast8_t>(a->m_prio); // the next AO

    // loop until no more ready-to-run AOs of higher prio than the initial
    do  {
        a = QP::QF::active_[p]; // obtain the pointer to the AO

        QXK_attr_.actPrio = static_cast<uint8_t>(p); // new active prio
        QXK_attr_.next = static_cast<QP::QActive *>(0); // clear the next AO

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT,
                         QP::QS::priv_.locFilter[QP::QS::AO_OBJ], a)
            QS_TIME_();         // timestamp
            QS_2U8_(static_cast<uint8_t>(p), /* next prio */
                    static_cast<uint8_t>(pprev)); // prev prio
        QS_END_NOCRIT_()

#if (defined QXK_ON_CONTEXT_SW) || (defined Q_SPY)
        if (p != pprev) {  // changing threads?

#ifdef QXK_ON_CONTEXT_SW
            // context-switch callback
            QXK_onContextSw(((pprev != static_cast<uint_fast8_t>(0))
                             ? QP::QF::active_[pprev]
                             : static_cast<QP::QActive *>(0)),
                             a);
#endif // QXK_ON_CONTEXT_SW

             pprev = p; // update previous priority
         }
#endif // QXK_ON_CONTEXT_SW || Q_SPY

        QF_INT_ENABLE(); // unconditionally enable interrupts

        // perform the run-to-completion (RTC) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and QActive_get_() asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QP::QEvt const *e = a->get_();
        a->dispatch(e);
        QP::QF::gc(e);

        QF_INT_DISABLE(); // unconditionally disable interrupts

        if (a->m_eQueue.isEmpty()) { // empty queue?
            QXK_attr_.readySet.remove(p);
        }

        // find new highest-prio AO ready to run...
        // NOTE: this part must match the QXK_sched_(),
        // current is a basic-thread path.
        p = QXK_attr_.readySet.findMax();

        if (p <= static_cast<uint_fast8_t>(QXK_attr_.lockPrio)) {//below lock?
            p = static_cast<uint_fast8_t>(QXK_attr_.lockHolder);
            Q_ASSERT_ID(710, (p == static_cast<uint_fast8_t>(0))
                             || QXK_attr_.readySet.hasElement(p));
        }
        a = QP::QF::active_[p];

        // the AO must be registered in QF
        Q_ASSERT_ID(720, a != static_cast<QP::QActive *>(0));

        // is the next a basic thread?
        if (a->m_osObject == static_cast<void *>(0)) {
            if (p > pin) {
                QXK_attr_.next = a;
            }
            else {
                QXK_attr_.next = static_cast<QP::QActive *>(0);
                p = static_cast<uint_fast8_t>(0); // no activation needed
            }
        }
        else {  // next is the-extened thread

            QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT,
                             QP::QS::priv_.locFilter[QP::QS::AO_OBJ], a)
                QS_TIME_(); // timestamp
                QS_2U8_(static_cast<uint8_t>(p), /* next prio */
                        QXK_attr_.actPrio);      // curr prio
            QS_END_NOCRIT_()

            QXK_attr_.next = a;
            p = static_cast<uint_fast8_t>(0); // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
    } while (p != static_cast<uint_fast8_t>(0)); // while activation needed

    QXK_attr_.actPrio = static_cast<uint8_t>(pin); // restore base prio

#if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)
    if (pin != static_cast<uint_fast8_t>(0)) { // resuming an active object?
        a = QP::QF::active_[pin]; // the pointer to the preempted AO

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_RESUME,
                         QP::QS::priv_.locFilter[QP::QS::AO_OBJ], a)
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(p),      /* resumed prio */
                    static_cast<uint8_t>(pprev)); // previous prio
        QS_END_NOCRIT_()
    }
    else {  // resuming priority==0 --> idle
        a = static_cast<QP::QActive *>(0);

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_IDLE,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_(); // timestamp
            QS_U8_(static_cast<uint8_t>(pprev)); // previous prio
        QS_END_NOCRIT_()
    }

#ifdef QXK_ON_CONTEXT_SW
    // context-switch callback
    QXK_onContextSw(QP::QF::active_[pprev], a);
#endif // QXK_ON_CONTEXT_SW

#endif // QK_ON_CONTEXT_SW || Q_SPY
}

//****************************************************************************
QP::QActive *QXK_current(void) {
    QP::QActive *curr;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();

    /// @pre the QXK kernel must be running
    Q_REQUIRE_ID(800,
        QXK_attr_.lockPrio <= static_cast<uint8_t>(QF_MAX_ACTIVE));

    curr = QXK_attr_.curr;
    if (curr == static_cast<QP::QActive *>(0)) { // basic thread?
        curr = QP::QF::active_[QXK_attr_.actPrio];
    }
    QF_CRIT_EXIT_();

    //! @post the current thread must be valid
    Q_ENSURE_ID(890, curr != static_cast<QP::QActive *>(0));

    return curr;
}

} // extern "C"

