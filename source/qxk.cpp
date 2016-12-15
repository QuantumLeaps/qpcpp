/// @file
/// @brief QXK/C++ preemptive kernel core functions
/// public interface.
/// @ingroup qxk
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
/// function. Typically, QF_init() is called from main() even before
/// initializing the Board Support Package (BSP).
///
/// @note QF::init() clears the internal QF variables, so that the framework
/// can start correctly even if the startup code fails to clear the
/// uninitialized data (as is required by the C+ Standard).
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
    QXK_attr_.lockPrio = static_cast<uint_fast8_t>(QF_MAX_ACTIVE + 1);

    // setup the QXK idle loop...
    active_[0] = &l_idleThread; // register the idle thread with QF
    QXK_attr_.actPrio = static_cast<uint_fast8_t>(0); // set idle thread prio

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

    QXK_attr_.lockPrio = static_cast<uint_fast8_t>(0); // unlock the scheduler

    // any active objects need to be scheduled before starting event loop?
    if (QXK_sched_() != static_cast<uint_fast8_t>(0)) {
        QXK_activate_(); // process all events produced so far
    }
}

//****************************************************************************
/// @description
/// QP::QF::run() is typically called from your startup code after you
/// initialize the QF and start at least one basic- or extended-thread
/// (with QP::QActive::start() or QP::QXThread::start(), respectively).
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
// This initial transition is taken in the callee's thread of execution.//
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
    Q_REQUIRE_ID(500, (!QXK_ISR_CONTEXT_()) /* don't start AO's in an ISR! */
                      && (prio <= (uint_fast8_t)QF_MAX_ACTIVE)
                      && (qSto != static_cast<QEvt const **>(0))
                      && (qLen != static_cast<uint_fast16_t>(0))
                      && (stkSto == static_cast<void *>(0))
                      && (stkSize == static_cast<uint_fast16_t>(0)));

    m_eQueue.init(qSto, qLen);   // initialize QEQueue of this AO

    m_thread = static_cast<void *>(0); // no private stack for AO
    m_prio = prio;    // set the QF priority of this AO

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    QF::add_(this);   // make QF aware of this AO
    QF_CRIT_EXIT_();

    this->init(ie); // take the top-most initial tran. (virtual)
    QS_FLUSH();     // flush the trace buffer to the host

    // see if this AO needs to be scheduled in case QXK is running
    QF_CRIT_ENTRY_();
    if (QXK_sched_() != static_cast<uint_fast8_t>(0)) { // activation needed?
        QXK_activate_();
    }
    QF_CRIT_EXIT_();
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
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    /// @pre QActive_stop() must be called from the AO that wants to stop.
    Q_REQUIRE_ID(300, (!QXK_ISR_CONTEXT_()) /* don't stop AO's from an ISR! */
                      && (this == QXK_attr_.curr));

    QF::remove_(this); // remove this active object from the QF


    QXK_attr_.readySet.remove(m_prio);
    if (QXK_sched_() != static_cast<uint_fast8_t>(0)) {
        QXK_activate_();
    }
    QF_CRIT_EXIT_();
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

    if (p <= QXK_attr_.lockPrio) { // is it below the lock prio?
        p = QXK_attr_.lockHolder; // prio of the thread holding the lock
    }

    QP::QActive *next = QP::QF::active_[p];

    // the thread found must be registered in QF
    Q_ASSERT_ID(610, next != static_cast<QP::QActive *>(0));

    // is the current thread a basic-thread?
    if (QXK_attr_.curr == static_cast<void *>(0)) {

        // is next a basic-thread?
        if (next->m_thread == static_cast<void *>(0)) {
            if (p <= QXK_attr_.actPrio) {
                QXK_attr_.next = static_cast<void *>(0);
                p = static_cast<uint_fast8_t>(0); // no activation needed
            }
            else {
                QXK_attr_.next = next;
            }
        }
        else {  // this is an extened-thread

            QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT, QP::QS::priv_.aoObjFilter,
                             QXK_attr_.next)
                QS_TIME_();         // timestamp
                QS_2U8_(static_cast<uint8_t>(p), // prio of the next thread
                        static_cast<uint8_t>(    // prio of the curent thread
                             QXK_attr_.actPrio));
            QS_END_NOCRIT_()

            QXK_attr_.next = next;
            p = static_cast<uint_fast8_t>(0); // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
    }
    else { // currently executing an extended-thread

        // is the new prio different from the current prio?
        if (p != static_cast<QP::QActive volatile*>(QXK_attr_.curr)->m_prio) {
            QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT, QP::QS::priv_.aoObjFilter,
                             QXK_attr_.next)
                QS_TIME_();         // timestamp
                QS_2U8_(static_cast<uint8_t>(p), // prio of the next thread
                        static_cast<uint8_t>(    // prio of the curent thread
                        static_cast<QP::QActive *>(QXK_attr_.curr)->m_prio));
            QS_END_NOCRIT_()

            QXK_attr_.next = next;
            p = static_cast<uint_fast8_t>(0); // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
        else {
            QXK_attr_.next = static_cast<void *>(0);
            p = static_cast<uint_fast8_t>(0); // no activation needed
        }
    }
    return p;
}

//****************************************************************************
/// @attention
/// QXK_activate_() must be always called with interrupts **disabled**  and
/// returns with interrupts **disabled**.
///
/// @note
/// The activate function might enable interrupts internally, but it always
/// returns with interrupts **disabled**.
///
void QXK_activate_(void) {
    uint_fast8_t p =
        static_cast<QP::QActive volatile *>(QXK_attr_.next)->m_prio;
    uint_fast8_t pin = QXK_attr_.actPrio; // save the initial active prio
    QP::QActive *a;

    // QS tracing or thread-local storage?
#ifdef Q_SPY
    uint_fast8_t pprev = pin;
#endif // Q_SPY

    // loop until no more ready-to-run AOs of higher prio than the initial
    do  {
        a = QP::QF::active_[p]; // obtain the pointer to the AO

        QXK_attr_.actPrio = p; // this becomes the active prio
        QXK_attr_.next = static_cast<void *>(0); // clear the next AO

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT, QP::QS::priv_.aoObjFilter, a)
            QS_TIME_();         // timestamp
            QS_2U8_(static_cast<uint8_t>(p), // prio of the next thread
                    static_cast<uint8_t>(pprev)); // prio of the prev thread
        QS_END_NOCRIT_()

#ifdef Q_SPY
        if (p != pprev) {  // changing priorities?
            pprev = p;     // update previous priority
        }
#endif // Q_SPY

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
        p = QXK_attr_.readySet.findMax();

        if (p <= QXK_attr_.lockPrio) { // is it below the lock prio?
            p = QXK_attr_.lockHolder; // prio of the thread holding the lock
        }
        a = QP::QF::active_[p];

        // the AO must be registered in QF
        Q_ASSERT_ID(710, a != static_cast<QP::QActive *>(0));

        // is the next an AO-thread?
        if (a->m_thread == static_cast<void *>(0)) {
            if (p <= pin) {
                QXK_attr_.next = static_cast<void *>(0);
                p = static_cast<uint_fast8_t>(0); // no activation needed
            }
            else {
                QXK_attr_.next = a;
            }
        }
        else {  // next is the-extened thread

            QS_BEGIN_NOCRIT_(QP::QS_SCHED_NEXT, QP::QS::priv_.aoObjFilter,
                             QXK_attr_.next)
                QS_TIME_(); // timestamp
                QS_2U8_(static_cast<uint8_t>(p), // prio of the next thread
                        static_cast<uint8_t>(    // prioof the curent thread
                            QXK_attr_.actPrio));
            QS_END_NOCRIT_()

            QXK_attr_.next = a;
            p = static_cast<uint_fast8_t>(0); // no activation needed
            QXK_CONTEXT_SWITCH_();
        }
    } while (p != static_cast<uint_fast8_t>(0)); // while activation needed

    QXK_attr_.actPrio = pin; // restore the active priority (!)

#ifdef Q_SPY
    if (pin != static_cast<uint_fast8_t>(0)) { // resuming an active object?
        a = QP::QF::active_[pin]; // the pointer to the preempted AO

        QS_BEGIN_NOCRIT_(QP::QS_SCHED_RESUME, QP::QS::priv_.aoObjFilter, a)
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(p),      // prio of the next thread
                    static_cast<uint8_t>(pprev)); // prio of the prev thread
        QS_END_NOCRIT_()
    }
    else {  // resuming priority==0 --> idle
        QS_BEGIN_NOCRIT_(QP::QS_SCHED_IDLE,
                         static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_(); // timestamp
            QS_U8_(static_cast<uint8_t>(pprev)); // previous prio
        QS_END_NOCRIT_()
    }
#endif // Q_SPY
}

} // extern "C"
