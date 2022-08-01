//! @file
//! @brief QK/C++ port to PIC32, preemptive QK kernel, XC32 toolchain
//! @cond
//============================================================================
//! Last updated for version 7.0.1
//! Last updated on  2022-06-30
//!
//!                    Q u a n t u m  L e a P s
//!                    ------------------------
//!                    Modern Embedded Software
//!
//! Copyright (C) 2005 Quantum Leaps. All rights reserved.
//!
//! This program is open source software: you can redistribute it and/or
//! modify it under the terms of the GNU General Public License as published
//! by the Free Software Foundation, either version 3 of the License, or
//! (at your option) any later version.
//!
//! Alternatively, this program may be distributed and modified under the
//! terms of Quantum Leaps commercial licenses, which expressly supersede
//! the GNU General Public License and are specifically designed for
//! licensees interested in retaining the proprietary status of their code.
//!
//! This program is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//! GNU General Public License for more details.
//!
//! You should have received a copy of the GNU General Public License
//! along with this program. If not, see <www.gnu.org/licenses>.
//!
//! Contact information:
//! <www.state-machine.com/licensing>
//! <info@state-machine.com>
//============================================================================
//! @endcond

#ifndef QK_PORT_HPP
#define QK_PORT_HPP

// QK interrupt entry and exit
#define QK_ISR_ENTRY() do { \
    QF_INT_DISABLE();       \
    ++QP::QF::intNest_;     \
    QF_INT_ENABLE();        \
} while (false)

#define QK_ISR_EXIT() do {   \
    QF_INT_DISABLE();        \
    --QP::QF::intNest_;      \
    if (QK_sched_() != 0U) { \
        IFS0SET = _IFS0_CS0IF_MASK; \
    }                        \
    QF_INT_ENABLE();         \
} while (false)

// initialization of the QK kernel
#define QK_INIT() QK_init()
extern "C" void QK_init(void);

#include "qk.hpp"  // QK platform-independent public interface

//============================================================================
// NOTE1:
// Any interrupt service routine that interacts with QP must begin with the
// QK_ISR_ENTRY() macro and must end with the QK_ISR_EXIT() macro. The source
// file containing the interrupt service routine must #include <p32xxxx.h> or
// <plib.h>. Core software interrupt 0 and Interrupt Priority Level 1 are
// reserved for use by QK.
//

#endif // QK_PORT_HPP

