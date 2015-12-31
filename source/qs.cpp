/// @file
/// @brief QS software tracing services
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 5.6.0
/// Last updated on  2015-12-26
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps. All rights reserved.
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qs_port.h"      // QS port
#include "qs_pkg.h"       // QS package-scope internal interface
#include "qassert.h"      // QP assertions

namespace QP {

Q_DEFINE_THIS_MODULE("qs")

//****************************************************************************
extern char_t const BUILD_DATE[12];
extern char_t const BUILD_TIME[9];

QS QS::priv_; // QS private data

//****************************************************************************
/// @description
/// This function should be called from QP::QS::onStartup() to provide QS with
/// the data buffer. The first argument @a sto[] is the address of the memory
/// block, and the second argument @a stoSize is the size of this block
/// in bytes. Currently the size of the QS buffer cannot exceed 64KB.
///
/// @note QS can work with quite small data buffers, but you will start losing
/// data if the buffer is too small for the bursts of tracing activity.
/// The right size of the buffer depends on the data production rate and
/// the data output rate. QS offers flexible filtering to reduce the data
/// production rate.
///
/// @note If the data output rate cannot keep up with the production rate,
/// QS will start overwriting the older data with newer data. This is
/// consistent with the "last-is-best" QS policy. The record sequence counters
/// and check sums on each record allow the QSPY host uitiliy to easily detect
/// any data loss.
///
void QS::initBuf(uint8_t sto[], uint_fast16_t const stoSize) {
    // the provided buffer must be at least 8 bytes long
    Q_REQUIRE_ID(100, stoSize > static_cast<uint_fast16_t>(8));

    // This function initializes all the internal QS variables, so that the
    // tracing can start correctly even if the startup code fails to clear
    // any uninitialized data (as is required by the C Standard).
    //
    filterOff(QS_ALL_RECORDS); // disable all maskable filters

    priv_.smObjFilter = static_cast<void *>(0);
    priv_.aoObjFilter = static_cast<void *>(0);
    priv_.mpObjFilter = static_cast<void *>(0);
    priv_.eqObjFilter = static_cast<void *>(0);
    priv_.teObjFilter = static_cast<void *>(0);
    priv_.apObjFilter = static_cast<void *>(0);

    priv_.buf      = &sto[0];
    priv_.end      = static_cast<QSCtr>(stoSize);
    priv_.head     = static_cast<QSCtr>(0);
    priv_.tail     = static_cast<QSCtr>(0);
    priv_.used     = static_cast<QSCtr>(0);
    priv_.seq      = static_cast<uint8_t>(0);
    priv_.chksum   = static_cast<uint8_t>(0);
    priv_.critNest = static_cast<uint_fast8_t>(0);

    // produce an empty record to "flush" the QS trace buffer
    beginRec(QS_REC_NUM_(QS_EMPTY));
    endRec();

    // produce the Target info QS record
    QS_target_info_(static_cast<uint8_t>(0xFF));

    // wait with flushing after successfull initialization (see QS_INIT())
}

//****************************************************************************
/// @description
/// This function sets up the QS filter to enable the record type @a rec.
/// The argument #QS_ALL_RECORDS specifies to filter-in all records.
/// This function should be called indirectly through the macro QS_FILTER_ON.
///
/// @note Filtering based on the record-type is only the first layer of
/// filtering. The second layer is based on the object-type. Both filter
/// layers must be enabled for the QS record to be inserted in the QS buffer.
///
/// @sa QP::QS::filterOff(), QS_FILTER_SM_OBJ, QS_FILTER_AO_OBJ,
/// QS_FILTER_MP_OBJ, QS_FILTER_EQ_OBJ, and QS_FILTER_TE_OBJ.
///
void QS::filterOn(uint_fast8_t const rec) {
    if (rec == QS_ALL_RECORDS) {
        uint_fast8_t i;
        for (i = static_cast<uint_fast8_t>(0);
             i < static_cast<uint_fast8_t>(sizeof(priv_.glbFilter) - 1U);
             ++i)
        {
            priv_.glbFilter[i] = static_cast<uint8_t>(0xFF);
        }
        // never turn the last 3 records on (0x7D, 0x7E, 0x7F)
        priv_.glbFilter[sizeof(priv_.glbFilter) - 1U] =
            static_cast<uint8_t>(0x1F);
    }
    else {
        // record numbers can't exceed QS_ESC, so they don't need escaping
        Q_ASSERT_ID(210, rec < static_cast<uint_fast8_t>(QS_ESC));
        priv_.glbFilter[rec >> 3] |=
            static_cast<uint8_t>(1U << (rec & static_cast<uint_fast8_t>(7)));
    }
}

//****************************************************************************
/// @description
/// This function sets up the QS filter to disable the record type @a rec.
/// The argument #QS_ALL_RECORDS specifies to suppress all records.
/// This function should be called indirectly through the macro QS_FILTER_OFF.
///
/// @note Filtering records based on the record-type is only the first layer
/// of filtering. The second layer is based on the object-type. Both filter
/// layers must be enabled for the QS record to be inserted in the QS buffer.
///
void QS::filterOff(uint_fast8_t const rec) {
    uint8_t tmp;

    if (rec == QS_ALL_RECORDS) {
        // first clear all global filters
        for (tmp = static_cast<uint8_t>(
                        static_cast<uint8_t>(sizeof(priv_.glbFilter))
                                            - static_cast<uint8_t>(1));
             tmp > static_cast<uint8_t>(0);
             --tmp)
        {
            priv_.glbFilter[tmp] = static_cast<uint8_t>(0);
        }

        // next leave the specific filters enabled
        priv_.glbFilter[0] = static_cast<uint8_t>(0x01);
        priv_.glbFilter[7] = static_cast<uint8_t>(0xF0);
        priv_.glbFilter[8] = static_cast<uint8_t>(0x3F);
    }
    else {
        // record numbers can't exceed QS_ESC, so they don't need escaping
        Q_ASSERT_ID(310, rec < static_cast<uint_fast8_t>(QS_ESC));
        tmp = static_cast<uint8_t>(
                  1U << (rec & static_cast<uint_fast8_t>(7)));
        tmp ^= static_cast<uint8_t>(0xFF);
        priv_.glbFilter[rec >> 3] &= tmp;
    }
}

//****************************************************************************
/// @description
/// This function must be called at the beginning of each QS record.
/// This function should be called indirectly through the macro #QS_BEGIN,
/// or #QS_BEGIN_NOCRIT, depending if it's called in a normal code or from
/// a critical section.
///
void QS::beginRec(uint_fast8_t const rec) {
    uint8_t b = static_cast<uint8_t>(priv_.seq + static_cast<uint8_t>(1));
    uint8_t chksum_ = static_cast<uint8_t>(0); // reset the checksum
    uint8_t *buf_   = priv_.buf;   // put in a temporary (register)
    QSCtr   head_   = priv_.head;  // put in a temporary (register)
    QSCtr   end_    = priv_.end;   // put in a temporary (register)

    priv_.seq = b; // store the incremented sequence num
    priv_.used += static_cast<QSCtr>(2); // 2 bytes about to be added

    QS_INSERT_ESC_BYTE(b)

    chksum_ = static_cast<uint8_t>(chksum_ + static_cast<uint8_t>(rec));
    QS_INSERT_BYTE(static_cast<uint8_t>(rec)) // rec does not need escaping

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
}

//****************************************************************************
/// @description
/// This function must be called at the end of each QS record.
/// This function should be called indirectly through the macro #QS_END,
/// or #QS_END_NOCRIT, depending if it's called in a normal code or from
/// a critical section.
///
void QS::endRec(void) {
    uint8_t *buf_ = priv_.buf;  // put in a temporary (register)
    QSCtr   head_ = priv_.head;
    QSCtr   end_  = priv_.end;
    uint8_t b     = priv_.chksum;
    b ^= static_cast<uint8_t>(0xFF); // invert the bits in the checksum


    priv_.used += static_cast<QSCtr>(2); // 2 bytes about to be added

    if ((b != QS_FRAME) && (b != QS_ESC)) {
        QS_INSERT_BYTE(b)
    }
    else {
        QS_INSERT_BYTE(QS_ESC)
        QS_INSERT_BYTE(b ^ QS_ESC_XOR)
        ++priv_.used; // account for the ESC byte
    }

    QS_INSERT_BYTE(QS_FRAME) // do not escape this QS_FRAME

    priv_.head = head_; // save the head
    if (priv_.used > end_) { // overrun over the old data?
        priv_.used = end_;   // the whole buffer is used
        priv_.tail = head_;  // shift the tail to the old data
    }
}

//****************************************************************************
void QS_target_info_(uint8_t const isReset) {

    QS::beginRec(static_cast<uint_fast8_t>(QS_TARGET_INFO));
        QS_U8_(isReset);

        QS_U16_(QP_VERSION); // two-byte version number

        // send the object sizes...
        QS_U8_(static_cast<uint8_t>(Q_SIGNAL_SIZE)
               | static_cast<uint8_t>(
                     static_cast<uint8_t>(QF_EVENT_SIZ_SIZE) << 4));

#ifdef QF_EQUEUE_CTR_SIZE
        QS_U8_(static_cast<uint8_t>(QF_EQUEUE_CTR_SIZE)
               | static_cast<uint8_t>(
                     static_cast<uint8_t>(QF_TIMEEVT_CTR_SIZE) << 4));
#else
        QS_U8_(static_cast<uint8_t>(
                   static_cast<uint8_t>(QF_TIMEEVT_CTR_SIZE) << 4));
#endif // ifdef QF_EQUEUE_CTR_SIZE

#ifdef QF_MPOOL_CTR_SIZE
        QS_U8_(static_cast<uint8_t>(QF_MPOOL_SIZ_SIZE)
               | static_cast<uint8_t>(
                     static_cast<uint8_t>(QF_MPOOL_CTR_SIZE) << 4));
#else
        QS_U8_((uint8_t)0);
#endif // ifdef QF_MPOOL_CTR_SIZE

        QS_U8_(static_cast<uint8_t>(QS_OBJ_PTR_SIZE)
               | static_cast<uint8_t>(
                     static_cast<uint8_t>(QS_FUN_PTR_SIZE) << 4));
        QS_U8_(static_cast<uint8_t>(QS_TIME_SIZE));

        // send the limits...
        QS_U8_(static_cast<uint8_t>(QF_MAX_ACTIVE));
        QS_U8_(static_cast<uint8_t>(QF_MAX_EPOOL)
               | static_cast<uint8_t>(
                     static_cast<uint8_t>(QF_MAX_TICK_RATE) << 4));

        // send the build time in three bytes (sec, min, hour)...
        QS_U8_(static_cast<uint8_t>(
                   static_cast<uint8_t>(10)
                   *(static_cast<uint8_t>(BUILD_TIME[6])
                         - static_cast<uint8_t>('0')))
                + (static_cast<uint8_t>(BUILD_TIME[7])
                         - static_cast<uint8_t>('0')));
        QS_U8_(static_cast<uint8_t>(
                   static_cast<uint8_t>(10)
                   *(static_cast<uint8_t>(BUILD_TIME[3])
                         - static_cast<uint8_t>('0')))
                + (static_cast<uint8_t>(BUILD_TIME[4])
                         - static_cast<uint8_t>('0')));
        if (static_cast<uint8_t>(BUILD_TIME[0])
            == static_cast<uint8_t>(' '))
        {
            QS_U8_(static_cast<uint8_t>(BUILD_TIME[1])
                   - static_cast<uint8_t>('0'));
        }
        else {
            QS_U8_(static_cast<uint8_t>(
                static_cast<uint8_t>(10)*(
                    static_cast<uint8_t>(BUILD_TIME[0])
                        - static_cast<uint8_t>('0')))
                    + (static_cast<uint8_t>(BUILD_TIME[1])
                        - static_cast<uint8_t>('0')));
        }

        // send the build date in three bytes (day, month, year) ...
        if (static_cast<uint8_t>(BUILD_DATE[4])
            == static_cast<uint8_t>(' '))
        {
            QS_U8_(static_cast<uint8_t>(BUILD_DATE[5])
                   - static_cast<uint8_t>('0'));
        }
        else {
            QS_U8_(static_cast<uint8_t>(
                       static_cast<uint8_t>(10)*(
                           static_cast<uint8_t>(BUILD_DATE[4])
                               - static_cast<uint8_t>('0')))
                       + (static_cast<uint8_t>(BUILD_DATE[5])
                               - static_cast<uint8_t>('0')));
        }
        // convert the 3-letter month to a number 1-12 ...
        uint8_t b;
        switch (static_cast<int_t>(BUILD_DATE[0])
                + static_cast<int_t>(BUILD_DATE[1])
                + static_cast<int_t>(BUILD_DATE[2]))
        {
            case static_cast<int_t>('J')
                 + static_cast<int_t>('a')
                 + static_cast<int_t>('n'):
                b = static_cast<uint8_t>(1);
                break;
            case static_cast<int_t>('F')
                 + static_cast<int_t>('e')
                 + static_cast<int_t>('b'):
                b = static_cast<uint8_t>(2);
                break;
            case static_cast<int_t>('M')
                 + static_cast<int_t>('a')
                 + static_cast<int_t>('r'):
                b = static_cast<uint8_t>(3);
                break;
            case static_cast<int_t>('A')
                 + static_cast<int_t>('p')
                 + static_cast<int_t>('r'):
                b = static_cast<uint8_t>(4);
                break;
            case static_cast<int_t>('M')
                 + static_cast<int_t>('a')
                 + static_cast<int_t>('y'):
                b = static_cast<uint8_t>(5);
                break;
            case static_cast<int_t>('J')
                 + static_cast<int_t>('u')
                 + static_cast<int_t>('n'):
                b = static_cast<uint8_t>(6);
                break;
            case static_cast<int_t>('J')
                 + static_cast<int_t>('u')
                 + static_cast<int_t>('l'):
                b = static_cast<uint8_t>(7);
                break;
            case static_cast<int_t>('A')
                 + static_cast<int_t>('u')
                 + static_cast<int_t>('g'):
                b = static_cast<uint8_t>(8);
                break;
            case static_cast<int_t>('S')
                 + static_cast<int_t>('e')
                 + static_cast<int_t>('p'):
                b = static_cast<uint8_t>(9);
                break;
            case static_cast<int_t>('O')
                 + static_cast<int_t>('c')
                 + static_cast<int_t>('t'):
                b = static_cast<uint8_t>(10);
                break;
            case static_cast<int_t>('N')
                 + static_cast<int_t>('o')
                 + static_cast<int_t>('v'):
                b = static_cast<uint8_t>(11);
                break;
            case static_cast<int_t>('D')
                 + static_cast<int_t>('e')
                 + static_cast<int_t>('c'):
                b = static_cast<uint8_t>(12);
                break;
            default:
                b = static_cast<uint8_t>(0);
                break;
        }
        QS_U8_(b); // store the month
        QS_U8_(static_cast<uint8_t>(
                   static_cast<uint8_t>(10)*(
                       static_cast<uint8_t>(BUILD_DATE[9])
                           - static_cast<uint8_t>('0')))
                   + (static_cast<uint8_t>(BUILD_DATE[10])
                           - static_cast<uint8_t>('0')));
    QS::endRec();
}

//****************************************************************************
/// @description
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u8(uint8_t const format, uint8_t const d) {
    uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;    // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr   end_    = priv_.end;    // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(2); // 2 bytes about to be added

    QS_INSERT_ESC_BYTE(format)
    QS_INSERT_ESC_BYTE(d)

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
}

//****************************************************************************
/// @description
/// This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u16(uint8_t format, uint16_t d) {
    uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;    // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr   end_    = priv_.end;    // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(3); // 3 bytes about to be added

    QS_INSERT_ESC_BYTE(format)

    format = static_cast<uint8_t>(d);
    QS_INSERT_ESC_BYTE(format)

    d >>= 8;
    format = static_cast<uint8_t>(d);
    QS_INSERT_ESC_BYTE(format)

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u32(uint8_t format, uint32_t d) {
    uint8_t chksum_ = priv_.chksum;  // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;     // put in a temporary (register)
    QSCtr   head_   = priv_.head;    // put in a temporary (register)
    QSCtr   end_    = priv_.end;     // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(5); // 5 bytes about to be added
    QS_INSERT_ESC_BYTE(format) // insert the format byte

    for (int_t i = static_cast<int_t>(4); i != static_cast<int_t>(0); --i) {
        format = static_cast<uint8_t>(d);
        QS_INSERT_ESC_BYTE(format)
        d >>= 8;
    }

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u8_(uint8_t const d) {
    uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;    // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr   end_    = priv_.end;    // put in a temporary (register)

    ++priv_.used;  // 1 byte about to be added
    QS_INSERT_ESC_BYTE(d)

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u8u8_(uint8_t const d1, uint8_t const d2) {
    uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;    // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr   end_    = priv_.end;    // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(2); // 2 bytes about to be added
    QS_INSERT_ESC_BYTE(d1)
    QS_INSERT_ESC_BYTE(d2)

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u16_(uint16_t d) {
    uint8_t b = static_cast<uint8_t>(d);
    uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;    // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr   end_    = priv_.end;    // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(2); // 2 bytes about to be added

    QS_INSERT_ESC_BYTE(b)

    d >>= 8;
    b = static_cast<uint8_t>(d);
    QS_INSERT_ESC_BYTE(b)

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u32_(uint32_t d) {
    uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;    // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr   end_    = priv_.end;    // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(4); // 4 bytes about to be added
    for (int_t i = static_cast<int_t>(4); i != static_cast<int_t>(0); --i) {
        uint8_t b = static_cast<uint8_t>(d);
        QS_INSERT_ESC_BYTE(b)
        d >>= 8;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::str_(char_t const *s) {
    uint8_t b = static_cast<uint8_t>(*s);
    uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    uint8_t *buf_   = priv_.buf;    // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr   end_    = priv_.end;    // put in a temporary (register)
    QSCtr   used_   = priv_.used;   // put in a temporary (register)

    while (b != static_cast<uint8_t>(0)) {
        chksum_ += b;      // update checksum
        QS_INSERT_BYTE(b)  // ASCII characters don't need escaping
        QS_PTR_INC_(s);
        b = static_cast<uint8_t>(*s);
        ++used_;
    }
    QS_INSERT_BYTE(static_cast<uint8_t>(0)) // zero-terminate the string
    ++used_;

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
    priv_.used   = used_;    // save # of used buffer space
}

//****************************************************************************
/// @description
/// This function delivers one byte at a time from the QS data buffer.
///
/// @returns the byte in the least-significant 8-bits of the 16-bit return
/// value if the byte is available. If no more data is available at the time,
/// the function returns QP::QS_EOD (End-Of-Data).
///
/// @note QP::QS::getByte() is __not__ protected with a critical section.
///
uint16_t QS::getByte(void) {
    uint16_t ret;
    if (priv_.used == static_cast<QSCtr>(0)) {
        ret = QS_EOD; // set End-Of-Data
    }
    else {
        uint8_t *buf_ = priv_.buf;  // put in a temporary (register)
        QSCtr tail_   = priv_.tail; // put in a temporary (register)

        // the byte to return
        ret = static_cast<uint16_t>(QS_PTR_AT_(buf_, tail_));

        ++tail_;  // advance the tail
        if (tail_ == priv_.end) {  // tail wrap around?
            tail_ = static_cast<QSCtr>(0);
        }
        priv_.tail = tail_;  // update the tail
        --priv_.used;        // one less byte used
    }
    return ret;  // return the byte or EOD
}

//****************************************************************************/
/// @description
/// This function delivers a contiguous block of data from the QS data buffer.
/// The function returns the pointer to the beginning of the block, and writes
/// the number of bytes in the block to the location pointed to by @a pNbytes.
/// The argument @a pNbytes is also used as input to provide the maximum size
/// of the data block that the caller can accept.
///
/// @returns if data is available, the function returns pointer to the
/// contiguous block of data and sets the value pointed to by @p pNbytes
/// to the # available bytes. If data is available at the time the function is
/// called, the function returns NULL pointer and sets the value pointed to by
/// @p pNbytes to zero.
///
/// @note
/// Only the NULL return from QP::QS::getBlock() indicates that the QS buffer
/// is empty at the time of the call. The non-NULL return often means that
/// the block is at the end of the buffer and you need to call
/// QP::QS::getBlock() again to obtain the rest of the data that
/// "wrapped around" to the beginning of the QS data buffer.
///
/// @note QP::QS::getBlock() is __not__ protected with a critical section.
///
uint8_t const *QS::getBlock(uint16_t * const pNbytes) {
    QSCtr used_ = priv_.used;  // put in a temporary (register)
    uint8_t *buf_;

    // any bytes used in the ring buffer?
    if (used_ == static_cast<QSCtr>(0)) {
        *pNbytes = static_cast<uint16_t>(0);  // no bytes available right now
        buf_     = static_cast<uint8_t *>(0); // no bytes available right now
    }
    else {
        QSCtr tail_ = priv_.tail; // put in a temporary (register)
        QSCtr end_  = priv_.end;  // put in a temporary (register)
        QSCtr n = static_cast<QSCtr>(end_ - tail_);
        if (n > used_) {
            n = used_;
        }
        if (n > static_cast<QSCtr>(*pNbytes)) {
            n = static_cast<QSCtr>(*pNbytes);
        }
        *pNbytes = static_cast<uint16_t>(n); // n-bytes available
        buf_ = priv_.buf;
        buf_ = &QS_PTR_AT_(buf_, tail_); // the bytes are at the tail

        priv_.used -= n;
        tail_      += n;
        if (tail_ == end_) {
            tail_ = static_cast<QSCtr>(0);
        }
        priv_.tail = tail_;
    }
    return buf_;
}

//****************************************************************************
/// @note This function is only to be used through macro QS_SIG_DICTIONARY()
///
void QS::sig_dict(enum_t const sig, void const * const obj,
                  char_t const *name)
{
    QS_CRIT_STAT_

    if (*name == static_cast<char_t>('&')) {
        QS_PTR_INC_(name);
    }
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_SIG_DICT));
    QS_SIG_(static_cast<QSignal>(sig));
    QS_OBJ_(obj);
    QS_STR_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macro QS_OBJ_DICTIONARY()
///
void QS::obj_dict(void const * const obj,
                  char_t const *name)
{
    QS_CRIT_STAT_

    if (*name == static_cast<char_t>('&')) {
        QS_PTR_INC_(name);
    }
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_OBJ_DICT));
    QS_OBJ_(obj);
    QS_STR_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macro QS_FUN_DICTIONARY()
///
void QS::fun_dict(void (* const fun)(void), char_t const *name) {
    QS_CRIT_STAT_

    if (*name == static_cast<char_t>('&')) {
        QS_PTR_INC_(name);
    }
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_FUN_DICT));
    QS_FUN_(fun);
    QS_STR_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macro QS_USR_DICTIONARY()
///
void QS::usr_dict(enum_t const rec,
                  char_t const * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_USR_DICT));
    QS_U8_(static_cast<uint8_t>(rec));
    QS_STR_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
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

    // output the 'size' number of bytes
    while (size != static_cast<uint8_t>(0)) {
        b = *blk;
        QS_INSERT_ESC_BYTE(b)
        QS_PTR_INC_(blk);
        --size;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
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

} // namespace QP


