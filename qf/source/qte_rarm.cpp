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
/// \brief QTimeEvt::rearm() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qte_rarm")

//............................................................................
bool QTimeEvt::rearm(QTimeEvtCtr const nTicks) {
    Q_REQUIRE((nTicks > tc_0)                  /* cannot rearm with 0 ticks */
              && (m_act != null_act)         /* active object must be valid */
              && (static_cast<enum_t>(sig) >= Q_USER_SIG));    // valid signal

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    bool isArmed;
    if (m_ctr == tc_0) {                       // is this time event disarmed?
        isArmed = false;
        m_next = QF_timeEvtListHead_;
        if (QF_timeEvtListHead_ != null_tevt) {
            m_next = QF_timeEvtListHead_;
            QF_timeEvtListHead_ = this;
            QF_EVT_REF_CTR_INC_(this);                       // mark as linked
        }
    }
    else {
        isArmed = true;
    }
    m_ctr = nTicks;            // re-load the tick counter (shift the phasing)

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_REARM, QS::teObj_, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(this);                               // this time event object
        QS_OBJ_(m_act);                                   // the active object
        QS_TEC_(m_ctr);                                 // the number of ticks
        QS_TEC_(m_interval);                                   // the interval
        QS_U8_(isArmed ? u8_1 : u8_0);                 // was the timer armed?
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
    return isArmed;
}

QP_END_

