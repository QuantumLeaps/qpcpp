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
#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

#include <cstdint>        // Exact-width types. C++11 Standard
#include <pthread.h>      // POSIX-thread API
#include "qp_config.hpp"  // QP configuration from the application

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// static assertion (C++11 Standard)
#define Q_ASSERT_STATIC(expr_)  static_assert((expr_), "QP static assert")

// QActive event queue and thread types for POSIX
#define QACTIVE_EQUEUE_TYPE  QEQueue
#define QACTIVE_OS_OBJ_TYPE  pthread_cond_t
#define QACTIVE_THREAD_TYPE  bool

// QF critical section for POSIX, see NOTE1
#define QF_CRIT_STAT
#define QF_CRIT_ENTRY()      QP::QF::enterCriticalSection_()
#define QF_CRIT_EXIT()       QP::QF::leaveCriticalSection_()
#define QF_CRIT_EST()        QP::QF::enterCriticalSection_()

// QF_LOG2 not defined -- use the internal LOG2() implementation

namespace QP {
namespace QF {

// internal functions for critical section management
void enterCriticalSection_();
void leaveCriticalSection_();

// set clock tick rate and priority
void setTickRate(uint32_t ticksPerSec, int tickPrio);

// clock tick callback
void onClockTick();

#ifdef QF_CONSOLE
    // abstractions for console access...
    void consoleSetup();
    void consoleCleanup();
    int consoleGetKey();
    int consoleWaitForKey();
#endif

} // namespace QF
} // namespace QP

// include files -------------------------------------------------------------
#include "qequeue.hpp"   // POSIX port needs the native event-queue
#include "qmpool.hpp"    // POSIX port needs the native memory-pool
#include "qp.hpp"        // QP platform-independent public interface

//============================================================================
// interface used only inside QF implementation, but not in applications

#ifdef QP_IMPL

    // QF scheduler locking for POSIX (not used at this point, see NOTE2)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) (static_cast<void>(0))
    #define QF_SCHED_UNLOCK_()    (static_cast<void>(0))

    // QF event queue customization for POSIX...
    #define QACTIVE_EQUEUE_WAIT_(me_) do { \
        while ((me_)->m_eQueue.m_frontEvt == nullptr) { \
            Q_ASSERT_INCRIT(400, QF::critSectNest_ == 1); \
            --QF::critSectNest_; \
            pthread_cond_wait(&(me_)->m_osObject, &QF::critSectMutex_); \
            Q_ASSERT_INCRIT(302, QF::critSectNest_ == 0); \
            ++QF::critSectNest_; \
        } \
    } while (false)

    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        pthread_cond_signal(&(me_)->m_osObject)

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

namespace QP {
namespace QF {
    extern pthread_mutex_t critSectMutex_;
    extern int_t critSectNest_;
} // namespace QF
} // namespace QP

#endif // QP_IMPL

//============================================================================
// NOTE1:
// QP, like all real-time frameworks, needs to execute certain sections of
// code exclusively, meaning that only one thread can execute the code at
// the time. Such sections of code are called "critical sections".
//
// This port uses a pair of functions QF::enterCriticalSection_() /
// QF::leaveCriticalSection_() to enter/leave the critical section,
// respectively.
//
// These functions are implemented in the qf_port.cpp module, where they
// manipulate the file-scope POSIX mutex object QF::critSectMutex_
// to protect all critical sections. Using the single mutex for all critical
// section guarantees that only one thread at a time can execute inside a
// critical section. This prevents race conditions and data corruption.
//
// Please note, however, that the POSIX mutex implementation behaves
// differently than interrupt disabling. A common POSIX mutex ensures
// that only one thread at a time can execute a critical section, but it
// does not guarantee that a context switch cannot occur within the
// critical section. In fact, such context switches probably will happen,
// but they should not cause concurrency hazards because the critical
// section eliminates all race conditionis.
//
// Unlinke simply disabling and enabling interrupts, the mutex approach is
// also subject to priority inversions. However, the p-thread mutex
// implementation, such as POSIX threads, should support the priority-
// inheritance protocol.
//
// NOTE2:
// Scheduler locking (used inside QActive_publish_()) is NOT implemented
// in this port. This means that event multicasting is NOT atomic, so thread
// preemption CAN happen during that time, especially when a low-priority
// thread publishes events to higher-priority threads. This can lead to
// (occasionally) unexpected event sequences.
//

#endif // QP_PORT_HPP_

