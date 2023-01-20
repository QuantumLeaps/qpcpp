//============================================================================
// Product: BSP for system-testing QXK
// Last updated for version 7.3.0
// Last updated on  2023-09-06
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
#ifndef BSP_HPP_
#define BSP_HPP_

namespace BSP {

void init(void);
void terminate(int16_t const result);

// for testing...
void trace(QP::QActive const *thr, char const *msg);
void wait4PB1(void);
void ledOn(void);
void ledOff(void);
void trigISR(void);

uint32_t romRead(int32_t offset, uint32_t fromEnd);
void romWrite(int32_t offset, uint32_t fromEnd, uint32_t value);

uint32_t ramRead(int32_t offset, uint32_t fromEnd);
void ramWrite(int32_t offset, uint32_t fromEnd, uint32_t value);

} // namespace BSP

enum TestSignals {
    TEST0_SIG = QP::Q_USER_SIG,
    TEST1_SIG,
    TEST2_SIG,
    TEST3_SIG,
    MAX_PUB_SIG,    // the last published signal

    MAX_SIG         // the last signal
};

#endif // BSP_HPP_

