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
//! @brief Internal (package scope) QXK/C++ interface.

#ifndef QXK_PKG_HPP
#define QXK_PKG_HPP

namespace QP {

//! timeout signals
enum QXK_Timeouts : std::uint8_t {
    QXK_DELAY_SIG = Q_USER_SIG,
    QXK_QUEUE_SIG,
    QXK_SEMA_SIG
};

} // namespace QP

//============================================================================
extern "C" {

//! initialize the private stack of a given AO
void QXK_stackInit_(void *thr, QP::QXThreadHandler const handler,
             void * const stkSto, std::uint_fast16_t const stkSize) noexcept;

//! called when a thread function returns
//!
//! @description
//! Called when the extended-thread handler function returns.
//!
//! @note
//! Most thread handler functions are structured as endless loops that never
//! return. But it is also possible to structure threads as one-shot functions
//! that perform their job and return. In that case this function performs
//! cleanup after the thread.
//!
void QXK_threadRet_(void) noexcept;

} // extern "C"

//! internal macro to encapsulate casting of pointers for MISRA deviations
//!
//! @description
//! This macro is specifically and exclusively used for casting pointers
//! that are never de-referenced, but only used for internal bookkeeping and
//! checking (via assertions) the correct operation of the QXK kernel.
//! Such pointer casting is not compliant with MISRA C++ Rule 5-2-7
//! as well as other messages (e.g., PC-Lint-Plus warning 826).
//! Defining this specific macro for this purpose allows to selectively
//! disable the warnings for this particular case.
//!
#define QXK_PTR_CAST_(type_, ptr_) (reinterpret_cast<type_>(ptr_))

#include "qf_pkg.hpp"  // QF package-scope interface

#endif // QXK_PKG_HPP
