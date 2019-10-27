/// @file
/// @brief QK/C++ port to ARM, preemptive QK kernel, generic C++ compiler
/// @cond
///***************************************************************************
/// Last updated for version 6.6.0
/// Last updated on  2019-07-30
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef QK_PORT_HPP
#define QK_PORT_HPP

extern "C" {
    void QK_irq(void);

    void BSP_irq(void);
    // void BSP_fiq(void);  see NOTE1
}

#include "qk.hpp" // QK platform-independent public interface

//****************************************************************************
// NOTE1:
// The FIQ-type interrupts are never disabled in this port, so the FIQ is
// a "kernel-unaware" interrupt.
//
// If the FIQ is ever used in the application, it must be implemented in
// assembly.
//

#endif // QK_PORT_HPP

