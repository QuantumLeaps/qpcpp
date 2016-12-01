/// @file
/// @brief QMActive::QMActive() and virtual functions
/// @cond
///***************************************************************************
/// Last updated for version 5.8.0
/// Last updated on  2016-11-19
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
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port

//! Internal macro to cast a QP::QMActive pointer @p qact_ to QP::QMsm*
/// @note
/// Casting pointer to pointer pointer violates the MISRA-C++ 2008 Rule 5-2-7,
/// cast from pointer to pointer. Additionally this cast violates the MISRA-
/// C++ 2008 Rule 5-2-8 Unusual pointer cast (incompatible indirect types).
/// Encapsulating these violations in a macro allows to selectively suppress
/// this specific deviation.
#define QF_QMACTIVE_TO_QMSM_CAST_(qact_) \
    reinterpret_cast<QMsm *>((qact_))

//! Internal macro to cast a QP::QMActive pointer @p qact_ to QP::QMsm const *
#define QF_QMACTIVE_TO_QMSM_CONST_CAST_(qact_) \
    reinterpret_cast<QMsm const *>((qact_))

namespace QP {

//****************************************************************************
QMActive::QMActive(QStateHandler const initial)
  : QActive(initial)
{
    m_state.obj = &QMsm::msm_top_s;
    m_temp.fun  = initial;
}

//****************************************************************************
void QMActive::init(QEvt const * const e) {
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::init(e);
}
//****************************************************************************
void QMActive::init(void) {
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::init();
}
//****************************************************************************
void QMActive::dispatch(QEvt const * const e) {
    QF_QMACTIVE_TO_QMSM_CAST_(this)->QMsm::dispatch(e);
}
//****************************************************************************
bool QMActive::isInState(QMState const * const st) const {
    return QF_QMACTIVE_TO_QMSM_CONST_CAST_(this)->QMsm::isInState(st);
}
//****************************************************************************
QMState const *QMActive::childStateObj(QMState const * const parent) const {
    return QF_QMACTIVE_TO_QMSM_CONST_CAST_(this)->QMsm::childStateObj(parent);
}

} // namespace QP
