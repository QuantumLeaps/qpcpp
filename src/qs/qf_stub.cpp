//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005-2026 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: LicenseRef-QL-commercial
//
// This software is licensed under the terms of the Quantum Leaps commercial
// license agreement. This header is a notice of that license, not a
// substitute for it; the signed agreement governs in case of conflict.
// Use of this software requires a valid Quantum Leaps commercial or
// evaluation license. Contact Quantum Leaps for licensing options.
//
// RESTRICTIONS
// Unless otherwise agreed in writing with Quantum Leaps, the licensee
// may NOT:
// (a) redistribute, encumber, sell, rent, lease, sublicense, or otherwise
//     transfer rights in this software to any third party,
// (b) remove, obscure, or alter any trademark, copyright, or other
//     proprietary notice, legend, symbol, or label present in this
//     software,
// (c) create derivative works, or modify or repackage this software, for
//     the purpose of concealing its origin or evading the licensing
//     obligations that apply to it,
// (d) use this software, or any portion of it, as input to train,
//     fine-tune, evaluate, or otherwise develop any artificial
//     intelligence or machine learning model, or use such a model to
//     replicate, emulate, or compete with the functionality of this
//     software, whether for internal use or in any product or service
//     offered to a third party.
//
// This software is provided "AS IS," without warranty of any kind, express
// or implied, except as expressly stated in the governing license agreement.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
// only build when Q_UTEST is defined
#ifdef Q_UTEST

#define QP_IMPL         // this is QP implementation
#include "qp_port.hpp"  // QP port
#include "qp_pkg.hpp"   // QP package-scope interface
#include "qsafe.h"      // QP Functional Safety (FuSa) Subsystem
#include "qs_port.hpp"  // QS port
#include "qs_pkg.hpp"   // QS package-scope internal interface

//============================================================================
//! @cond INTERNAL

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_stub")
} // unnamed namespace

// QP-stub for QUTest
// NOTE: The QP-stub is needed for unit testing QP Applications,
// but might NOT be needed for testing QP itself.

//----------------------------------------------------------------------------
namespace QP {
namespace QF {

//............................................................................
void init() {
}

//............................................................................
void stop() {
    QS::onReset();
}
//............................................................................
// NOTE: The interrupt disabling/enabling and the critical section are
// emulated with one lock counter (QS::tstPriv_.intLock), which plays the role
// of PRIMASK/BASEPRI in the ARM Cortex-M ports. The checks and the error
// identifiers are the same as in the ARM Cortex-M ports:
// 100 interrupts disabled while already disabled,
// 101 interrupts enabled while not disabled,
// 110 critical section entered while already inside,
// 111 critical section exited while not inside.
void intDisable() {
#ifndef Q_UNSAFE
    if (QS::tstPriv_.intLock != 0U) {
        Q_onError(&Q_this_module_[0], 100);
    }
#endif
    ++QS::tstPriv_.intLock;
}
//............................................................................
void intEnable() {
#ifndef Q_UNSAFE
    if (QS::tstPriv_.intLock == 0U) {
        Q_onError(&Q_this_module_[0], 101);
    }
#endif
    --QS::tstPriv_.intLock;
}
//............................................................................
QCritStatus critEntry() {
#ifndef Q_UNSAFE
    if (QS::tstPriv_.intLock != 0U) {
        Q_onError(&Q_this_module_[0], 110);
    }
#endif
    QCritStatus const critStat = QS::tstPriv_.intLock;
    ++QS::tstPriv_.intLock;
    return critStat;
}
//............................................................................
void critExit(QCritStatus critStat) {
#ifndef Q_UNSAFE
    if (QS::tstPriv_.intLock == 0U) {
        Q_onError(&Q_this_module_[0], 111);
    }
#endif
    // restore the saved status
    QS::tstPriv_.intLock = static_cast<std::uint8_t>(critStat);
}
//............................................................................
QSchedStatus schedLock(uint_fast8_t const ceiling) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // first store the previous lock prio
    QSchedStatus stat = 0xFFU; // assume scheduler NOT locked
    if (ceiling > QS::tstPriv_.lockCeil) { // raising the lock ceiling?
        QS_BEGIN_PRE(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE();   // timestamp
            // the previous lock ceiling & new lock ceiling
            QS_2U8_PRE(QS::tstPriv_.lockCeil,
                       static_cast<std::uint8_t>(ceiling));
        QS_END_PRE()

        // previous status of the lock
        stat = static_cast<QSchedStatus>(QS::tstPriv_.lockCeil);

        // new status of the lock
        QS::tstPriv_.lockCeil = ceiling;
    }
    QF_CRIT_EXIT();

    return stat; // return the status to be saved in a stack variable
}
//............................................................................
void schedUnlock(QSchedStatus const prevCeil) {
    // has the scheduler been actually locked by the last QF::schedLock()?
    if (prevCeil != 0xFFU) {
        QF_CRIT_STAT
        QF_CRIT_ENTRY();

        Q_REQUIRE_INCRIT(200, QS::tstPriv_.lockCeil > prevCeil);

        QS_BEGIN_PRE(QS_SCHED_UNLOCK, 0U)
            QS_TIME_PRE(); // timestamp
            // current lock ceiling (old), previous lock ceiling (new)
            QS_2U8_PRE(QS::tstPriv_.lockCeil,
                       static_cast<std::uint8_t>(prevCeil));
        QS_END_PRE()

        // restore the previous lock ceiling
        QS::tstPriv_.lockCeil = prevCeil;

        QF_CRIT_EXIT();
    }
}
//............................................................................
void readySetInsert_(std::uint_fast8_t const prio) {
    // NOTE: called inside a critical section (QACTIVE_EQUEUE_SIGNAL_())
    QS::tstPriv_.readySet.insert(prio);
}
//............................................................................
int_t run() {
    // function dictionaries for the standard API
    QS_FUN_DICTIONARY(&QActive::post_);
    QS_FUN_DICTIONARY(&QActive::postx_);
    QS_FUN_DICTIONARY(&QActive::postLIFO);
    QS_FUN_DICTIONARY(&QS::processTestEvts_);

    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    QS_CRIT_EXIT();

    QS::processTestEvts_(); // process all events posted so far
    QS::onTestLoop(); // run the test loop
    QS::onCleanup();  // application cleanup
    return 0; // return no error
}

} // namespace QF

//----------------------------------------------------------------------------
void QActive::start(
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // prio. of the AO
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U);   // preemption-thre.
    register_(); // make QF aware of this AO

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO

    this->init(par, m_prio); // top-most initial tran. (virtual call)
}

//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    if (QActive_subscrList_ != nullptr) {
        unsubscribeAll(); // unsubscribe from all events
    }
    unregister_();    // un-register this active object
}
#endif // def QACTIVE_CAN_STOP

} // namespace QP

#endif // Q_UTEST
