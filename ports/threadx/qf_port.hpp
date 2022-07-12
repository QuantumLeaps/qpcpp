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
//! @brief QF/C++ port to ThreadX kernel, all supported compilers

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// ThreadX event queue and thread types
#define QF_EQUEUE_TYPE       TX_QUEUE
#define QF_THREAD_TYPE       TX_THREAD
#define QF_OS_OBJECT_TYPE    bool

// QF priority offset within ThreadX priority numbering scheme, see NOTE1
#define QF_TX_PRIO_OFFSET    8U

// The maximum number of active objects in the application, see NOTE2
#define QF_MAX_ACTIVE        (31U - QF_TX_PRIO_OFFSET)

// QF critical section for ThreadX, see NOTE3
#define QF_CRIT_STAT_TYPE    UINT
#define QF_CRIT_ENTRY(stat_) ((stat_) = tx_interrupt_control(TX_INT_DISABLE))
#define QF_CRIT_EXIT(stat_)  ((void)tx_interrupt_control(stat_))

namespace QP {

enum ThreadX_ThreadAttrs {
    THREAD_NAME_ATTR
};

} // namespace QP

#include "tx_api.h"      // ThreadX API

#include "qep_port.hpp"  // QEP port
#include "qequeue.hpp"   // used for event deferral
#include "qmpool.hpp"    // native QF event pool
#include "qf.hpp"        // QF platform-independent public interface


//============================================================================
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL

    // ThreadX-specific scheduler locking (implemented in qf_port.cpp)
    #define QF_SCHED_STAT_ QFSchedLock lockStat_;
    #define QF_SCHED_LOCK_(prio_) do {            \
        if (TX_THREAD_GET_SYSTEM_STATE() != 0U) { \
            lockStat_.m_lockPrio = 0U;            \
        } else {                                  \
            lockStat_.lock((prio_));              \
        } \
    } while (false)
    #define QF_SCHED_UNLOCK_() do {       \
        if (lockStat_.m_lockPrio != 0U) { \
            lockStat_.unlock();           \
        }                                 \
    } while (false)

    namespace QP {
        struct QFSchedLock {
            uint_fast8_t m_lockPrio; //!< lock prio [QF numbering scheme]
            UINT m_prevThre;         //!< previoius preemption threshold
            TX_THREAD *m_lockHolder; //!< the thread holding the lock

            void lock(uint_fast8_t prio);
            void unlock(void) const;
        };
    } // namespace QP
    extern "C" {
        // internal TX interrupt counter for TX_THREAD_GET_SYSTEM_STATE()
        extern ULONG volatile _tx_thread_system_state;
    }

    // native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

#endif // ifdef QP_IMPL


//============================================================================
// NOTE1:
// QF_TX_PRIO_OFFSET specifies the number of highest-urgency ThreadX
// priorities not available to QP active objects. These highest-urgency
// priorities might be used by ThreadX threads that run "above" QP active
// objects.
//
// Because the ThreadX priority numbering is "upside down" compared
// to the QP priority numbering, the ThreadX priority for an active object
// thread is calculated as follows:
//     tx_prio = QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - qp_prio
//
// NOTE2:
// The maximum number of active objects in QP can be increased to 63,
// inclusive, but it can be reduced to save some memory. Also, the number of
// active objects cannot exceed the number of TreadX thread priorities
// TX_MAX_PRIORITIES, because each QP active object requires a unique
// priority level. Due to the artificial limitations of the ThreadX demo
// for Windows, QF_MAX_ACTIVE is set here lower to not exceed the total
// available priority levels.
//
// NOTE3:
// The ThreadX critical section must be able to nest, which is the case with
// the tx_interrupt_control() API.

#endif // QF_PORT_HPP
