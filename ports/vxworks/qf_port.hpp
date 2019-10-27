/// @file
/// @brief QF/C++ port to VxWorks API
/// @ingroup ports
/// @cond
///***************************************************************************
/// Last updated for version 6.6.0
/// Last updated on  2019-07-30
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
/// <www.state-machine.com>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond
///
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
#define QF_MAX_ACTIVE        64

// The number of system clock tick rates
#define QF_MAX_TICK_RATE     2

// various QF object sizes configuration for this port
#define QF_EVENT_SIZ_SIZE    4
#define QF_EQUEUE_CTR_SIZE   4
#define QF_MPOOL_SIZ_SIZE    4
#define QF_MPOOL_CTR_SIZE    4
#define QF_TIMEEVT_CTR_SIZE  4

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
#include "qpset.hpp"     // VxWorks port needs priority-set
#include "qf.hpp"        // QF platform-independent public interface


//****************************************************************************
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
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        while ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt const *>(0)) { \
            UINT32 eventsReceived; \
            QF_CRIT_EXIT_(); \
            Q_ALLEGE_ID(405, eventReceive(QF_EQUEUE_EVT, EVENTS_WAIT_ANY, \
                        WAIT_FOREVER, &eventsReceived) == OK); \
            QF_CRIT_ENTRY_(); \
        }

    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        Q_ASSERT_ID(410, QF::active_[(me_)->m_prio] \
                         != static_cast<QActive *>(0)); \
        Q_ALLEGE_ID(415, eventSend((me_)->m_thread, QF_EQUEUE_EVT) == OK)

    // event pool operations
    #define QF_EPOOL_TYPE_  QMPool

    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))

    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_)     ((p_).put(e_))

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
