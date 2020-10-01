/// @file
/// @ingroup qs
/// @brief Internal (package scope) QS/C++ interface.
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-18
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

#ifndef QS_PKG_HPP
#define QS_PKG_HPP

//! Internal QS macro to insert an un-escaped byte into the QS buffer
#define QS_INSERT_BYTE_(b_) \
    buf_[head_] = (b_);     \
    ++head_;                \
    if (head_ == end_) {    \
        head_ = 0U;         \
    }

//! Internal QS macro to insert an escaped byte into the QS buffer
#define QS_INSERT_ESC_BYTE_(b_)                   \
    chksum_ += (b_);                              \
    if (((b_) != QS_FRAME) && ((b_) != QS_ESC)) { \
        QS_INSERT_BYTE_(b_)                       \
    }                                             \
    else {                                        \
        QS_INSERT_BYTE_(QS_ESC)                   \
        QS_INSERT_BYTE_(static_cast<std::uint8_t>((b_) ^ QS_ESC_XOR)) \
        ++priv_.used;                             \
    }

//****************************************************************************
// Macros for use inside other macros or internally in the QP code

//! Internal QS macro to begin a predefined QS record with critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level.
/// @sa QS_BEGIN_ID()
///
#define QS_BEGIN_PRE_(rec_, qs_id_)                           \
    if (QS_GLB_CHECK_(rec_) && QS_LOC_CHECK_(qs_id_)) {       \
        QS_CRIT_E_();                                         \
        QP::QS::beginRec_(static_cast<std::uint_fast8_t>(rec_));

//! Internal QS macro to end a predefined QS record with critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level.
/// @sa QS_END()
///
#define QS_END_PRE_()      \
        QP::QS::endRec_(); \
        QS_CRIT_X_();      \
    }

//! Internal QS macro to begin a predefined QS record without critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level.
/// @sa QS_BEGIN_NOCRIT_PRE_()
#define QS_BEGIN_NOCRIT_PRE_(rec_, qs_id_)                    \
    if (QS_GLB_CHECK_(rec_) && QS_LOC_CHECK_(qs_id_)) {       \
        QP::QS::beginRec_(static_cast<std::uint_fast8_t>(rec_));

//! Internal QS macro to end a predefiend QS record without critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_END_NOCRIT
#define QS_END_NOCRIT_PRE_() \
        QP::QS::endRec_(); \
    }

#if (Q_SIGNAL_SIZE == 1U)
    //! Internal QS macro to output an unformatted event signal data element
    /// @note
    /// The size of the pointer depends on the macro #Q_SIGNAL_SIZE.
    #define QS_SIG_PRE_(sig_) \
        (QP::QS::u8_raw_(static_cast<std::uint8_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 2U)
    #define QS_SIG_PRE_(sig_) \
        (QP::QS::u16_raw_(static_cast<std::uint16_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 4U)
    #define QS_SIG_PRE_(sig_) \
        (QP::QS::u32_raw_(static_cast<std::uint32_t>(sig_)))
#endif

//! Internal QS macro to output an unformatted uint8_t data element
#define QS_U8_PRE_(data_) \
    (QP::QS::u8_raw_(static_cast<std::uint8_t>(data_)))

//! Internal QS macro to output 2 unformatted uint8_t data elements
#define QS_2U8_PRE_(data1_, data2_) \
    (QP::QS::u8u8_raw_(static_cast<std::uint8_t>(data1_), \
                       static_cast<std::uint8_t>(data2_)))

//! Internal QS macro to output an unformatted uint16_t data element
#define QS_U16_PRE_(data_) \
    (QP::QS::u16_raw_(static_cast<std::uint16_t>(data_)))

//! Internal QS macro to output an unformatted uint32_t data element
#define QS_U32_PRE_(data_) \
    (QP::QS::u32_raw_(static_cast<std::uint32_t>(data_)))

//! Internal QS macro to output a zero-terminated ASCII string
/// data element
#define QS_STR_PRE_(msg_)     (QP::QS::str_raw_(msg_))

//! Internal QS macro to output object pointer data element
#define QS_OBJ_PRE_(obj_)     (QP::QS::obj_raw_(obj_))

#if (QS_FUN_PTR_SIZE == 1U)
    #define QS_FUN_PRE_(fun_) \
        (QP::QS::u8_raw_(reinterpret_cast<std::uint8_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 2U)
    #define QS_FUN_PRE_(fun_) \
        (QP::QS::u16_raw_(reinterpret_cast<std::uint16_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 4U)
    #define QS_FUN_PRE_(fun_) \
        (QP::QS::u32_raw_(reinterpret_cast<std::uint32_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 8U)
    #define QS_FUN_PRE_(fun_) \
        (QP::QS::u64_raw_(reinterpret_cast<std::uint64_t>(fun_)))
#else

    //! Internal QS macro to output an unformatted function pointer
    //! data element
    /// @note
    /// The size of the pointer depends on the macro #QS_FUN_PTR_SIZE.
    /// If the size is not defined the size of pointer is assumed 4-bytes.
    #define QS_FUN_PRE_(fun_) \
        (QP::QS::u32_raw_(reinterpret_cast<std::uint32_t>(fun_)))
#endif

#if (QF_EQUEUE_CTR_SIZE == 1U)

    //! Internal QS macro to output an unformatted event queue
    //! counter data element
    /// @note the counter size depends on the macro #QF_EQUEUE_CTR_SIZE.
    #define QS_EQC_PRE_(ctr_)   \
        QS::u8_raw_(static_cast<std::uint8_t>(ctr_))
#elif (QF_EQUEUE_CTR_SIZE == 2U)
    #define QS_EQC_PRE_(ctr_)  \
        QS::u16_raw_(static_cast<std::uint16_t>(ctr_))
#elif (QF_EQUEUE_CTR_SIZE == 4U)
    #define QS_EQC_PRE_(ctr_) \
        QS::u32_raw_(static_cast<std::uint32_t>(ctr_))
#else
    #error "QF_EQUEUE_CTR_SIZE not defined"
#endif


#if (QF_EVENT_SIZ_SIZE == 1U)

    //! Internal QS macro to output an unformatted event size
    //! data element
    /// @note the event size depends on the macro #QF_EVENT_SIZ_SIZE.
    #define QS_EVS_PRE_(size_) \
        QS::u8_raw_(static_cast<std::uint8_t>(size_))
#elif (QF_EVENT_SIZ_SIZE == 2U)
    #define QS_EVS_PRE_(size_) \
        QS::u16_raw_(static_cast<std::uint16_t>(size_))
#elif (QF_EVENT_SIZ_SIZE == 4U)
    #define QS_EVS_PRE_(size_) \
        QS::u32_raw_(static_cast<std::uint32_t>(size_))
#endif


#if (QF_MPOOL_SIZ_SIZE == 1U)

    //! Internal QS macro to output an unformatted memory pool
    /// block-size data element
    /// @note the block-size depends on the macro #QF_MPOOL_SIZ_SIZE.
    #define QS_MPS_PRE_(size_) \
        QS::u8_raw_(static_cast<std::uint8_t>(size_))
#elif (QF_MPOOL_SIZ_SIZE == 2U)
    #define QS_MPS_PRE_(size_) \
        QS::u16_raw_(static_cast<std::uint16_t>(size_))
#elif (QF_MPOOL_SIZ_SIZE == 4U)
    #define QS_MPS_PRE_(size_) \
        QS::u32_raw_(static_cast<std::uint32_t>(size_))
#endif

#if (QF_MPOOL_CTR_SIZE == 1U)

    //! Internal QS macro to output an unformatted memory pool
    //! block-counter data element
    /// @note the counter size depends on the macro #QF_MPOOL_CTR_SIZE.
    #define QS_MPC_PRE_(ctr_) \
        QS::u8_raw_(static_cast<std::uint8_t>(ctr_))
#elif (QF_MPOOL_CTR_SIZE == 2U)
    #define QS_MPC_PRE_(ctr_) \
        QS::u16_raw_(static_cast<std::uint16_t>(ctr_))
#elif (QF_MPOOL_CTR_SIZE == 4U)
    #define QS_MPC_PRE_(ctr_) \
        QS::u32_raw_(static_cast<std::uint32_t>(ctr_))
#endif


#if (QF_TIMEEVT_CTR_SIZE == 1U)

    //! Internal QS macro to output an unformatted time event
    //! tick-counter data element
    /// @note the counter size depends on the macro #QF_TIMEEVT_CTR_SIZE.
    #define QS_TEC_PRE_(ctr_) \
        QS::u8_raw_(static_cast<std::uint8_t>(ctr_))
#elif (QF_TIMEEVT_CTR_SIZE == 2U)
    #define QS_TEC_PRE_(ctr_) \
        QS::u16_raw_(static_cast<std::uint16_t>(ctr_))
#elif (QF_TIMEEVT_CTR_SIZE == 4U)
    #define QS_TEC_PRE_(ctr_) \
        QS::u32_raw_(static_cast<std::uint32_t>(ctr_))
#endif

//! Internal QS macro to cast enumerated QS record number to uint8_t
///
/// @note Casting from enum to unsigned char violates the MISRA-C++ 2008 rules
/// 5-2-7, 5-2-8 and 5-2-9. Encapsulating this violation in a macro allows to
/// selectively suppress this specific deviation.
#define QS_REC_NUM_(enum_) (static_cast<std::uint_fast8_t>(enum_))

namespace QP {

/// @brief Frame character of the QS output protocol
constexpr std::uint8_t QS_FRAME = 0x7EU;

/// @brief Escape character of the QS output protocol
constexpr std::uint8_t QS_ESC   = 0x7DU;

/// @brief Escape modifier of the QS output protocol
///
/// The escaped byte is XOR-ed with the escape modifier before it is inserted
/// into the QS buffer.
constexpr std::uint8_t QS_ESC_XOR = 0x20U;

/// @brief Escape character of the QS output protocol
constexpr std::uint8_t QS_GOOD_CHKSUM = 0xFFU;

//! send the Target info (object sizes, build time-stamp, QP version)
void QS_target_info_(std::uint8_t const isReset) noexcept;

} // namespace QP

#endif // QS_PKG_HPP
