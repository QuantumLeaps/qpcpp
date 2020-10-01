/// @file
/// @brief Cooperative QV kernel, definition of QP::QV_readySet_ and
/// implementation of kernel-specific functions.
/// @ingroup qv
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

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope internal interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QV_HPP
    #error "Source file included in a project NOT based on the QV kernel"
#endif // QV_HPP

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
/// the uninitialized data (as is required by the C++ Standard).
///
void QF::init(void) {
    QF_maxPool_      = 0U;
    QF_subscrList_   = nullptr;
    QF_maxPubSignal_ = 0;

    bzero(&QF::timeEvtHead_[0], sizeof(QF::timeEvtHead_));
    bzero(&active_[0], sizeof(active_));
    bzero(&QV_readySet_, sizeof(QV_readySet_));

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
/// @attention
/// After calling QF::stop() the application must terminate and cannot
/// continue. In particular, QF::stop() is **not** intended to be followed
/// by a call to QF::init() to "resurrect" the application.
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
    std::uint_fast8_t pprev = 0U; // previous priority
#endif

    onStartup(); // startup callback

    // the combined event-loop and background-loop of the QV kernel...
    QF_INT_DISABLE();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_NOCRIT_PRE_(QS_QF_RUN, 0U)
    QS_END_NOCRIT_PRE_()

    for (;;) {

        // find the maximum priority AO ready to run
        if (QV_readySet_.notEmpty()) {
            std::uint_fast8_t const p = QV_readySet_.findMax();
            QActive * const a = active_[p];

#ifdef Q_SPY
            QS_BEGIN_NOCRIT_PRE_(QS_SCHED_NEXT, a->m_prio)
                QS_TIME_PRE_(); // timestamp
                QS_2U8_PRE_(p, pprev);// scheduled prio & previous prio
            QS_END_NOCRIT_PRE_()

            pprev = p; // update previous priority
#endif // Q_SPY

            QF_INT_ENABLE();

            // perform the run-to-completion (RTC) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The "Vanialla" kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const * const e = a->get_();
            a->dispatch(e, a->m_prio);
            gc(e);

            QF_INT_DISABLE();

            if (a->m_eQueue.isEmpty()) { // empty queue?
                QV_readySet_.rmove(p);
            }
        }
        else { // no AO ready to run --> idle
#ifdef Q_SPY
            if (pprev != 0U) {
                QS_BEGIN_NOCRIT_PRE_(QS_SCHED_IDLE, 0U)
                    QS_TIME_PRE_();    // timestamp
                    QS_U8_PRE_(pprev); // previous prio
                QS_END_NOCRIT_PRE_()

                pprev = 0U; // update previous prio
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
    return 0;
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
/// @param[in] stkSto  pointer to the stack storage (must be nullptr in QV)
/// @param[in] stkSize stack size [bytes]
/// @param[in] par     pointer to an extra parameter (might be nullptr)
///
/// @usage
/// The following example shows starting an AO when a per-task stack is needed
/// @include qf_start.cpp
///
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    static_cast<void>(stkSize); // unused paramteter in the QV port

    /// @pre the priority must be in range and the stack storage must not
    /// be provided, because the QV kernel does not need per-AO stacks.
    ///
    Q_REQUIRE_ID(500,
        (0U < prio) && (prio <= QF_MAX_ACTIVE)
        && (stkSto == nullptr));

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO
    m_prio = static_cast<std::uint8_t>(prio);  // set the QF prio of this AO

    QF::add_(this); // make QF aware of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    QS_FLUSH(); // flush the trace buffer to the host
}

} // namespace QP

