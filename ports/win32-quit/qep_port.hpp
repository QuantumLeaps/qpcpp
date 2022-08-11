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
//! @date Last updated on: 2022-06-07
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief port to Win32 with GNU or Visual Studio C/C++ compilers

#ifndef QEP_PORT_HPP
#define QEP_PORT_HPP

#ifdef __GNUC__

    /*! no-return function specifier (GCC-ARM compiler) */
    #define Q_NORETURN   __attribute__ ((noreturn)) void

#elif (defined _MSC_VER) && (defined __cplusplus)

    /* no-return function specifier (Microsoft Visual Studio compiler) */
    #define Q_NORETURN   [[ noreturn ]] void

#endif

// uncomment to provide QEvt constructors and destructors
//#define Q_EVT_CTOR 1
//#define Q_EVT_XTOR 1

#include <cstdint>  // Exact-width types. C++11 Standard

#include "qep.hpp"  // QEP platform-independent public interface

#endif // QEP_PORT_HPP
