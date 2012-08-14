//////////////////////////////////////////////////////////////////////////////
// Product: QEP/C++
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
#include "qep_pkg.h"
#include "qassert.h"

/// \file
/// \ingroup qep
/// \brief QHsm::isIn() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qhsm_in")

//............................................................................
bool QHsm::isIn(QStateHandler const s) {
    Q_REQUIRE(m_temp == m_state);        // state configuration must be stable

    bool inState = false;            // assume that this HSM is not in 'state'
    QState r;
    do {
        if (m_temp == s) {                             // do the states match?
            inState = true;                        // match found, return TRUE
            r = Q_RET_IGNORED;                        // break out of the loop
        }
        else {
            r = QEP_TRIG_(m_temp, QEP_EMPTY_SIG_);
        }
    } while (r != Q_RET_IGNORED);                // QHsm_top state not reached
    m_temp = m_state;                // restore the stable state configuration
    return inState;                                       // return the status
}

QP_END_
