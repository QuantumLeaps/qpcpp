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
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief Application build time-stamp
//!
//! @note
//! This module needs to be re-compiled in every new software build.
//! To achieve this, it is recommended to delete the object file
//! (qstamp.o or qstamp.obj) in the build directory before each build.
//! (Most development tools allow you to specify a pre-build action,
//! which is the ideal place to delete the qstamp object file.)

#include "qstamp.hpp"

namespace QP {

//! the calendar date of the last translation of the form: "Mmm dd yyyy"
char const BUILD_DATE[12] = __DATE__;

//! the time of the last translation of the form: "hh:mm:ss"
char const BUILD_TIME[9] = __TIME__;

} // namespace QP
