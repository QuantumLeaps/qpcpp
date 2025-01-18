//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Version 8.0.2
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

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_defer")
} // unnamed namespace

namespace QP {

//............................................................................
bool QActive::defer(
    QEQueue * const eq,
    QEvt const * const e) const noexcept
{
    bool const status = eq->post(e, 0U, m_prio);

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_ACTIVE_DEFER, m_prio)
        QS_TIME_PRE();      // time stamp
        QS_OBJ_PRE(this);   // this active object
        QS_OBJ_PRE(eq);     // the deferred queue
        QS_SIG_PRE(e->sig); // the signal of the event
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
    QS_END_PRE()
    QS_CRIT_EXIT();

    return status;
}

//............................................................................
bool QActive::recall(QEQueue * const eq) noexcept {
    QEvt const * const e = eq->get(m_prio); // get evt from deferred queue
    QF_CRIT_STAT

    bool recalled;
    if (e != nullptr) { // event available?
        postLIFO(e); // post it to the _front_ of the AO's queue

        QF_CRIT_ENTRY();

        if (e->poolNum_ != 0U) { // is it a mutable event?

            // after posting to the AO's queue the event must be referenced
            // at least twice: once in the deferred event queue (eq->get()
            // did NOT decrement the reference counter) and once in the
            // AO's event queue.
            Q_ASSERT_INCRIT(205, e->refCtr_ >= 2U);

            // we need to decrement the reference counter once, to account
            // for removing the event from the deferred event queue.
            QEvt_refCtr_dec_(e); // decrement the reference counter
        }

        QS_BEGIN_PRE(QS_QF_ACTIVE_RECALL, m_prio)
            QS_TIME_PRE();      // time stamp
            QS_OBJ_PRE(this);   // this active object
            QS_OBJ_PRE(eq);     // the deferred queue
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_END_PRE()

        QF_CRIT_EXIT();

        recalled = true;
    }
    else {
        QS_CRIT_ENTRY();

        QS_BEGIN_PRE(QS_QF_ACTIVE_RECALL_ATTEMPT, m_prio)
            QS_TIME_PRE();      // time stamp
            QS_OBJ_PRE(this);   // this active object
            QS_OBJ_PRE(eq);     // the deferred queue
        QS_END_PRE()

        QS_CRIT_EXIT();

        recalled = false;
    }
    return recalled;
}

//............................................................................
std::uint_fast16_t QActive::flushDeferred(
    QEQueue * const eq,
    std::uint_fast16_t const num) const noexcept
{
    std::uint_fast16_t n = 0U;
    while (n < num) {
        QEvt const * const e = eq->get(m_prio);
        if (e != nullptr) {
            ++n; // count one more flushed event
#if (QF_MAX_EPOOL > 0U)
            QF::gc(e); // garbage collect
#endif
        }
        else {
            break;
        }
    }

    return n;
}

} // namespace QP
