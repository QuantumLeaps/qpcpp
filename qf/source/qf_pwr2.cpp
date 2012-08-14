//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 19, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qf_pkg.h"

/// \file
/// \ingroup qf
/// \brief QF_pwr2Lkup[], QF_invPwr2Lkup[], and QF_div8Lkup[] definitions.

QP_BEGIN_

// Global objects ------------------------------------------------------------
uint8_t const Q_ROM Q_ROM_VAR QF_pwr2Lkup[65] = {
    static_cast<uint8_t>(0x00),                             // unused location
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80),
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80),
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80),
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80),
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80),
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80),
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80),
    static_cast<uint8_t>(0x01), static_cast<uint8_t>(0x02),
    static_cast<uint8_t>(0x04), static_cast<uint8_t>(0x08),
    static_cast<uint8_t>(0x10), static_cast<uint8_t>(0x20),
    static_cast<uint8_t>(0x40), static_cast<uint8_t>(0x80)
};

uint8_t const Q_ROM Q_ROM_VAR QF_invPwr2Lkup[65] = {
    static_cast<uint8_t>(0xFF),                             // unused location
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F),
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F),
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F),
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F),
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F),
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F),
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F),
    static_cast<uint8_t>(0xFE), static_cast<uint8_t>(0xFD),
    static_cast<uint8_t>(0xFB), static_cast<uint8_t>(0xF7),
    static_cast<uint8_t>(0xEF), static_cast<uint8_t>(0xDF),
    static_cast<uint8_t>(0xBF), static_cast<uint8_t>(0x7F)
};

uint8_t const Q_ROM Q_ROM_VAR QF_div8Lkup[65] = {
    static_cast<uint8_t>(0),                                // unused location
    static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(0),
    static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(0),
    static_cast<uint8_t>(0), static_cast<uint8_t>(0),
    static_cast<uint8_t>(1), static_cast<uint8_t>(1), static_cast<uint8_t>(1),
    static_cast<uint8_t>(1), static_cast<uint8_t>(1), static_cast<uint8_t>(1),
    static_cast<uint8_t>(1), static_cast<uint8_t>(1),
    static_cast<uint8_t>(2), static_cast<uint8_t>(2), static_cast<uint8_t>(2),
    static_cast<uint8_t>(2), static_cast<uint8_t>(2), static_cast<uint8_t>(2),
    static_cast<uint8_t>(2), static_cast<uint8_t>(2),
    static_cast<uint8_t>(3), static_cast<uint8_t>(3), static_cast<uint8_t>(3),
    static_cast<uint8_t>(3), static_cast<uint8_t>(3), static_cast<uint8_t>(3),
    static_cast<uint8_t>(3), static_cast<uint8_t>(3),
    static_cast<uint8_t>(4), static_cast<uint8_t>(4), static_cast<uint8_t>(4),
    static_cast<uint8_t>(4), static_cast<uint8_t>(4), static_cast<uint8_t>(4),
    static_cast<uint8_t>(4), static_cast<uint8_t>(4),
    static_cast<uint8_t>(5), static_cast<uint8_t>(5), static_cast<uint8_t>(5),
    static_cast<uint8_t>(5), static_cast<uint8_t>(5), static_cast<uint8_t>(5),
    static_cast<uint8_t>(5), static_cast<uint8_t>(5),
    static_cast<uint8_t>(6), static_cast<uint8_t>(6), static_cast<uint8_t>(6),
    static_cast<uint8_t>(6), static_cast<uint8_t>(6), static_cast<uint8_t>(6),
    static_cast<uint8_t>(6), static_cast<uint8_t>(6),
    static_cast<uint8_t>(7), static_cast<uint8_t>(7), static_cast<uint8_t>(7),
    static_cast<uint8_t>(7), static_cast<uint8_t>(7), static_cast<uint8_t>(7),
    static_cast<uint8_t>(7), static_cast<uint8_t>(7)
};

QP_END_
