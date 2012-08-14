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
#include "qassert.h"

/// \file
/// \ingroup qf
/// \brief QActive::subscribe() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qa_sub")

//............................................................................
void QActive::subscribe(enum_t const sig) const {
    uint8_t p = m_prio;
    Q_REQUIRE((Q_USER_SIG <= sig)
              && (sig < QF_maxSignal_)
              && (u8_0 < p) && (p <= static_cast<uint8_t>(QF_MAX_ACTIVE))
              && (QF::active_[p] == this));

    uint8_t const i = Q_ROM_BYTE(QF_div8Lkup[p]);

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_SUBSCRIBE, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(sig);                              // the signal of this event
        QS_OBJ_(this);                                   // this active object
    QS_END_NOCRIT_()
                                                       // set the priority bit
    QF_PTR_AT_(QF_subscrList_, sig).m_bits[i] |= Q_ROM_BYTE(QF_pwr2Lkup[p]);
    QF_CRIT_EXIT_();
}

QP_END_
