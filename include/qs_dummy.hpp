//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-02-20
//! @version Last updated for: @ref qpcpp_7_0_0
//!
//! @file
//! @brief Dummy definitions of the QS macros that avoid code generation from
//! the QS instrumentation.

#ifndef QS_DUMMY_HPP
#define QS_DUMMY_HPP

#ifdef Q_SPY
    #error "Q_SPY must NOT be defined to include qs_dummy.hpp"
#endif

#define QS_INIT(arg_)                   (true)
#define QS_EXIT()                       static_cast<void>(0)
#define QS_DUMP()                       static_cast<void>(0)
#define QS_GLB_FILTER(rec_)             static_cast<void>(0)
#define QS_LOC_FILTER(qs_id_)           static_cast<void>(0)

#define QS_GET_BYTE(pByte_)             (0xFFFFU)
#define QS_GET_BLOCK(pSize_)            (nullptr)

#define QS_BEGIN_ID(rec_, qs_id_)       if (false) {
#define QS_END()                        }
#define QS_BEGIN_NOCRIT(rec_, qs_id_)   if (false) {
#define QS_END_NOCRIT()                 }

#define QS_I8(width_, data_)            static_cast<void>(0)
#define QS_U8(width_, data_)            static_cast<void>(0)
#define QS_I16(width_, data_)           static_cast<void>(0)
#define QS_U16(width_, data_)           static_cast<void>(0)
#define QS_I32(width_, data_)           static_cast<void>(0)
#define QS_U32(width_, data_)           static_cast<void>(0)
#define QS_F32(width_, data_)           static_cast<void>(0)
#define QS_F64(width_, data_)           static_cast<void>(0)
#define QS_U64(width_, data_)           static_cast<void>(0)
#define QS_STR(str_)                    static_cast<void>(0)
#define QS_MEM(mem_, size_)             static_cast<void>(0)
#define QS_SIG(sig_, obj_)              static_cast<void>(0)
#define QS_OBJ(obj_)                    static_cast<void>(0)
#define QS_FUN(fun_)                    static_cast<void>(0)

#define QS_SIG_DICTIONARY(sig_, obj_)   static_cast<void>(0)
#define QS_OBJ_DICTIONARY(obj_)         static_cast<void>(0)
#define QS_OBJ_ARR_DICTIONARY(obj_, idx_) static_cast<void>(0)
#define QS_FUN_DICTIONARY(fun_)         static_cast<void>(0)
#define QS_USR_DICTIONARY(rec_)         static_cast<void>(0)
#define QS_ASSERTION(module_, loc_, delay_) static_cast<void>(0)
#define QS_FLUSH()                      static_cast<void>(0)

#define QS_TEST_PROBE_DEF(fun_)
#define QS_TEST_PROBE(code_)
#define QS_TEST_PROBE_ID(id_, code_)
#define QS_TEST_PAUSE()                 static_cast<void>(0)

#define QS_OUTPUT()                     static_cast<void>(0)
#define QS_RX_INPUT()                   static_cast<void>(0)

namespace QP {

//! QS ID offsets for QS_LOC_FILTER()
enum QSpyIdOffsets : std::int16_t {
    QS_AO_ID = 0,  //!< offset for AO priorities
    QS_EP_ID = 64, //!< offset for event-pool IDs
    QS_EQ_ID = 80, //!< offset for event-queue IDs
    QS_AP_ID = 96, //!< offset for Appl-spec IDs
};

//! QS ID groups for QS_LOC_FILTER()
enum QSpyIdGroups : std::int16_t {
    QS_ALL_IDS = 0xF0,            //!< all QS IDs
    QS_AO_IDS  = 0x80 + QS_AO_ID, //!< AO IDs (priorities)
    QS_EP_IDS  = 0x80 + QS_EP_ID, //!< event-pool IDs
    QS_EQ_IDS  = 0x80 + QS_EQ_ID, //!< event-queue IDs
    QS_AP_IDS  = 0x80 + QS_AP_ID, //!< Application-specific IDs
};

//! QS ID type for applying local filtering
struct QSpyId {
    std::uint8_t m_prio;
    std::uint_fast8_t getPrio(void) const noexcept {
        return static_cast<std::uint_fast8_t>(m_prio);
    }
};

} // namespace QP

//============================================================================
// internal QS macros used only in the QP components

#ifdef QP_IMPL
    // predefined QS trace records
    #define QS_BEGIN_PRE_(rec_, qs_id_)  if (false) {
    #define QS_END_PRE_()               }
    #define QS_BEGIN_NOCRIT_PRE_(rec_, qs_id_) if (false) {
    #define QS_END_NOCRIT_PRE_()        }
    #define QS_U8_PRE_(data_)           static_cast<void>(0)
    #define QS_2U8_PRE_(data1_, data2_) static_cast<void>(0)
    #define QS_U16_PRE_(data_)          static_cast<void>(0)
    #define QS_U32_PRE_(data_)          static_cast<void>(0)
    #define QS_TIME_PRE_()              static_cast<void>(0)
    #define QS_SIG_PRE_(sig_)           static_cast<void>(0)
    #define QS_EVS_PRE_(size_)          static_cast<void>(0)
    #define QS_OBJ_PRE_(obj_)           static_cast<void>(0)
    #define QS_FUN_PRE_(fun_)           static_cast<void>(0)
    #define QS_EQC_PRE_(ctr_)           static_cast<void>(0)
    #define QS_MPC_PRE_(ctr_)           static_cast<void>(0)
    #define QS_MPS_PRE_(size_)          static_cast<void>(0)
    #define QS_TEC_PRE_(ctr_)           static_cast<void>(0)

    #define QS_CRIT_STAT_
    #define QF_QS_CRIT_ENTRY()          static_cast<void>(0)
    #define QF_QS_CRIT_EXIT()           static_cast<void>(0)
    #define QF_QS_ISR_ENTRY(isrnest_, prio_) static_cast<void>(0)
    #define QF_QS_ISR_EXIT(isrnest_, prio_)  static_cast<void>(0)
    #define QF_QS_ACTION(act_)          static_cast<void>(0)
#endif // QP_IMPL

#endif // QS_DUMMY_HPP
