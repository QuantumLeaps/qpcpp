/// @file
/// @ingroup qs
/// @brief Internal (package scope) QS/C++ interface.
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

#ifndef qs_pkg_h
#define qs_pkg_h

//! Internal QS macro to insert an un-escaped byte into the QS buffer
#define QS_INSERT_BYTE(b_) \
    QS_PTR_AT_(buf_, head_) = (b_); \
    ++head_; \
    if (head_ == end_) { \
        head_ = static_cast<QSCtr>(0); \
    }

//! Internal QS macro to insert an escaped byte into the QS buffer
#define QS_INSERT_ESC_BYTE(b_) \
    chksum_ += (b_); \
    if (((b_) != QS_FRAME) && ((b_) != QS_ESC)) { \
        QS_INSERT_BYTE(b_) \
    } \
    else { \
        QS_INSERT_BYTE(QS_ESC) \
        QS_INSERT_BYTE(static_cast<uint8_t>((b_) ^ QS_ESC_XOR)) \
        ++priv_.used; \
    }

//! Internal QS macro to increment the given pointer argument @a ptr_
///
/// @note Incrementing a pointer violates the MISRA-C 2004 Rule 17.4(req),
/// pointer arithmetic other than array indexing. Encapsulating this violation
/// in a macro allows to selectively suppress this specific deviation.
#define QS_PTR_INC_(ptr_) (++(ptr_))

//! Internal QS macro to cast enumerated QS record number to uint8_t
///
/// @note Casting from enum to unsigned char violates the MISRA-C++ 2008 rules
/// 5-2-7, 5-2-8 and 5-2-9. Encapsulating this violation in a macro allows to
/// selectively suppress this specific deviation.
#define QS_REC_NUM_(enum_) (static_cast<uint_fast8_t>(enum_))

namespace QP {

/// @brief Frame character of the QS output protocol
uint8_t const QS_FRAME = static_cast<uint8_t>(0x7E);

/// @brief Escape character of the QS output protocol
uint8_t const QS_ESC   = static_cast<uint8_t>(0x7D);

/// @brief Escape modifier of the QS output protocol
///
/// The escaped byte is XOR-ed with the escape modifier before it is inserted
/// into the QS buffer.
uint8_t const QS_ESC_XOR = static_cast<uint8_t>(0x20);

/// @brief Escape character of the QS output protocol
uint8_t const QS_GOOD_CHKSUM = static_cast<uint8_t>(0xFF);

//! send the Target info (object sizes, build time-stamp, QP version)
void QS_target_info_(uint8_t const isReset);

} // namespace QP

#endif // qs_pkg_h
