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
//! @brief QF/C++ port to PIC32, cooperative QV kernel, XC32 toolchain

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE          32U

// The maximum number of system clock tick rates
#define QF_MAX_TICK_RATE       2U

// QF interrupt disable/enable, see NOTE2
#define QF_INT_DISABLE()       __asm__ volatile("DI")
#define QF_INT_ENABLE()        __asm__ volatile("EI")

// QF critical section entry/exit, see NOTE2
// QF_CRIT_STAT_TYPE not defined: "unconditional interrupt disabling" policy
#define QF_CRIT_ENTRY(dummy)   QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)    QF_INT_ENABLE()

// fast log-base-2 with CLZ instruction
#define QF_LOG2(n_) (static_cast<std::uint8_t>(32U - _clz(n_)))

#include <xc.h>       // for _clz()

#include "qep_port.hpp" // QEP port
#include "qv_port.hpp"  // QV cooperative kernel port

//============================================================================
// NOTE1:
// The maximum number of active objects QF_MAX_ACTIVE can be increased
// up to 64, if necessary. Here it is set to a lower level to save some RAM.
//
// NOTE2:
// The DI/EI instructions are used for fast, unconditional disabling and
// enabling of interrupts.
//
// CAUTION: This QP port assumes that interrupt nesting is _enabled_,
// which is the default in the PIC32 processors. Interrupt nesting should
// never be disabled by setting the NSTDIS control bit (INTCON1<15>). If you
// don't want interrupts to nest, you can always prioritize them at the same
// level. For example, the default priority level for all interrupts is 4 out
// of reset. If you don't change this level for any interrupt the nesting of
// interrupt will not occur.
//

#endif // QF_PORT_HPP

