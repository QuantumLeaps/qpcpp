/// @file
/// @brief QF/C++ port to POSIX API with cooperative QV scheduler (posix-qv)
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-21
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

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// event queue and thread types
#define QF_EQUEUE_TYPE       QEQueue
// QF_OS_OBJECT_TYPE  not used
// QF_THREAD_TYPE     not used

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE        64U

// The number of system clock tick rates
#define QF_MAX_TICK_RATE     2U

// Activate the QF QActive::stop() API
#define QF_ACTIVE_STOP       1

// various QF object sizes configuration for this port
#define QF_EVENT_SIZ_SIZE    4U
#define QF_EQUEUE_CTR_SIZE   4U
#define QF_MPOOL_SIZ_SIZE    4U
#define QF_MPOOL_CTR_SIZE    4U
#define QF_TIMEEVT_CTR_SIZE  4U

// QF critical section entry/exit for POSIX, see NOTE1
// QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy) QP::QF_enterCriticalSection_()
#define QF_CRIT_EXIT(dummy)  QP::QF_leaveCriticalSection_()

// QF_LOG2 not defined -- use the internal LOG2() implementation

#include "qep_port.hpp"  // QEP port
#include "qequeue.hpp"   // POSIX-QV needs event-queue
#include "qmpool.hpp"    // POSIX-QV needs memory-pool
#include "qpset.hpp"     // POSIX-QV needs priority-set
#include "qf.hpp"        // QF platform-independent public interface

namespace QP {

void QF_enterCriticalSection_(void);
void QF_leaveCriticalSection_(void);

// set clock tick rate and p-thread priority
// (NOTE: ticksPerSec==0 disables the "ticker thread"
void QF_setTickRate(uint32_t ticksPerSec, int_t tickPrio);

// clock tick callback (NOTE not called when "ticker thread" is not running)
void QF_onClockTick(void);

// abstractions for console access...
void QF_consoleSetup(void);
void QF_consoleCleanup(void);
int  QF_consoleGetKey(void);
int  QF_consoleWaitForKey(void);

} // namespace QP

//****************************************************************************
// interface used only inside QF implementation, but not in applications
//
#ifdef QP_IMPL

    // scheduler locking (not needed in single-thread port)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) ((void)0)
    #define QF_SCHED_UNLOCK_()    ((void)0)

    // event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT((me_)->m_eQueue.m_frontEvt != nullptr)

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
        QV_readySet_.insert((me_)->m_prio); \
        pthread_cond_signal(&QV_condVar_); \
    } while (false)

    // event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

    #include <pthread.h>   // POSIX-thread API

    namespace QP {
        extern QPSet QV_readySet_; // QV-ready set of active objects
        extern pthread_cond_t QV_condVar_; // Cond.var. to signal events
    } // namespace QP

#endif // QP_IMPL

// NOTES: ====================================================================
//
// NOTE1:
// QF, like all real-time frameworks, needs to execute certain sections of
// code exclusively, meaning that only one thread can execute the code at
// the time. Such sections of code are called "critical sections"
//
// This port uses a pair of functions QF_enterCriticalSection_() /
// QF_leaveCriticalSection_() to enter/leave the cirtical section,
// respectively.
//
// These functions are implemented in the qf_port.c module, where they
// manipulate the file-scope POSIX mutex object l_pThreadMutex_
// to protect all critical sections. Using the single mutex for all crtical
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

#endif // QF_PORT_HPP

