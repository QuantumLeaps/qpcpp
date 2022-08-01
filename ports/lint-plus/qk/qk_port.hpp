//! @file
//! @brief QK/C++ port to Lint, Generic C++ compiler
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

//lint -save -e1960    MISRA-C++:2008 Rule 7-3-1, Global declaration

//============================================================================
//! enable the context-switch callback
#define QK_ON_CONTEXT_SW 1

//============================================================================
// QK interrupt entry and exit

//! Define the ISR entry sequence, if the compiler supports writing
//! interrupts in C++.
//! @note This is just an example of #QK_ISR_ENTRY. You need to define
//! the macro appropriately for the CPU/compiler you're using. Also, some
//! QK ports will not define this macro, but instead will provide ISR
//! skeleton code in assembly.
#define QK_ISR_ENTRY() do { \
    ++QP::QF::intNest_;     \
} while (false)

//! Define the ISR exit sequence, if the compiler supports writing
//! interrupts in C++.
//! @note This is just an example of #QK_ISR_EXIT. You need to define
//! the macro appropriately for the CPU/compiler you're using. Also, some
//! QK ports will not define this macro, but instead will provide ISR
//! skeleton code in assembly.
#define QK_ISR_EXIT()     do {   \
    --QP::QF::intNest_;          \
    if (QP::QF::intNest_ == 0U) {\
        if (QK_sched_() != 0U) { \
            QK_activate_();      \
        }                        \
    }                            \
    else {                       \
        Q_ERROR();               \
    }                            \
} while (false)

extern "C" {

void FPU_save(void *ctx);     // defined in assembly
void FPU_restore(void *ctx);  // defined in assembly
extern void *impure_ptr;

} // extern "C"

//lint -restore

#include "qk.hpp" // QK platform-independent public interface

#endif // QK_PORT_HPP
