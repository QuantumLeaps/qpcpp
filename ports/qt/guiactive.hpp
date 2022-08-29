//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
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
//============================================================================
//! @date Last updated on: 2022-08-25
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QP/C++ port to Qt

#ifndef GUIACTIVE_HPP
#define GUIACTIVE_HPP

namespace QP {

//............................................................................
class GuiQActive : public QActive {
public:
    GuiQActive(QStateHandler const initial) : QActive(initial) {}
    void start(QPrioSpec const prioSpec,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize,
               void const * const par) override;
    bool post_(QEvt const * const e, std::uint_fast16_t const margin,
               void const * const sender) noexcept override;
    void postLIFO(QEvt const * const e) noexcept override;
};

//............................................................................
class GuiQMActive : public QMActive {
public:
    GuiQMActive(QStateHandler const initial) : QMActive(initial) {}
    void start(QPrioSpec const prioSpec,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize,
               void const * const par) override;
    bool post_(QEvt const * const e, std::uint_fast16_t const margin,
                       void const * const sender) noexcept override;
    void postLIFO(QEvt const * const e) noexcept override;
};

} // namespace QP

#endif // GUIACTIVE_HPP
