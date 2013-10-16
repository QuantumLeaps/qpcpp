//****************************************************************************
// Product: QF/C++
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
#include "qf_pkg.h"

/// \file
/// \ingroup qf
/// \brief QTimeEvt::ctr() implementation.

namespace QP {

//............................................................................
QTimeEvtCtr QTimeEvt::ctr(void) const {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QTimeEvtCtr ret = m_ctr;

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_CTR, QS::priv_.teObjFilter, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(this);                               // this time event object
        QS_OBJ_(m_act);                                       // the target AO
        QS_TEC_(ret);                                   // the current counter
        QS_TEC_(m_interval);                                   // the interval
        QS_U8_(static_cast<uint8_t>(refCtr_ & u8_0x7F));          // tick rate
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
    return ret;
}

}                                                              // namespace QP

