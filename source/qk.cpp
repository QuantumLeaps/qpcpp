/// @file
/// @brief QK preemptive kernel core functions
/// @ingroup qk
/// @cond
///***************************************************************************
/// Last updated for version 5.8.1
/// Last updated on  2016-12-11
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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

    QK_attr_.actPrio  = static_cast<uint_fast8_t>(0); // prio of QK idle loop
    QK_attr_.lockPrio = static_cast<uint_fast8_t>(QF_MAX_ACTIVE); // locked

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
    QK_attr_.lockPrio = static_cast<uint_fast8_t>(0); // scheduler unlocked

    // any active objects need to be scheduled before starting event loop?
    if (QK_sched_() != static_cast<uint_fast8_t>(0)) {
        QK_activate_(); // process all events produced so far
    }
}

//****************************************************************************
// @description
// QP::QF::run() is typically called from your startup code after you
// initialize the QF and start at least one active object with
// QP::QActive::start().
//
// @returns QP::QF::run() typically does not return in embedded applications.
// However, when QP runs on top of an operating system, QP::QF::run() might
// return and in this case the return represents the error code (0 for
// success). Typically the value returned from QP::QF::run() is subsequently
// passed on as return from main().
//
// @note This function is strongly platform-dependent and is not implemented
// in the QF, but either in the QF port or in the Board Support Package (BSP)
// for the given application. All QF ports must implement QP::QF::run().
//
int_t QF::run(void) {
    QF_INT_DISABLE();
    initial_events(); // process all events posted during initialization
    onStartup();      // application-specific startup callback
    QF_INT_ENABLE();

    // the QK idle loop...
    for (;;) {
        QK::onIdle(); // invoke the QK on-idle callback
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
// @param[in] qLen    length of the event queue (in events)
// @param[in] stkSto  pointer to the stack storage (used only when
//                    per-AO stack is needed)
// @param[in] stkSize stack size (in bytes)
// @param[in] ie      pointer to the optional initialization event
//                    (might be NULL).
//
void QActive::start(uint_fast8_t const prio,
                     QEvt const *qSto[], uint_fast16_t const qLen,
                     void * const stkSto, uint_fast16_t const stkSize,
                     QEvt const * const ie)
{
    Q_REQUIRE_ID(500, (static_cast<uint_fast8_t>(0) < prio)
                      && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = prio;             // set the QF priority of this AO
    QF::add_(this);            // make QF aware of this AO

    // QK kernel does not need per-thread stack
    Q_ASSERT_ID(510, (stkSto == static_cast<void *>(0))
                     && (stkSize == static_cast<uint_fast16_t>(0)));

    this->init(ie); // take the top-most initial tran. (virtual)
    QS_FLUSH();     // flush the trace buffer to the host
}

//****************************************************************************
// @description
// The preferred way of calling this function is from within the active
// object that needs to stop. In other words, an active object should stop
// itself rather than being stopped by someone else. This policy works
// best, because only the active object itself "knows" when it has reached
// the appropriate state for the shutdown.
//
// @note
// By the time the AO calls QP::QActive::stop(), it should have unsubscribed
// from all events and no more events should be directly-posted to it.
//
void QActive::stop(void) {
    QF::remove_(this);  // remove this active object from the QF
}

} // namespace QP

//============================================================================
extern "C" {

//****************************************************************************
/// @description
/// This function finds out the priority of the highest-priority active object
/// that (1) has events to process, and (2) has priority that is above the
/// current priority, and (3) has priority that is above the mutex ceiling,
/// if mutex is configured in the port.
///
/// @returns the 1-based priority of the the active object, or zero if
/// no eligible active object is ready to run.
///
/// @attention QK_sched_() must be always called with interrupts
/// __disabled__  and returns with interrupts __disabled__.
///
uint_fast8_t QK_sched_(void) {
    // find the highest-prio AO with non-empty event queue
    uint_fast8_t p = QK_attr_.readySet.findMax();

    // is the highest-prio below the active prio?
    if (p <= QK_attr_.actPrio) {
        p = static_cast<uint_fast8_t>(0); // active object not eligible
    }
    else if (p <= QK_attr_.lockPrio) { // is it below the lock prio?
        p = static_cast<uint_fast8_t>(0); // active object not eligible
    }
    else {
        Q_ASSERT_ID(610, p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE));
        QK_attr_.nextPrio = p; // next AO to run
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
    uint_fast8_t pin = QK_attr_.actPrio; // save the active priority
    uint_fast8_t p   = QK_attr_.nextPrio; /* the next prio to run */
    QP::QActive *a;

    // QS tracing or thread-local storage?
#ifdef Q_SPY
    uint_fast8_t pprev = pin;
#endif // Q_SPY

    // QK_attr_.nextPrio must be non-zero upon entry to QK_activate_()
    Q_REQUIRE_ID(800, p != static_cast<uint_fast8_t>(0));

    QK_attr_.nextPrio = static_cast<uint_fast8_t>(0); // clear for next time

    // loop until no more ready-to-run AOs of higher prio than the initial
    do {
        a = QP::QF::active_[p]; // obtain the pointer to the AO
        QK_attr_.actPrio = p; // this becomes the active priority

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT, QP::QS::priv_.aoObjFilter, a)
            QS_TIME_();   // timestamp
            QS_2U8_(static_cast<uint8_t>(p), // prio of the scheduled AO
                    static_cast<uint8_t>(pprev)); // previous priority
        QS_END_NOCRIT_()

#ifdef Q_SPY
        if (p != pprev) { // changing priorities?
            pprev = p;    // update previous priority
        }
#endif // Q_SPY

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
        else if (p <= QK_attr_.lockPrio) { // is it below the lock prio?
            p = static_cast<uint_fast8_t>(0); // active object not eligible
        }
        else {
            Q_ASSERT_ID(710, p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE));
        }
    } while (p != static_cast<uint_fast8_t>(0));

    QK_attr_.actPrio = pin; // restore the active priority

#ifdef Q_SPY
    if (pin != static_cast<uint_fast8_t>(0)) { // resuming an active object?
        a = QP::QF::active_[pin]; // the pointer to the preempted AO

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_RESUME, QP::QS::priv_.aoObjFilter, a)
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(pin), // prio of the resumed AO
                    static_cast<uint8_t>(pprev)); // previous priority
        QS_END_NOCRIT_()
    }
    else {  // resuming priority==0 --> idle
        QS_BEGIN_NOCRIT_(QP::QS_SCHED_IDLE,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_();  // timestamp
            QS_U8_(static_cast<uint8_t>(pprev)); // previous priority
        QS_END_NOCRIT_()
    }
#endif // Q_SPY
}

} // extern "C"
