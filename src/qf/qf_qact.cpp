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
//! @brief QP::QActive::QActive() definition

#define QP_IMPL           // this is QP implementation
#include "qf_port.hpp"    // QF port

namespace QP {

//============================================================================
QActive::QActive(QStateHandler const initial) noexcept
  : QHsm(initial),
    m_prio(0U)
{
#ifdef QF_OS_OBJECT_TYPE
    QF::bzero(&m_osObject, sizeof(m_osObject));
#endif

#ifdef QF_THREAD_TYPE
    QF::bzero(&m_thread, sizeof(m_thread));
#endif
}

} // namespace QP
