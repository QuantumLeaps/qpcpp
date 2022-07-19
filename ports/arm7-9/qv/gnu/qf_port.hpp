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
//! @brief QF/C++ port to ARM, cooperative QV kernel, GNU-ARM toolset

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE           32U

// The maximum number of system clock tick rates
#define QF_MAX_TICK_RATE        2U

// fast unconditional interrupt disabling/enabling for ARM state, NOTE2
#define QF_INT_DISABLE()   \
    __asm volatile ("MSR cpsr_c,#(0x1F | 0x80)" ::: "cc")

#define QF_INT_ENABLE() \
    __asm volatile ("MSR cpsr_c,#(0x1F)" ::: "cc")

// QF critical section entry/exit...
#ifdef __thumb__  // THUMB mode?

    // QF interrupt disabling/enabling policy
    #define QF_CRIT_STAT_TYPE       unsigned int
    #define QF_CRIT_ENTRY(stat_)    ((stat_) = QF_int_disable_SYS())
    #define QF_CRIT_EXIT(stat_)     QF_int_enable_SYS(stat_)

    QF_CRIT_STAT_TYPE QF_int_disable_SYS(void);
    void QF_int_enable_SYS(QF_CRIT_STAT_TYPE stat);

#elif (defined __arm__) // ARM mode?

    #define QF_CRIT_STAT_TYPE       unsigned int
    #define QF_CRIT_ENTRY(stat_)    do { \
        __asm volatile ("MRS %0,cpsr" : "=r" (stat_) :: "cc"); \
        QF_INT_DISABLE(); \
    } while (false)
    #define QF_CRIT_EXIT(stat_) \
        __asm volatile ("MSR cpsr_c,%0" :: "r" (stat_) : "cc")

#else

    #error Incorrect CPU mode. Must be either __arm__ or __thumb__.

#endif

extern "C" {
    void QF_reset(void);
    void QF_undef(void);
    void QF_swi(void);
    void QF_pAbort(void);
    void QF_dAbort(void);
    void QF_reserved(void);
    void QF_unused(void);
}

#include "qep_port.hpp" // QEP port
#include "qv_port.hpp"  // QV cooperative kernel port

//============================================================================
// NOTE1:
// The maximum number of active objects QF_MAX_ACTIVE can be increased
// up to 63, if necessary. Here it is set to a lower level to save some RAM.
//
// NOTE2:
// The disabling/enabling of interrutps is only defined for the ARM state.
// The policy is to disable only the IRQ and NOT to disable the FIQ, which
// means that FIQ is a "QF-unaware" from the kernel perspective. This means
// that FIQ has "zero latency", but it also means that FIQ __cannot__ call
// any QP services. Specifically FIQ cannot post or publish events.
//

#endif // QF_PORT_HPP
