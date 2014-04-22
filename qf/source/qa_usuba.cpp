/// \file
/// \brief QP::QActive::unsubscribeAll() definition.
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

Q_DEFINE_THIS_MODULE("qa_usuba")

//****************************************************************************
/// \description
/// This function is part of the Publish-Subscribe event delivery mechanism
/// available in QF. Un-subscribing from all events means that the framework
/// will stop posting any published events to the event queue of the active
/// object.
///
/// \note Due to the latency of event queues, an active object should NOT
/// assume that no events will ever be dispatched to the state machine of
/// the active object after un-subscribing from all events.
/// The events might be already in the queue, or just about to be posted
/// and the un-subscribe operation will not flush such events. Also, the
/// alternative event-delivery mechanisms, such as direct event posting or
/// time events, can be still delivered to the event queue of the active
/// object.
///
/// \sa QP::QF::publish_(), QP::QActive::subscribe(), and
/// QP::QActive::unsubscribe()
///
void QActive::unsubscribeAll(void) const {
    uint_fast8_t const p = m_prio;

    Q_REQUIRE_ID(100, (uf8_0 < p)
                      && (p <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
                      && (QF::active_[p] == this));

    uint_fast8_t const i =
        static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_div8Lkup[p]));

    enum_t sig;
    for (sig = Q_USER_SIG; sig < QF_maxSignal_; ++sig) {
        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();
        if ((QF_PTR_AT_(QF_subscrList_, sig).m_bits[i]
             & Q_ROM_BYTE(QF_pwr2Lkup[p])) != u8_0)
        {

            QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_UNSUBSCRIBE,
                             QS::priv_.aoObjFilter, this)
                QS_TIME_();     // timestamp
                QS_SIG_(sig);   // the signal of this event
                QS_OBJ_(this);  // this active object
            QS_END_NOCRIT_()

            // clear the priority bit
            QF_PTR_AT_(QF_subscrList_, sig).m_bits[i] &=
                Q_ROM_BYTE(QF_invPwr2Lkup[p]);
        }
        QF_CRIT_EXIT_();
    }
}

} // namespace QP


