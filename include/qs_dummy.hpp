//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QS_DUMMY_HPP_
#define QS_DUMMY_HPP_

#ifdef Q_SPY
    #error Q_SPY must NOT be defined to include qs_dummy.hpp
#endif

#ifdef Q_UTEST
    #error Q_UTEST must NOT be defined to include qs_dummy.hpp
#endif

#define QS_INIT(arg_)                   (true)
#define QS_EXIT()                       static_cast<void>(0)
#define QS_GLB_FILTER(rec_)             static_cast<void>(0)
#define QS_LOC_FILTER(qsId_)            static_cast<void>(0)

#define QS_BEGIN_ID(rec_, qsId_)        {
#define QS_END()                        }
#define QS_BEGIN_INCRIT(rec_, qsId_)    {
#define QS_END_INCRIT()                 }

#define QS_I8(width_, data_)            static_cast<void>(0)
#define QS_U8(width_, data_)            static_cast<void>(0)
#define QS_I16(width_, data_)           static_cast<void>(0)
#define QS_U16(width_, data_)           static_cast<void>(0)
#define QS_I32(width_, data_)           static_cast<void>(0)
#define QS_U32(width_, data_)           static_cast<void>(0)
#define QS_F32(width_, data_)           static_cast<void>(0)
#define QS_F64(width_, data_)           static_cast<void>(0)
#define QS_I64(width_, data_)           static_cast<void>(0)
#define QS_U64(width_, data_)           static_cast<void>(0)
#define QS_ENUM(group_, value_)         static_cast<void>(0)
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
#define QS_ENUM_DICTIONARY(value_, group_)  static_cast<void>(0)
#define QS_ASSERTION(module_, loc_, delay_) static_cast<void>(0)
#define QS_FLUSH()                      static_cast<void>(0)

#define QS_TEST_PROBE_DEF(fun_)
#define QS_TEST_PROBE(code_)
#define QS_TEST_PROBE_ID(id_, code_)
#define QS_TEST_PAUSE()                 static_cast<void>(0)

#define QS_OUTPUT()                     static_cast<void>(0)
#define QS_RX_INPUT()                   static_cast<void>(0)
#define QS_ONLY(code_)                  static_cast<void>(0)

#define QS_CRIT_STAT
#define QS_CRIT_ENTRY()                 static_cast<void>(0)
#define QS_CRIT_EXIT()                  static_cast<void>(0)

//============================================================================
// interface used only for internal implementation, but not in applications
#ifdef QP_IMPL
    // predefined QS trace records
    #define QS_BEGIN_PRE(rec_, qsId_)   if (false) {
    #define QS_END_PRE()                }
    #define QS_U8_PRE(data_)            static_cast<void>(0)
    #define QS_2U8_PRE(data1_, data2_)  static_cast<void>(0)
    #define QS_U16_PRE(data_)           static_cast<void>(0)
    #define QS_U32_PRE(data_)           static_cast<void>(0)
    #define QS_TIME_PRE()               static_cast<void>(0)
    #define QS_SIG_PRE(sig_)            static_cast<void>(0)
    #define QS_EVS_PRE(size_)           static_cast<void>(0)
    #define QS_OBJ_PRE(obj_)            static_cast<void>(0)
    #define QS_FUN_PRE(fun_)            static_cast<void>(0)
    #define QS_EQC_PRE(ctr_)            static_cast<void>(0)
    #define QS_MPC_PRE(ctr_)            static_cast<void>(0)
    #define QS_MPS_PRE(size_)           static_cast<void>(0)
    #define QS_TEC_PRE(ctr_)            static_cast<void>(0)

    #define QS_TR_CRIT_ENTRY()          static_cast<void>(0)
    #define QS_TR_CRIT_EXIT()           static_cast<void>(0)
    #define QS_TR_ISR_ENTRY(isrnest_, prio_) static_cast<void>(0)
    #define QS_TR_ISR_EXIT(isrnest_, prio_)  static_cast<void>(0)
#endif // QP_IMPL

#endif // QS_DUMMY_HPP_
