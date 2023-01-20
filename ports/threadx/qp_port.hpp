//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
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
//! @date Last updated on: 2023-09-07
//! @version Last updated for: @ref qpcpp_7_3_0
//!
//! @file
//! @brief QP/C++ port to ThreadX (a.k.a, Azure RTOS), generic C++11 compiler

#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

#include <cstdint>  // Exact-width types. C++11 Standard

#ifdef QP_CONFIG
#include "qp_config.hpp" // external QP configuration
#endif

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// QActive customization for ThreadX
#define QACTIVE_EQUEUE_TYPE     TX_QUEUE
#define QACTIVE_OS_OBJ_TYPE     bool
#define QACTIVE_THREAD_TYPE     TX_THREAD

// QF priority offset within ThreadX priority numbering scheme, see NOTE1
#define QF_TX_PRIO_OFFSET       8U

// QF critical section for ThreadX, see NOTE3
#define QF_CRIT_STAT            UINT int_ctrl_;
#define QF_CRIT_ENTRY()         (int_ctrl_ = tx_interrupt_control(TX_INT_DISABLE))
#define QF_CRIT_EXIT()          ((void)tx_interrupt_control(int_ctrl_))

// include files -------------------------------------------------------------
#include "tx_api.h"    // ThreadX API

#include "qequeue.hpp" // QP event queue (for deferring events)
#include "qmpool.hpp"  // QP memory pool (for event pools)
#include "qp.hpp"      // QP platform-independent public interface


namespace QP {

enum ThreadX_ThreadAttrs {
    THREAD_NAME_ATTR
};

} // namespace QP


//============================================================================
// interface used only inside QF, but not in applications

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

    // native QF event pool customization
    #define QF_EPOOL_TYPE_        QMPool
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
// NOTE3:
// The ThreadX critical section must be able to nest, which is the case with
// the tx_interrupt_control() API.
//

#endif // QP_PORT_HPP_

