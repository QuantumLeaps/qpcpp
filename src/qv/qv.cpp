//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QV_HPP_
    #error Source file included in a project NOT based on the QV kernel
#endif // QV_HPP_

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qv")
} // unnamed namespace

namespace QP {
namespace QV {

QV::Attr priv_;

//............................................................................
void schedDisable(std::uint_fast8_t const ceiling) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    if (ceiling > priv_.schedCeil) { // raising the scheduler ceiling?

        QS_BEGIN_PRE(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE();   // timestamp
            // the previous sched ceiling & new sched ceiling
            QS_2U8_PRE(static_cast<std::uint8_t>(priv_.schedCeil),
                        static_cast<std::uint8_t>(ceiling));
        QS_END_PRE()

        priv_.schedCeil = ceiling;
    }
    QF_CRIT_EXIT();
}

//............................................................................
void schedEnable() {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    if (priv_.schedCeil != 0U) { // actually enabling the scheduler?

        QS_BEGIN_PRE(QS_SCHED_UNLOCK, 0U)
            QS_TIME_PRE(); // timestamp
            // current sched ceiling (old), previous sched ceiling (new)
            QS_2U8_PRE(static_cast<std::uint8_t>(priv_.schedCeil), 0U);
        QS_END_PRE()

        priv_.schedCeil = 0U;
    }
    QF_CRIT_EXIT();
}

} // namespace QV

namespace QF {

//............................................................................
void init() {
    bzero_(&QF::priv_,                 sizeof(QF::priv_));
    bzero_(&QV::priv_,                 sizeof(QV::priv_));
    bzero_(&QActive::registry_[0],     sizeof(QActive::registry_));

#ifdef QV_INIT
    QV_INIT(); // port-specific initialization of the QV kernel
#endif
}

//............................................................................
void stop() {
    onCleanup(); // cleanup callback
    // nothing else to do for the QV kernel
}

//${QV::QF-cust::run} ........................................................
int_t run() {
    QF_INT_DISABLE();
#ifdef Q_SPY
    // produce the QS_QF_RUN trace record
    QS::beginRec_(QS_REC_NUM_(QS_QF_RUN));
    QS::endRec_();
#endif // Q_SPY

#ifdef QV_START
    QV_START(); // port-specific startup of the QV kernel
#endif

#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
    std::uint_fast8_t pprev = 0U; // previous prio.

#ifdef QF_ON_CONTEXT_SW
    // officially switch to the idle cotext
    QF_onContextSw(nullptr, nullptr);
#endif // def QF_ON_CONTEXT_SW

#endif // (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)

    QF_INT_ENABLE();

    onStartup(); // app. callback: configure and enable interrupts

    QF_INT_DISABLE();
    for (;;) { // QV event loop...
        // find the maximum prio. AO ready to run
        std::uint_fast8_t const p = (QV::priv_.readySet.notEmpty()
                               ? QV::priv_.readySet.findMax()
                               : 0U);

        if (p > QV::priv_.schedCeil) { // is it above the sched ceiling?
            QActive * const a = QActive::registry_[p];

#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
            if (p != pprev) { // changing threads?

                QS_BEGIN_PRE(QS_SCHED_NEXT, p)
                    QS_TIME_PRE();        // timestamp
                    QS_2U8_PRE(static_cast<std::uint8_t>(p),
                               static_cast<std::uint8_t>(pprev));
                QS_END_PRE()

#ifdef QF_ON_CONTEXT_SW
                QF_onContextSw(((pprev != 0U)
                               ? QActive::registry_[pprev]
                               : nullptr), a);
#endif // QF_ON_CONTEXT_SW

                pprev = p; // update previous prio.
            }
#endif // (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)

            QF_INT_ENABLE();

            QEvt const * const e = a->get_();

            // dispatch event (virtual call)
            a->dispatch(e, a->getPrio());
#if (QF_MAX_EPOOL > 0U)
            gc(e);
#endif
            QF_INT_DISABLE();

            if (a->getEQueue().isEmpty()) { // empty queue?
                QV::priv_.readySet.remove(p);
            }
        }
        else { // no AO ready to run --> idle
#if (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)
            if (pprev != 0U) {
                QS_BEGIN_PRE(QS_SCHED_IDLE, pprev)
                    QS_TIME_PRE();    // timestamp
                    QS_U8_PRE(static_cast<std::uint8_t>(pprev));
                QS_END_PRE()

#ifdef QF_ON_CONTEXT_SW
                QF_onContextSw(QActive::registry_[pprev], nullptr);
#endif // QF_ON_CONTEXT_SW

                pprev = 0U; // update previous prio
            }
#endif // (defined QF_ON_CONTEXT_SW) || (defined Q_SPY)

            // QV::onIdle() must be called with interrupts DISABLED because
            // the determination of the idle condition can change at any time
            // by an interrupt posting events to a queue.
            //
            // NOTE: QV::onIdle() MUST enable interrupts internally, ideally
            // atomically with putting the CPU into a power-saving mode.
            QV::onIdle();

            QF_INT_DISABLE();
        }
    }
#ifdef __GNUC__ // GNU compiler?
    return 0;
#endif
}

} // namespace QF

//............................................................................
void QActive::start(
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);  // not needed in QV
    Q_UNUSED_PAR(stkSize); // not needed in QV

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300,
        stkSto == nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); //  QF-prio.
    m_pthre = 0U; // not used
    register_(); // make QF aware of this AO

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    QS_FLUSH(); // flush the trace buffer to the host
}

} // namespace QP
