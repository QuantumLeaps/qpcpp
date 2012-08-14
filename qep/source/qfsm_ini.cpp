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
/// \brief QFsm::init() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qfsm_ini")

//............................................................................
QFsm::~QFsm() {
}
//............................................................................
void QFsm::init(QEvt const * const e) {
    Q_REQUIRE((m_temp != Q_STATE_CAST(0))             // ctor must be executed
              && (m_state == Q_STATE_CAST(0))); // initial tran. NOT taken yet

    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_STATE_INIT, QS::smObj_, this)
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(Q_STATE_CAST(0));        // the source (not defined for a FSM)
        QS_FUN_(m_temp);                       // the target of the transition
    QS_END_()

                                    // execute the top-most initial transition
    Q_ALLEGE((*m_temp)(this, e) == Q_RET_TRAN);    // transition must be taken

    (void)QEP_TRIG_(m_temp, Q_ENTRY_SIG);                  // enter the target
    m_state = m_temp;                           // record the new active state

    QS_BEGIN_(QS_QEP_INIT_TRAN, QS::smObj_, this)
        QS_TIME_();                                              // time stamp
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(m_state);                              // the new active state
    QS_END_()
}

QP_END_
