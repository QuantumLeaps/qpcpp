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
//! @date Last updated on: 2021-12-23
//! @version Last updated for: @ref qpcpp_7_0_0
//!
//! @file
//! @brief QEP/C++ port, generic C++11 compiler
//! @description
//! This is an example QP/C++ port with the documentation for the main
//! items, such as configuration macros, functions, and includes.

#ifndef QEP_PORT_HPP
#define QEP_PORT_HPP

//! no-return function specifier (C11/C++11 Standard)
#define Q_NORETURN  _Noreturn void

#include <cstdint>  // Exact-width types. C++11 Standard

#include "qep.hpp"  // QEP platform-independent public interface

#endif // QEP_PORT_HPP
