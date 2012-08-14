//////////////////////////////////////////////////////////////////////////////
// Product: QS/C++
// Last Updated for Version: 4.4.00
// Date of the Last Update:  Mar 28, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
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
#include "qs_pkg.h"

/// \file
/// \ingroup qs
/// \brief QS functions for internal use inside QP components

QP_BEGIN_

//............................................................................
void const *QS::smObj_;                  // local state machine for QEP filter
void const *QS::aoObj_;                   // local active object for QF filter
void const *QS::mpObj_;                     //  local event pool for QF filter
void const *QS::eqObj_;                      //  local raw queue for QF filter
void const *QS::teObj_;                     //  local time event for QF filter
void const *QS::apObj_;                    //  local object Application filter

QSTimeCtr QS::tickCtr_;              // tick counter for the QS_QF_TICK record

//............................................................................
void QS::u8_(uint8_t const d) {
    QS_INSERT_ESC_BYTE(d)
}
//............................................................................
void QS::u16_(uint16_t d) {
    QS_INSERT_ESC_BYTE(static_cast<uint8_t>(d))
    d >>= 8;
    QS_INSERT_ESC_BYTE(static_cast<uint8_t>(d))
}
//............................................................................
void QS::u32_(uint32_t d) {
    QS_INSERT_ESC_BYTE(static_cast<uint8_t>(d))
    d >>= 8;
    QS_INSERT_ESC_BYTE(static_cast<uint8_t>(d))
    d >>= 8;
    QS_INSERT_ESC_BYTE(static_cast<uint8_t>(d))
    d >>= 8;
    QS_INSERT_ESC_BYTE(static_cast<uint8_t>(d))
}
//............................................................................
void QS::str_(char_t const *s) {
    uint8_t b = static_cast<uint8_t>(*s);
    while (b != static_cast<uint8_t>(0)) {
                                       // ASCII characters don't need escaping
        QS_chksum_ = static_cast<uint8_t>(QS_chksum_ + b);
        QS_INSERT_BYTE(b)
        ++s;
        b = static_cast<uint8_t>(*s);
    }
    QS_INSERT_BYTE(static_cast<uint8_t>(0))
}
//............................................................................
void QS::str_ROM_(char_t const Q_ROM * Q_ROM_VAR s) {
    uint8_t b = static_cast<uint8_t>(Q_ROM_BYTE(*s));
    while (b != static_cast<uint8_t>(0)) {
                                       // ASCII characters don't need escaping
        QS_chksum_ = static_cast<uint8_t>(QS_chksum_ + b);
        QS_INSERT_BYTE(b)
        ++s;
        b = static_cast<uint8_t>(Q_ROM_BYTE(*s));
    }
    QS_INSERT_BYTE(static_cast<uint8_t>(0))
}

QP_END_
