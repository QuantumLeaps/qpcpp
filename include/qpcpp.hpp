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
#ifndef QPCPP_HPP_
#define QPCPP_HPP_

//============================================================================
#include "qp_port.hpp"      // QP port from the port directory
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // software tracing enabled?
    #include "qs_port.hpp"  // QS/C++ port from the port directory
#else
    #include "qs_dummy.hpp" // QS/C++ dummy (inactive) interface
#endif

#ifndef QP_API_VERSION
    #define QP_API_VERSION 0
#endif

// QP API compatibility layer...

#ifdef Q_SIGNAL_SIZE
_Static_assert(Q_SIGNAL_SIZE == 2U,
    "Q_SIGNAL_SIZE must be 2 bytes (16-bit signal space)");
#endif

// version 8.1.0 -------------------------------------------------------------
#if (QP_API_VERSION < 810)

using enum_t = int;

#ifdef Q_SPY

namespace QP {

//! @deprecated instead use: QS_Groups
enum QS_Groups_old : std::int16_t {
    QS_ALL_RECORDS = QS_GRP_ALL,
    QS_SM_RECORDS  = QS_GRP_SM,
    QS_AO_RECORDS  = QS_GRP_AO,
    QS_EQ_RECORDS  = QS_GRP_EQ,
    QS_MP_RECORDS  = QS_GRP_MP,
    QS_TE_RECORDS  = QS_GRP_TE,
    QS_QF_RECORDS  = QS_GRP_QF,
    QS_SC_RECORDS  = QS_GRP_SC,
    QS_SEM_RECORDS = QS_GRP_SEM,
    QS_MTX_RECORDS = QS_GRP_MTX,
    QS_U0_RECORDS  = QS_GRP_U0,
    QS_U1_RECORDS  = QS_GRP_U1,
    QS_U2_RECORDS  = QS_GRP_U2,
    QS_U3_RECORDS  = QS_GRP_U3,
    QS_U4_RECORDS  = QS_GRP_U4,
    QS_UA_RECORDS  = QS_GRP_UA,
};

// NOTE: deprecated QS::ObjKind_old provided in the QS class in qs.hpp

} // namespace QP

#endif // Q_SPY

// version 8.0.0 -------------------------------------------------------------
#if (QP_API_VERSION < 800)

//! @deprecated submachines no longer supported
#define QM_SM_STATE_DECL(subm_, state_) error "submachines no longer supported"

//! @deprecated submachines no longer supported
#define qm_super_sub(sm_state_) error "submachines no longer supported"

//! @deprecated submachines no longer supported
#define qm_tran_ep(tatbl_)      error "submachines no longer supported"

//! @deprecated submachines no longer supported
#define qm_tran_xp(xp_, tatbl_) error "submachines no longer supported"

//! @deprecated submachines no longer supported
#define qm_sm_exit(sm_state_)   error "submachines no longer supported"

#ifdef QEVT_DYN_CTOR
//! @deprecated please use #QEVT_PAR_INIT
#define QEVT_PAR_INIT
#endif

//! @deprecated plain 'char' is no longer forbidden in MISRA-C++:2023
using char_t = char;

//! @deprecated assertion failure handler, use custom error handler instead
//! Use Q_onError() instead.
#define Q_onAssert(module_, id_) Q_onError(module_, id_)

#ifdef Q_NASSERT

    // #Q_UNSAFE now replaces the functionality of Q_NASSERT
    #define Q_UNSAFE

    //! @deprecated general purpose assertion with user-specified ID
    //! number that **always** evaluates the `expr_` expression.
    #define Q_ALLEGE_ID(id_, expr_) (static_cast<void>(expr_))

#elif defined Q_UNSAFE

    //! @deprecated general purpose assertion with user-specified ID
    //! number that **always** evaluates the `expr_` expression.
    #define Q_ALLEGE_ID(id_, expr_) (static_cast<void>(expr_))

#else // QP FuSa Subsystem enabled

    //! @deprecated general purpose assertion with user-specified ID
    //! number that **always** evaluates the `expr_` expression.
    //! @note
    //! The use of this macro is no longer recommended.
    #define Q_ALLEGE_ID(id_, expr_) if (!(expr_)) { \
        QF_CRIT_STAT \
        QF_CRIT_ENTRY(); \
        Q_onError(&Q_this_module_[0], (id_)); \
        QF_CRIT_EXIT(); \
    } else ((void)0)

#endif

//! @deprecated general-purpose assertion that **always** evaluates
//! the `expr_` expression.
//! @note The use of this macro is no longer recommended.
#define Q_ALLEGE(expr_)         Q_ALLEGE_ID(__LINE__, (expr_))

//! Static (compile-time) assertion.
//! @deprecated
//! Use Q_ASSERT_STATIC() or better yet `static_assert()` instead.
#define Q_ASSERT_COMPILE(expr_) Q_ASSERT_STATIC(expr_)

#endif // QP_API_VERSION < 800
#endif // QP_API_VERSION < 810

#endif // QPCPP_HPP_
