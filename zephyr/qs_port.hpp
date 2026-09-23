//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005-2026 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License, version 3 or later (GPL-3.0-or-later), or,
// alternatively, under the terms of one of the closed-source Quantum Leaps
// commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
//
// NOTE:
// The GPL does NOT permit distributing this code as part of proprietary
// (closed-source) programs. For such use, please contact Quantum Leaps for
// a commercial license, which is an alternative to the GPL designed
// explicitly for closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QS_PORT_HPP_
#define QS_PORT_HPP_

// QS time-stamp size in bytes
#define QS_TIME_SIZE     4U

// object pointer size in bytes
#define QS_OBJ_PTR_SIZE  4U

// function pointer size in bytes
#define QS_FUN_PTR_SIZE  4U

//============================================================================
// NOTE: QS might be used with or without other QP components, in which
// case the separate definitions of the macros QF_CRIT_STAT, QF_CRIT_ENTRY(),
// and QF_CRIT_EXIT() are needed. In this port QS is configured to be used
// with the other QP component, by simply including "qp_port.h"
//*before* "qs.h".
#ifndef QP_PORT_H_
#include "qp_port.hpp" // use QS with QF
#endif

#include "qs.hpp"      // QS platform-independent public interface

#endif // QS_PORT_HPP_

