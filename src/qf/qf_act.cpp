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
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QP::QActive services and QF support code

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

//============================================================================
namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qf_act")

} // unnamed namespace

//============================================================================
namespace QP {

// public objects
QActive *QF::active_[QF_MAX_ACTIVE + 1U]; // to be used by QF ports only

//............................................................................
void QF::add_(QActive * const a) noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(a->m_prio);

    Q_REQUIRE_ID(100, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (active_[p] == nullptr));
    QF_CRIT_STAT_
    QF_CRIT_E_();
    active_[p] = a;  // registger the active object at this priority
    QF_CRIT_X_();
}

//............................................................................
void QF::remove_(QActive * const a) noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(a->m_prio);

    Q_REQUIRE_ID(200, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (active_[p] == a));
    QF_CRIT_STAT_
    QF_CRIT_E_();
    active_[p] = nullptr; // free-up the priority level
    a->m_state.fun = nullptr; // invalidate the state
    QF_CRIT_X_();
}

//............................................................................
void QF::bzero(void * const start, std::uint_fast16_t const len) noexcept {
    std::uint8_t *ptr = static_cast<std::uint8_t *>(start);
    for (std::uint_fast16_t n = len; n > 0U; --n) {
        *ptr = 0U;
        ++ptr;
    }
}

} // namespace QP

//============================================================================
// Log-base-2 calculations ...
#ifndef QF_LOG2

extern "C" {

//............................................................................
std::uint_fast8_t QF_LOG2(QP::QPSetBits x) noexcept {
    static std::uint8_t const log2LUT[16] = {
        0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
        4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
    };
    std::uint_fast8_t n = 0U;
    QP::QPSetBits t;

#if (QF_MAX_ACTIVE > 16U)
    t = static_cast<QP::QPSetBits>(x >> 16U);
    if (t != 0U) {
        n += 16U;
        x = t;
    }
#endif
#if (QF_MAX_ACTIVE > 8U)
    t = (x >> 8U);
    if (t != 0U) {
        n += 8U;
        x = t;
    }
#endif
    t = (x >> 4U);
    if (t != 0U) {
        n += 4U;
        x = t;
    }
    return n + log2LUT[x];
}

} // extern "C"

#endif // QF_LOG2
