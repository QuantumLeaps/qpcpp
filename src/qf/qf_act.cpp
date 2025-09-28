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
Q_DEFINE_THIS_MODULE("qf_act")
} // unnamed namespace

//----------------------------------------------------------------------------
namespace QP {

// QP version string embedded in the binary image
static constexpr char versionStr[24] = "QP/C++ " QP_VERSION_STR;

//............................................................................
char const *version() noexcept {
    // public access to versionStr[] so that it won't be eliminated as unused
    return &versionStr[0];
}
//............................................................................
void QEvt_refCtr_inc_(QEvt const * const me) noexcept {
    // NOTE: this function must be called *inside* a critical section

    // the event reference count must not exceed the number of AOs
    // in the system plus each AO possibly holding one event reference
    Q_REQUIRE_INCRIT(200, me->refCtr_ < (QF_MAX_ACTIVE + QF_MAX_ACTIVE));

    QEvt * const mut_me = const_cast<QEvt *>(me); // cast 'const' away
    ++mut_me->refCtr_;
}
//............................................................................
void QEvt_refCtr_dec_(QEvt const * const me) noexcept {
    // NOTE: this function must be called inside a critical section
    QEvt * const mut_me = const_cast<QEvt *>(me); // cast 'const' away
    --mut_me->refCtr_;
}

//----------------------------------------------------------------------------
QAsm::QAsm() noexcept // default QAsm ctor
  : m_state(),
    m_temp ()
{}
//............................................................................
QState QAsm::top(void * const me, QEvt const * const e) noexcept {
    Q_UNUSED_PAR(me);
    Q_UNUSED_PAR(e);
    return Q_RET_IGNORED; // the top state ignores all events
}
//............................................................................
void QAsm::init(std::uint_fast8_t const qsId) {
    // this init() overload delegates to the init() without margin parameter
    this->init(nullptr, qsId);
}

} // namespace QP
