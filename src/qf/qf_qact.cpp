/// @file
/// @brief QP::QActive::QActive() definition
/// @cond
///***************************************************************************
/// Last updated for version 6.2.0
/// Last updated on  2018-03-16
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) 2002-2018 Quantum Leaps. All rights reserved.
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
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port

namespace QP {

//****************************************************************************
QActive::QActive(QStateHandler const initial)
  : QHsm(initial),
    m_prio(static_cast<uint8_t>(0))
{
    m_state.fun = Q_STATE_CAST(&QHsm::top);

#ifdef QF_OS_OBJECT_TYPE
    QF::bzero(&m_osObject, static_cast<uint_fast16_t>(sizeof(m_osObject)));
#endif

#ifdef QF_THREAD_TYPE
    QF::bzero(&m_thread, static_cast<uint_fast16_t>(sizeof(m_thread)));
#endif
}

} // namespace QP

