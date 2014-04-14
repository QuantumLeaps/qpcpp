/// \file
/// \brief QP::QS::mem() implementation
/// \ingroup qs
/// \cond
///***************************************************************************
/// Product: QS/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-02-27
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
void QS::mem(uint8_t const *blk, uint8_t size) {
    uint8_t b = static_cast<uint8_t>(MEM_T);
    uint8_t chksum_ = static_cast<uint8_t>(priv_.chksum + b);
    uint8_t *buf_   = priv_.buf;   // put in a temporary (register)
    QSCtr   head_   = priv_.head;  // put in a temporary (register)
    QSCtr   end_    = priv_.end;   // put in a temporary (register)

    priv_.used += (static_cast<QSCtr>(size) // size+2 bytes to be added
                   + static_cast<QSCtr>(2));

    QS_INSERT_BYTE(b)
    QS_INSERT_ESC_BYTE(size)
    while (size != static_cast<uint8_t>(0)) {
        b = *blk;
        QS_INSERT_ESC_BYTE(b)
        QS_PTR_INC_(blk);
        --size;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

} // namespace QP

