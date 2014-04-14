/// \file
/// \brief QS internal variables and core QS functions implementations.
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
#include "qassert.h"      // QP assertions

namespace QP {

Q_DEFINE_THIS_MODULE("qs")

//****************************************************************************
QS QS::priv_; // QS private data

//****************************************************************************
/// \description
/// This function should be called from QP::QS::onStartup() to provide QS with
/// the data buffer. The first argument \a sto[] is the address of the memory
/// block, and the second argument \a stoSize is the size of this block
/// in bytes. Currently the size of the QS buffer cannot exceed 64KB.
///
/// \note QS can work with quite small data buffers, but you will start losing
/// data if the buffer is too small for the bursts of tracing activity.
/// The right size of the buffer depends on the data production rate and
/// the data output rate. QS offers flexible filtering to reduce the data
/// production rate.
///
/// \note If the data output rate cannot keep up with the production rate,
/// QS will start overwriting the older data with newer data. This is
/// consistent with the "last-is-best" QS policy. The record sequence counters
/// and check sums on each record allow the QSPY host uitiliy to easily detect
/// any data loss.
///
void QS::initBuf(uint8_t sto[], uint_fast16_t const stoSize) {
    uint8_t *buf_ = &sto[0];

    // at least 8 bytes in buf
    Q_REQUIRE_ID(100, stoSize > static_cast<uint_fast16_t>(8));

    QF::bzero(&priv_, static_cast<uint_fast16_t>(sizeof(priv_)));
    priv_.buf  = buf_;
    priv_.end  = static_cast<QSCtr>(stoSize);

    beginRec(QS_REC_NUM_(QS_EMPTY));
    endRec();
    beginRec(QS_REC_NUM_(QS_QP_RESET));
    endRec();
}

//****************************************************************************
/// \description
/// This function sets up the QS filter to enable the record type \a rec.
/// The argument #QS_ALL_RECORDS specifies to filter-in all records.
/// This function should be called indirectly through the macro QS_FILTER_ON.
///
/// \note Filtering based on the record-type is only the first layer of
/// filtering. The second layer is based on the object-type. Both filter
/// layers must be enabled for the QS record to be inserted in the QS buffer.
///
/// \sa QP::QS::filterOff(), QS_FILTER_SM_OBJ, QS_FILTER_AO_OBJ,
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
/// \description
/// This function sets up the QS filter to disable the record type \a rec.
/// The argument #QS_ALL_RECORDS specifies to suppress all records.
/// This function should be called indirectly through the macro QS_FILTER_OFF.
///
/// \note Filtering records based on the record-type is only the first layer
/// of filtering. The second layer is based on the object-type. Both filter
/// layers must be enabled for the QS record to be inserted in the QS buffer.
///
void QS::filterOff(uint_fast8_t const rec) {

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
        // record numbers can't exceed QS_ESC, so they don't need escaping
        Q_ASSERT_ID(310, rec < static_cast<uint_fast8_t>(QS_ESC));
        priv_.glbFilter[rec >> 3] &= static_cast<uint8_t>(
           ~static_cast<uint8_t>(1U << (rec & static_cast<uint_fast8_t>(7))));
    }
}

//****************************************************************************
/// \description
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
/// \description
/// This function must be called at the end of each QS record.
/// This function should be called indirectly through the macro #QS_END,
/// or #QS_END_NOCRIT, depending if it's called in a normal code or from
/// a critical section.
///
void QS::endRec(void) {
    uint8_t b = static_cast<uint8_t>(~priv_.chksum);
    uint8_t *buf_ = priv_.buf;  // put in a temporary (register)
    QSCtr   head_ = priv_.head;
    QSCtr   end_  = priv_.end;

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
/// \description
/// \note This function is only to be used through macros, never in the
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
/// \description
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
/// \note This function is only to be used through macros, never in the
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

} // namespace QP


