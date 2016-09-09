/// @file
/// @brief QS floating point output implementation
/// @ingroup qs
/// @cond
///***************************************************************************
/// Product: QS/C++
/// Last updated for version 5.7.0
/// Last updated on  2016-09-08
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QF/QK implementation
#include "qs_port.h"      // QS port
#include "qs_pkg.h"       // QS package-scope internal interface

namespace QP {

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::f32(uint8_t format, float32_t const d) {
    union F32Rep {
        float32_t f;
        uint32_t  u;
    } fu32; // the internal binary representation
    uint8_t chksum_ = priv_.chksum;  // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;     // put in a temporary (register)
    QSCtr   head_   = priv_.head;    // put in a temporary (register)
    QSCtr   end_    = priv_.end;     // put in a temporary (register)

    fu32.f = d; // assign the binary representation

    priv_.used += static_cast<QSCtr>(5); // 5 bytes about to be added
    QS_INSERT_ESC_BYTE(format)  // insert the format byte

    for (int_t i = static_cast<int_t>(4); i != static_cast<int_t>(0); --i) {
        format = static_cast<uint8_t>(fu32.u);
        QS_INSERT_ESC_BYTE(format)
        fu32.u >>= 8;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::f64(uint8_t format, float64_t const d) {
    union F64Rep {
        float64_t d;
        struct UInt2 {
            uint32_t u1;
            uint32_t u2;
        } i;
    } fu64;  // the internal binary representation
    uint8_t chksum_ = priv_.chksum;
    uint8_t *buf_   = priv_.buf;
    QSCtr   head_   = priv_.head;
    QSCtr   end_    = priv_.end;
    uint32_t i;
    // static constant untion to detect endianness of the machine
    static union U32Rep {
        uint32_t u32;
        uint8_t  u8;
    } const endian = { static_cast<uint32_t>(1) };

    fu64.d = d;  // assign the binary representation

    priv_.used += static_cast<QSCtr>(9); // 9 bytes about to be added
    QS_INSERT_ESC_BYTE(format)  // insert the format byte

    // is this a big-endian machine?
    if (endian.u8 == static_cast<uint8_t>(0)) {
        // swap fu64.i.u1 <-> fu64.i.u2...
        i = fu64.i.u1;
        fu64.i.u1 = fu64.i.u2;
        fu64.i.u2 = i;
    }

    // output 4 bytes from fu64.i.u1 ...
    for (i = static_cast<uint32_t>(4); i != static_cast<uint32_t>(0); --i) {
        format = static_cast<uint8_t>(fu64.i.u1);
        QS_INSERT_ESC_BYTE(format)
        fu64.i.u1 >>= 8;
    }

    // output 4 bytes from fu64.i.u2 ...
    for (i = static_cast<uint32_t>(4); i != static_cast<uint32_t>(0); --i) {
        format = static_cast<uint8_t>(fu64.i.u2);
        QS_INSERT_ESC_BYTE(format)
        fu64.i.u2 >>= 8;
    }

    priv_.head   = head_;   // update the head
    priv_.chksum = chksum_; // update the checksum
}

} // namespace QP

