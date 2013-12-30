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
#include "qassert.h"

/// \file
/// \ingroup qs
/// \brief QS internal variables definitions and core QS functions
/// implementations.

namespace QP {

Q_DEFINE_THIS_MODULE("qs")

//............................................................................
QS QS::priv_;                                               // QS private data

//............................................................................
void QS::initBuf(uint8_t sto[], uint_t const stoSize) {
    uint8_t *buf_ = &sto[0];

    Q_REQUIRE(stoSize > static_cast<uint_t>(8));    // at least 8 bytes in buf

    QF::bzero(&priv_, static_cast<uint_t>(sizeof(priv_)));       // see NOTE01
    priv_.buf  = buf_;
    priv_.end  = static_cast<QSCtr>(stoSize);

    beginRec(QS_REC_NUM_(QS_EMPTY));
    endRec();
    beginRec(QS_REC_NUM_(QS_QP_RESET));
    endRec();
}
//............................................................................
void QS::filterOn(uint8_t const rec) {
    if (rec == QS_ALL_RECORDS) {
        uint_t i;
        for (i = static_cast<uint_t>(0);
             i < static_cast<uint_t>(sizeof(priv_.glbFilter) - 1U);
             ++i)
        {
            priv_.glbFilter[i] = static_cast<uint8_t>(0xFF);
        }
        // never turn the last 3 records on (0x7D, 0x7E, 0x7F)
        priv_.glbFilter[sizeof(priv_.glbFilter) - 1U] =
            static_cast<uint8_t>(0x1F);
    }
    else {
        Q_ASSERT(rec < QS_ESC);  // so that record numbers don't need escaping
        priv_.glbFilter[rec >> 3] |=
            static_cast<uint8_t>(1U << (rec & static_cast<uint8_t>(7)));
    }
}
//............................................................................
void QS::filterOff(uint8_t const rec) {
    if (rec == QS_ALL_RECORDS) {
        // the following unrolled loop is designed to stop collecting trace
        // very fast in order to prevent overwriting the interesting data.
        // The code assumes that the size of priv_.glbFilter[] is 16.
        Q_ASSERT_COMPILE(sizeof(priv_.glbFilter) == 16U);

        uint8_t *glbFilter_ = &priv_.glbFilter[0];
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);

        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);

        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);

        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);  QS_PTR_INC_(glbFilter_);
        *glbFilter_ = static_cast<uint8_t>(0);
    }
    else {
        Q_ASSERT(rec < QS_ESC);  // so that record numbers don't need escaping
        priv_.glbFilter[rec >> 3] &= static_cast<uint8_t>(
            ~static_cast<uint8_t>(1U << (rec & static_cast<uint8_t>(7))));
    }
}
//............................................................................
void QS::beginRec(uint8_t const rec) {
    uint8_t b = static_cast<uint8_t>(priv_.seq + static_cast<uint8_t>(1));
    uint8_t chksum_ = static_cast<uint8_t>(0);           // reset the checksum
    uint8_t *buf_   = priv_.buf;              // put in a temporary (register)
    QSCtr   head_   = priv_.head;             // put in a temporary (register)
    QSCtr   end_    = priv_.end;              // put in a temporary (register)

    priv_.seq = b;                       // store the incremented sequence num
    priv_.used += static_cast<QSCtr>(2);          // 2 bytes about to be added

    QS_INSERT_ESC_BYTE(b)

    chksum_ = static_cast<uint8_t>(chksum_ + rec);          // update checksum
    QS_INSERT_BYTE(rec)                     // rec byte does not need escaping

    priv_.head   = head_;                                     // save the head
    priv_.chksum = chksum_;                               // save the checksum
}
//............................................................................
void QS::endRec(void) {
    uint8_t b = static_cast<uint8_t>(~priv_.chksum);
    uint8_t *buf_ = priv_.buf;                // put in a temporary (register)
    QSCtr   head_ = priv_.head;
    QSCtr   end_  = priv_.end;

    priv_.used += static_cast<QSCtr>(2);         // 2 bytes about to be added

    if ((b != QS_FRAME) && (b != QS_ESC)) {
        QS_INSERT_BYTE(b)
    }
    else {
        QS_INSERT_BYTE(QS_ESC)
        QS_INSERT_BYTE(b ^ QS_ESC_XOR)
        ++priv_.used;                              // account for the ESC byte
    }

    QS_INSERT_BYTE(QS_FRAME)                    // do not escape this QS_FRAME

    priv_.head = head_;                                       // save the head
    if (priv_.used > end_) {                     // overrun over the old data?
        priv_.used = end_;                         // the whole buffer is used
        priv_.tail = head_;                  // shift the tail to the old data
    }
}
//............................................................................
void QS::u8(uint8_t const format, uint8_t const d) {
    uint8_t chksum_ = priv_.chksum;           // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;              // put in a temporary (register)
    QSCtr   head_   = priv_.head;             // put in a temporary (register)
    QSCtr   end_    = priv_.end;              // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(2);          // 2 bytes about to be added

    QS_INSERT_ESC_BYTE(format)
    QS_INSERT_ESC_BYTE(d)

    priv_.head   = head_;                                     // save the head
    priv_.chksum = chksum_;                               // save the checksum
}
//............................................................................
void QS::u16(uint8_t format, uint16_t d) {
    uint8_t chksum_ = priv_.chksum;           // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;              // put in a temporary (register)
    QSCtr   head_   = priv_.head;             // put in a temporary (register)
    QSCtr   end_    = priv_.end;              // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(3);          // 3 bytes about to be added

    QS_INSERT_ESC_BYTE(format)

    format = static_cast<uint8_t>(d);
    QS_INSERT_ESC_BYTE(format)

    d >>= 8;
    format = static_cast<uint8_t>(d);
    QS_INSERT_ESC_BYTE(format)

    priv_.head   = head_;                                     // save the head
    priv_.chksum = chksum_;                               // save the checksum
}
//............................................................................
void QS::u32(uint8_t format, uint32_t d) {
    uint8_t chksum_ = priv_.chksum;           // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;              // put in a temporary (register)
    QSCtr   head_   = priv_.head;             // put in a temporary (register)
    QSCtr   end_    = priv_.end;              // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(5);          // 5 bytes about to be added
    QS_INSERT_ESC_BYTE(format)                       // insert the format byte

    for (int_t i = static_cast<int_t>(4); i != static_cast<int_t>(0); --i) {
        format = static_cast<uint8_t>(d);
        QS_INSERT_ESC_BYTE(format)
        d >>= 8;
    }

    priv_.head   = head_;                                     // save the head
    priv_.chksum = chksum_;                               // save the checksum
}

}                                                              // namespace QP

//****************************************************************************
// NOTE01:
// The QS::initBuf() function clears the internal QS variables, so that the
// tracing can start correctly even if the startup code fails to clear
// the uninitialized data (as is required by the C Standard).
//


