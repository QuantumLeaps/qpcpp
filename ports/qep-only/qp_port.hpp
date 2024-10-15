//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// The QP/C software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2024-09-30
//! @version Last updated for: @ref qpcpp_8_0_0
//!
//! @file
//! @brief QP/C dummy port

#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// include files -------------------------------------------------------------
#include <cstdint>        // Exact-width types. C++11 Standard
#include "qp_config.hpp"  // QP configuration from the application
#include "qp.hpp"         // QP platform-independent public interface

#endif // QP_PORT_HPP_
