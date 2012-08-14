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
//#include "qassert.h"

/// \file
/// \ingroup qf
/// \brief QEQueue::get() implementation.

QP_BEGIN_

//Q_DEFINE_THIS_MODULE("qeq_get")

//............................................................................
QEvt const *QEQueue::get(void) {
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    if (m_frontEvt == null_evt) {                       // is the queue empty?
        e = null_evt;                       // no event available at this time
    }
    else {
        e = m_frontEvt;

        if (m_nFree != m_end) {          // any events in the the ring buffer?
            m_frontEvt = QF_PTR_AT_(m_ring, m_tail);   // remove from the tail
            if (m_tail == static_cast<QEQueueCtr>(0)) {       // need to wrap?
                m_tail = m_end;                                 // wrap around
            }
            --m_tail;

            ++m_nFree;               // one more free event in the ring buffer

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET, QS::eqObj_, this)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                   // the signal of this event
                QS_OBJ_(this);                            // this queue object
                QS_U8_(QF_EVT_POOL_ID_(e));        // the pool Id of the event
                QS_U8_(QF_EVT_REF_CTR_(e));      // the ref count of the event
                QS_EQC_(m_nFree);                    // number of free entries
            QS_END_NOCRIT_()
        }
        else {
            m_frontEvt = null_evt;                  // the queue becomes empty

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET_LAST, QS::eqObj_, this)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                   // the signal of this event
                QS_OBJ_(this);                            // this queue object
                QS_U8_(QF_EVT_POOL_ID_(e));        // the pool Id of the event
                QS_U8_(QF_EVT_REF_CTR_(e));      // the ref count of the event
            QS_END_NOCRIT_()
        }
    }
    QF_CRIT_EXIT_();
    return e;
}

QP_END_
