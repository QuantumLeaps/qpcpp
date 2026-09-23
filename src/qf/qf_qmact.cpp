//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005-2026 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License, version 3 or later (GPL-3.0-or-later), or,
// alternatively, under the terms of one of the closed-source Quantum Leaps
// commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
//
// NOTE:
// The GPL does NOT permit distributing this code as part of proprietary
// (closed-source) programs. For such use, please contact Quantum Leaps for
// a commercial license, which is an alternative to the GPL designed
// explicitly for closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {
//Q_DEFINE_THIS_MODULE("qf_qmact")
} // unnamed namespace

namespace QP {

//............................................................................
QMActive::QMActive(QStateHandler const initial) noexcept
  : QActive(initial)
{
    // NOTE: QMActive indirectly inherits the abstract QAsm base class,
    // but it will delegate the state machine behavior to the QMsm class,
    // so the following initialization is identical as in QMsm ctor:
    m_state.obj = QMsm::topQMState();
    m_temp.fun  = initial;
}
//............................................................................
void QMActive::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
    // delegate to the QMsm class
    reinterpret_cast<QMsm *>(this)->QMsm::init(e, qsId);
}
//............................................................................
void QMActive::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
    // delegate to the QMsm class
    reinterpret_cast<QMsm *>(this)->QMsm::dispatch(e, qsId);
}
//............................................................................
bool QMActive::isIn(QStateHandler const stateHndl) noexcept {
    // delegate to the QMsm class
    return reinterpret_cast<QMsm *>(this)->QMsm::isIn(stateHndl);
}
//............................................................................
QMState const * QMActive::childStateObj(QMState const * const parent) const noexcept {
    // delegate to the QMsm class
    return reinterpret_cast<QMsm const *>(this)
               ->QMsm::childStateObj(parent);
}
//............................................................................
QStateHandler QMActive::getStateHandler() const noexcept {
    // delegate to the QMsm class
    return reinterpret_cast<QMsm const *>(this)->QMsm::getStateHandler();
}

} // namespace QP
