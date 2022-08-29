//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-08-25
//! @version Last updated Zephyr 3.1.99 and @ref qpcpp_7_1_0
//!
//! @file
//! @brief Blinky example
//!
#include "qpcpp.hpp"
#include "bsp.hpp"
#include "blinky.hpp"

//Q_DEFINE_THIS_FILE

//............................................................................
class Blinky : public QActive {
private:
    QTimeEvt m_timeEvt;

public:
    Blinky();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(off);
    Q_STATE_DECL(on);
};

// local bjects --------------------------------------------------------------
Blinky l_blinky;

// global objects ------------------------------------------------------------
QActive * const AO_Blinky = &l_blinky; // opaque pointer

//............................................................................
Blinky::Blinky()
  : QActive(&initial),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{
    // empty
}

// HSM definition ------------------------------------------------------------
Q_STATE_DEF(Blinky, initial) {
    (void)e; // unused parameter

    // arm the time event to expire in half a second and every half second
    m_timeEvt.armX(BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);
    return tran(&off);
}
//............................................................................
Q_STATE_DEF(Blinky, off) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledOff();
            status = Q_RET_HANDLED;
            break;
        }
        case TIMEOUT_SIG: {
            status = tran(&on);
            break;
        }
        default: {
            status = super(&top);
            break;
        }
    }
    return status;
}
//............................................................................
Q_STATE_DEF(Blinky, on) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledOn();
            status = Q_RET_HANDLED;
            break;
        }
        case TIMEOUT_SIG: {
            status = tran(&off);
            break;
        }
        default: {
            status = super(&top);
            break;
        }
    }
    return status;
}
