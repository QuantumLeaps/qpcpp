//****************************************************************************
// Product: QEP/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 26, 2013
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
/// \brief QHsm ctor and QHsm::init() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qhsm_ini")

//............................................................................
QHsm::QHsm(QStateHandler const initial)
  : QMsm(initial)
{
    m_state.fun = Q_STATE_CAST(&QHsm::top);
}
//............................................................................
void QHsm::init(QEvt const * const e) {
    QStateHandler t = m_state.fun;

    Q_REQUIRE((m_temp.fun != Q_STATE_CAST(0))         // ctor must be executed
              && (t == Q_STATE_CAST(&QHsm::top)));  // initial tran. NOT taken

                              // the top-most initial transition must be taken
    Q_ALLEGE((*m_temp.fun)(this, e) == Q_RET_TRAN);

    QS_CRIT_STAT_
    do {                                           // drill into the target...
        QStateHandler path[MAX_NEST_DEPTH];
        int_t ip = s_0;                         // transition entry path index

        QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_(t);                                    // the source state
            QS_FUN_(m_temp.fun);       // the target of the initial transition
        QS_END_()

        path[0] = m_temp.fun;
        (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);
        while (m_temp.fun != t) {
            ++ip;
            path[ip] = m_temp.fun;
            (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);
        }
        m_temp.fun = path[0];
                                               // entry path must not overflow
        Q_ASSERT(ip < MAX_NEST_DEPTH);

        do {           // retrace the entry path in reverse (desired) order...
            QEP_ENTER_(path[ip]);                            // enter path[ip]
            --ip;
        } while (ip >= s_0);

        t = path[0];                   // current state becomes the new source
    } while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN);

    QS_BEGIN_(QS_QEP_INIT_TRAN, QS::priv_.smObjFilter, this)
        QS_TIME_();                                              // time stamp
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(t);                                    // the new active state
    QS_END_()

    m_state.fun = t;                        // change the current active state
    m_temp.fun  = t;                       // mark the configuration as stable
}

}                                                              // namespace QP


