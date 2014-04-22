/// \file
/// \brief QP::QMsm::msm_top_s, QP::QMsm::QMsm(), QP::QMsm::~QMsm(),
/// and QP::QMsm::init() definitions.
/// \ingroup qep
/// \cond
///***************************************************************************
/// Product: QEP/C++
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
#include "qep_port.h"     // QEP port
#include "qep_pkg.h"      // QEP internal interface
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qmsm_ini")

//****************************************************************************
QMState const QMsm::msm_top_s = {
    static_cast<QMState const *>(0),
    Q_STATE_CAST(0),
    Q_ACTION_CAST(0),
    Q_ACTION_CAST(0),
    Q_ACTION_CAST(0)
};

//****************************************************************************
/// \description
/// Performs the first step of initialization by assigning the initial
/// pseudostate to the currently active state of the state machine.
///
/// \arguments
/// \arg[in] \c initial  the top-most initial transition for the MSM.
///
/// \note The constructor is protected to prevent direct instantiating
/// of the QP::QMsm objects. This class is intended for subclassing only.
///
/// \sa The QP::QMsm example illustrates how to use the QMsm constructor
/// in the constructor initializer list of the derived state machines.
///
QMsm::QMsm(QStateHandler const initial) {
    m_state.obj = &msm_top_s;
    m_temp.fun  = initial;
}

//****************************************************************************
/// \description
/// Virtual destructor of the QMsm state machine and any of its subclasses.
///
QMsm::~QMsm() {
}

//****************************************************************************
/// \description
/// Executes the top-most initial transition in a MSM.
///
/// \arguments
/// \arg[in] \c e  a constant pointer to QP::QEvt or a class derived from
/// QP::QEvt
///
/// \attention
/// QP::QMsm::init() must be called exactly __once__ before
/// QP::QMsm::dispatch()
///
void QMsm::init(QEvt const * const e) {
    QS_CRIT_STAT_

    /// \pre the top-most initial transition must be initialized, and the
    /// initial transition must not be taken yet.
    Q_REQUIRE_ID(200, (m_temp.fun != Q_STATE_CAST(0))
                      && (m_state.obj == &msm_top_s));

    QState r = (*m_temp.fun)(this, e); // execute the top-most initial tran.

    // initial tran. must be taken
    Q_ASSERT_ID(210, r == Q_RET_TRAN_INIT);

    QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
        QS_OBJ_(this);  // this state machine object
        QS_FUN_(m_state.obj->stateHandler);          // source state handler
        QS_FUN_(m_temp.tatbl->target->stateHandler); // target state handler
    QS_END_()

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    do {
        r = execTatbl_(m_temp.tatbl); // execute the transition-action table
    } while (r >= Q_RET_TRAN_INIT);

    QS_BEGIN_(QS_QEP_INIT_TRAN, QS::priv_.smObjFilter, this)
        QS_TIME_();                         // time stamp
        QS_OBJ_(this);                      // this state machine object
        QS_FUN_(m_state.obj->stateHandler); // the new current state
    QS_END_()
}

} // namespace QP


