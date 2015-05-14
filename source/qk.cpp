/// @file
/// @brief QK preemptive kernel core functions
/// @ingroup qk
/// @cond
///***************************************************************************
/// Product: QK/C++
/// Last updated for version 5.4.0
/// Last updated on  2015-04-30
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QF/QK implementation
#include "qf_port.h"      // QF port
#include "qk_pkg.h"       // QK package-scope internal interface
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"      // QP assertions

// Public-scope objects ******************************************************
extern "C" {

#if (QF_MAX_ACTIVE <= 8)
    QP::QPSet8  QK_readySet_;  // ready set of AOs
#else
    QP::QPSet64 QK_readySet_;  // ready set of AOs
#endif

uint_fast8_t volatile QK_currPrio_;
uint_fast8_t volatile QK_intNest_;

} // extern "C"

namespace QP {

Q_DEFINE_THIS_MODULE("qk")

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
    extern uint_fast8_t QF_maxPool_;

    QK_intNest_  = static_cast<uint_fast8_t>(0); // no nesting level
    // scheduler locked
    QK_currPrio_ = static_cast<uint_fast8_t>(QF_MAX_ACTIVE + 1);
#ifndef QK_NO_MUTEX
    QK_ceilingPrio_ = static_cast<uint_fast8_t>(0);
#endif
    QF_maxPool_  = static_cast<uint_fast8_t>(0);
    bzero(&QK_readySet_,  static_cast<uint_fast16_t>(sizeof(QK_readySet_)));
    bzero(&QF::timeEvtHead_[0],
          static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));

    QK_init(); // QK initialization ("C" linkage, might be assembly)
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
static void initialize(void) {
    // set priority for the QK idle loop
    QK_currPrio_ = static_cast<uint_fast8_t>(0);
    uint_fast8_t p = QK_schedPrio_();

    // any active objects need to be scheduled before starting event loop?
    if (p != static_cast<uint_fast8_t>(0)) {
        QK_sched_(p); // process all events produced so far
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
    initialize();  // process all events posted during initialization
    onStartup();   // application-specific startup callback
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
void QMActive::start(uint_fast8_t const prio,
                     QEvt const *qSto[], uint_fast16_t const qLen,
                     void * const stkSto, uint_fast16_t const stkSize,
                     QEvt const * const ie)
{
    Q_REQUIRE_ID(500, (static_cast<uint_fast8_t>(0) < prio)
                      && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = prio;             // set the QF priority of this AO
    QF::add_(this);            // make QF aware of this AO

#if defined(QK_TLS)
    // in the QK port the parameter stkSize is used as the thread flags
    m_osObject = static_cast<uint8_t>(stkSize); // m_osObject contains flags

    // in the QK port the parameter stkSto is used as the thread-local-storage
    m_thread = stkSto; // contains the pointer to the thread-local-storage
#else
    Q_ASSERT((stkSto == static_cast<void *>(0))
             && (stkSize == static_cast<uint_fast16_t>(0)));
#endif // defined(QK_TLS)

    this->init(ie); // execute initial transition (virtual call)
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
void QMActive::stop(void) {
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
/// @attention QK_schedPrio_() must be always called with interrupts
/// __disabled__  and returns with interrupts __disabled__.
///
uint_fast8_t QK_schedPrio_(void) {
    uint_fast8_t p = QK_readySet_.findMax();

    // is the priority below the current preemption threshold?
    if (p <= QK_currPrio_) {
        p = static_cast<uint_fast8_t>(0);
    }
#ifndef QK_NO_MUTEX
    // is the priority below the mutex ceiling??
    else if (p <= QK_ceilingPrio_) {
        p = static_cast<uint_fast8_t>(0);
    }
    else {
        // empty
    }
#endif // QK_NO_MUTEX

    return p;
}

//****************************************************************************
/// @param[in] p  priority of the next AO to schedule
///
/// @attention
/// QK_sched_() must be always called with interrupts **disabled**  and
/// returns with interrupts **disabled**.
///
/// @note
/// The scheduler might enable interrupts internally, but always
/// returns with interrupts **disabled**.
///
void QK_sched_(uint_fast8_t p) {

    uint_fast8_t pin = QK_currPrio_; // save the initial priority
    QP::QMActive *a;
#ifdef QK_TLS // thread-local storage used?
    uint_fast8_t pprev = pin;
#endif
    do {
        a = QP::QF::active_[p]; // obtain the pointer to the AO
        QK_currPrio_ = p; // this becomes the current task priority

#ifdef QK_TLS // thread-local storage used?
        // are we changing threads?
        if (p != pprev) {
            QK_TLS(a); // switch new thread-local storage
            pprev = p;
        }
#endif // QK_TLS

        QS_BEGIN_NOCRIT_(QP::QS_QK_SCHEDULE, QP::QS::priv_.aoObjFilter, a)
            QS_TIME_();  // timestamp
            QS_U8_(p);   // the priority of the active object
            QS_U8_(pin); // the preempted priority
        QS_END_NOCRIT_()

        QF_INT_ENABLE();    // unconditionally enable interrupts

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
        p = QK_readySet_.findMax(); // new highest-prio AO ready to run

        // below the current preemption threshold?
        if (p <= pin) {
            p = static_cast<uint_fast8_t>(0);
        }
#ifndef QK_NO_MUTEX
        // below the mutex ceiling?
        else if (p <= QK_ceilingPrio_) {
            p = static_cast<uint_fast8_t>(0);
        }
        else {
            // empty
        }
#endif // QK_NO_MUTEX

    } while (p != static_cast<uint_fast8_t>(0));

    QK_currPrio_ = pin; // restore the initial priority

#ifdef QK_TLS // thread-local storage used?
    // no extended context for idle loop
    if (pin != static_cast<uint_fast8_t>(0)) {
        a = QP::QF::active_[pin]; // the pointer to the preempted AO
        QK_TLS(a); // restore the original TLS
    }
#endif // QK_TLS
}

} // extern "C"
