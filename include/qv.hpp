//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QV/C++ platform-independent public interface.

#ifndef QV_HPP
#define QV_HPP

#include "qequeue.hpp" // QV kernel uses the native QF event queue
#include "qmpool.hpp"  // QV kernel uses the native QF memory pool
#include "qpset.hpp"   // QV kernel uses the native QF priority set

//============================================================================
// QF configuration for QK

// QV event-queue used for AOs
#define QF_EQUEUE_TYPE  QEQueue

//============================================================================
namespace QP {

//! QV non-preemptive (cooperative) run-to-completion kernel
//!
//! @description
//! This class groups together QV services. It has only static members and
//! should not be instantiated.
//!
//! @note
//! The QV ready set, etc. belong conceptually to the QV class (as static
//! class members). However, to avoid C++ potential name-mangling problems in
//! assembly language, these elements are defined outside of the QV class and
//! use the extern "C" linkage specification.
//!
class QV {
public:
    //! QV idle callback (customized in BSPs for QV)
    //!
    //! @description
    //! QV::onIdle() must be called with interrupts DISABLED because
    //! the determination of the idle condition (no events in the
    //! queues) can change at any time by an interrupt posting events
    //! to a queue. QV::onIdle() MUST enable interrupts internally,
    //! perhaps at the same time as putting the CPU into a power-saving
    //! mode.
    //!
    //! @sa QP::QK::onIdle()
    static void onIdle(void);
};

} // namespace QP

//============================================================================
extern "C" {
    extern QP::QPSet QV_readySet_;  //!< ready set of AOs
} // extern "C"

//============================================================================
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    // QV-specific scheduler locking (not needed in QV)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) (static_cast<void>(0))
    #define QF_SCHED_UNLOCK_()    (static_cast<void>(0))

    // QV-specific native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT_ID(110, (me_)->m_eQueue.m_frontEvt != nullptr)
    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        (QV_readySet_.insert(static_cast<std::uint_fast8_t>((me_)->m_prio)))

    // QV-specific native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).m_blockSize)
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

#endif // QP_IMPL

#endif // QV_HPP
