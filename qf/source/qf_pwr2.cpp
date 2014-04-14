/// \file
/// \brief QP::QF_pwr2Lkup[], QP::QF_invPwr2Lkup[], QP::QF_div8Lkup[]
/// definitions.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-09
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"

namespace QP {

// Global objects ************************************************************
uint8_t const Q_ROM QF_pwr2Lkup[65] = {
    static_cast<uint8_t>(0x00), // unused location
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

uint8_t const Q_ROM QF_invPwr2Lkup[65] = {
    static_cast<uint8_t>(0xFF), // unused location
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

uint8_t const Q_ROM QF_div8Lkup[65] = {
    static_cast<uint8_t>(0), // unused location
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

} // namespace QP

