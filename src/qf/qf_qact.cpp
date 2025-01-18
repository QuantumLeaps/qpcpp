//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Version 8.0.2
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
Q_DEFINE_THIS_MODULE("qf_qact")
} // unnamed namespace

namespace QP {

//............................................................................
void QActive::register_() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    if (m_pthre == 0U) { // preemption-threshold not defined?
        m_pthre = m_prio; // apply the default
    }

#ifndef Q_UNSAFE
    Q_REQUIRE_INCRIT(100,
        (0U < m_prio) && (m_prio <= QF_MAX_ACTIVE)
        && (registry_[m_prio] == nullptr)
        && (m_prio <= m_pthre));

    std::uint8_t prev_thre = m_pthre;
    std::uint8_t next_thre = m_pthre;

    std::uint_fast8_t p;
    for (p = static_cast<std::uint_fast8_t>(m_prio) - 1U; p > 0U; --p) {
        if (registry_[p] != nullptr) {
            prev_thre = registry_[p]->m_pthre;
            break;
        }
    }
    for (p = static_cast<std::uint_fast8_t>(m_prio) + 1U;
         p <= QF_MAX_ACTIVE; ++p)
    {
        if (registry_[p] != nullptr) {
            next_thre = registry_[p]->m_pthre;
            break;
        }
    }

    Q_ASSERT_INCRIT(190,
        (prev_thre <= m_pthre)
        && (m_pthre <= next_thre));
#endif // Q_UNSAFE

    // register the AO at the QF-prio.
    registry_[m_prio] = this;

    QF_CRIT_EXIT();
}

//............................................................................
void QActive::unregister_() noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (registry_[p] == this));
    registry_[p] = nullptr; // free-up the priority level
    m_state.fun = nullptr; // invalidate the state

    QF_CRIT_EXIT();
}

} // namespace QP
