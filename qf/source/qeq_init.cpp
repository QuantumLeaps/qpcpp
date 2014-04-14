/// \file
/// \brief QP::QEQueue::QEQueue() and QP::QEQueue::init() definitions.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-13
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

namespace QP {

//****************************************************************************
/// \description
/// Default constructor
///
QEQueue::QEQueue(void)
  : m_frontEvt(null_evt),
    m_ring(static_cast<QEvt const **>(0)),
    m_end(static_cast<QEQueueCtr>(0)),
    m_head(static_cast<QEQueueCtr>(0)),
    m_tail(static_cast<QEQueueCtr>(0)),
    m_nFree(static_cast<QEQueueCtr>(0)),
    m_nMin(static_cast<QEQueueCtr>(0))
{}

//****************************************************************************
/// \description
/// Initialize the event queue by giving it the storage for the ring buffer.
///
/// \arguments
/// \arg[in] \c qSto an array of pointers to QP::QEvt to sereve as the
///                  ring buffer for the event queue
/// \arg[in] \c qLen the length of the qSto[] buffer (in ::QEvt pointers)
///
/// \note The actual capacity of the queue is qLen + 1, because of the extra
/// location forntEvt.
///
/// \note This function is also used to initialize the event queues of active
/// objects in the built-int QK and "Vanilla" kernels, as well as other
/// QP ports to OSes/RTOSes that do provide a suitable message queue.
///
void QEQueue::init(QEvt const *qSto[], uint_fast16_t const qLen) {
    m_frontEvt = null_evt; // no events in the queue
    m_ring     = &qSto[0];
    m_end      = static_cast<QEQueueCtr>(qLen);
    m_head     = static_cast<QEQueueCtr>(0);
    m_tail     = static_cast<QEQueueCtr>(0);
    m_nFree    = static_cast<QEQueueCtr>(qLen + uf16_1); //+1 for frontEvt
    m_nMin     = m_nFree;

    QS_CRIT_STAT_
    QS_BEGIN_(QS_QF_EQUEUE_INIT, QS::priv_.eqObjFilter, this)
        QS_OBJ_(this);   // this QEQueue object
        QS_EQC_(m_end);  // the length of the queue
    QS_END_()
}

} // namespace QP

