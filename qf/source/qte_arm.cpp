/// \file
/// \brief QP::QTimeEvt::armX() definition.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-10
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

Q_DEFINE_THIS_MODULE("qte_arm")

//****************************************************************************
/// \description
/// Arms a time event to fire in a specified number of clock ticks and with
/// a specified interval. If the interval is zero, the time event is armed for
/// one shot ('one-shot' time event). The time event gets directly posted
/// (using the FIFO policy) into the event queue of the host active object.
///
/// \arguments
/// \arg[in] \c nTicks   number of clock ticks (at the associated rate)
///                      to rearm the time event with.
/// \arg[in] \c interval interval (in clock ticks) for periodic time event.
///
/// \note After posting, a one-shot time event gets automatically disarmed
/// while a periodic time event (interval != 0) is automatically re-armed.
///
/// \note A time event can be disarmed at any time by calling the
/// QP::QTimeEvt::disarm() function. Also, a time event can be re-armed to fire
/// in a different number of clock ticks by calling the QP::QTimeEvt::rearm()
/// function.
///
/// \usage
/// The following example shows how to arm a one-shot time event from a state
/// machine of an active object:
/// \include qf_state.cpp
///
void QTimeEvt::armX(QTimeEvtCtr const nTicks, QTimeEvtCtr const interval) {
    uint8_t tickRate = static_cast<uint8_t>(refCtr_ & u8_0x7F);
    QTimeEvtCtr cntr = m_ctr;  // temporary to hold volatile
    QF_CRIT_STAT_

    /// \pre the host AO must be valid, time evnet must be disarmed,
    /// number of clock ticks cannot be zero, and the signal must be valid.
    ///
    Q_REQUIRE_ID(100, (m_act != null_void)
                      && (cntr == tc_0)
                      && (nTicks != tc_0)
                      && (tickRate < static_cast<uint8_t>(QF_MAX_TICK_RATE))
                      && (static_cast<enum_t>(sig) >= Q_USER_SIG));

    QF_CRIT_ENTRY_();
    m_ctr = nTicks;
    m_interval = interval;

    // is the time event unlinked?
    // NOTE: For the duration of a single clock tick of the specified tick
    // rate a time event can be disarmed and yet still linked into the list,
    // because un-linking is performed exclusively in the QF_tickX() function.
    //
    if ((refCtr_ & u8_0x80) == u8_0) {
        refCtr_ |= u8_0x80;  // mark as linked

        // The time event is initially inserted into the separate
        // "freshly armed" link list based on QF::timeEvtHead_[tickRate].act.
        // Only later, inside the QF::tickX() function, the "freshly armed"
        // list is appended to the main list of armed time events based on
        // QF::timeEvtHead_[tickRate].next. Again, this is to keep any
        // changes to the main list exclusively inside the QF::tickX()
        // function.
        //
        m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
        QF::timeEvtHead_[tickRate].m_act = this;
    }

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_ARM, QS::priv_.teObjFilter, this)
        QS_TIME_();        // timestamp
        QS_OBJ_(this);     // this time event object
        QS_OBJ_(m_act);    // the active object
        QS_TEC_(nTicks);   // the number of ticks
        QS_TEC_(interval); // the interval
        QS_U8_(tickRate);  // tick rate
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
}

} // namespace QP

