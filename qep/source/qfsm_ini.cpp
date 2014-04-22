/// \file
/// \brief QP::QFsm::QFsm() and QP::QFsm::init() definitions
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

#define QP_IMPL           // this is QP implementation
#include "qep_port.h"     // QEP port
#include "qep_pkg.h"      // QEP internal interface
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qfsm_ini")

//****************************************************************************
/// \description
/// Performs the first step of FSM initialization by assigning the initial
/// pseudostate to the currently active state of the state machine.
///
/// \arguments
/// \arg[in] \c initial pointer to the top-most initial state-handler
///                     function in the derived state machine
///
QFsm::QFsm(QStateHandler const initial)
   : QMsm(initial)
{
    m_state.fun = Q_STATE_CAST(0);
}

//****************************************************************************
/// \description
/// Executes the top-most initial transition in a FSM.
///
/// \arguments
/// \arg[in] \c e  pointer to the initialization event (might be NULL)
///
/// \note Must be called only __once__ before QP::QFsm::dispatch().
///
void QFsm::init(QEvt const * const e) {

    /// \pre the virtual pointer must be initialized, the top-most initial
    /// transition must be initialized, and the initial transition must not
    /// be taken yet.
    Q_REQUIRE_ID(200, (m_temp.fun != Q_STATE_CAST(0))
              && (m_state.fun == Q_STATE_CAST(0)));

    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
        QS_OBJ_(this);            // this state machine object
        QS_FUN_(Q_STATE_CAST(0)); // the source (not defined for a FSM)
        QS_FUN_(m_temp.fun);      // the target of the transition
    QS_END_()

    // execute the top-most initial transition, which must be taken
    Q_ALLEGE((*m_temp.fun)(this, e) == Q_RET_TRAN);

    (void)QEP_TRIG_(m_temp.fun, Q_ENTRY_SIG); // enter the target
    m_state.fun = m_temp.fun;  // record the new active state
}

} // namespace QP

