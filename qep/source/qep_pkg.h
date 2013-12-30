//****************************************************************************
// Product: QEP/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 27, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
#ifndef qep_pkg_h
#define qep_pkg_h

/// \file
/// \ingroup qep
/// \brief Internal (package scope) QEP/C++ interface.

#ifndef QP_API_VERSION
    #define QP_API_VERSION 9999
#endif

#include "qep_port.h"                                              // QEP port

#ifdef Q_SPY                                   // QS software tracing enabled?
    #include "qs_port.h"                                    // include QS port
#else
    #include "qs_dummy.h"                   // disable the QS software tracing
#endif                                                                // Q_SPY

namespace QP {

//****************************************************************************
/// preallocated reserved events
extern QEvt const QEP_reservedEvt_[4];

/// empty signal for internal use only
QSignal const QEP_EMPTY_SIG_ = static_cast<QSignal>(0);

uint8_t const u8_0  = static_cast<uint8_t>(0); ///< \brief constant (uint8_t)0
uint8_t const u8_1  = static_cast<uint8_t>(1); ///< \brief constant (uint8_t)1
int_t   const s_0   = static_cast<int_t>(0);    ///< \brief constant  (int_t)0
int_t   const s_1   = static_cast<int_t>(1);    ///< \brief constant  (int_t)1
int_t   const s_n1  = static_cast<int_t>(-1);   ///< \brief constant (int_t)-1

}                                                              // namespace QP

/// helper macro to trigger internal event in an HSM
#define QEP_TRIG_(state_, sig_) \
    ((*(state_))(this, &QEP_reservedEvt_[sig_]))

/// helper macro to trigger exit action in an HSM
#define QEP_EXIT_(state_) do { \
    if (QEP_TRIG_(state_, Q_EXIT_SIG) == Q_RET_HANDLED) { \
        QS_BEGIN_(QS_QEP_STATE_EXIT, QS::priv_.smObjFilter, this) \
            QS_OBJ_(this); \
            QS_FUN_(state_); \
        QS_END_() \
    } \
} while (false)

/// helper macro to trigger entry action in an HSM
#define QEP_ENTER_(state_) do { \
    if (QEP_TRIG_(state_, Q_ENTRY_SIG) == Q_RET_HANDLED) { \
        QS_BEGIN_(QS_QEP_STATE_ENTRY, QS::priv_.smObjFilter, this) \
            QS_OBJ_(this); \
            QS_FUN_(state_); \
        QS_END_() \
    } \
} while (false)

/// \brief Internal QEP macro to increment the given action table \a act_
///
/// \note Incrementing a pointer violates the MISRA-C 2004 Rule 17.4(req),
/// pointer arithmetic other than array indexing. Encapsulating this violation
/// in a macro allows to selectively suppress this specific deviation.
///
#define QEP_ACT_PTR_INC_(act_) (++(act_))

#endif                                                            // qep_pkg_h


