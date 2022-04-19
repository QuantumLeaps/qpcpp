//============================================================================
// Product: Simple Blinky example
// Last updated for version: 6.8.0
// Last updated on: 2020-01-24
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
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
  : QActive(Q_STATE_CAST(&Blinky::initial)),
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
