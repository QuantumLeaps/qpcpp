//****************************************************************************
// Product: QF/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 27, 2013
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
/// \brief QEQueue::QEQueue() and QEQueue::init() implementation.

namespace QP {

//............................................................................
QEQueue::QEQueue(void)
  : m_frontEvt(null_evt),
    m_ring(static_cast<QEvt const **>(0)),
    m_end(static_cast<QEQueueCtr>(0)),
    m_head(static_cast<QEQueueCtr>(0)),
    m_tail(static_cast<QEQueueCtr>(0)),
    m_nFree(static_cast<QEQueueCtr>(0)),
    m_nMin(static_cast<QEQueueCtr>(0))
{}
//............................................................................
void QEQueue::init(QEvt const *qSto[], uint_t const qLen) {
    m_frontEvt = null_evt;                           // no events in the queue
    m_ring     = &qSto[0];
    m_end      = static_cast<QEQueueCtr>(qLen);
    m_head     = static_cast<QEQueueCtr>(0);
    m_tail     = static_cast<QEQueueCtr>(0);
    m_nFree    = static_cast<QEQueueCtr>(qLen + u_1);  //+1 for extra frontEvt
    m_nMin     = m_nFree;

    QS_CRIT_STAT_
    QS_BEGIN_(QS_QF_EQUEUE_INIT, QS::priv_.eqObjFilter, this)
        QS_OBJ_(this);                                  // this QEQueue object
        QS_EQC_(m_end);                             // the length of the queue
    QS_END_()
}

}                                                              // namespace QP

