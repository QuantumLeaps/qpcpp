/// \file
/// \brief QP::QS::str() and QP::QS::str_ROM() implementation
/// \ingroup qs
/// \cond
///***************************************************************************
/// Product: QS/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-10
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
/// \endcond

#define QP_IMPL           // this is QF/QK implementation
#include "qs_port.h"      // QS port
#include "qs_pkg.h"       // QS package-scope internal interface

namespace QP {

//****************************************************************************
/// \note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::str(char_t const *s) {
    uint8_t b       = static_cast<uint8_t>(*s);
    uint8_t chksum_ = static_cast<uint8_t>(
                          priv_.chksum + static_cast<uint8_t>(STR_T));
    uint8_t *buf_   = priv_.buf;  // put in a temporary (register)
    QSCtr   head_   = priv_.head; // put in a temporary (register)
    QSCtr   end_    = priv_.end;  // put in a temporary (register)
    QSCtr   used_   = priv_.used; // put in a temporary (register)

    used_ += static_cast<QSCtr>(2); // the format byte and the terminating-0

    QS_INSERT_BYTE(static_cast<uint8_t>(STR_T))
    while (b != static_cast<uint8_t>(0)) {
        // ASCII characters don't need escaping
        chksum_ += b;  // update checksum
        QS_INSERT_BYTE(b)
        QS_PTR_INC_(s);
        b = static_cast<uint8_t>(*s);
        ++used_;
    }
    QS_INSERT_BYTE(static_cast<uint8_t>(0)) // zero-terminate the string

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
    priv_.used   = used_;   // save # of used buffer space
}

//****************************************************************************
/// \note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::str_ROM(char_t const Q_ROM *s) {
    uint8_t b       = static_cast<uint8_t>(Q_ROM_BYTE(*s));
    uint8_t chksum_ = static_cast<uint8_t>(
                          priv_.chksum + static_cast<uint8_t>(STR_T));
    uint8_t *buf_   = priv_.buf;  // put in a temporary (register)
    QSCtr   head_   = priv_.head; // put in a temporary (register)
    QSCtr   end_    = priv_.end;  // put in a temporary (register)
    QSCtr   used_   = priv_.used; // put in a temporary (register)

    used_ += static_cast<QSCtr>(2); // the format byte and the terminating-0

    QS_INSERT_BYTE(static_cast<uint8_t>(STR_T))
    while (b != static_cast<uint8_t>(0)) {
        // ASCII characters don't need escaping
        chksum_ += b;     // update checksum
        QS_INSERT_BYTE(b)
        QS_PTR_INC_(s);
        b = static_cast<uint8_t>(Q_ROM_BYTE(*s));
        ++used_;
    }
    QS_INSERT_BYTE(static_cast<uint8_t>(0)) // zero-terminate the string

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
    priv_.used   = used_;   // save # of used buffer space
}

} // namespace QP

