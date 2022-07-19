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
//! @brief QF/C++ port for QK kernel, Generic C++ compiler

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// interrupt disabling mechanism
#define QF_INT_DISABLE()            intDisable()
#define QF_INT_ENABLE()             intEnable()

// QF critical section mechanism
#define QF_CRIT_STAT_TYPE           unsigned
#define QF_CRIT_ENTRY(stat_)        ((stat_) = critEntry())
#define QF_CRIT_EXIT(stat_)         critExit(stat_)

#include "qep_port.hpp" // QEP port
#include "qk_port.hpp"  // QK preemptive, run-to-completion kernel port

extern "C" {

void intDisable(void);
void intEnable(void);

QF_CRIT_STAT_TYPE critEntry(void);
void critExit(QF_CRIT_STAT_TYPE stat);

} // extern "C"

#endif // QF_PORT_HPP
