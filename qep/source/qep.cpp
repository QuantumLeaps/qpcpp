//////////////////////////////////////////////////////////////////////////////
// Product: QEP/C++
// Last Updated for Version: 4.5.04
// Date of the Last Update:  Feb 09, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qep_pkg.h"

/// \file
/// \ingroup qep
/// \brief ::QEP_reservedEvt_ definition and QEP::getVersion() implementation.

QP_BEGIN_

// Package-scope objects -----------------------------------------------------
QEvt const QEP_reservedEvt_[4] = {
#ifdef Q_EVT_CTOR                         // Is the QEvt constructor provided?
    static_cast<QSignal>(0),
    static_cast<QSignal>(1),
    static_cast<QSignal>(2),
    static_cast<QSignal>(3)
#else                                    // QEvt is a POD (Plain Old Datatype)
    { static_cast<QSignal>(0), u8_0, u8_0 },
    { static_cast<QSignal>(1), u8_0, u8_0 },
    { static_cast<QSignal>(2), u8_0, u8_0 },
    { static_cast<QSignal>(3), u8_0, u8_0 }
#endif
};
//............................................................................
char_t const Q_ROM * Q_ROM_VAR QEP::getVersion(void) {
    uint8_t const u8_zero = static_cast<uint8_t>('0');
    static char_t const Q_ROM Q_ROM_VAR version[] = {
        static_cast<char_t>(((QP_VERSION >> 12) & 0xFU) + u8_zero),
        static_cast<char_t>('.'),
        static_cast<char_t>(((QP_VERSION >>  8) & 0xFU) + u8_zero),
        static_cast<char_t>('.'),
        static_cast<char_t>(((QP_VERSION >>  4) & 0xFU) + u8_zero),
        static_cast<char_t>((QP_VERSION         & 0xFU) + u8_zero),
        static_cast<char_t>('\0')
    };
    return version;
}

QP_END_
