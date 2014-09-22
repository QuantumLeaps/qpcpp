/// \file
/// \brief QP::QTimeEvt::QTimeEvt() constructors.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.1
/// Last updated on  2014-09-05
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
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qte_ctor")

//****************************************************************************
/// \description
/// When creating a time event, you must commit it to a specific active object
/// \a act, tick rate \a tickRate and event signal \a sgnl. You cannot change
/// these attributes later.
///
/// \arguments
/// \arg[in] \c act  pointer to the active object associated with this
///          time event. The time event will post itself to this AO.
/// \arg[in] \c sgnl signal to associate with this time event.
/// \arg[in] \c tickRate system tick rate to associate with this time event.
///
QTimeEvt::QTimeEvt(QActive * const act,
                   enum_t const sgnl, uint8_t const tickRate)
  :
#ifdef Q_EVT_CTOR
    QEvt(static_cast<QSignal>(sgnl)),
#endif
    m_next(null_tevt),
    m_act(act),
    m_ctr(tc_0),
    m_interval(tc_0)
{
    /// \pre The signal must be valid and the tick rate in range
    Q_REQUIRE_ID(100, (sgnl >= Q_USER_SIG)
                      && (tickRate < static_cast<uint8_t>(QF_MAX_TICK_RATE)));

#ifndef Q_EVT_CTOR
    sig = static_cast<QSignal>(sgnl); // set QEvt::sig of this time event
#endif

    // Setting the POOL_ID event attribute to zero is correct only for
    // events not allocated from event pools, which must be the case
    // for Time Events.
    //
    poolId_ = u8_0;

    // The reference counter attribute is not used in static events,
    // so for the Time Events it is reused to hold the tickRate in the
    // bits [0..6] and the linkedFlag in the MSB (bit [7]). The linkedFlag
    // is 0 for time events unlinked from any list and 1 otherwise.
    //
    refCtr_ = tickRate;
}

//****************************************************************************
/// \note
/// private default ctor for internal use only
///
QTimeEvt::QTimeEvt()
  :
#ifdef Q_EVT_CTOR
    QEvt(static_cast<QSignal>(0)),
#endif // Q_EVT_CTOR

    m_next(null_tevt),
    m_act(null_act),
    m_ctr(tc_0),
    m_interval(tc_0)
{
#ifndef Q_EVT_CTOR
    sig = static_cast<QSignal>(0);
#endif  // Q_EVT_CTOR

    // Setting the POOL_ID event attribute to zero is correct only for
    // events not allocated from event pools, which must be the case
    // for Time Events.
    //
    poolId_ = u8_0; // not from any event pool

    // The reference counter attribute is not used in static events,
    // so for the Time Events it is reused to hold the tickRate in the
    // bits [0..6] and the linkedFlag in the MSB (bit [7]). The linkedFlag
    // is 0 for time events unlinked from any list and 1 otherwise.
    //
    refCtr_ = u8_0; // default rate 0
}

} // namespace QP
