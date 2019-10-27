/// @file
/// @brief QF/C++ port to POSIX/P-threads
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

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// event queue and thread types
#define QF_EQUEUE_TYPE       QEQueue
#define QF_OS_OBJECT_TYPE    pthread_cond_t
#define QF_THREAD_TYPE       uint8_t

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

// QF critical section entry/exit for POSIX, see NOTE1
// QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy) QP::QF_enterCriticalSection_()
#define QF_CRIT_EXIT(dummy)  QP::QF_leaveCriticalSection_()

#include <pthread.h>   // POSIX-thread API
#include "qep_port.hpp"  // QEP port
#include "qequeue.hpp"   // POSIX needs event-queue
#include "qmpool.hpp"    // POSIX needs memory-pool
#include "qpset.hpp"     // POSIX needs priority-set
#include "qf.hpp"        // QF platform-independent public interface

namespace QP {

void QF_enterCriticalSection_(void);
void QF_leaveCriticalSection_(void);

// set clock tick rate and p-thread priority
void QF_setTickRate(uint32_t ticksPerSec, int_t tickPrio);

// clock tick callback (provided in the app)
void QF_onClockTick(void);

// abstractions for console access...
void QF_consoleSetup(void);
void QF_consoleCleanup(void);
int  QF_consoleGetKey(void);
int  QF_consoleWaitForKey(void);

extern pthread_mutex_t QF_pThreadMutex_; // mutex for QF critical section

} // namespace QP

//****************************************************************************
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL

    // POSIX-specific scheduler locking (not used at this point)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) ((void)0)
    #define QF_SCHED_UNLOCK_()    ((void)0)

    // native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        while ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt const *>(0)) \
            pthread_cond_wait(&(me_)->m_osObject, &QF_pThreadMutex_)

    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        Q_ASSERT_ID(410, QF::active_[(me_)->m_prio] \
                         != static_cast<QActive *>(0)); \
        pthread_cond_signal(&(me_)->m_osObject) \

    // event pool operations...
    #define QF_EPOOL_TYPE_  QMPool

    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))

    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_)     ((p_).put(e_))

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
// manipulate the file-scope POSIX mutex object QF_pThreadMutex_
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

