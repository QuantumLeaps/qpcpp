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
#include "qassert.h"

/// \file
/// \ingroup qf
/// \brief QF_timeEvtListHead_ definition and QTimeEvt::armX() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qte_arm")

// Package-scope objects -----------------------------------------------------
QTimeEvt QF::timeEvtHead_[QF_MAX_TICK_RATE];      // heads of time event lists

//............................................................................
void QTimeEvt::armX(QTimeEvtCtr const nTicks, QTimeEvtCtr const interval) {
    uint8_t tickRate = static_cast<uint8_t>(refCtr_ & u8_0x7F);
    QTimeEvtCtr cntr = m_ctr;
    QF_CRIT_STAT_

    Q_REQUIRE((m_act != null_void)                      /* AO must be valid */
              && (cntr == tc_0)                         /* must be disarmed */
              && (nTicks != tc_0)                /* cannot arm with 0 ticks */
              && (tickRate < static_cast<uint8_t>(QF_MAX_TICK_RATE))
              && (static_cast<enum_t>(sig) >= Q_USER_SIG));    // valid signal

    QF_CRIT_ENTRY_();
    m_ctr = nTicks;
    m_interval = interval;
    if ((refCtr_ & u8_0x80) == u8_0) {    // is the timer unlinked? see NOTE01
        refCtr_ |= u8_0x80;                                  // mark as linked
        m_next = QF::timeEvtHead_[tickRate].toTimeEvt();         // see NOTE02
        QF::timeEvtHead_[tickRate].m_act = this;
    }

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_ARM, QS::priv_.teObjFilter, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(this);                               // this time event object
        QS_OBJ_(m_act);                                   // the active object
        QS_TEC_(nTicks);                                // the number of ticks
        QS_TEC_(interval);                                     // the interval
        QS_U8_(tickRate);                                         // tick rate
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
}

}                                                              // namespace QP

//****************************************************************************
// NOTE01:
// For a duration of a single clock tick of the specified tick rate a time
// event can be disarmed and yet still linked into the list, because unlinking
// is performed exclusively in the QF::tickX() function.
//
// NOTE02:
// The time event is initially inserted into the separate "freshly armed"
// link list based on QF::timeEvtHead_[tickRate].m_act. Only later, inside
// the QF::tickX() function, the "freshly armed" list is appended to the
// main list of armed time events based on QF::timeEvtHead_[tickRate].m_next.
// Again, this is to keep any changes to the main list exclusively inside
// the QF::tickX() function.
//

