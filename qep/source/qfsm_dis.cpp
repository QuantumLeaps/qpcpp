//****************************************************************************
// Product: QEP/C++
// Last Updated for Version: 5.1.0
// Date of the Last Update:  Sep 28, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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
//****************************************************************************
#include "qep_pkg.h"
#include "qassert.h"

/// \file
/// \ingroup qep
/// \brief QFsm::dispatch() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qfsm_dis")

//............................................................................
void QFsm::dispatch(QEvt const * const e) {
    Q_REQUIRE(m_state.fun == m_temp.fun);// state configuration must be stable

    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_DISPATCH, QS::priv_.smObjFilter, this)
        QS_TIME_();                                              // time stamp
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(m_state.fun);                             // the current state
    QS_END_()

    QState r = (*m_state.fun)(this, e);              // call the event handler
    if (r == Q_RET_TRAN) {                                // transition taken?

        QS_BEGIN_(QS_QEP_TRAN, QS::priv_.smObjFilter, this)
            QS_TIME_();                                          // time stamp
            QS_SIG_(e->sig);                        // the signal of the event
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_(m_state.fun);              // the source of the transition
            QS_FUN_(m_temp.fun);               // the target of the transitoin
        QS_END_()

        QEP_EXIT_(m_state.fun);                             // exit the source
        QEP_ENTER_(m_temp.fun);                            // enter the target
        m_state.fun = m_temp.fun;               // record the new active state
    }
    else {
#ifdef Q_SPY

        if (r == Q_RET_UNHANDLED) {
            QS_BEGIN_(QS_QEP_UNHANDLED, QS::priv_.smObjFilter, this)
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state.fun);                     // the current state
            QS_END_()
        }

        if (r == Q_RET_HANDLED) {

            QS_BEGIN_(QS_QEP_INTERN_TRAN, QS::priv_.smObjFilter, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state.fun);      // the state that handled the event
            QS_END_()

        }
        else {

            QS_BEGIN_(QS_QEP_IGNORED, QS::priv_.smObjFilter, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state.fun);                     // the current state
            QS_END_()

        }
#endif
    }
}

}                                                              // namespace QP


