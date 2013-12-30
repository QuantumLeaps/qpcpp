//****************************************************************************
// Product: QF/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Nov 30, 2013
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
/// \brief QActive::post_() implementation.
///
/// \note this source file is only included in the QF library when the native
/// QF active object queue is used (instead of a message queue of an RTOS).

namespace QP {

Q_DEFINE_THIS_MODULE("qa_fifo")

//............................................................................
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e, uint_t const margin)
#else
bool QActive::post_(QEvt const * const e, uint_t const margin,
                    void const * const sender)
#endif
{
    bool status;
    QF_CRIT_STAT_

    Q_REQUIRE(e != null_evt);                       // the event must be valid

    QF_CRIT_ENTRY_();
    QEQueueCtr nFree = m_eQueue.m_nFree;// tmp to avoid UB for volatile access
    if (nFree > static_cast<QEQueueCtr>(margin)) {        // margin available?

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::priv_.aoObjFilter, this)
            QS_TIME_();                                           // timestamp
            QS_OBJ_(sender);                              // the sender object
            QS_SIG_(e->sig);                        // the signal of the event
            QS_OBJ_(this);                               // this active object
            QS_2U8_(e->poolId_, e->refCtr_);    // pool Id & refCtr of the evt
            QS_EQC_(nFree);                          // number of free entries
            QS_EQC_(m_eQueue.m_nMin);            // min number of free entries
        QS_END_NOCRIT_()

        if (e->poolId_ != u8_0) {                    // is it a dynamic event?
            QF_EVT_REF_CTR_INC_(e);         // increment the reference counter
        }

        --nFree;                                // one free entry just used up
        m_eQueue.m_nFree = nFree;                       // update the volatile
        if (m_eQueue.m_nMin > nFree) {
            m_eQueue.m_nMin = nFree;                  // update minimum so far
        }

        if (m_eQueue.m_frontEvt == null_evt) {          // is the queue empty?
            m_eQueue.m_frontEvt = e;                 // deliver event directly
            QACTIVE_EQUEUE_SIGNAL_(this);            // signal the event queue
        }
        else {           // queue is not empty, leave event in the ring-buffer
                              // insert event pointer e into the buffer (FIFO)
            QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_head) = e;
            if (m_eQueue.m_head == static_cast<QEQueueCtr>(0)) {      // wrap?
                m_eQueue.m_head = m_eQueue.m_end;               // wrap around
            }
            --m_eQueue.m_head;
        }
        status = true;                            // event posted successfully
    }
    else {
        Q_ASSERT(margin != u_0);               // can tollerate dropping evts?

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT, QS::priv_.aoObjFilter,
                         this)
            QS_TIME_();                                           // timestamp
            QS_OBJ_(sender);                              // the sender object
            QS_SIG_(e->sig);                        // the signal of the event
            QS_OBJ_(this);                               // this active object
            QS_2U8_(e->poolId_, e->refCtr_);    // pool Id & refCtr of the evt
            QS_EQC_(nFree);                          // number of free entries
            QS_EQC_(static_cast<QEQueueCtr>(margin));      // margin requested
        QS_END_NOCRIT_()

        QF::gc(e);                        // recycle the evnet to avoid a leak
        status = false;                                    // event not posted
    }
    QF_CRIT_EXIT_();

    return status;
}

}                                                              // namespace QP

//****************************************************************************
// NOTE01:
// The zero value of the 'margin' argument is special and denotes situation
// when the post_() operation is assumed to succeed (event delivery guarantee).
// An assertion fires, when the event cannot be delivered in this case.
//

