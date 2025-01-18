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
#ifndef QP_PKG_HPP_
#define QP_PKG_HPP_

//============================================================================
#define QF_CONST_CAST_(type_, ptr_)    const_cast<type_>(ptr_)
#define Q_PTR2UINT_CAST_(ptr_)         (reinterpret_cast<std::uintptr_t>(ptr_))
#define QF_PTR_RANGE_(x_, min_, max_)  (((min_) <= (x_)) && ((x_) <= (max_)))

//============================================================================
namespace QP {
namespace QF {

class Attr {
public:

#if (QF_MAX_EPOOL > 0U)
    QF_EPOOL_TYPE_ ePool_[QF_MAX_EPOOL];
    std::uint_fast8_t maxPool_;
#else
    std::uint8_t dummy;
#endif //  (QF_MAX_EPOOL == 0U)
}; // class Attr

extern QF::Attr priv_;

void bzero_(
    void * const start,
    std::uint_fast16_t const len) noexcept;

} // namespace QF

//============================================================================
// Bitmasks are for the QTimeEvt::flags attribute
constexpr std::uint8_t QTE_FLAG_IS_LINKED    {1U << 7U};
constexpr std::uint8_t QTE_FLAG_WAS_DISARMED {1U << 6U};

//============================================================================
inline void QEvt_refCtr_inc_(QEvt const * const e) noexcept {
    // NOTE: this function must be called inside a critical section
    std::uint8_t rc = e->refCtr_ + 1U;
    (QF_CONST_CAST_(QEvt*, e))->refCtr_ = rc; // cast away 'const'
}

inline void QEvt_refCtr_dec_(QEvt const * const e) noexcept {
    // NOTE: this function must be called inside a critical section
    std::uint8_t rc = e->refCtr_ - 1U;
    (QF_CONST_CAST_(QEvt*, e))->refCtr_ = rc; // cast away 'const'
}

} // namespace QP

#endif // QP_PKG_HPP_
