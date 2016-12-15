/// @file
/// @brief Cooperative QV kernel, definition of QP::QV_readySet_ and
/// implementation of kernel-specific functions.
/// @ingroup qv
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
#include "qf_pkg.h"       // QF package-scope internal interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef qv_h
    #error "Source file included in a project NOT based on the QV kernel"
#endif // qv_h

namespace QP {

Q_DEFINE_THIS_MODULE("qv")

/// @note The functions implemented in this module can have a different
/// implementation in other QF ports. The implementations included here
/// are appropriate for the cooperative QV kernel only.

// Package-scope objects *****************************************************
extern "C" {
    QPSet  QV_readySet_;  // ready set of AOs
} // extern "C"

//****************************************************************************
/// @description
/// Initializes QF and must be called exactly once before any other QF
/// function. Typcially, QP::QF::init() is called from main() even before
/// initializing the Board Support Package (BSP).
///
/// @note QP::QF::init() clears the internal QF variables, so that the
/// framework can start correctly even if the startup code fails to clear
/// the uninitialized data (as is required by the C Standard).
///
void QF::init(void) {
    QF_maxPool_      = static_cast<uint_fast8_t>(0);
    QF_subscrList_   = static_cast<QSubscrList *>(0);
    QF_maxPubSignal_ = static_cast<enum_t>(0);

    bzero(&QV_readySet_, static_cast<uint_fast16_t>(sizeof(QV_readySet_)));
    bzero(&QF::timeEvtHead_[0],
          static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));

#ifdef QV_INIT
    QV_INIT(); // port-specific initialization of the QV kernel
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
    onCleanup(); // cleanup callback
    // nothing else to do for the "vanilla" kernel
}

//****************************************************************************
/// @description
/// QP::QF::run() is typically called from your startup code after you
/// initialize the QF and start at least one active object with
/// QP::QActive::start().
///
/// @returns QP::QF::run() typically does not return in embedded applications.
/// However, when QP runs on top of an operating system, QP::QF::run() might
/// return and in this case the return represents the error code (0 for
/// success). Typically the value returned from QP::QF::run() is subsequently
/// passed on as return from main().
///
/// @note This function is strongly platform-dependent and is not implemented
/// in the QF, but either in the QF port or in the Board Support Package (BSP)
/// for the given application. All QF ports must implement QP::QF::run().
///
int_t QF::run(void) {
#ifdef Q_SPY
    uint_fast8_t pprev = static_cast<uint_fast8_t>(0); // previous priority
#endif

    onStartup(); // startup callback

    // the combined event-loop and background-loop of the QV kernel...
    QF_INT_DISABLE();
    for (;;) {
        if (QV_readySet_.notEmpty()) {
            uint_fast8_t p = QV_readySet_.findMax();
            QActive *a = active_[p];

#ifdef Q_SPY
            QS_BEGIN_NOCRIT_(QS_SCHED_NEXT, QS::priv_.aoObjFilter, a)
                QS_TIME_(); // timestamp
                QS_2U8_(static_cast<uint8_t>(p), // prio of the scheduled AO
                        static_cast<uint8_t>(pprev)); // previous priority
            QS_END_NOCRIT_()

            pprev = p; /* update previous priority */
#endif // Q_SPY

            QF_INT_ENABLE();

            // perform the run-to-completion (RTS) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The "Vanialla" kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const *e = a->get_();
            a->dispatch(e);
            gc(e);

            QF_INT_DISABLE();

            if (a->m_eQueue.isEmpty()) { // empty queue?
                QV_readySet_.remove(p);
            }
        }
        else {
#ifdef Q_SPY
            if (pprev != static_cast<uint_fast8_t>(0)) {
                QS_BEGIN_NOCRIT_(QS_SCHED_IDLE,
                    static_cast<void *>(0), static_cast<void *>(0))
                    QS_TIME_(); // timestamp
                    QS_U8_(static_cast<uint8_t>(pprev)); // previous prio
                QS_END_NOCRIT_()

                pprev = static_cast<uint_fast8_t>(0); // update previous prio
            }
#endif // Q_SPY

            // QV::onIdle() must be called with interrupts DISABLED because
            // the determination of the idle condition (no events in the
            // queues) can change at any time by an interrupt posting events
            // to a queue. QV::onIdle() MUST enable interrupts internally,
            // perhaps at the same time as putting the CPU into a power-saving
            // mode.
            QP::QV::onIdle();

            QF_INT_DISABLE();
        }
    }

#ifdef __GNUC__ // GNU compiler?
    return static_cast<int_t>(0);
#endif
}

//****************************************************************************
/// @description
/// Starts execution of the AO and registers the AO with the framework.
///
/// @param[in] prio    priority at which to start the active object
/// @param[in] qSto    pointer to the storage for the ring buffer of the
///                    event queue (used only with the built-in QP::QEQueue)
/// @param[in] qLen    length of the event queue (in events)
/// @param[in] stkSto  pointer to the stack storage (used only when
///                    per-AO stack is needed)
/// @param[in] stkSize stack size (in bytes)
/// @param[in] ie      pointer to the optional initialization event
///                    (might be NULL).
///
/// @note This function should be called via the macro START().
///
/// @usage
/// The following example shows starting an AO when a per-task stack is needed
/// @include qf_start.cpp
///
void QActive::start(uint_fast8_t const prio,
                     QEvt const *qSto[], uint_fast16_t const qLen,
                     void * const stkSto, uint_fast16_t const,
                     QEvt const * const ie)
{
    /// @pre the priority must be in range and the stack storage must not
    /// be provided, because "Vanilla" kernel does not need per-AO stacks.
    ///
    Q_REQUIRE_ID(400, (static_cast<uint_fast8_t>(0) < prio)
              && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
              && (stkSto == static_cast<void *>(0)));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = prio;  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO
    this->init(ie); // execute initial transition (virtual call)

    QS_FLUSH();     // flush the trace buffer to the host
}

//****************************************************************************
/// @description
/// The preferred way of calling this function is from within the active
/// object that needs to stop. In other words, an active object should stop
/// itself rather than being stopped by someone else. This policy works
/// best, because only the active object itself "knows" when it has reached
/// the appropriate state for the shutdown.
///
/// @note By the time the AO calls QP::QActive::stop(), it should have
/// unsubscribed from all events and no more events should be directly-posted
/// to it.
///
void QActive::stop(void) {
    QF::remove_(this);
}

} // namespace QP

