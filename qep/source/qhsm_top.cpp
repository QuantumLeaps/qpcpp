/// \file
/// \brief QP::QHsm::top() definition
/// \ingroup qep
/// \cond
///***************************************************************************
/// Product: QEP/C++
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

#include "qep_port.h"     // QEP port
#include "qep_pkg.h"      // QEP internal interface

namespace QP {

//****************************************************************************/
/// \description
/// The QP::QHsm::top() state handler is the ultimate root of state hierarchy
/// in all HSMs derived from QP::QHsm.
///
/// \arguments
/// \arg[in] \c e  pointer to the event to be dispatched to the FSM
///
/// \returns Always returns #Q_RET_IGNORED, which means that the top state
///          ignores all events.
///
/// \note
/// The arguments to this state handler are not used. They are provided for
/// conformance with the state-handler function signature QP::QStateHandler.
///
QState QHsm::top(void * const, QEvt const * const) {
    return Q_RET_IGNORED; // the top state ignores all events
}

} // namespace QP

