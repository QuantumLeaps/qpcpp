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
//!
//! @date Last updated on: 2022-08-24
//! @version Last updated for: Zephyr 3.1.99 and @ref qpcpp_7_1_0
//!
//! @file
//! @brief Blinky example

#ifndef BLINKY_HPP
#define BLINKY_HPP

using namespace QP;

enum BlinkySignals {
    DUMMY_SIG = Q_USER_SIG,
    MAX_PUB_SIG,  // the last published signal

    TIMEOUT_SIG,
    MAX_SIG       // the last signal
};

extern QActive * const AO_Blinky; // opaque pointer

#endif // BLINKY_HPP

