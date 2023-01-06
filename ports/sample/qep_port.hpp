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
//! @version Last updated for: @ref qpcpp_7_2_0
//!
//! @file
//! @brief QEP/C++ sample port with all configurable options

#ifndef QEP_PORT_HPP_
#define QEP_PORT_HPP_

//! The size (in bytes) of the signal of an event. Valid values:
//! 1U, 2U, or 4U; default 2U
//!
//! @details
//! This macro can be defined in the QEP port file (qep_port.hpp) to
//! configure the ::QSignal type. When the macro is not defined, the
//! default of 2 bytes is applied in the `qep.hpp` header file.
//!
#define Q_SIGNAL_SIZE   2U

//! No-return function specifier for the Q_onAssert() callback function.
//!
//! The `Q_NORETURN` macro is supposed to be defined in the QP/C port
//! (file `qep_port.hpp`). If such definition is NOT provided, the default
//! definition is provided in the `qassert.h` header file, where
//! it assumes only `void` type returned from Q_onAssert().
//!
//! @trace
//! @tr{PQA01_4}
//!
#define Q_NORETURN  _Noreturn void

#include <cstdint>  // Exact-width types. C++11 Standard

#include "qep.hpp"  // QEP platform-independent public interface

#endif // QEP_PORT_HPP_
