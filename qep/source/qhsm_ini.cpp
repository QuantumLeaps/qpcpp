/// \file
/// \brief QP::QHsm::QHsm() and QP::QHsm::init() definitions.
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

Q_DEFINE_THIS_MODULE("qhsm_ini")

//****************************************************************************
/// \description
/// Performs the first step of HSM initialization by assigning the initial
/// pseudostate to the currently active state of the state machine.
///
/// \arguments
/// \arg[in] \c initial pointer to the top-most initial state-handler
///                     function in the derived state machine
///
QHsm::QHsm(QStateHandler const initial)
  : QMsm(initial)
{
    m_state.fun = Q_STATE_CAST(&QHsm::top);
}

//****************************************************************************
/// \description
/// Executes the top-most initial transition in a HSM.
///
/// \arguments
/// \arg[in] \c e  pointer to the initialization event (might be NULL)
///
/// \note Must be called exactly __once__ before the QP::QHsm::dispatch().
///
void QHsm::init(QEvt const * const e) {
    QStateHandler t = m_state.fun;

    /// \pre ctor must be executed and initial tran. NOT taken
    Q_REQUIRE_ID(100, (m_temp.fun != Q_STATE_CAST(0))
                      && (t == Q_STATE_CAST(&QHsm::top)));

    // the top-most initial transition must be taken
    Q_ALLEGE_ID(110, (*m_temp.fun)(this, e) == Q_RET_TRAN);

    QS_CRIT_STAT_
    // drill into the target...
    do {
        QStateHandler path[MAX_NEST_DEPTH_];
        int_fast8_t ip = sf8_0;  // transition entry path index

        QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
            QS_OBJ_(this);       // this state machine object
            QS_FUN_(t);          // the source state
            QS_FUN_(m_temp.fun); // the target of the initial transition
        QS_END_()

        path[0] = m_temp.fun;
        (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);
        while (m_temp.fun != t) {
            ++ip;
            Q_ASSERT(ip < static_cast<int_fast8_t>(Q_DIM(path)));
            path[ip] = m_temp.fun;
            (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);
        }
        m_temp.fun = path[0];

        // retrace the entry path in reverse (desired) order...
        do {
            QEP_ENTER_(path[ip]); // enter path[ip]
            --ip;
        } while (ip >= sf8_0);

        t = path[0]; // current state becomes the new source
    } while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN);

    QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
        QS_OBJ_(this);       // this state machine object
        QS_FUN_(&QHsm::top); // source of initial tran (the top state)
        QS_FUN_(t);          // the new active state
    QS_END_()

    m_state.fun = t; // change the current active state
    m_temp.fun  = t; // mark the configuration as stable
}

} // namespace QP


