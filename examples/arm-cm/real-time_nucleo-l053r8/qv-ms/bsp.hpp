//============================================================================
// "real-time" example to demonstrate timing in QP/C++
// Last Updated for Version: 7.3.0
// Date of the Last Update:  2023-10-02
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef HPP_
#define HPP_

namespace BSP {

constexpr std::uint32_t TICKS_PER_SEC {1000U};

void init();
void start();

void d1on();
void d1off();

void d2on();
void d2off();

void d3on();
void d3off();

void d4on();
void d4off();

void d5on();
void d5off();

void d6on();
void d6off();

void d7on();
void d7off();

// immutable events for Periodic active objects
QP::QEvt const *getEvtPeriodic1(std::uint8_t num);
QP::QEvt const *getEvtPeriodic4(std::uint8_t num);

} // namespace BSP

#endif // HPP_
