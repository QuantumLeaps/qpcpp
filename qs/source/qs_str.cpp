//****************************************************************************
// Product: QS/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 02, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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
//****************************************************************************
#include "qs_pkg.h"

/// \file
/// \ingroup qs
/// \brief QS::str() and QS::str_ROM() implementation

namespace QP {

//............................................................................
void QS::str(char_t const *s) {
    uint8_t b       = static_cast<uint8_t>(*s);
    uint8_t chksum_ = static_cast<uint8_t>(
                          priv_.chksum + static_cast<uint8_t>(STR_T));
    uint8_t *buf_   = priv_.buf;              // put in a temporary (register)
    QSCtr   head_   = priv_.head;             // put in a temporary (register)
    QSCtr   end_    = priv_.end;              // put in a temporary (register)
    QSCtr   used_   = priv_.used;             // put in a temporary (register)

    used_ += static_cast<QSCtr>(2);   // the format byte and the terminating-0

    QS_INSERT_BYTE(static_cast<uint8_t>(STR_T))
    while (b != static_cast<uint8_t>(0)) {
                                       // ASCII characters don't need escaping
        chksum_ += b;                                       // update checksum
        QS_INSERT_BYTE(b)
        QS_PTR_INC_(s);
        b = static_cast<uint8_t>(*s);
        ++used_;
    }
    QS_INSERT_BYTE(static_cast<uint8_t>(0))       // zero-terminate the string

    priv_.head   = head_;                                     // save the head
    priv_.chksum = chksum_;                               // save the checksum
    priv_.used   = used_;                       // save # of used buffer space
}
//............................................................................
void QS::str_ROM(char_t const Q_ROM *s) {
    uint8_t b       = static_cast<uint8_t>(Q_ROM_BYTE(*s));
    uint8_t chksum_ = static_cast<uint8_t>(
                          priv_.chksum + static_cast<uint8_t>(STR_T));
    uint8_t *buf_   = priv_.buf;              // put in a temporary (register)
    QSCtr   head_   = priv_.head;             // put in a temporary (register)
    QSCtr   end_    = priv_.end;              // put in a temporary (register)
    QSCtr   used_   = priv_.used;             // put in a temporary (register)

    used_ += static_cast<QSCtr>(2);   // the format byte and the terminating-0

    QS_INSERT_BYTE(static_cast<uint8_t>(STR_T))
    while (b != static_cast<uint8_t>(0)) {
                                       // ASCII characters don't need escaping
        chksum_ += b;                                       // update checksum
        QS_INSERT_BYTE(b)
        QS_PTR_INC_(s);
        b = static_cast<uint8_t>(Q_ROM_BYTE(*s));
        ++used_;
    }
    QS_INSERT_BYTE(static_cast<uint8_t>(0))       // zero-terminate the string

    priv_.head   = head_;                                     // save the head
    priv_.chksum = chksum_;                               // save the checksum
    priv_.used   = used_;                       // save # of used buffer space
}

}                                                              // namespace QP

