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
/// \brief QActive::postLIFO() implementation.
///
/// \note this source file is only included in the QF library when the native
/// QF active object queue is used (instead of a message queue of an RTOS).

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qa_lifo")

//............................................................................
void QActive::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(QF_EVT_POOL_ID_(e));                // the pool Id of the event
        QS_U8_(QF_EVT_REF_CTR_(e));              // the ref count of the event
        QS_EQC_(m_eQueue.m_nFree);                   // number of free entries
        QS_EQC_(m_eQueue.m_nMin);                // min number of free entries
    QS_END_NOCRIT_()

    if (QF_EVT_POOL_ID_(e) != u8_0) {                // is it a dynamic event?
        QF_EVT_REF_CTR_INC_(e);             // increment the reference counter
    }

    if (m_eQueue.m_frontEvt == null_evt) {              // is the queue empty?
        m_eQueue.m_frontEvt = e;                     // deliver event directly
        QACTIVE_EQUEUE_SIGNAL_(this);                // signal the event queue
    }
    else {               // queue is not empty, leave event in the ring-buffer
                                        // queue must accept all posted events
        Q_ASSERT(m_eQueue.m_nFree != static_cast<QEQueueCtr>(0));

        ++m_eQueue.m_tail;
        if (m_eQueue.m_tail == m_eQueue.m_end) {     // need to wrap the tail?
            m_eQueue.m_tail = static_cast<QEQueueCtr>(0);       // wrap around
        }

        QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_tail) = m_eQueue.m_frontEvt;
        m_eQueue.m_frontEvt = e;                         // put event to front

        --m_eQueue.m_nFree;                    // update number of free events
        if (m_eQueue.m_nMin > m_eQueue.m_nFree) {
            m_eQueue.m_nMin = m_eQueue.m_nFree;       // update minimum so far
        }
    }
    QF_CRIT_EXIT_();
}

QP_END_
