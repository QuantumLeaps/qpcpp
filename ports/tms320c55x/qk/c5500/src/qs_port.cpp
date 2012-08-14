//////////////////////////////////////////////////////////////////////////////
// Product: QS/C++ port TMS320C55x, TI-C5500 compiler
// Last Updated for Version: 4.4.00
// Date of the Last Update:  Apr 19, 2012
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
#include "qassert.h"

//Q_DEFINE_THIS_MODULE(qs_port)


#undef  QS_INSERT_BYTE
#define QS_INSERT_BYTE(b_) \
    QS_ring_[QS_head_] = (b_) & 0xFFU; \
    if (++QS_head_ == QS_end_) { \
        QS_head_ = (QSCtr)0; \
    } \
    ++QS_used_;

#undef  QS_INSERT_ESC_BYTE
#define QS_INSERT_ESC_BYTE(b_) \
    QS_chksum_ = (uint8_t)(QS_chksum_ + (b_)); \
    if (((b_) == QS_FRAME) || ((b_) == QS_ESC)) { \
        QS_INSERT_BYTE(QS_ESC) \
        (b_) ^= QS_ESC_XOR; \
    } \
    QS_INSERT_BYTE(b_)



// from qs.cpp ===============================================================
uint8_t QS::glbFilter_[32];                                // global QS filter

//............................................................................
uint8_t *QS_ring_;                  // pointer to the start of the ring buffer
QSCtr QS_end_;                         // offset of the end of the ring buffer
QSCtr QS_head_;                  // offset to where next byte will be inserted
QSCtr QS_tail_;                 // offset of where next byte will be extracted
QSCtr QS_used_;                // number of bytes currently in the ring buffer
uint8_t QS_seq_;                                 // the record sequence number
uint8_t QS_chksum_;                      // the checksum of the current record
uint8_t QS_full_;                       // the ring buffer is temporarily full

//............................................................................
void QS::initBuf(uint8_t sto[], uint32_t stoSize) {
    QS_ring_ = &sto[0];
    QS_end_  = (QSCtr)stoSize;
}

//............................................................................
void QS_zero(void) {                                             // see NOTE01
    QS_head_   = (QSCtr)0;
    QS_tail_   = (QSCtr)0;
    QS_used_   = (QSCtr)0;
    QS_seq_    = (uint8_t)0;
    QS_chksum_ = (uint8_t)0;
    QS_full_   = (uint8_t)0;

    QS::smObj_ = (void *)0;
    QS::aoObj_ = (void *)0;
    QS::mpObj_ = (void *)0;
    QS::eqObj_ = (void *)0;
    QS::teObj_ = (void *)0;
    QS::apObj_ = (void *)0;

    bzero(QS::glbFilter_, sizeof(QS::glbFilter_));
}

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The standard TI startup code (c_int00) does NOT zero the uninitialized
// variables, as required by the C-standard. Since QF relies on the clearing
// of the static uninitialized variables, the critical QF objects are cleared
// explicitly in this port.
//

//............................................................................
void QS::filterOn(uint8_t rec) {
    uint8_t i;
    if (rec == QS_ALL_RECORDS) {
        for (i = (uint8_t)0; i < (uint8_t)sizeof(glbFilter_); ++i) {
            glbFilter_[i] = 0xFFU;
        }
    }
    else {
        glbFilter_[rec >> 3] |= (uint8_t)(1U << (rec & 0x07U));
    }
}
//............................................................................
void QS::filterOff(uint8_t rec) {
    uint8_t i;
    if (rec == QS_ALL_RECORDS) {
        for (i = (uint8_t)0; i < (uint8_t)sizeof(glbFilter_); ++i) {
            glbFilter_[i] = (uint8_t)0;
        }
    }
    else {
        glbFilter_[rec >> 3] &= (uint8_t)(~(1U << (rec & 0x07U)));
    }
}
//............................................................................
void QS::begin(uint8_t rec) {
    uint8_t b;
    QS_chksum_ = (uint8_t)0;                             // clear the checksum
    ++QS_seq_;                                   // increment the sequence num
    b = QS_seq_ & 0xFFU;
    QS_INSERT_ESC_BYTE(b)                         // store the sequence number
    QS_INSERT_ESC_BYTE(rec)                             // store the record ID
}
//............................................................................
void QS::end(void) {
    QS_chksum_ = ~QS_chksum_ & 0xFFU;
    if ((QS_chksum_ == QS_FRAME) || (QS_chksum_ == QS_ESC)) {
        QS_INSERT_BYTE(QS_ESC)
        QS_chksum_ ^= QS_ESC_XOR;
    }
    QS_INSERT_BYTE(QS_chksum_)
    QS_INSERT_BYTE(QS_FRAME)
    if (QS_used_ > QS_end_) {                    // overrun over the old data?
        QS_tail_ = QS_head_;                 // shift the tail to the old data
        QS_used_ = QS_end_;                        // the whole buffer is used
    }
}
//............................................................................
void QS::u8(uint8_t format, uint8_t d) {
    QS_INSERT_ESC_BYTE(format)
    d &= 0xFFU;
    QS_INSERT_ESC_BYTE(d)
}
//............................................................................
void QS::u16(uint8_t format, uint16_t d) {
    uint8_t b;
    QS_INSERT_ESC_BYTE(format)
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    b = (uint8_t)((d >> 8) & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
}
//............................................................................
void QS::u32(uint8_t format, uint32_t d) {
    uint8_t b;
    QS_INSERT_ESC_BYTE(format)
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    d >>= 8;
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    d >>= 8;
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    b = (uint8_t)((d >> 8) & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
}

// from qs_.cpp ==============================================================
void const *QS::smObj_;                      // local state machine QEP filter
void const *QS::aoObj_;                       // local active object QF filter
void const *QS::mpObj_;                          // local event pool QF filter
void const *QS::eqObj_;                           // local raw queue QF filter
void const *QS::teObj_;                          // local time event QF filter
void const *QS::apObj_;                     // local object Application filter

QSTimeCtr volatile QS::tickCtr_;     // tick counter for the QS_QF_TICK record

//............................................................................
void QS::u8_(uint8_t d) {
    d &= 0xFFU;
    QS_INSERT_ESC_BYTE(d)
}
//............................................................................
void QS::u16_(uint16_t d) {
    uint8_t b;
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    b = (uint8_t)((d >> 8) & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
}
//............................................................................
void QS::u32_(uint32_t d) {
    uint8_t b;
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    d >>= 8;
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    d >>= 8;
    b = (uint8_t)(d & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    b = (uint8_t)((d >> 8) & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_(char const *s) {
    while (*s != '\0') {
                                       // ASCII characters don't need escaping
        QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)*s);
        QS_INSERT_BYTE((uint8_t)*s)
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_ROM_(char const Q_ROM * Q_ROM_VAR s) {
    uint8_t b;
    while ((b = (uint8_t)Q_ROM_BYTE(*s)) != (uint8_t)0) {
                                      // ASCII characters don't need escaping
        QS_chksum_ = (uint8_t)(QS_chksum_ + b);
        QS_INSERT_BYTE(b)
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}


// from qs_byte.cpp ==========================================================
uint16_t QS::getByte(void) {
    uint8_t byte;
    if (QS_used_ == (QSCtr)0) {
        return QS_EOD;                                   // return End-Of-Data
    }
    byte = QS_ring_[QS_tail_] & 0xFFU;               // set the byte to return
    ++QS_tail_;                                            // advance the tail
    if (QS_tail_ == QS_end_) {                            // tail wrap around?
        QS_tail_ = (QSCtr)0;
    }
    --QS_used_;                                          // one less byte used
    return byte;                                            // return the byte
}


// from qs_mem.cpp ===========================================================
void QS::mem(uint8_t const *blk, uint8_t size) {
    uint8_t b;
    QS_INSERT_BYTE((uint8_t)QS_MEM_T)
    QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)QS_MEM_T);
    QS_INSERT_ESC_BYTE(size)
    while (size-- != (uint8_t)0) {
        b = *blk & 0xFFU;
        QS_INSERT_ESC_BYTE(b)
        b = (*blk >> 8) & 0xFFU;
        QS_INSERT_ESC_BYTE(b)
        ++blk;
    }
}

// from qs_f32.cpp ===========================================================
void QS::f32(uint8_t format, float f) {
    uint8_t b;
    union F32Rep {
        float f;
        uint32_t u;
    } fu32;
    fu32.f = f;

    QS_INSERT_ESC_BYTE(format)
    b = (uint8_t)(fu32.u & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    fu32.u >>= 8;
    b = (uint8_t)(fu32.u & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    fu32.u >>= 8;
    b = (uint8_t)(fu32.u & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    b = (uint8_t)((fu32.u >> 8) & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
}


// from qs_f64.cpp ===========================================================
void QS::f64(uint8_t format, double d) {
    uint8_t b;
    union F64Rep {
        double d;
        struct UInt2 {
            uint32_t u1, u2;
        } i;
    } fu64;
    fu64.d = d;

    QS_INSERT_ESC_BYTE(format)

    b = (uint8_t)(fu64.i.u1 & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    fu64.i.u1 >>= 8;
    b = (uint8_t)(fu64.i.u1 & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    fu64.i.u1 >>= 8;
    b = (uint8_t)(fu64.i.u1 & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    b = (uint8_t)((fu64.i.u1 >> 8) & 0xFFU);
    QS_INSERT_ESC_BYTE(b)

    b = (uint8_t)(fu64.i.u2 & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    fu64.i.u2 >>= 8;
    b = (uint8_t)(fu64.i.u2 & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    fu64.i.u2 >>= 8;
    b = (uint8_t)(fu64.i.u2 & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
    b = (uint8_t)((fu64.i.u2 >> 8) & 0xFFU);
    QS_INSERT_ESC_BYTE(b)
}


// from qs_str.cpp ===========================================================
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str(char const *s) {
    QS_INSERT_BYTE((uint8_t)QS_STR_T)
    QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)QS_STR_T);
    while ((*s) != (char)'\0') {
                                       // ASCII characters don't need escaping
        QS_INSERT_BYTE((uint8_t)(*s))
        QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)(*s));
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_ROM(char const Q_ROM * Q_ROM_VAR s) {
    uint8_t b;
    QS_INSERT_BYTE((uint8_t)QS_STR_T)
    QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)QS_STR_T);
    while ((b = (uint8_t)Q_ROM_BYTE(*s)) != (uint8_t)0) {
                                       // ASCII characters don't need escaping
        QS_INSERT_BYTE(b)
        QS_chksum_ = (uint8_t)(QS_chksum_ + b);
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}
