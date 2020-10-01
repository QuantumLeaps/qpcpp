/// @file
/// @brief Dummy definitions of the QS macros that avoid code generation from
/// the QS instrumentation.
/// @ingroup qs qpspy
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-30
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

//****************************************************************************
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

