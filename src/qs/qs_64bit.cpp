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
//! @brief QS long-long (64-bit) output

#define QP_IMPL           // this is QF/QK implementation
#include "qs_port.hpp"    // QS port
#include "qs_pkg.hpp"     // QS package-scope internal interface

namespace QP {

//============================================================================
//! @note This function is only to be used through macros, never in the
//! client code directly.
//!
void QS::u64_raw_(std::uint64_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;
    std::uint8_t * const buf_ = priv_.buf;
    QSCtr head_      = priv_.head;
    QSCtr const end_ = priv_.end;

    priv_.used += 8U; // 8 bytes are about to be added
    for (std::int_fast8_t i = 8U; i != 0U; --i) {
        std::uint8_t const b = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(b)
        d >>= 8U;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//============================================================================
//! @note This function is only to be used through macros, never in the
//! client code directly.
//!
void QS::u64_fmt_(std::uint8_t format, std::uint64_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;
    std::uint8_t * const buf_ = priv_.buf;
    QSCtr head_      = priv_.head;
    QSCtr const end_ = priv_.end;

    priv_.used += static_cast<QSCtr>(9); // 9 bytes are about to be added
    QS_INSERT_ESC_BYTE_(format)  // insert the format byte

    for (std::int_fast8_t i = 8U; i != 0U; --i) {
        format = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(format)
        d >>= 8U;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

} // namespace QP
