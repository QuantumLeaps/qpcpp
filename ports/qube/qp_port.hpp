//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005-2026 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License, version 3 or later (GPL-3.0-or-later), or,
// alternatively, under the terms of one of the closed-source Quantum Leaps
// commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
//
// NOTE:
// The GPL does NOT permit distributing this code as part of proprietary
// (closed-source) programs. For such use, please contact Quantum Leaps for
// a commercial license, which is an alternative to the GPL designed
// explicitly for closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

#include <cstdint>        // Exact-width types. C++11 Standard
#include <array>          // std::array<> template. C++11 Standard
#include "qp_config.hpp"  // QP configuration from the application

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// static assertion (C++11 Standard)
#define Q_ASSERT_STATIC(expr_)  static_assert((expr_), "QP static assert")

// QActive event queue type
#define QACTIVE_EQUEUE_TYPE     QEQueue
#define QACTIVE_OS_OBJ_TYPE     void *
#define QACTIVE_THREAD_TYPE     void const *

// Qube critical section (single-threaded, only nesting is counted)
#define QF_CRIT_STAT     std::uint_fast8_t critStat_;
#define QF_CRIT_ENTRY()  (critStat_ = QP::QF_intLock_++)
#define QF_CRIT_EXIT()   (QP::QF_intLock_ = critStat_)
#define QF_CRIT_EST()    (++QP::QF_intLock_)

// QF_LOG2 not defined -- use the internal LOG2() implementation

// include files -------------------------------------------------------------
#include "qequeue.hpp"   // Qube port uses QEQueue event-queue
#include "qmpool.hpp"    // Qube port uses QMPool memory-pool
#include "qp.hpp"        // QP platform-independent public interface

namespace QP {

extern std::uint_fast8_t QF_intLock_; // critical-section nesting

namespace Qube {

// set the current AO (the target of the events entered by the user)
void setAO(QActive * const ao) noexcept;

// application callback to generate an event from the signal
// and the optional parameters entered by the user
QEvt const * onGenEvt(QSignal const sig, char const * const params);

} // namespace Qube
} // namespace QP

//============================================================================
// interface used only inside QF implementation, but not in applications

#ifdef QP_IMPL

namespace QP {
extern QPSet QF_readySet_; // set of AOs with non-empty event queues
} // namespace QP

    // Qube scheduler locking (not used)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) (static_cast<void>(0))
    #define QF_SCHED_UNLOCK_()    (static_cast<void>(0))

    // native QEQueue operations
    #define QACTIVE_EQUEUE_WAIT_(me_) (static_cast<void>(0))
    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        (QP::QF_readySet_.insert( \
            static_cast<std::uint_fast8_t>((me_)->m_prio)))

    // QMPool operations
    #define QF_EPOOL_TYPE_ QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))
    #define QF_EPOOL_PUT_(p_, e_, qsId_) ((p_).put((e_), (qsId_)))
    #define QF_EPOOL_USE_(ePool_)   ((ePool_)->getUse())
    #define QF_EPOOL_FREE_(ePool_)  ((ePool_)->getFree())
    #define QF_EPOOL_MIN_(ePool_)   ((ePool_)->getMin())

#endif // QP_IMPL

#endif // QP_PORT_HPP_
