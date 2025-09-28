//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QS_PORT_HPP_
#define QS_PORT_HPP_

#if defined(__LP64__) || defined(_LP64) || defined(_WIN64) // 64-bit?
    #define QS_OBJ_PTR_SIZE 8U
    #define QS_FUN_PTR_SIZE 8U
#else                                   // 32-bit architecture
    #define QS_OBJ_PTR_SIZE 4U
    #define QS_FUN_PTR_SIZE 4U
#endif

// flush the QS output buffer after each QS record
#define QS_REC_DONE()  QS::onFlush()

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
