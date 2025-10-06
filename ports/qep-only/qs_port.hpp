//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#ifndef QS_PORT_HPP_
#define QS_PORT_HPP_

#if defined(_WIN64) || defined(__LP64__) || defined(_LP64)  // 64-bit OS?
    #define QS_OBJ_PTR_SIZE 8U
    #define QS_FUN_PTR_SIZE 8U
#else  // 32-bit OS
    #define QS_OBJ_PTR_SIZE 4U
    #define QS_FUN_PTR_SIZE 4U
#endif

namespace QP {
void QS_output(void);    // handle the QS output
void QS_rx_input(void);  // handle the QS-RX input
}

//============================================================================
// NOTE: QS might be used with or without other QP components, in which
// case the separate definitions of the macros QF_CRIT_STAT, QF_CRIT_ENTRY(),
// and QF_CRIT_EXIT() are needed. In this port QS is configured to be used
// with the other QP component, by simply including "qp_port.hpp"
// *before* "qs.hpp".
#ifndef QP_PORT_HPP_
#include "qp_port.hpp" // use QS with QP
#endif

#include "qs.hpp"      // QS platform-independent public interface

#endif // QS_PORT_HPP_
