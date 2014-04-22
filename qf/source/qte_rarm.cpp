/// \file
/// \brief QP::QTimeEvt::rearm() definition.
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

Q_DEFINE_THIS_MODULE("qte_rarm")

/****************************************************************************/
/**
* \description
* Rearms  a time event with a new number of clock ticks. This function can
* be used to adjust the current period of a periodic time event or to
* prevent a one-shot time event from expiring (e.g., a watchdog time event).
* Rearming a periodic timer leaves the interval unchanged and is a convenient
* method to adjust the phasing of a periodic time event.
*
* \arguments
* \arg[in] \c nTicks number of clock ticks (at the associated rate)
*                    to rearm the time event with.
*
* \returns 'true' if the time event was running as it
* was re-armed. The 'false' return means that the time event was
* not truly rearmed because it was not running. The 'false' return is only
* possible for one-shot time events that have been automatically disarmed
* upon expiration. In this case the 'false' return means that the time event
* has already been posted or published and should be expected in the
* active object's state machine.
*/
bool QTimeEvt::rearm(QTimeEvtCtr const nTicks) {
    uint8_t tickRate = static_cast<uint8_t>(refCtr_ & u8_0x7F);
    QF_CRIT_STAT_

    /// \pre AO must be valid, tick rate must be in range, nTicks must not
    /// be zero, and the signal of this time event must be valid
    ///
    Q_REQUIRE_ID(100, (m_act != null_void)
                      && (tickRate < static_cast<uint8_t>(QF_MAX_TICK_RATE))
                      && (nTicks != tc_0)
                      && (static_cast<enum_t>(sig) >= Q_USER_SIG));

    QF_CRIT_ENTRY_();
    bool isArmed;

    // is the time evt not running? */
    if (m_ctr == tc_0) {
        isArmed = false;

        // is the time event unlinked?
        // NOTE: For a duration of a single clock tick of the specified
        // tick rate a time event can be disarmed and yet still linked into
        // the list, because unlinking is performed exclusively in the
        // QF::tickX() function.
        //
        if ((refCtr_ & u8_0x80) == u8_0) {
            refCtr_ |= u8_0x80; // mark as linked

            // The time event is initially inserted into the separate
            // "freshly armed" list based on QF_timeEvtHead_[tickRate].act.
            // Only later, inside the QF_tickX() function, the "freshly armed"
            // list is appended to the main list of armed time events based on
            // QF_timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside the QF_tickX()
            // function.
            //
            m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
            QF::timeEvtHead_[tickRate].m_act = this;
        }
    }
    // the time event is armed
    else {
        isArmed = true;
    }
    m_ctr = nTicks; // re-load the tick counter (shift the phasing)

    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_REARM, QS::priv_.teObjFilter, this)
        QS_TIME_();          // timestamp
        QS_OBJ_(this);       // this time event object
        QS_OBJ_(m_act);      // the target AO
        QS_TEC_(m_ctr);      // the number of ticks
        QS_TEC_(m_interval); // the interval
        QS_2U8_(tickRate, (isArmed ? u8_1 : u8_0));// tick rate & armed status
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
    return isArmed;
}

} // namespace QP
