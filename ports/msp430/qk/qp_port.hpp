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
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C++ software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C++ in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

#include <cstdint>        // Exact-width types. C++11 Standard
#include "qp_config.hpp"  // QP configuration from the application

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// QF configuration for QK -- data members of the QActive class...

// QActive event queue type
#define QACTIVE_EQUEUE_TYPE     QEQueue
// QACTIVE_OS_OBJ_TYPE not used in this port
// QACTIVE_THREAD_TYPE not used in this port

// QF interrupt disable/enable...
#define QF_INT_DISABLE()        __disable_interrupt()
#define QF_INT_ENABLE()         __enable_interrupt()

// QF critical section entry/exit...
#define QF_CRIT_STAT            unsigned short int_state_;
#define QF_CRIT_ENTRY() do {               \
    int_state_ =  __get_interrupt_state(); \
    __disable_interrupt();                 \
} while (false)

#define QF_CRIT_EST()           __disable_interrupt()
#define QF_CRIT_EXIT()          __set_interrupt_state(int_state_)

// Check if the code executes in the ISR context
#define QK_ISR_CONTEXT_() (QK_priv_.intNest != 0U)

// QK interrupt entry and exit...
#define QK_ISR_ENTRY()    (++QK_priv_.intNest)

#define QK_ISR_EXIT()     do {       \
    --QK_priv_.intNest;              \
    if (QK_priv_.intNest == 0U) {    \
        if (QP::QK_sched_() != 0U) { \
            QP::QK_activate_();      \
        }                            \
    }                                \
} while (false)

// include files -------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h>
#elif defined(__GNUC__)
    #include <msp430.h>
    #include "in430.h"
#endif

#include "qequeue.hpp"   // QK kernel uses the native QP event queue
#include "qmpool.hpp"    // QK kernel uses the native QP memory pool
#include "qp.hpp"        // QP framework
#include "qk.hpp"        // QK kernel

#endif // QP_PORT_HPP_

