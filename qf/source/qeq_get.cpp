/// \file
/// \brief QP::QEQueue::get() implementation.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-09
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qeq_get")

//****************************************************************************
/// \description
/// Retrieves an event from the front of the "raw" thread-safe queue and
/// returns a pointer to this event to the caller.
///
/// \returns pointer to event at the front of the queue, if the queue is
/// not empty and NULL if the queue is empty.
///
/// \note this function is used for the "raw" thread-safe queues and __not__
/// for the queues of active objects.
///
/// \sa QP::QEQueue::post(), QP::QEQueue::postLIFO(), QP::QActive::recall()
///
QEvt const *QEQueue::get(void) {
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    e = m_frontEvt;  // always remove the event from the front location

    // is the queue not empty?
    if (e != static_cast<QEvt const *>(0)) {
        QEQueueCtr nFree = m_nFree + static_cast<QEQueueCtr>(1);
        m_nFree = nFree;  // upate the number of free

        // any events in the the ring buffer?
        if (nFree <= m_end) {
            m_frontEvt = QF_PTR_AT_(m_ring, m_tail); // remove from the tail
            if (m_tail == static_cast<QEQueueCtr>(0)) { // need to wrap?
                m_tail = m_end; // wrap around
            }
            --m_tail;

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET, QS::priv_.eqObjFilter, this)
                QS_TIME_();              // timestamp
                QS_SIG_(e->sig);         // the signal of this event
                QS_OBJ_(this);           // this queue object
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
                QS_EQC_(nFree);          // number of free entries
            QS_END_NOCRIT_()
        }
        else {
            m_frontEvt = null_evt; // the queue becomes empty

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_ID(110, nFree == (m_end + static_cast<QEQueueCtr>(1)));

            QS_BEGIN_NOCRIT_(QS_QF_EQUEUE_GET_LAST, QS::priv_.eqObjFilter,
                             this)
                QS_TIME_();              // timestamp
                QS_SIG_(e->sig);         // the signal of this event
                QS_OBJ_(this);           // this queue object
                QS_2U8_(e->poolId_, e->refCtr_);// pool Id & refCtr of the evt
            QS_END_NOCRIT_()
        }
    }
    QF_CRIT_EXIT_();
    return e;
}

} // namespace QP

