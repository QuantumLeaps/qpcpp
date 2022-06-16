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
//! @date Last updated on: 2022-06-07
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QMActive::QMActive() and virtual functions

#define QP_IMPL           // this is QP implementation
#include "qf_port.hpp"    // QF port
#include "qassert.h"      // QP embedded systems-friendly assertions

//! Internal macro to cast a QP::QMActive pointer @p qact_ to QP::QMsm*
//! @note
//! Casting pointer to pointer pointer violates the MISRA-C++ 2008 Rule 5-2-7,
//! cast from pointer to pointer. Additionally this cast violates the MISRA-
//! C++ 2008 Rule 5-2-8 Unusual pointer cast (incompatible indirect types).
//! Encapsulating these violations in a macro allows to selectively suppress
//! this specific deviation.
#define QF_QMACTIVE_TO_QMSM_CAST_(qact_) \
    reinterpret_cast<QMsm *>((qact_))

//! Internal macro to cast a QP::QMActive pointer @p qact_ to QP::QMsm const *
#define QF_QMACTIVE_TO_QMSM_CONST_CAST_(qact_) \
    reinterpret_cast<QMsm const *>((qact_))

// unnamed namespace for local definitions with internal linkage
namespace {

//Q_DEFINE_THIS_MODULE("qf_qmact")

} // unnamed namespace

//============================================================================
namespace QP {

//............................................................................
QMActive::QMActive(QStateHandler const initial) noexcept
  : QActive(initial)
{
    m_temp.fun  = initial;
}

//............................................................................
void QMActive::init(void const * const e, std::uint_fast8_t const qs_id) {
    m_state.obj = &QMsm::msm_top_s;
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::init(e, qs_id);
}
//............................................................................
void QMActive::init(std::uint_fast8_t const qs_id) {
    m_state.obj = &QMsm::msm_top_s;
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::init(qs_id);
}
//............................................................................
void QMActive::dispatch(QEvt const * const e,
                          std::uint_fast8_t const qs_id)
{
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::dispatch(e, qs_id);
}

//............................................................................
bool QMActive::isInState(QMState const * const st) const noexcept {
    return QF_QMACTIVE_TO_QMSM_CONST_CAST_(this)->QMsm::isInState(st);
}
//............................................................................
QMState const *QMActive::childStateObj(QMState const * const parent)
    const noexcept
{
    return QF_QMACTIVE_TO_QMSM_CONST_CAST_(this)
               ->QMsm::childStateObj(parent);
}

//============================================================================
#ifdef Q_SPY

QStateHandler QMActive::getStateHandler() noexcept {
    return QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::getStateHandler();
}

#endif

} // namespace QP
