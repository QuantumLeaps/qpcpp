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
/// \brief QMsm::init() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qmsm_ini")

//............................................................................
QActionHandler const QMsm::s_emptyAction_[1] = {
    Q_ACTION_CAST(0)
};

//............................................................................
QMsm::~QMsm() {                                                // virtual xtor
}
//............................................................................
void QMsm::init(QEvt const * const e) {
#ifdef Q_SPY
    QMState const *t = static_cast<QMState const *>(0); // current state (top)
    QS_CRIT_STAT_
#endif

    Q_REQUIRE((m_temp.fun != Q_STATE_CAST(0))         // ctor must be executed
        && (m_state.obj == static_cast<QMState const *>(0)));// ini. NOT taken

    QState r = (*m_temp.fun)(this, e);   // execute the top-most initial tran.
    Q_ASSERT(r == Q_RET_INITIAL);               // initial tran. must be taken

    while (r == Q_RET_INITIAL) {

        QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_((t == static_cast<QMState const *>(0))
                    ? Q_STATE_CAST(0)
                    : t->stateHandler);             // source of initial tran.
            QS_FUN_(m_state.obj->stateHandler);        // target state handler
        QS_END_()

#ifdef Q_SPY
        t = m_state.obj;                 // store the target of the transition
#endif
        r = static_cast<QState>(0);             // invalidate the return value
        for (QActionHandler const *a = m_temp.act;
             *a != Q_ACTION_CAST(0);
             QEP_ACT_PTR_INC_(a))
        {
            r = (*(*a))(this);                           // execute the action
#ifdef Q_SPY
            if (r == Q_RET_ENTRY) {
                QS_BEGIN_(QS_QEP_STATE_ENTRY, QS::priv_.smObjFilter, this)
                    QS_OBJ_(this);
                    QS_FUN_(m_temp.obj->stateHandler);        // entered state
                QS_END_()
            }
#endif
        }
    }

    QS_BEGIN_(QS_QEP_INIT_TRAN, QS::priv_.smObjFilter, this)
        QS_TIME_();                                              // time stamp
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(m_state.obj->stateHandler);           // the new current state
    QS_END_()
}

}                                                              // namespace QP


