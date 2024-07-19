//$file${include::qp_pkg.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${include::qp_pkg.hpp}
//
// This code has been generated by QM 6.2.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2025-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${include::qp_pkg.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef QP_PKG_HPP_
#define QP_PKG_HPP_

//$declare${QF::QF-pkg} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QF {

//${QF::QF-pkg::Attr} ........................................................
class Attr {
public:

#if (QF_MAX_EPOOL > 0U)
    QF_EPOOL_TYPE_ ePool_[QF_MAX_EPOOL];
#endif //  (QF_MAX_EPOOL > 0U)

#if (QF_MAX_EPOOL > 0U)
    std::uint_fast8_t maxPool_;
#endif //  (QF_MAX_EPOOL > 0U)

#if (QF_MAX_EPOOL == 0U)
    std::uint8_t dummy;
#endif //  (QF_MAX_EPOOL == 0U)
}; // class Attr

//${QF::QF-pkg::priv_} .......................................................
extern QF::Attr priv_;

//${QF::QF-pkg::bzero_} ......................................................
void bzero_(
    void * const start,
    std::uint_fast16_t const len) noexcept;

} // namespace QF
} // namespace QP
//$enddecl${QF::QF-pkg} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define QF_CONST_CAST_(type_, ptr_) const_cast<type_>(ptr_)
#define QF_PTR_RANGE_(x_, min_, max_)  (((min_) <= (x_)) && ((x_) <= (max_)))
#define Q_UINTPTR_CAST_(ptr_) (reinterpret_cast<std::uintptr_t>(ptr_))
#define Q_ACTION_CAST(act_)   (reinterpret_cast<QP::QActionHandler>(act_))

namespace QP {

// Bitmasks are for the QTimeEvt::refCtr_ attribute (inherited from QEvt).
// In QTimeEvt this attribute is NOT used for reference counting.
constexpr std::uint8_t TE_IS_LINKED    = 1U << 7U;  // flag
constexpr std::uint8_t TE_WAS_DISARMED = 1U << 6U;  // flag
constexpr std::uint8_t TE_TICK_RATE    = 0x0FU;     // bitmask

inline void QEvt_refCtr_inc_(QEvt const * const e) noexcept {
    (QF_CONST_CAST_(QEvt*, e))->refCtr_ = e->refCtr_ + 1U;
}

inline void QEvt_refCtr_dec_(QEvt const * const e) noexcept {
    (QF_CONST_CAST_(QEvt*, e))->refCtr_ = e->refCtr_ - 1U;
}

} // namespace QP

#endif // QP_PKG_HPP_
