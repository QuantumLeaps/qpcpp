/// \file
/// \brief QP::QActive::get_() definition.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-08
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

Q_DEFINE_THIS_MODULE("qa_get_")

/// \note
/// This source file is only included in the QF library when the native
/// QF active object queue is used (instead of a message queue of an RTOS).

//****************************************************************************
/// \description
/// The behavior of this function depends on the kernel used in the QF port.
/// For built-in kernels (Vanilla or QK) the function can be called only when
/// the queue is not empty, so it doesn't block. For a blocking kernel/OS
/// the function can block and wait for delivery of an event.
///
/// \returns a pointer to the received event. The returned pointer is always
/// valid (can't be NULL).
QEvt const *QActive::get_(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QACTIVE_EQUEUE_WAIT_(this); // wait for event to arrive directly

    QEvt const *e = m_eQueue.m_frontEvt; // always remove evt from the front
    QEQueueCtr nFree= m_eQueue.m_nFree + static_cast<QEQueueCtr>(1);
    m_eQueue.m_nFree = nFree; // upate the number of free

    // any events in the ring buffer?
    if (nFree <= m_eQueue.m_end) {

        // remove event from the tail
        m_eQueue.m_frontEvt = QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_tail);
        if (m_eQueue.m_tail == static_cast<QEQueueCtr>(0)) { // need to wrap?
            m_eQueue.m_tail = m_eQueue.m_end; // wrap around
        }
        --m_eQueue.m_tail;

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_GET, QS::priv_.aoObjFilter, this)
            QS_TIME_();                      // timestamp
            QS_SIG_(e->sig);                 // the signal of this event
            QS_OBJ_(this);                   // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
            QS_EQC_(nFree);                  // number of free entries
        QS_END_NOCRIT_()
    }
    else {
        m_eQueue.m_frontEvt = null_evt; // the queue becomes empty

        // all entries in the queue must be free (+1 for fronEvt)
        Q_ASSERT_ID(110, nFree ==
                         (m_eQueue.m_end + static_cast<QEQueueCtr>(1)));

        QACTIVE_EQUEUE_ONEMPTY_(this);

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_GET_LAST, QS::priv_.aoObjFilter, this)
            QS_TIME_();                      // timestamp
            QS_SIG_(e->sig);                 // the signal of this event
            QS_OBJ_(this);                   // this active object
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_END_NOCRIT_()
    }
    QF_CRIT_EXIT_();
    return e;
}

//****************************************************************************
/// \description
/// Queries the minimum of free ever present in the given event queue of
/// an active object with priority \a prio, since the active object
/// was started.
///
/// \note
/// QP::QF::getQueueMin() is available only when the native QF event
/// queue implementation is used. Requesting the queue minimum of an unused
/// priority level raises an assertion in the QF. (A priority level becomes
/// used in QF after the call to the QP::QF::add_() function.)
///
/// \arguments
/// \arg[in] \c prio  Priority of the active object, whose queue is queried
///
/// \returns the minimum of free ever present in the given event
/// queue of an active object with priority \a prio, since the active object
/// was started.
uint_fast16_t QF::getQueueMin(uint_fast8_t const prio) {

    Q_REQUIRE_ID(200, (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
                      && (active_[prio] != static_cast<QActive *>(0)));

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    uint_fast16_t min =
        static_cast<uint_fast16_t>(active_[prio]->m_eQueue.m_nMin);
    QF_CRIT_EXIT_();

    return min;
}

} // namespace QP


