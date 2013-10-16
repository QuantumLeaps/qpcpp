//****************************************************************************
// Product: QF/C++
// Last Updated for Version: 5.1.1
// Date of the Last Update:  Oct 08, 2013
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
/// \brief QEQueue::get() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qeq_get")

//............................................................................
QEvt const *QEQueue::get(void) {
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    e = m_frontEvt;

    if (e != static_cast<QEvt const *>(0)) {        // is the queue not empty?
        QEQueueCtr nFree = m_nFree + static_cast<QEQueueCtr>(1);
        m_nFree = nFree;                           // upate the number of free

        if (nFree <= m_end) {            // any events in the the ring buffer?
            m_frontEvt = QF_PTR_AT_(m_ring, m_tail);   // remove from the tail
            if (m_tail == static_cast<QEQueueCtr>(0)) {       // need to wrap?
                m_tail = m_end;                                 // wrap around
            }
            --m_tail;

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET, QS::priv_.eqObjFilter, this)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                   // the signal of this event
                QS_OBJ_(this);                            // this queue object
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
                QS_EQC_(nFree);                      // number of free entries
            QS_END_NOCRIT_()
        }
        else {
            m_frontEvt = null_evt;                  // the queue becomes empty

                     // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT(nFree == (m_end + static_cast<QEQueueCtr>(1)));

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET_LAST, QS::priv_.eqObjFilter,
                             this)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                   // the signal of this event
                QS_OBJ_(this);                            // this queue object
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
            QS_END_NOCRIT_()
        }
    }
    QF_CRIT_EXIT_();
    return e;
}

}                                                              // namespace QP

