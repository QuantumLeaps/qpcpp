/// @file
/// @brief QS long-long (64-bit) output
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.9.2
/// Last updated on  2021-01-16
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QF/QK implementation
#include "qs_port.hpp"    // QS port
#include "qs_pkg.hpp"     // QS package-scope internal interface

namespace QP {

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u64_raw_(std::uint64_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;
    std::uint8_t *buf_   = priv_.buf;
    QSCtr   head_   = priv_.head;
    QSCtr   end_    = priv_.end;

    priv_.used += 8U; // 8 bytes are about to be added
    for (std::int_fast8_t i = 8U; i != 0U; --i) {
        std::uint8_t b = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(b)
        d >>= 8;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u64_fmt_(std::uint8_t format, std::uint64_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;
    std::uint8_t *buf_   = priv_.buf;
    QSCtr   head_   = priv_.head;
    QSCtr   end_    = priv_.end;

    priv_.used += static_cast<QSCtr>(9); // 9 bytes are about to be added
    QS_INSERT_ESC_BYTE_(format)  // insert the format byte

    for (std::int_fast8_t i = 8U; i != 0U; --i) {
        format = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(format)
        d >>= 8;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

} // namespace QP

