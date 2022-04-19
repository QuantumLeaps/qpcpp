//============================================================================
// Product: Simple Blinky example
// Last Updated for Version: 5.8.0
// Date of the Last Update:  2016-11-30
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
// https://state-machine.com
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
    static QState initial(Blinky * const me, QEvt const * const e);
    static QState off(Blinky * const me, QEvt const * const e);
    static QState on(Blinky * const me, QEvt const * const e);
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
QState Blinky::initial(Blinky * const me, QEvt const * const e) {
    (void)e; // unused parameter

    // arm the time event to expire in half a second and every half second
    me->m_timeEvt.armX(BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);
    return Q_TRAN(&Blinky::off);
}
//............................................................................
QState Blinky::off(Blinky * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledOff();
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            status = Q_TRAN(&Blinky::on);
            break;
        }
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}
//............................................................................
QState Blinky::on(Blinky * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_ledOn();
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            status = Q_TRAN(&Blinky::off);
            break;
        }
        default: {
            status = Q_SUPER(&QHsm::top);
            break;
        }
    }
    return status;
}
