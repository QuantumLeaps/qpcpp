/// @file
/// @brief QF/C++ port to ARM Cortex-M, preemptive QK kernel, IAR-ARM toolset
/// @cond
///***************************************************************************
/// Last updated for version 5.6.0
/// Last updated on  2015-12-31
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qf_port_h
#define qf_port_h

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE           32

// The maximum number of system clock tick rates
#define QF_MAX_TICK_RATE        2

// QF interrupt disable/enable and log2()...
#if (__CORE__ == __ARM6M__)  // Cortex-M0/M0+/M1 ?, see NOTE02

    #define QF_INT_DISABLE()    __disable_interrupt()
    #define QF_INT_ENABLE()     __enable_interrupt()

    // QF-aware ISR priority for CMSIS function NVIC_SetPriority(), NOTE2
    #define QF_AWARE_ISR_CMSIS_PRI 0

#else // Cortex-M3/M4/M4F, see NOTE03

    #define QF_INT_DISABLE()    __set_BASEPRI(QF_BASEPRI)
    #define QF_INT_ENABLE()     __set_BASEPRI(0U)

    // NOTE: keep in synch with the value defined in "qk_port.s", see NOTE4
    #define QF_BASEPRI          (0xFFU >> 2)

    // QF-aware ISR priority for CMSIS function NVIC_SetPriority(), NOTE5
    #define QF_AWARE_ISR_CMSIS_PRI (QF_BASEPRI >> (8 - __NVIC_PRIO_BITS))

    // Cortex-R provide the CLZ instruction for fast LOG2
    #define QF_LOG2(n_) ((uint8_t)(32U - __CLZ(n_)))
#endif

// QF critical section entry/exit...
// QF_CRIT_STAT_TYPE not defined: unconditional interrupt disabling" policy
#define QF_CRIT_ENTRY(dummy)    QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)     QF_INT_ENABLE()
#define QF_CRIT_EXIT_NOP()      __ISB()

#include <intrinsics.h> // IAR intrinsic functions
#include "qep_port.h"   // QEP port
#include "qk_port.h"    // QK port
#include "qf.h"         // QF platform-independent public interface

#endif // qf_port_h
