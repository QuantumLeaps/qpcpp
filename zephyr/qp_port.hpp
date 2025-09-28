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

#include <cstdint>         // Exact-width types. C++11 Standard
#include <zephyr/kernel.h> // Zephyr kernel API
#include "qp_config.hpp"   // QP configuration from the application

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// QActive event queue and thread types
#define QACTIVE_EQUEUE_TYPE  struct k_msgq
#define QACTIVE_THREAD_TYPE  struct k_thread

// QF critical section entry/exit for Zephyr, see NOTE1/
#define QF_CRIT_STAT         k_spinlock_key_t key_;
#define QF_CRIT_ENTRY()      ((key_) = k_spin_lock(&QP::QF::spinlock))
#define QF_CRIT_EXIT()       k_spin_unlock(&QP::QF::spinlock, key_)
#define QF_CRIT_EST()        static_cast<void>(k_spin_lock(&QP::QF::spinlock))

// Q_PRINTK() macro to avoid conflicts with Zephyr's printk()
// when Q_SPY configuration is used
#ifndef Q_SPY
#define Q_PRINTK(fmt_, ...)  printk(fmt_, ##__VA_ARGS__)
#else
#define Q_PRINTK(dummy, ...) static_cast<void>(0)
#endif

// include files -------------------------------------------------------------
#include "qequeue.hpp"     // used for event deferral
#include "qmpool.hpp"      // this QP port uses the native QF memory pool
#include "qp.hpp"          // QP platform-independent public interface

namespace QP {
namespace QF {

// Zephyr spinlock for QF critical section
extern struct k_spinlock spinlock;

} // namespace QF
} // namespace QP

//============================================================================
// interface used only inside QF implementation, but not in applications
#ifdef QP_IMPL

    // scheduler locking, see NOTE2
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) do { \
        if (!k_is_in_isr()) {       \
            k_sched_lock();         \
        }                           \
    } while (false)

    #define QF_SCHED_UNLOCK_() do { \
        if (!k_is_in_isr()) {       \
            k_sched_unlock();       \
        } \
    } while (false)

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

//============================================================================
// NOTE1:
// This QP port to Zephyr assumes that Active Objects will use only the
// preemptive Zephyr priorities [0..(CONFIG_NUM_PREEMPT_PRIORITIES - 1U)].
// In this priority numbering, the QP AO priority QF_MAX_ACTIVE (highest)
// maps to Zephyr priority 0 (highest). The QP AO priority 1 (lowest) maps
// to Zephyr priority (CONFIG_NUM_PREEMPT_PRIORITIES - 2U).
//
// NOTE2:
// Zephyr does not support selective scheduler locking up to a given
// priority ceiling. Therefore, this port uses global Zephyr scheduler lock.
//

#endif // QP_PORT_HPP_

