//! @file
//! @brief QK/C++ port to MSP430
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

// QK interrupt entry and exit...
#define QK_ISR_ENTRY()    (++QP::QF::intNest_)

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

#include "qk.hpp"  // QK platform-independent public interface

#endif // QK_PORT_HPP

