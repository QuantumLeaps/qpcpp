//////////////////////////////////////////////////////////////////////////////
// Product: QEP/C++
// Last Updated for Version: 4.5.01
// Date of the Last Update:  Jun 13, 2012
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
/// \brief QFsm::dispatch() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qfsm_dis")

//............................................................................
void QFsm::dispatch(QEvt const * const e) {
    Q_REQUIRE(m_state == m_temp);        // state configuration must be stable

    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_DISPATCH, QS::smObj_, this)
        QS_TIME_();                                              // time stamp
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(m_state);                                 // the current state
    QS_END_()

    QState r = (*m_state)(this, e);                  // call the event handler
    if (r == Q_RET_TRAN) {                                // transition taken?

        QS_BEGIN_(QS_QEP_TRAN, QS::smObj_, this)
            QS_TIME_();                                          // time stamp
            QS_SIG_(e->sig);                        // the signal of the event
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_(m_state);                  // the source of the transition
            QS_FUN_(m_temp);                   // the target of the transitoin
        QS_END_()

        QEP_EXIT_(m_state);                                 // exit the source
        QEP_ENTER_(m_temp);                                // enter the target
        m_state = m_temp;                       // record the new active state
    }
    else {
#ifdef Q_SPY

        if (r == Q_RET_UNHANDLED) {
            QS_BEGIN_(QS_QEP_UNHANDLED, QS::smObj_, this)
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state);                         // the current state
            QS_END_()
        }

        if (r == Q_RET_HANDLED) {

            QS_BEGIN_(QS_QEP_INTERN_TRAN, QS::smObj_, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state);          // the state that handled the event
            QS_END_()

        }
        else {

            QS_BEGIN_(QS_QEP_IGNORED, QS::smObj_, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state);                         // the current state
            QS_END_()

        }
#endif
    }
}

QP_END_

