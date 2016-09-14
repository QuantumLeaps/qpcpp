/// @file
/// @brief QP::QActive::QActive() definition
/// @cond
///***************************************************************************
/// Last updated for version 5.7.0
/// Last updated on  2016-09-14
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

//! Internal macro to cast a QP::QActive pointer @p qact_ to QP::QHsm*
/// @note
/// Casting pointer to pointer pointer violates the MISRA-C++ 2008 Rule 5-2-7,
/// cast from pointer to pointer. Additionally this cast violates the MISRA-
/// C++ 2008 Rule 5-2-8 Unusual pointer cast (incompatible indirect types).
/// Encapsulating these violations in a macro allows to selectively suppress
/// this specific deviation.
#define QF_QACTIVE_TO_QHSM_CAST_(qact_) reinterpret_cast<QHsm *>((qact_))


namespace QP {

//****************************************************************************
QActive::QActive(QStateHandler const initial)
  : QMActive(initial)
{
    m_state.fun = Q_STATE_CAST(&QHsm::top);
}
//****************************************************************************
void QActive::init(QEvt const * const e) {
    QF_QACTIVE_TO_QHSM_CAST_(this)->QHsm::init(e);
}
//****************************************************************************
void QActive::init(void) {
    QF_QACTIVE_TO_QHSM_CAST_(this)->QHsm::init();
}
//****************************************************************************
void QActive::dispatch(QEvt const * const e) {
    QF_QACTIVE_TO_QHSM_CAST_(this)->QHsm::dispatch(e);
}
//****************************************************************************
bool QActive::isIn(QStateHandler const s) {
    return QF_QACTIVE_TO_QHSM_CAST_(this)->QHsm::isIn(s);
}
//****************************************************************************
QStateHandler QActive::childState(QStateHandler const parent) {
    return QF_QACTIVE_TO_QHSM_CAST_(this)->QHsm::childState(parent);
}

} // namespace QP
