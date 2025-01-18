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
//Q_DEFINE_THIS_MODULE("qf_act")
} // unnamed namespace

namespace QP {

// QP version string embedded in the binary image
char const versionStr[] = "QP/C++ " QP_VERSION_STR;

QActive * QActive::registry_[QF_MAX_ACTIVE + 1U];

namespace QF {

QF::Attr priv_;

void bzero_(
    void * const start,
    std::uint_fast16_t const len) noexcept
{
    std::uint8_t *ptr = static_cast<std::uint8_t *>(start);
    for (std::uint_fast16_t n = len; n > 0U; --n) {
        *ptr = 0U;
        ++ptr;
    }
}

} // namespace QF

//............................................................................
#ifndef QF_LOG2
std::uint_fast8_t QF_LOG2(QP::QPSetBits const bitmask) noexcept {
    static constexpr std::uint8_t log2LUT[16] = {
        0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
        4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
    };
    std::uint_fast8_t n = 0U;
    QP::QPSetBits x = bitmask;
    QP::QPSetBits tmp;

#if (QF_MAX_ACTIVE > 16U)
    tmp = static_cast<QP::QPSetBits>(x >> 16U);
    if (tmp != 0U) {
        n += 16U;
        x = tmp;
    }
#endif
#if (QF_MAX_ACTIVE > 8U)
    tmp = (x >> 8U);
    if (tmp != 0U) {
        n += 8U;
        x = tmp;
    }
#endif
    tmp = (x >> 4U);
    if (tmp != 0U) {
        n += 4U;
        x = tmp;
    }
    return n + log2LUT[x];
}
#endif // ndef QF_LOG2

//............................................................................
#ifndef Q_UNSAFE
QPtrDis::QPtrDis(void const * const ptr) noexcept
  : m_ptr_dis(static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(ptr)))
{}
#endif

} // namespace QP

