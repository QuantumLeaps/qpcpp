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
//! @brief QP/C++ port for QXK kernel, Generic C++

#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

#include <cstdint>  // Exact-width types. C++11 Standard

#ifdef QP_CONFIG
#include "qp_config.hpp" // external QP configuration
#endif

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// QF configuration for QXK -- data members of the QActive class...

// QXK event-queue type used for AOs and eXtended threads.
#define QACTIVE_EQUEUE_TYPE  QEQueue

// QXK OS-Object type used for the private stack pointer for eXtended threads.
// (The private stack pointer is NULL for basic-threads).
#define QACTIVE_OS_OBJ_TYPE  void*

// QF "thread" type used to store the MPU settings in the AO
#define QACTIVE_THREAD_TYPE  void const *

// interrupt disabling mechanism
#define QF_INT_DISABLE()     intDisable()
#define QF_INT_ENABLE()      intEnable()

// QF critical section mechanism
#define QF_CRIT_STAT         std::uint32_t crit_stat_;
#define QF_CRIT_ENTRY()      (crit_stat_ = critEntry())
#define QF_CRIT_EXIT()       critExit(crit_stat_)

// Check if the code executes in the ISR context
#define QXK_ISR_CONTEXT_()   (QXK_get_IPSR() != 0U)

// Define the ISR entry sequence
#define QXK_ISR_ENTRY()      (static_cast<void>(0))

// Define the ISR exit sequence
#define QXK_ISR_EXIT()  do {                                  \
    QF_INT_DISABLE();                                         \
    if (QXK_sched_() != 0U) {                                 \
        *Q_UINT2PTR_CAST(uint32_t, 0xE000ED04U) = (1U << 28U);\
    }                                                         \
    QF_INT_ENABLE();                                          \
} while (false)

#define QXK_CONTEXT_SWITCH_() (QXK_trigPendSV())

extern "C" {

void intDisable(void);
void intEnable(void);
std::uint32_t critEntry(void);
void critExit(std::uint32_t stat);
std::uint32_t QXK_get_IPSR(void);
void QXK_trigPendSV(void);

} // extern "C"

// include files -------------------------------------------------------------
#include "qequeue.hpp"  // QK kernel uses the native QP event queue
#include "qmpool.hpp"   // QK kernel uses the native QP memory pool
#include "qp.hpp"       // QP framework
#include "qxk.hpp"      // QXK kernel

#endif // QP_PORT_HPP_

