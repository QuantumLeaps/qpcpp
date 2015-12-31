/// @file
/// @brief QXK/C++ preemptive kernel core functions
/// public interface.
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 5.6.0
/// Last updated on  2015-12-30
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

// Public-scope objects ******************************************************
extern "C" {
    QXK_Attr QXK_attr_;   // global attributes of the QXK kernel
} // extern "C"

namespace QP {

Q_DEFINE_THIS_MODULE("qxk")

// Local-scope objects *******************************************************
static void thread_ao(void *par);
static void thread_idle(void *par);

static QXThread l_idleThread(&thread_idle, static_cast<uint_fast8_t>(0));

//****************************************************************************
static void thread_ao(void *par) { // signature of QXThreadHandler
    // event-loop of an AO's thread...
    for (;;) {
        QEvt const *e = static_cast<QMActive *>(par)->get_();
        static_cast<QMActive *>(par)->dispatch(e); // dispatch to the AO's SM
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
}

//****************************************************************************
static void thread_idle(void *par) { // signature of QXThreadHandler
    (void)par;

    QF_INT_DISABLE();
    QF::onStartup(); // application-specific startup callback
    QF_INT_ENABLE();

    for (;;) {
        QXK::onIdle();
    }
}

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
    // clear the internal QF variables, so that the framework can start
    // correctly even if the startup code fails to clear the uninitialized
    // data (as is required by the C Standard).
    //
    QF_maxPool_ = static_cast<uint_fast8_t>(0);
    bzero(&timeEvtHead_[0], static_cast<uint_fast16_t>(sizeof(timeEvtHead_)));
    bzero(&active_[0],      static_cast<uint_fast16_t>(sizeof(active_)));
    bzero(&QXK_attr_,       static_cast<uint_fast16_t>(sizeof(QXK_attr_)));
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
/// @description
/// QP::QF::run() is typically called from your startup code after you
/// initialize the QF and start at least one active object or "naked" thread
/// (with QP::QMActive::start() or QP::QXThread::start(), respectively).
///
/// @returns QP::QF::run() typically does not return in embedded applications.
/// However, when QP runs on top of an operating system, QP::QF::run() might
/// return and in this case the return represents the error code (0 for
/// success). Typically the value returned from QP::QF::run() is subsequently
/// passed on as return from main().
///
int_t QF::run(void) {
    /// @pre QXK_init() must be called __before__ QP::QF::run() to initialize
    /// the QXK idle thread.
    Q_REQUIRE_ID(100, active_[0] == &l_idleThread);

    // switch to the highest-priority task
    QF_INT_DISABLE();
    QXK_attr_.curr = &l_idleThread; // mark QXK as running
    uint_fast8_t p = QXK_attr_.readySet.findMax(); // next priority to run
    QXK_attr_.next = active_[p];
    QXK_start_(); // start QXK multitasking (NOTE: enables interrupts)

    /* the QXK start should not return, but just in case... */
    Q_ERROR_ID(110);

    return (int_t)0;
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
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_()) /* don't start AO's in an ISR! */
                      && (prio <= (uint_fast8_t)QF_MAX_ACTIVE)
                      && (qSto != static_cast<QEvt const **>(0))
                      && (qLen != static_cast<uint_fast16_t>(0))
                      && (stkSto != static_cast<void *>(0))
                      && (stkSize != static_cast<uint_fast16_t>(0)));

    m_eQueue.init(qSto, qLen);   // initialize QEQueue of this AO

    // initialize the stack of the private thread
    QXK_stackInit_(this, &thread_ao, stkSto, stkSize);

    m_prio = prio;               // set the QF priority of this AO
    m_thread.m_startPrio = prio; // set the start priority of the AO
    QF::add_(this);              // make QF aware of this AO

    this->init(ie); // take the top-most initial tran. (virtual)
    QS_FLUSH();     // flush the trace buffer to the host

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    QXK_attr_.readySet.insert(m_prio);
    if (QXK_attr_.curr != static_cast<QMActive *>(0)) { // is QXK running?
        QXK_sched_();
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
void QMActive::stop(void) {
    QF_CRIT_STAT_

    /// @pre QActive_stop() must be called from the AO that wants to stop.
    Q_REQUIRE_ID(300, (!QXK_ISR_CONTEXT_()) /* don't stop AO's from an ISR! */
                      && (this == QXK_attr_.curr));

    QF::remove_(this); // remove this active object from the QF

    QF_CRIT_ENTRY_();
    QXK_attr_.readySet.remove(m_prio);
    QXK_sched_();
    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// Initializes QXK and must be called by the application exactly once
/// before QP::QF::run().
///
/// @param[in]  idleStkSto  pointer to the stack storage for the idle thread
/// @param[in]  idleStkSize idle thread stack size (in bytes)
///
/// @note QP::QXK::init() must be called once before QP::QF::run().
///
void QXK::init(void *idleStkSto, uint_fast16_t idleStkSize) {
    // initialize the stack of the idle thread
    QXK_stackInit_(&l_idleThread,
        reinterpret_cast<QXThread::QXThreadHandler>(l_idleThread.m_temp.act),
        idleStkSto, idleStkSize);

    // idle thread priority is zero
    l_idleThread.m_prio = static_cast<uint_fast8_t>(0);

    QF_INT_DISABLE();
    QF::active_[0] = &l_idleThread;
    QF_INT_ENABLE();
}

} // namespace QP


//============================================================================
extern "C" {

//****************************************************************************
/// @description
/// Peforms QXK scheduling and context switch to the highest-priority thread
/// that is ready to run.
///
/// @attention
/// QXK_sched_() must be always called with interrupts **disabled** and
/// returns with interrupts **disabled**.
///
void QXK_sched_(void) {
    // find the highest-priority thread that is ready to run
    uint_fast8_t p = QXK_attr_.readySet.findMax();
    uint_fast8_t p_curr =
        static_cast<QP::QMActive const *>(QXK_attr_.curr)->getPrio();

    // is the new priority different from the currently executing thread?
    if (p != p_curr) {

        QXK_attr_.next = QP::QF::active_[p];

        QS_BEGIN_NOCRIT_(QP::QS_QVK_SCHEDULE, QP::QS::priv_.aoObjFilter,
                         QXK_attr_.next)
            QS_TIME_();                            // timestamp
            QS_2U8_(static_cast<uint8_t>(p),       // prio of the next AO
                    static_cast<uint8_t>(p_curr)); // prio of the curent AO
        QS_END_NOCRIT_()

        QXK_CONTEXT_SWITCH_();
    }
}

//****************************************************************************
/// @description
/// Called when the thread handler function returns.
///
/// @note
/// Most thread handler functions are structured as endless loops that never
/// return. But it is also possible to structure threads as on-shot functions
/// that perform their job and return. In that case this function peforms
/// cleanup after the thread.
///
void QXK_threadRet_(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    uint_fast8_t p_curr =
        static_cast<QP::QMActive const *>(QXK_attr_.curr)->getPrio();

    // remove this AO/Thread from the QF
    QP::QF::active_[p_curr] = static_cast<QP::QMActive *>(0);
    QXK_attr_.readySet.remove(p_curr);
    QXK_sched_();
    QF_CRIT_EXIT_();
}

} // extern "C"
