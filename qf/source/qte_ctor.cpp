//****************************************************************************
// Product: QF/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 03, 2013
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
/// \brief QTimeEvt::QTimeEvt() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qte_ctor")

//............................................................................
QTimeEvt::QTimeEvt(QActive * const act,
                   enum_t const sgnl, uint8_t const tickRate)
  :
#ifdef Q_EVT_CTOR
    QEvt(static_cast<QSignal>(sgnl)),
#endif
    m_next(null_tevt),
    m_act(act),
    m_ctr(tc_0),
    m_interval(tc_0)
{
    Q_REQUIRE(tickRate < static_cast<uint8_t>(QF_MAX_TICK_RATE));// valid rate

#ifndef Q_EVT_CTOR
    Q_REQUIRE(sgnl >= Q_USER_SIG);                     // signal must be valid
    sig = static_cast<QSignal>(sgnl);      // set QEvt::sig of this time event
#endif
                                      // time event must be static, see NOTE01
    poolId_ = u8_0;                                 // not from any event pool
    refCtr_ = tickRate;                                          // see NOTE02
}
//............................................................................
QTimeEvt::QTimeEvt()             // private default ctor for internal use only
  :
#ifdef Q_EVT_CTOR
    QEvt(static_cast<QSignal>(0)),
#endif
    m_next(null_tevt),
    m_act(null_act),                                             // see NOTE03
    m_ctr(tc_0),
    m_interval(tc_0)
{
#ifndef Q_EVT_CTOR
    sig = static_cast<QSignal>(0);
#endif
                                      // time event must be static, see NOTE01
    poolId_ = u8_0;                                 // not from any event pool
    refCtr_ = u8_0;                                         //  default rate 0
}

}                                                              // namespace QP

//****************************************************************************
// NOTE01:
// Setting the POOL_ID event attribute to zero is correct only for events not
// allocated from event pools, which must be the case for Time Events.
//
// NOTE02:
// The reference counter attribute is not used in static events, so for the
// Time Events it is reused to hold the tickRate in the bits [0..6] and the
// linkedFlag in the MSB (bit [7]). The linkedFlag is 0 for time events
// unlinked from any list and 1 otherwise.
//
