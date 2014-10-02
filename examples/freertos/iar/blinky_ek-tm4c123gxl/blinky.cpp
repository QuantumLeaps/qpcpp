//****************************************************************************
// Product: Simple Blinky example
// Last Updated for Version: 5.3.1
// Date of the Last Update:  2014-09-29
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. state-machine.com.
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
// Web:   http://www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qp_port.h"
#include "bsp.h"

using namespace QP;

//Q_DEFINE_THIS_FILE

//............................................................................
enum BlinkySignals {
    TIMEOUT_SIG = QP::Q_USER_SIG, // the periodic timeout signal
};


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

//............................................................................
Blinky::Blinky()
  : QActive(Q_STATE_CAST(&Blinky::initial)),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{
    // empty
}

// HSM definition ------------------------------------------------------------
QState Blinky::initial(Blinky * const me, QEvt const * const e) {
    (void)e; // avoid compiler warning about unused argument

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

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static Blinky l_blinky;              // the Blinky active object
static QEvt const *l_blinkyQSto[10]; // Event queue storage for Blinky

//............................................................................
int main() {
    BSP_init(); // initialize the Board Support Package
    QF::init(); // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QF::psInit()
    // dynamic event allocation not used, no call to QF::poolInit()

    // instantiate and start the active objects...
    l_blinky.start(1U,
                   l_blinkyQSto, Q_DIM(l_blinkyQSto),
                   (void *)0, 512U);

    return QF::run(); // run the QF application
}
