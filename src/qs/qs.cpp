/// @file
/// @brief QS software tracing services
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-01-20
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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

#define QP_IMPL           // this is QP implementation
#include "qs_port.hpp"    // QS port
#include "qs_pkg.hpp"     // QS package-scope internal interface
#include "qassert.h"      // QP assertions

namespace QP {

Q_DEFINE_THIS_MODULE("qs")

//****************************************************************************
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
void QS::initBuf(std::uint8_t * const sto,
                 std::uint_fast16_t const stoSize) noexcept
{
    // the provided buffer must be at least 8 bytes long
    Q_REQUIRE_ID(100, stoSize > 8U);

    // This function initializes all the internal QS variables, so that the
    // tracing can start correctly even if the startup code fails to clear
    // any uninitialized data (as is required by the C Standard).
    //
    QS_FILTER_OFF(QS_ALL_RECORDS); // disable all maskable filters

    priv_.locFilter[SM_OBJ] = nullptr;
    priv_.locFilter[AO_OBJ] = nullptr;
    priv_.locFilter[MP_OBJ] = nullptr;
    priv_.locFilter[EQ_OBJ] = nullptr;
    priv_.locFilter[TE_OBJ] = nullptr;
    priv_.locFilter[TE_OBJ] = nullptr;

    priv_.buf      = sto;
    priv_.end      = static_cast<QSCtr>(stoSize);
    priv_.head     = 0U;
    priv_.tail     = 0U;
    priv_.used     = 0U;
    priv_.seq      = 0U;
    priv_.chksum   = 0U;
    priv_.critNest = 0U;

    // produce an empty record to "flush" the QS trace buffer
    beginRec_(QS_REC_NUM_(QS_EMPTY));
    endRec_();

    // produce the Target info QS record
    QS_target_info_(0xFFU);

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
void QS::filterOn_(std::uint_fast8_t const rec) noexcept {
    if (rec == static_cast<std::uint_fast8_t>(QS_ALL_RECORDS)) {
        for (std::uint_fast8_t i = 0U; i < 15U; ++i) {
            priv_.glbFilter[i] = 0xFFU; // set all bits
        }
        // never turn the last 3 records on (0x7D, 0x7E, 0x7F)
        priv_.glbFilter[sizeof(priv_.glbFilter) - 1U] = 0x1FU;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_SM_RECORDS)) {
        priv_.glbFilter[0] |= 0xFEU;
        priv_.glbFilter[1] |= 0x03U;
        priv_.glbFilter[6] |= 0x80U;
        priv_.glbFilter[7] |= 0x03U;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_AO_RECORDS)) {
        priv_.glbFilter[1] |= 0xFCU;
        priv_.glbFilter[2] |= 0x07U;
        priv_.glbFilter[5] |= 0x20U;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_EQ_RECORDS)) {
        priv_.glbFilter[2] |= 0x78U;
        priv_.glbFilter[5] |= 0x40U;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_MP_RECORDS)) {
        priv_.glbFilter[3] |= 0x03U;
        priv_.glbFilter[5] |= 0x80U;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_QF_RECORDS)) {
        priv_.glbFilter[3] |= 0xFCU;
        priv_.glbFilter[4] |= 0xC0U;
        priv_.glbFilter[5] |= 0x1FU;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_TE_RECORDS)) {
        priv_.glbFilter[4] |= 0x7FU;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_SC_RECORDS)) {
        priv_.glbFilter[6] |= 0x7FU;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U0_RECORDS)) {
        priv_.glbFilter[12] |= 0xF0U;
        priv_.glbFilter[13] |= 0x01U;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U1_RECORDS)) {
        priv_.glbFilter[13] |= 0x1EU;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U2_RECORDS)) {
        priv_.glbFilter[13] |= 0xE0U;
        priv_.glbFilter[14] |= 0x03U;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U3_RECORDS)) {
        priv_.glbFilter[14] |= 0xF8U;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U4_RECORDS)) {
        priv_.glbFilter[15] |= 0x1FU;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_UA_RECORDS)) {
        priv_.glbFilter[12] |= 0xF0U;
        priv_.glbFilter[13] |= 0xFFU;
        priv_.glbFilter[14] |= 0xFFU;
        priv_.glbFilter[15] |= 0x1FU;
    }
    else {
        // record numbers can't exceed QS_ESC, so they don't need escaping
        Q_ASSERT_ID(210, rec < static_cast<std::uint_fast8_t>(QS_ESC));

        priv_.glbFilter[rec >> 3] |=
            static_cast<std::uint8_t>(1U << (rec & 7U));
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
void QS::filterOff_(std::uint_fast8_t const rec) noexcept {
    std::uint8_t tmp;

    if (rec == static_cast<std::uint_fast8_t>(QS_ALL_RECORDS)) {
        // first clear all global filters
        for (tmp = 15U; tmp > 0U; --tmp) {
            priv_.glbFilter[tmp] = 0U;
        }
        // next leave the specific filters enabled
        priv_.glbFilter[0] = 0x01U;
        priv_.glbFilter[7] = 0xFCU;
        priv_.glbFilter[8] = 0x3FU;
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_SM_RECORDS)) {
        priv_.glbFilter[0] &= static_cast<std::uint8_t>(~0xFEU);
        priv_.glbFilter[1] &= static_cast<std::uint8_t>(~0x03U);
        priv_.glbFilter[6] &= static_cast<std::uint8_t>(~0x80U);
        priv_.glbFilter[7] &= static_cast<std::uint8_t>(~0x03U);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_AO_RECORDS)) {
        priv_.glbFilter[1] &= static_cast<std::uint8_t>(~0xFCU);
        priv_.glbFilter[2] &= static_cast<std::uint8_t>(~0x07U);
        priv_.glbFilter[5] &= static_cast<std::uint8_t>(~0x20U);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_EQ_RECORDS)) {
        priv_.glbFilter[2] &= static_cast<std::uint8_t>(~0x78U);
        priv_.glbFilter[5] &= static_cast<std::uint8_t>(~0x40U);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_MP_RECORDS)) {
        priv_.glbFilter[3] &= static_cast<std::uint8_t>(~0x03U);
        priv_.glbFilter[5] &= static_cast<std::uint8_t>(~0x80U);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_QF_RECORDS)) {
        priv_.glbFilter[3] &= static_cast<std::uint8_t>(~0xFCU);
        priv_.glbFilter[4] &= static_cast<std::uint8_t>(~0xC0U);
        priv_.glbFilter[5] &= static_cast<std::uint8_t>(~0x1FU);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_TE_RECORDS)) {
        priv_.glbFilter[4] &= static_cast<std::uint8_t>(~0x7FU);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_SC_RECORDS)) {
        priv_.glbFilter[6] &= static_cast<std::uint8_t>(~0x7FU);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U0_RECORDS)) {
        priv_.glbFilter[12] &= static_cast<std::uint8_t>(~0xF0U);
        priv_.glbFilter[13] &= static_cast<std::uint8_t>(~0x01U);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U1_RECORDS)) {
        priv_.glbFilter[13] &= static_cast<std::uint8_t>(~0x1EU);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U2_RECORDS)) {
        priv_.glbFilter[13] &= static_cast<std::uint8_t>(~0xE0U);
        priv_.glbFilter[14] &= static_cast<std::uint8_t>(~0x03U);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U3_RECORDS)) {
        priv_.glbFilter[14] &= static_cast<std::uint8_t>(~0xF8U);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_U4_RECORDS)) {
        priv_.glbFilter[15] &= static_cast<std::uint8_t>(~0x1FU);
    }
    else if (rec == static_cast<std::uint_fast8_t>(QS_UA_RECORDS)) {
        priv_.glbFilter[12] &= static_cast<std::uint8_t>(~0xF0U);
        priv_.glbFilter[13] = 0U;
        priv_.glbFilter[14] = 0U;
        priv_.glbFilter[15] &= static_cast<std::uint8_t>(~0x1FU);
    }
    else {
        // record IDs can't exceed QS_ESC, so they don't need escaping
        Q_ASSERT_ID(310, rec < static_cast<std::uint_fast8_t>(QS_ESC));

        priv_.glbFilter[rec >> 3] &= static_cast<std::uint8_t>(
                    ~static_cast<std::uint8_t>(1U << (rec & 7U)));
    }
}

//****************************************************************************
/// @description
/// This function must be called at the beginning of each QS record.
/// This function should be called indirectly through the macro #QS_BEGIN,
/// or #QS_BEGIN_NOCRIT, depending if it's called in a normal code or from
/// a critical section.
///
void QS::beginRec_(std::uint_fast8_t const rec) noexcept {
    std::uint8_t const b = priv_.seq + 1U;
    std::uint8_t chksum_ = 0U; // reset the checksum
    std::uint8_t * const buf_   = priv_.buf; // put in a temporary (register)
    QSCtr head_      = priv_.head;  // put in a temporary (register)
    QSCtr const end_ = priv_.end;   // put in a temporary (register)

    priv_.seq = b; // store the incremented sequence num
    priv_.used += 2U; // 2 bytes about to be added

    QS_INSERT_ESC_BYTE_(b)

    chksum_ += static_cast<std::uint8_t>(rec);
    QS_INSERT_BYTE_(static_cast<std::uint8_t>(rec)) // no need for escaping

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
void QS::endRec_(void) noexcept {
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr head_ = priv_.head;
    QSCtr const end_ = priv_.end;
    std::uint8_t b = priv_.chksum;
    b ^= 0xFFU; // invert the bits in the checksum

    priv_.used += 2U; // 2 bytes about to be added

    if ((b != QS_FRAME) && (b != QS_ESC)) {
        QS_INSERT_BYTE_(b)
    }
    else {
        QS_INSERT_BYTE_(QS_ESC)
        QS_INSERT_BYTE_(b ^ QS_ESC_XOR)
        ++priv_.used; // account for the ESC byte
    }

    QS_INSERT_BYTE_(QS_FRAME) // do not escape this QS_FRAME

    priv_.head = head_; // save the head
    if (priv_.used > end_) { // overrun over the old data?
        priv_.used = end_;   // the whole buffer is used
        priv_.tail = head_;  // shift the tail to the old data
    }
}

//****************************************************************************
void QS_target_info_(std::uint8_t const isReset) noexcept {
    static constexpr std::uint8_t ZERO = static_cast<std::uint8_t>('0');
    static std::uint8_t const * const TIME =
        reinterpret_cast<std::uint8_t const *>(&BUILD_TIME[0]);
    static std::uint8_t const * const DATE =
        reinterpret_cast<std::uint8_t const *>(&BUILD_DATE[0]);

    QS::beginRec_(static_cast<std::uint_fast8_t>(QS_TARGET_INFO));
        QS::u8_raw_(isReset);
        QS::u16_raw_(QP_VERSION); // two-byte version number

        // send the object sizes...
        QS::u8_raw_(Q_SIGNAL_SIZE
                    | static_cast<std::uint8_t>(QF_EVENT_SIZ_SIZE << 4));

#ifdef QF_EQUEUE_CTR_SIZE
        QS::u8_raw_(QF_EQUEUE_CTR_SIZE
                    | static_cast<std::uint8_t>(QF_TIMEEVT_CTR_SIZE << 4));
#else
        QS::u8_raw_(static_cast<std::uint8_t>(QF_TIMEEVT_CTR_SIZE << 4));
#endif // ifdef QF_EQUEUE_CTR_SIZE

#ifdef QF_MPOOL_CTR_SIZE
        QS::u8_raw_(QF_MPOOL_SIZ_SIZE
                    | static_cast<std::uint8_t>(QF_MPOOL_CTR_SIZE << 4));
#else
        QS::u8_raw_(0U);
#endif // ifdef QF_MPOOL_CTR_SIZE

        QS::u8_raw_(QS_OBJ_PTR_SIZE | (QS_FUN_PTR_SIZE << 4));
        QS::u8_raw_(QS_TIME_SIZE);

        // send the limits...
        QS::u8_raw_(QF_MAX_ACTIVE);
        QS::u8_raw_(QF_MAX_EPOOL | (QF_MAX_TICK_RATE << 4));

        // send the build time in three bytes (sec, min, hour)...
        QS::u8_raw_((10U * (TIME[6] - ZERO)) + (TIME[7] - ZERO));
        QS::u8_raw_((10U * (TIME[3] - ZERO)) + (TIME[4] - ZERO));
        if (BUILD_TIME[0] == ' ') {
            QS::u8_raw_(TIME[1] - ZERO);
        }
        else {
            QS::u8_raw_((10U * (TIME[0] - ZERO)) + (TIME[1] - ZERO));
        }

        // send the build date in three bytes (day, month, year) ...
        if (BUILD_DATE[4] == ' ') {
            QS::u8_raw_(DATE[5] - ZERO);
        }
        else {
            QS::u8_raw_((10U * (DATE[4] - ZERO)) + (DATE[5] - ZERO));
        }
        // convert the 3-letter month to a number 1-12 ...
        std::uint8_t b;
        switch (DATE[0] + DATE[1] + DATE[2]) {
            case 'J' + 'a' +'n':
                b = 1U;
                break;
            case 'F' + 'e' + 'b':
                b = 2U;
                break;
            case 'M' + 'a' +'r':
                b = 3U;
                break;
            case 'A' + 'p' + 'r':
                b = 4U;
                break;
            case 'M' + 'a' + 'y':
                b = 5U;
                break;
            case 'J' + 'u' + 'n':
                b = 6U;
                break;
            case 'J' + 'u' + 'l':
                b = 7U;
                break;
            case 'A' + 'u' + 'g':
                b = 8U;
                break;
            case 'S' + 'e' + 'p':
                b = 9U;
                break;
            case 'O' + 'c' + 't':
                b = 10U;
                break;
            case 'N' + 'o' + 'v':
                b = 11U;
                break;
            case 'D' + 'e' + 'c':
                b = 12U;
                break;
            default:
                b = 0U;
                break;
        }
        QS::u8_raw_(b); // store the month
        QS::u8_raw_((10U * (DATE[9] - ZERO)) + (DATE[10] - ZERO));
    QS::endRec_();
}

//****************************************************************************
/// @description
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u8_fmt_(std::uint8_t const format, std::uint8_t const d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;  // put in a temporary (register)
    std::uint8_t *const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;    // put in a temporary (register)
    QSCtr const end_= priv_.end;     // put in a temporary (register)

    priv_.used += 2U; // 2 bytes about to be added

    QS_INSERT_ESC_BYTE_(format)
    QS_INSERT_ESC_BYTE_(d)

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
}

//****************************************************************************
/// @description
/// This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u16_fmt_(std::uint8_t format, std::uint16_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum; // put in a temporary (register)
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;   // put in a temporary (register)
    QSCtr const end_= priv_.end;    // put in a temporary (register)

    priv_.used += 3U; // 3 bytes about to be added

    QS_INSERT_ESC_BYTE_(format)

    format = static_cast<std::uint8_t>(d);
    QS_INSERT_ESC_BYTE_(format)

    d >>= 8;
    format = static_cast<std::uint8_t>(d);
    QS_INSERT_ESC_BYTE_(format)

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u32_fmt_(std::uint8_t format, std::uint32_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;  // put in a temporary (register)
    std::uint8_t * const buf_= priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;    // put in a temporary (register)
    QSCtr const end_= priv_.end;     // put in a temporary (register)

    priv_.used += static_cast<QSCtr>(5); // 5 bytes about to be added
    QS_INSERT_ESC_BYTE_(format) // insert the format byte

    for (std::uint_fast8_t i = 4U; i != 0U; --i) {
        format = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(format)
        d >>= 8;
    }

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macro QS_USR_DICTIONARY()
///
void QS::usr_dict_pre_(enum_t const rec,
                       char_t const * const name) noexcept
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec_(static_cast<std::uint_fast8_t>(QS_USR_DICT));
    QS_U8_PRE_(rec);
    QS_STR_PRE_(name);
    endRec_();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::mem_fmt_(std::uint8_t const *blk, std::uint8_t size) noexcept {
    std::uint8_t b = static_cast<std::uint8_t>(MEM_T);
    std::uint8_t chksum_ = priv_.chksum + b;
    std::uint8_t * const buf_= priv_.buf; // put in a temporary (register)
    QSCtr head_     = priv_.head;         // put in a temporary (register)
    QSCtr const end_= priv_.end;          // put in a temporary (register)

    // size+2 bytes to be added
    priv_.used += static_cast<std::uint8_t>(size + 2U);

    QS_INSERT_BYTE_(b)
    QS_INSERT_ESC_BYTE_(size)

    // output the 'size' number of bytes
    for (; size != 0U; --size) {
        b = *blk;
        QS_INSERT_ESC_BYTE_(b)
        ++blk;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::str_fmt_(char_t const *s) noexcept {
    std::uint8_t b       = static_cast<std::uint8_t>(*s);
    std::uint8_t chksum_ = static_cast<std::uint8_t>(
                           priv_.chksum + static_cast<std::uint8_t>(STR_T));
    std::uint8_t * const buf_= priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;    // put in a temporary (register)
    QSCtr const end_= priv_.end;     // put in a temporary (register)
    QSCtr   used_   = priv_.used;    // put in a temporary (register)

    used_ += 2U; // the format byte and the terminating-0

    QS_INSERT_BYTE_(static_cast<std::uint8_t>(STR_T))
    while (b != 0U) {
        // ASCII characters don't need escaping
        chksum_ += b;  // update checksum
        QS_INSERT_BYTE_(b)
        ++s;
        b = static_cast<std::uint8_t>(*s);
        ++used_;
    }
    QS_INSERT_BYTE_(0U) // zero-terminate the string

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
    priv_.used   = used_;   // save # of used buffer space
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u8_raw_(std::uint8_t const d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;   // put in a temporary (register)
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;     // put in a temporary (register)
    QSCtr const end_= priv_.end;      // put in a temporary (register)

    priv_.used += 1U;  // 1 byte about to be added
    QS_INSERT_ESC_BYTE_(d)

    priv_.head   = head_;   // save the head
    priv_.chksum = chksum_; // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u8u8_raw_(std::uint8_t const d1, std::uint8_t const d2) noexcept {
    std::uint8_t chksum_ = priv_.chksum;   // put in a temporary (register)
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;     // put in a temporary (register)
    QSCtr const end_= priv_.end;      // put in a temporary (register)

    priv_.used += 2U; // 2 bytes about to be added
    QS_INSERT_ESC_BYTE_(d1)
    QS_INSERT_ESC_BYTE_(d2)

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u16_raw_(std::uint16_t d) noexcept {
    std::uint8_t b = static_cast<std::uint8_t>(d);
    std::uint8_t chksum_ = priv_.chksum;   // put in a temporary (register)
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;     // put in a temporary (register)
    QSCtr const end_= priv_.end;      // put in a temporary (register)

    priv_.used += 2U; // 2 bytes about to be added

    QS_INSERT_ESC_BYTE_(b)

    d >>= 8;
    b = static_cast<std::uint8_t>(d);
    QS_INSERT_ESC_BYTE_(b)

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::u32_raw_(std::uint32_t d) noexcept {
    std::uint8_t chksum_ = priv_.chksum;   // put in a temporary (register)
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;     // put in a temporary (register)
    QSCtr const end_= priv_.end;      // put in a temporary (register)

    priv_.used += 4U; // 4 bytes about to be added
    for (std::uint_fast8_t i = 4U; i != 0U; --i) {
        std::uint8_t const b = static_cast<std::uint8_t>(d);
        QS_INSERT_ESC_BYTE_(b)
        d >>= 8;
    }

    priv_.head   = head_;    // save the head
    priv_.chksum = chksum_;  // save the checksum
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::obj_raw_(void const * const obj) noexcept {
#if (QS_OBJ_PTR_SIZE == 1U)
    u8_raw_(reinterpret_cast<std::uint8_t>(obj));
#elif (QS_OBJ_PTR_SIZE == 2U)
    u16_raw_(reinterpret_cast<std::uint16_t>(obj));
#elif (QS_OBJ_PTR_SIZE == 4U)
    u32_raw_(reinterpret_cast<std::uint32_t>(obj));
#elif (QS_OBJ_PTR_SIZE == 8U)
    u64_raw_(reinterpret_cast<std::uint64_t>(obj));
#else
    u32_raw_(reinterpret_cast<std::uint32_t>(obj));
#endif
}

//****************************************************************************
/// @note This function is only to be used through macros, never in the
/// client code directly.
///
void QS::str_raw_(char_t const *s) noexcept {
    std::uint8_t b = static_cast<std::uint8_t>(*s);
    std::uint8_t chksum_ = priv_.chksum;   // put in a temporary (register)
    std::uint8_t * const buf_ = priv_.buf; // put in a temporary (register)
    QSCtr   head_   = priv_.head;     // put in a temporary (register)
    QSCtr const end_= priv_.end;      // put in a temporary (register)
    QSCtr   used_   = priv_.used;     // put in a temporary (register)

    while (b != 0U) {
        chksum_ += b;      // update checksum
        QS_INSERT_BYTE_(b) // ASCII characters don't need escaping
        ++s;
        b = static_cast<std::uint8_t>(*s);
        ++used_;
    }
    QS_INSERT_BYTE_(0U) // zero-terminate the string
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
std::uint16_t QS::getByte(void) noexcept {
    std::uint16_t ret;
    if (priv_.used == 0U) {
        ret = QS_EOD; // set End-Of-Data
    }
    else {
        std::uint8_t * const buf_ = priv_.buf; //put in a temporary (register)
        QSCtr tail_ = priv_.tail;         // put in a temporary (register)

        // the byte to return
        ret = static_cast<std::uint16_t>(buf_[tail_]);

        ++tail_;  // advance the tail
        if (tail_ == priv_.end) {  // tail wrap around?
            tail_ = 0U;
        }
        priv_.tail = tail_;  // update the tail
        --priv_.used;        // one less byte used
    }
    return ret;  // return the byte or EOD
}

//****************************************************************************
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
std::uint8_t const *QS::getBlock(std::uint16_t * const pNbytes) noexcept {
    QSCtr const used_ = priv_.used; // put in a temporary (register)
    std::uint8_t *buf_;

    // any bytes used in the ring buffer?
    if (used_ == 0U) {
        *pNbytes = 0U;  // no bytes available right now
        buf_     = nullptr; // no bytes available right now
    }
    else {
        QSCtr tail_      = priv_.tail; // put in a temporary (register)
        QSCtr const end_ = priv_.end;  // put in a temporary (register)
        QSCtr n = static_cast<QSCtr>(end_ - tail_);
        if (n > used_) {
            n = used_;
        }
        if (n > static_cast<QSCtr>(*pNbytes)) {
            n = static_cast<QSCtr>(*pNbytes);
        }
        *pNbytes = static_cast<std::uint16_t>(n); // n-bytes available
        buf_ = priv_.buf;
        buf_ = &buf_[tail_]; // the bytes are at the tail

        priv_.used -= n;
        tail_      += n;
        if (tail_ == end_) {
            tail_ = 0U;
        }
        priv_.tail = tail_;
    }
    return buf_;
}

//****************************************************************************
/// @note This function is only to be used through macro QS_SIG_DICTIONARY()
///
void QS::sig_dict_pre_(enum_t const sig, void const * const obj,
                       char_t const *name) noexcept
{
    QS_CRIT_STAT_

    if (*name == '&') {
        ++name;
    }
    QS_CRIT_ENTRY_();
    beginRec_(static_cast<std::uint_fast8_t>(QS_SIG_DICT));
    QS_SIG_PRE_(sig);
    QS_OBJ_PRE_(obj);
    QS_STR_PRE_(name);
    endRec_();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macro QS_OBJ_DICTIONARY()
///
void QS::obj_dict_pre_(void const * const obj,
                       char_t const *name) noexcept
{
    QS_CRIT_STAT_

    if (*name == '&') {
        ++name;
    }
    QS_CRIT_ENTRY_();
    beginRec_(static_cast<std::uint_fast8_t>(QS_OBJ_DICT));
    QS_OBJ_PRE_(obj);
    QS_STR_PRE_(name);
    endRec_();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macro QS_FUN_DICTIONARY()
///
void QS::fun_dict_pre_(void (* const fun)(void),
                       char_t const *name) noexcept
{
    QS_CRIT_STAT_

    if (*name == '&') {
        ++name;
    }
    QS_CRIT_ENTRY_();
    beginRec_(static_cast<std::uint_fast8_t>(QS_FUN_DICT));
    QS_FUN_PRE_(fun);
    QS_STR_PRE_(name);
    endRec_();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// @note This function is only to be used through macro QS_ASSERTION()
///
void QS::assertion_pre_(char_t const * const module, int_t const loc,
                        std::uint32_t delay)
{
    QS_BEGIN_NOCRIT_PRE_(QP::QS_ASSERT_FAIL, nullptr, nullptr)
        QS_TIME_PRE_();
        QS_U16_PRE_(loc);
        QS_STR_PRE_((module != nullptr) ? module : "?");
    QS_END_NOCRIT_PRE_()
    QP::QS::onFlush();
    for (std::uint32_t volatile ctr = delay; ctr > 0U; --ctr) {
    }
    QP::QS::onCleanup();
}

//............................................................................
void QS::crit_entry_pre_(void) {
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_CRIT_ENTRY, nullptr, nullptr)
        QS_TIME_PRE_();
        ++QS::priv_.critNest;
        QS_U8_PRE_(QS::priv_.critNest);
    QS_END_NOCRIT_PRE_()
}

//............................................................................
void QS::crit_exit_pre_(void) {
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_CRIT_EXIT, nullptr, nullptr)
        QS_TIME_PRE_();
        QS_U8_PRE_(QS::priv_.critNest);
        --QS::priv_.critNest;
    QS_END_NOCRIT_PRE_()
}

//............................................................................
void QS::isr_entry_pre_(std::uint8_t const isrnest,
                        std::uint8_t const prio)
{
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_ISR_ENTRY, nullptr, nullptr)
        QS_TIME_PRE_();
        QS_U8_PRE_(isrnest);
        QS_U8_PRE_(prio);
    QS_END_NOCRIT_PRE_()
}

//............................................................................
void QS::isr_exit_pre_(std::uint8_t const isrnest,
                       std::uint8_t const prio)
{
    QS_BEGIN_NOCRIT_PRE_(QP::QS_QF_ISR_EXIT, nullptr, nullptr)
        QS_TIME_PRE_();
        QS_U8_PRE_(isrnest);
        QS_U8_PRE_(prio);
    QS_END_NOCRIT_PRE_()
}

} // namespace QP

