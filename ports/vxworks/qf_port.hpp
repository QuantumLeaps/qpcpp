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
//! @date Last updated on: 2022-06-30
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QF/C++ port to VxWorks API

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// VxWorks event queue and thread types
// queue:  QP queue
// os-obj: used for VxWorks task options
// thred:  used for VxWorks task ID
//
#define QF_EQUEUE_TYPE       QEQueue
#define QF_OS_OBJECT_TYPE    int
#define QF_THREAD_TYPE       int

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE        64U

// The number of system clock tick rates
#define QF_MAX_TICK_RATE     2U

// various QF object sizes configuration for this port
#define QF_EVENT_SIZ_SIZE    4U
#define QF_EQUEUE_CTR_SIZE   4U
#define QF_MPOOL_SIZ_SIZE    4U
#define QF_MPOOL_CTR_SIZE    4U
#define QF_TIMEEVT_CTR_SIZE  4U

// QF priority offset within VxWorks priority numbering scheme, see NOTE1
#define QF_VX_PRIO_OFFSET    108

// QF critical section entry/exit for VxWorks
#define QF_CRIT_STAT_TYPE    int
#define QF_CRIT_ENTRY(stat_) ((stat_) = intLock())
#define QF_CRIT_EXIT(stat_)  (intUnlock(stat_))

// QF_LOG2 not defined -- use the internal LOG2() implementation

#include "intLib.h"    // for prototypes of intLock()/intUnlock()

#include "qep_port.hpp"  // QEP port
#include "qequeue.hpp"   // VxWorks port needs event-queue
#include "qmpool.hpp"    // VxWorks port needs memory-pool
#include "qf.hpp"        // QF platform-independent public interface


//============================================================================
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    #define QF_EQUEUE_EVT   (0x00800000)

    // scheduler locking
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) if (intContext() == FALSE) { \
        Q_ALLEGE_ID(110, OK == taskLock()); \
    } else ((void)0)
    #define QF_SCHED_UNLOCK_() if (intContext() == FALSE) { \
        Q_ALLEGE_ID(115, OK == taskUnlock()); \
    } else ((void)0)

    // event queue customization
    #define QACTIVE_EQUEUE_WAIT_(me_)                                     \
        while ((me_)->m_eQueue.m_frontEvt == nullptr) {                   \
            UINT32 eventsReceived;                                        \
            QF_CRIT_X_();                                              \
            Q_ALLEGE_ID(405, eventReceive(QF_EQUEUE_EVT, EVENTS_WAIT_ANY, \
                        WAIT_FOREVER, &eventsReceived) == OK);            \
            QF_CRIT_E_();                                             \
        }

    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        Q_ASSERT_ID(410, QActive::registry_[(me_)->m_prio] != nullptr); \
        Q_ALLEGE_ID(415, eventSend((me_)->m_thread, QF_EQUEUE_EVT) == OK)

    // event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_)  ((p_).put((e_), (qs_id_)))

    #include "taskLib.h"      // for taskXXX() functions
    #include "eventLib.h"     // for eventXXX() functions

#endif  // QP_IMPL

// NOTES: ====================================================================
//
// NOTE1:
// QF_VX_PRIO_OFFSET specifies the number of highest-urgency VxWorks
// priorities not available to QP active objects. These highest-urgency
// priorities might be used by VxWorks tasks that run "above" QP active
// objects.
//
// Because the VxWorks priority numbering is "upside down" compared
// to the QP priority numbering, the VxWorks priority for an active object
// task is calculated as follows:
//     vx_prio = QF_VX_PRIO_OFFSET + QF_MAX_ACTIVE - qp_prio
//

#endif // QF_PORT_HPP
