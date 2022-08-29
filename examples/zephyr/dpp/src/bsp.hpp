//============================================================================
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
//! @date Last updated on: 2022-08-25
//! @version Last updated for: Zephyr 3.1.99 and @ref qpcpp_7_1_0
//!
//! @file
//! @brief BSP for Zephyr, DPP example

#ifndef BSP_HPP
#define BSP_HPP

namespace DPP {

class BSP {
public:
    enum { TICKS_PER_SEC = 1000 };

    static void init(void);
    static void displayPaused(uint8_t const paused);
    static void displayPhilStat(uint8_t const n, char const *stat);
    static void terminate(int16_t const result);

    static void randomSeed(uint32_t const seed); // random seed
    static uint32_t random(void); // pseudo-random generator

    // for testing...
    static void ledOn(void);
    static void ledOff(void);
};

extern QP::QTicker ticker0;

} // namespace DPP

#endif // BSP_HPP

