//! @file
//! @brief QF/C++ port for QV kernel, Generic C++ compiler
//! @cond
//============================================================================
//! Last updated for version 6.8.2
//! Last updated on  2020-07-17
//!
//!                    Q u a n t u m  L e a P s
//!                    ------------------------
//!                    Modern Embedded Software
//!
//! Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// interrupt disabling mechanism
#define QF_INT_DISABLE()            intDisable()
#define QF_INT_ENABLE()             intEnable()

// QF critical section mechansim
#define QF_CRIT_STAT_TYPE           unsigned
#define QF_CRIT_ENTRY(stat_)        ((stat_) = critEntry())
#define QF_CRIT_EXIT(stat_)         critExit(stat_)

#include "qep_port.hpp" // QEP port
#include "qv_port.hpp"  // QV port
#include "qf.hpp"       // QF platform-independent public interface

extern "C" {

void intDisable(void);
void intEnable(void);

QF_CRIT_STAT_TYPE critEntry(void);
void critExit(QF_CRIT_STAT_TYPE stat);

} // extern "C"

#endif // QF_PORT_HPP
