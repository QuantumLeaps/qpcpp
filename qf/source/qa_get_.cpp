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
/// \brief QActive::get_() and QF::getQueueMargin() definitions.
///
/// \note this source file is only included in the QF library when the native
/// QF active object queue is used (instead of a message queue of an RTOS).

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qa_get_")

//............................................................................
QEvt const *QActive::get_(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QACTIVE_EQUEUE_WAIT_(this);           // wait for event to arrive directly

    QEvt const *e = m_eQueue.m_frontEvt;

    if (m_eQueue.m_nFree != m_eQueue.m_end) { //any events in the ring buffer?
                                                 // remove event from the tail
        m_eQueue.m_frontEvt = QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_tail);
        if (m_eQueue.m_tail == static_cast<QEQueueCtr>(0)) {  // need to wrap?
            m_eQueue.m_tail = m_eQueue.m_end;                   // wrap around
        }
        --m_eQueue.m_tail;

        ++m_eQueue.m_nFree;          // one more free event in the ring buffer

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_GET, QS::aoObj_, this)
            QS_TIME_();                                           // timestamp
            QS_SIG_(e->sig);                       // the signal of this event
            QS_OBJ_(this);                               // this active object
            QS_U8_(QF_EVT_POOL_ID_(e));            // the pool Id of the event
            QS_U8_(QF_EVT_REF_CTR_(e));          // the ref count of the event
            QS_EQC_(m_eQueue.m_nFree);               // number of free entries
        QS_END_NOCRIT_()
    }
    else {
        m_eQueue.m_frontEvt = null_evt;             // the queue becomes empty
        QACTIVE_EQUEUE_ONEMPTY_(this);

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_GET_LAST, QS::aoObj_, this)
            QS_TIME_();                                           // timestamp
            QS_SIG_(e->sig);                       // the signal of this event
            QS_OBJ_(this);                               // this active object
            QS_U8_(QF_EVT_POOL_ID_(e));            // the pool Id of the event
            QS_U8_(QF_EVT_REF_CTR_(e));          // the ref count of the event
        QS_END_NOCRIT_()
    }
    QF_CRIT_EXIT_();
    return e;
}
//............................................................................
uint32_t QF::getQueueMargin(uint8_t const prio) {
    Q_REQUIRE((prio <= static_cast<uint8_t>(QF_MAX_ACTIVE))
              && (active_[prio] != static_cast<QActive *>(0)));

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    uint32_t margin = static_cast<uint32_t>(active_[prio]->m_eQueue.m_nMin);
    QF_CRIT_EXIT_();

    return margin;
}

QP_END_

