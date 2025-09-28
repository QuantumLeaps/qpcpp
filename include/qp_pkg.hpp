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
#ifndef QP_PKG_HPP_
#define QP_PKG_HPP_

#include <array>    // for std::array

#ifdef QP_IMPL

namespace QP {

extern std::array<QActive*, QF_MAX_ACTIVE + 1U> QActive_registry_;
extern QSubscrList *QActive_subscrList_;
extern QSignal QActive_maxPubSignal_;

#if (QF_MAX_TICK_RATE > 0U)
extern std::array<QTimeEvt, QF_MAX_TICK_RATE> QTimeEvt_head_;

// Bitmasks are for the QTimeEvt::flags attribute
constexpr std::uint8_t QTE_FLAG_IS_LINKED    {1U << 7U};
constexpr std::uint8_t QTE_FLAG_WAS_DISARMED {1U << 6U};

#endif // (QF_MAX_TICK_RATE > 0U)

//============================================================================
void QEvt_refCtr_inc_(QEvt const * const me) noexcept;
void QEvt_refCtr_dec_(QEvt const * const me) noexcept;

//============================================================================
namespace QF {

class Attr {
public:

#if (QF_MAX_EPOOL > 0U)
    std::array<QF_EPOOL_TYPE_, QF_MAX_EPOOL> ePool_;
    std::uint8_t maxPool_;
#else
    std::uint8_t dummy;
#endif // (QF_MAX_EPOOL == 0U)
}; // class Attr

extern QF::Attr priv_;

} // namespace QF
} // namespace QP

#endif // QP_IMPL

#endif // QP_PKG_HPP_
