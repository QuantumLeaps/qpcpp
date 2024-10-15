//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// The QP/C software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2024-09-30
//! @version Last updated for: @ref qpcpp_8_0_0
//!
//! @file
//! @brief QP/C++ port to ThreadX (a.k.a, Azure RTOS), generic C++11 compiler

#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

#include <cstdint>        // Exact-width types. C++11 Standard
#include "qp_config.hpp"  // QP configuration from the application

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// QActive customization for ThreadX
#define QACTIVE_EQUEUE_TYPE     TX_QUEUE
#define QACTIVE_THREAD_TYPE     TX_THREAD
#define QACTIVE_OS_OBJ_TYPE     std::uint8_t

// QF priority offset within ThreadX priority numbering scheme
#define QF_TX_PRIO_OFFSET       2U

#ifndef QF_MAX_ACTIVE
#define QF_MAX_ACTIVE    (TX_MAX_PRIORITIES - QF_TX_PRIO_OFFSET)
#else
#error "QF_MAX_ACTIVE shouild not be externally defined in QP-ThreadX port"
#endif

// mapping between QF-priority and TX-priority, see NOTE1
#define QF_TO_TX_PRIO_MAP(qp_prio_) \
    (TX_MAX_PRIORITIES - QF_TX_PRIO_OFFSET - (qp_prio_))

// mapping between TX-priority and QF-priority, see NOTE1
#define TX_TO_QF_PRIO_MAP(tx_prio_) \
    (TX_MAX_PRIORITIES - QF_TX_PRIO_OFFSET - (tx_prio_))

// QF critical section for ThreadX, see NOTE3
#define QF_CRIT_STAT     UINT int_ctrl_;
#define QF_CRIT_ENTRY()  (int_ctrl_ = tx_interrupt_control(TX_INT_DISABLE))
#define QF_CRIT_EXIT()   ((void)tx_interrupt_control(int_ctrl_))

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
// interface used only inside QF implementation, but not in applications
#ifdef QP_IMPL

    // ThreadX-specific scheduler locking (implemented in qf_port.cpp)
    #define QF_SCHED_STAT_ QFSchedLock lockStat_;
    #define QF_SCHED_LOCK_(prio_) do {            \
        if (TX_THREAD_GET_SYSTEM_STATE() != 0U) { \
            lockStat_.m_lockPrio = 0U;            \
        } else {                                  \
            lockStat_.lock((prio_));              \
        }                                         \
    } while (false)
    #define QF_SCHED_UNLOCK_() do {       \
        if (lockStat_.m_lockPrio != 0U) { \
            lockStat_.unlock();           \
        }                                 \
    } while (false)

    namespace QP {
        struct QFSchedLock {
            uint_fast8_t m_lockPrio; // lock prio [QF numbering scheme]
            UINT m_prevThre;         // previoius preemption threshold
            TX_THREAD *m_lockHolder; // the thread holding the lock

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
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))
    #define QF_EPOOL_PUT_(p_, e_, qsId_) ((p_).put((e_), (qsId_)))

#endif // ifdef QP_IMPL

//============================================================================
// NOTE1:
// Because the ThreadX priority numbering is "upside down" compared
// to the QP priority numbering, the ThreadX priority for an active object
// thread is calculated as specified in the macro QF_TO_TX_PRIO_MAP(prio_).
// This mapping leaves the QF_TX_PRIO_OFFSET number of lowest-priority
// ThreadX threads NOT available as threads for QP Active Objects. The use
// of such lowest-priority ThredX threads is, for example, to emulate idle
// thread, etc.
//
// NOTE3:
// The ThreadX critical section must be able to nest, which is the case with
// the tx_interrupt_control() API.
//

#endif // QP_PORT_HPP_

