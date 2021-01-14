/// @file
/// @brief QS floating point output implementation
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.9.2
/// Last updated on  2021-01-13
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
void QS::f32_fmt_(std::uint8_t format, float32_t const d) noexcept {
    union F32Rep {
        float32_t      f;
        std::uint32_t  u;
    } fu32; // the internal binary representation
    std::uint8_t chksum_  = priv_.chksum;  // put in a temporary (register)
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_    = priv_.head;    // put in a temporary (register)
    QSCtr const end_ = priv_.end;     // put in a temporary (register)

    fu32.f = d; // assign the binary representation

    priv_.used += 5U; // 5 bytes about to be added
    QS_INSERT_ESC_BYTE_(format)  // insert the format byte

    for (std::uint_fast8_t i = 4U; i != 0U; --i) {
        format = static_cast<std::uint8_t>(fu32.u);
        QS_INSERT_ESC_BYTE_(format)
        fu32.u >>= 8U;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::f64_fmt_(std::uint8_t format, float64_t const d) noexcept {
    union F64Rep {
        float64_t     d;
        std::uint32_t u[2];
    } fu64;  // the internal binary representation
    std::uint8_t chksum_  = priv_.chksum;
    std::uint8_t * const buf_ = priv_.buf;
    QSCtr   head_    = priv_.head;
    QSCtr const end_ = priv_.end;
    std::uint32_t i;
    // static constant untion to detect endianness of the machine
    static union U32Rep {
        std::uint32_t u32;
        std::uint8_t  u8;
    } const endian = { 1U };

    fu64.d = d;  // assign the binary representation

    // is this a big-endian machine?
    if (endian.u8 == 0U) {
        // swap fu64.u[0] <-> fu64.u[1]...
        i = fu64.u[0];
        fu64.u[0] = fu64.u[1];
        fu64.u[1] = i;
    }

    priv_.used += 9U; // 9 bytes about to be added
    QS_INSERT_ESC_BYTE_(format)  // insert the format byte

    // output 4 bytes from fu64.u[0]...
    for (i = 4U; i != 0U; --i) {
        QS_INSERT_ESC_BYTE_(static_cast<std::uint8_t>(fu64.u[0]))
        fu64.u[0] >>= 8U;
    }

    // output 4 bytes from fu64.u[1]...
    for (i = 4U; i != 0U; --i) {
        QS_INSERT_ESC_BYTE_(static_cast<std::uint8_t>(fu64.u[1]))
        fu64.u[1] >>= 8U;
    }

    priv_.head   = head_;   // update the head
    priv_.chksum = chksum_; // update the checksum
}

} // namespace QP

