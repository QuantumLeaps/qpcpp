/// @file
/// @brief QK/C++ preemptive kernel core functions
/// @ingroup qk
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

#define QP_IMPL           // this is QF/QK implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"       // QF package-scope internal interface
#include "qassert.h"      // QP assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef qk_h
    #error "Source file included in a project NOT based on the QK kernel"
#endif // qk_h

// Public-scope objects ******************************************************
extern "C" {

Q_DEFINE_THIS_MODULE("qk")

QK_Attr QK_attr_; // global attributes of the QK kernel

} // extern "C"


namespace QP {

//****************************************************************************
/// @description
/// Initializes QF and must be called exactly once before any other QF
/// function. Typcially, QP::QF::init() is called from main() even before
/// initializing the Board Support Package (BSP).
///
/// @note
/// QP::QF::init() clears the internal QF variables, so that the framework
/// can start correctly even if the startup code fails to clear the
/// uninitialized data (as is required by the C Standard).
///
void QF::init(void) {
    QF_maxPool_      = static_cast<uint_fast8_t>(0);
    QF_subscrList_   = static_cast<QSubscrList *>(0);
    QF_maxPubSignal_ = static_cast<enum_t>(0);

    bzero(&QF::timeEvtHead_[0],
          static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));
    bzero(&QK_attr_,   static_cast<uint_fast16_t>(sizeof(QK_attr_)));

    QK_attr_.actPrio  = static_cast<uint8_t>(0); // prio of QK idle loop
    QK_attr_.lockPrio = static_cast<uint8_t>(QF_MAX_ACTIVE); // locked

#ifdef QK_INIT
    QK_INIT(); // port-specific initialization of the QK kernel
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
/// @sa QP::QF::onCleanup()
///
void QF::stop(void) {
    QF::onCleanup();  // cleanup callback
    // nothing else to do for the QK preemptive kernel
}

//****************************************************************************
//! process all events posted during initialization */
static void initial_events(void); // prototype
static void initial_events(void) {
    QK_attr_.lockPrio = static_cast<uint8_t>(0); // scheduler unlocked

    // any active objects need to be scheduled before starting event loop?
    if (QK_sched_() != static_cast<uint_fast8_t>(0)) {
        QK_activate_(); // activate AOs to process all events posted so far
    }
}

//****************************************************************************
/// @description
///
/// QP::QF::run() is typically called from your startup code after you
/// initialize the QF and start at least one active object with
/// QP::QActive::start().
///
/// @returns In QK, the QP::QF::run() function does not return.
///
int_t QF::run(void) {
    QF_INT_DISABLE();
    initial_events(); // process all events posted during initialization
    onStartup();      // application-specific startup callback
    QF_INT_ENABLE();

    // the QK idle loop...
    for (;;) {
        QK::onIdle(); // application-specific QK on-idle callback
    }
#ifdef __GNUC__  // GNU compiler?
    return static_cast<int_t>(0);
#endif
}

//****************************************************************************
// @description
// Starts execution of the AO and registers the AO with the framework.
//
// @param[in] prio    priority at which to start the active object
// @param[in] qSto    pointer to the storage for the ring buffer of the
//                    event queue (used only with the built-in QP::QEQueue)
// @param[in] qLen    length of the event queue [events]
// @param[in] stkSto  pointer to the stack storage (must be NULL in QK)
// @param[in] stkSize stack size [bytes]
// @param[in] ie      pointer to the optional initial event (might be NULL)
//
// @usage
// The following example shows starting an AO when a per-task stack is needed:
// @include
// qf_start.cpp
//
void QActive::start(uint_fast8_t const prio,
                    QEvt const *qSto[], uint_fast16_t const qLen,
                    void * const stkSto, uint_fast16_t const,
                    QEvt const * const ie)
{
    /// @pre AO cannot be started from an ISR, the priority must be in range
    /// and the stack storage must not be provided, because the QK kernel does
    /// not need per-AO stacks.
    Q_REQUIRE_ID(300, (!QK_ISR_CONTEXT_())
                      && (static_cast<uint_fast8_t>(0) < prio)
                      && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
                      && (stkSto == static_cast<void *>(0)));

    m_eQueue.init(qSto, qLen); // initialize the built-in queue

    m_prio = static_cast<uint8_t>(prio);  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO

    this->init(ie); // take the top-most initial tran. (virtual)
    QS_FLUSH();     // flush the trace buffer to the host

    // See if this AO needs to be scheduled in case QK is already running
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    if (QK_sched_() != static_cast<uint_fast8_t>(0)) { // activation needed?
        QK_activate_();
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
///
/// @description
/// This function locks the QK scheduler to the specified ceiling.
///
/// @param[in]   ceiling    priority ceiling to which the QK scheduler
///                         needs to be locked
///
/// @returns
/// The previous QK Scheduler lock status, which is to be used to unlock
/// the scheduler by restoring its previous lock status in
/// QP::QK::schedUnlock().
///
/// @note
/// QP::QK::schedLock() must be always followed by the corresponding
/// QP::QK::schedUnlock().
///
/// @sa QK_schedUnlock()
///
/// @usage
/// The following example shows how to lock and unlock the QK scheduler:
/// @include qk_lock.cpp
///
QSchedStatus QK::schedLock(uint_fast8_t const ceiling) {
    QSchedStatus stat;
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    /// @pre The QK scheduler lock:
    /// - cannot be called from an ISR;
    Q_REQUIRE_ID(600, !QK_ISR_CONTEXT_());

    // first store the previous lock prio if it is below the ceiling
    if (static_cast<uint_fast8_t>(QK_attr_.lockPrio) < ceiling) {
        stat = (static_cast<QSchedStatus>(QK_attr_.lockPrio) << 8);
        QK_attr_.lockPrio = static_cast<uint8_t>(ceiling);

        QS_BEGIN_NOCRIT_(QS_SCHED_LOCK,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_(); // timestamp
            QS_2U8_(static_cast<uint8_t>(stat), /* the previous lock prio */
                    QK_attr_.lockPrio); // new lock prio
        QS_END_NOCRIT_()

        // add the previous lock holder priority
        stat |= static_cast<QSchedStatus>(QK_attr_.lockHolder);

        QK_attr_.lockHolder = QK_attr_.actPrio;
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
/// This function unlocks the QK scheduler to the previous status.
///
/// @param[in]   stat       previous QK Scheduler lock status returned from
///                         QP::QK::schedLock()
/// @note
/// QP::QK::schedUnlock() must always follow the corresponding
/// QP::QK::schedLock().
///
/// @sa QP::QK::schedLock()
///
/// @usage
/// The following example shows how to lock and unlock the QK scheduler:
/// @include qk_lock.cpp
///
void QK::schedUnlock(QSchedStatus const stat) {
    // has the scheduler been actually locked by the last QK_schedLock()?
    if (stat != static_cast<QSchedStatus>(0xFF)) {
        uint_fast8_t lockPrio = static_cast<uint_fast8_t>(QK_attr_.lockPrio);
        uint_fast8_t prevPrio = static_cast<uint_fast8_t>(stat >> 8);
        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();

        /// @pre The scheduler cannot be unlocked:
        /// - from the ISR context; and
        /// - the current lock priority must be greater than the previous
        Q_REQUIRE_ID(700, (!QK_ISR_CONTEXT_())
                          && (lockPrio > prevPrio));

        QS_BEGIN_NOCRIT_(QS_SCHED_UNLOCK,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_(); // timestamp
            QS_2U8_(static_cast<uint8_t>(lockPrio),/* prio before unlocking */
                    static_cast<uint8_t>(prevPrio));// prio after unlocking
        QS_END_NOCRIT_()

        // restore the previous lock priority and lock holder
        QK_attr_.lockPrio   = static_cast<uint8_t>(prevPrio);
        QK_attr_.lockHolder =
            static_cast<uint8_t>(stat & static_cast<QSchedStatus>(0xFF));

        // find the highest-prio thread ready to run
        if (QK_sched_() != static_cast<uint_fast8_t>(0)) { // priority found?
            QK_activate_(); // activate any unlocked basic threads
        }

        QF_CRIT_EXIT_();
    }
}

} // namespace QP

//============================================================================
extern "C" {

//****************************************************************************
/// @description
/// The QK scheduler finds out the priority of the highest-priority AO
/// that (1) has events to process and (2) has priority that is above the
/// current priority.
///
/// @returns the 1-based priority of the the active object, or zero if
/// no eligible active object is ready to run.
///
/// @attention
/// QK_sched_() must be always called with interrupts **disabled** and
/// returns with interrupts **disabled**.
///
uint_fast8_t QK_sched_(void) {
    // find the highest-prio AO with non-empty event queue
    uint_fast8_t p = QK_attr_.readySet.findMax();

    // is the highest-prio below the active prio?
    if (p <= static_cast<uint_fast8_t>(QK_attr_.actPrio)) {
        p = static_cast<uint_fast8_t>(0); // active object not eligible
    }
    else if (p <= static_cast<uint_fast8_t>(QK_attr_.lockPrio)) {//below lock?
        p = static_cast<uint_fast8_t>(0); // active object not eligible
    }
    else {
        Q_ASSERT_ID(610, p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE));
        QK_attr_.nextPrio = static_cast<uint8_t>(p); // next AO to run
    }
    return p;
}

//****************************************************************************
/// @description
/// QK_activate_() activates ready-to run AOs that are above the initial
/// active priority (QK_attr_.actPrio).
///
/// @note
/// The activator might enable interrupts internally, but always returns with
/// interrupts **disabled**.
///
void QK_activate_(void) {
    uint_fast8_t pin = static_cast<uint_fast8_t>(QK_attr_.actPrio);
    uint_fast8_t p   = static_cast<uint_fast8_t>(QK_attr_.nextPrio);
    QP::QActive *a;

    // QK Context switch callback defined or QS tracing enabled?
#if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)
    uint_fast8_t pprev = pin;
#endif // QK_ON_CONTEXT_SW || Q_SPY

    // QK_attr_.nextPrio must be non-zero upon entry to QK_activate_()
    Q_REQUIRE_ID(800, p != static_cast<uint_fast8_t>(0));

    QK_attr_.nextPrio = static_cast<uint8_t>(0); // clear for next time

    // loop until no more ready-to-run AOs of higher prio than the initial
    do {
        a = QP::QF::active_[p]; // obtain the pointer to the AO
        QK_attr_.actPrio = static_cast<uint8_t>(p); // the new active prio

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT,
                         QP::QS::priv_.locFilter[QP::QS::AO_OBJ], a)
            QS_TIME_();   // timestamp
            QS_2U8_(static_cast<uint8_t>(p), // prio of the scheduled AO
                    static_cast<uint8_t>(pprev)); // previous priority
        QS_END_NOCRIT_()

#if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)
        if (p != pprev) {  // changing threads?

#ifdef QK_ON_CONTEXT_SW
            // context-switch callback
            QK_onContextSw(((pprev != static_cast<uint_fast8_t>(0))
                           ? QP::QF::active_[pprev]
                           : static_cast<QP::QActive *>(0)), a);
#endif // QK_ON_CONTEXT_SW

            pprev = p;    // update previous priority
        }
#endif // QK_ON_CONTEXT_SW || Q_SPY

        QF_INT_ENABLE();  // unconditionally enable interrupts

        // perform the run-to-completion (RTS) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and QActive_get_() asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QP::QEvt const *e = a->get_();
        a->dispatch(e);
        QP::QF::gc(e);

        // determine the next highest-priority AO ready to run...
        QF_INT_DISABLE();

        if (a->m_eQueue.isEmpty()) { // empty queue?
            QK_attr_.readySet.remove(p);
        }

        // find new highest-prio AO ready to run...
        p = QK_attr_.readySet.findMax();

        // is the new priority below the initial preemption threshold?
        if (p <= pin) {
            p = static_cast<uint_fast8_t>(0); // active object not eligible
        }
        else if (p <= static_cast<uint_fast8_t>(QK_attr_.lockPrio)) {
            p = static_cast<uint_fast8_t>(0); // active object not eligible
        }
        else {
            Q_ASSERT_ID(710, p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE));
        }
    } while (p != static_cast<uint_fast8_t>(0));

    QK_attr_.actPrio = static_cast<uint8_t>(pin); // restore the active prio

#if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)

    if (pin != static_cast<uint_fast8_t>(0)) { // resuming an active object?
        a = QP::QF::active_[pin]; // the pointer to the preempted AO

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_RESUME,
                         QP::QS::priv_.locFilter[QP::QS::AO_OBJ], a)
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(pin), /* prio of the resumed AO */
                    static_cast<uint8_t>(pprev)); // previous priority
        QS_END_NOCRIT_()
    }
    else {  // resuming priority==0 --> idle
        a = static_cast<QP::QActive *>(0); // QK idle loop

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_IDLE,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_();  // timestamp
            QS_U8_(static_cast<uint8_t>(pprev)); // previous priority
        QS_END_NOCRIT_()
    }

#ifdef QK_ON_CONTEXT_SW
    QK_onContextSw(QP::QF::active_[pprev], a); // context-switch callback
#endif /* QK_ON_CONTEXT_SW */

#endif /* QK_ON_CONTEXT_SW || Q_SPY */
}

} // extern "C"

