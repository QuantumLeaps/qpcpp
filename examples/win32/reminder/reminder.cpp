//****************************************************************************
// Product: Reminder state pattern example
// Last Updated for Version: 5.4.2
// Date of the Last Update:  2015-06-05
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
// Web  : http://www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qf_port.h"
#include "bsp.h"
#include "qassert.h"

#include <stdio.h>

Q_DEFINE_THIS_FILE

using namespace QP;

//............................................................................
enum SensorSignals {
    TERMINATE_SIG = Q_USER_SIG, // terminate the application
    TIMEOUT_SIG,                // the periodic timeout signal
    DATA_READY_SIG,             // the invented reminder signal

    MAX_SIG                     // keep always last
};
//............................................................................
class Sensor : public QActive { // the Sensor active object
private:
    QTimeEvt m_timeEvt;         // private time event generator
    uint16_t m_pollCtr;
    uint16_t m_procCtr;

public:
    Sensor()
        : QActive(Q_STATE_CAST(&Sensor::initial)),
          m_timeEvt(this, TIMEOUT_SIG, 0U)
    {}

private:
    // hierarchical state machine ...
    static QState initial   (Sensor * const me, QEvt const * const e);
    static QState polling   (Sensor * const me, QEvt const * const e);
    static QState processing(Sensor * const me, QEvt const * const e);
    static QState idle      (Sensor * const me, QEvt const * const e);
    static QState busy      (Sensor * const me, QEvt const * const e);
    static QState final     (Sensor * const me, QEvt const * const e);
};

// HSM definition ------------------------------------------------------------
QState Sensor::initial(Sensor * const me, QEvt const *) {
    me->m_pollCtr = 0;
    me->m_procCtr = 0;

    // NOTE: don't forget to subscribe to any signals of interest,
    // if you're using publish-subscribe...
    //
    // subscribe(...);

    return Q_TRAN(&Sensor::polling);
}
//............................................................................
QState Sensor::final(Sensor * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> final\n");
            QF::stop(); // stop QF and cleanup
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Sensor::polling(Sensor * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // periodic timeout in 1/2 second and every 1/2 second
            me->m_timeEvt.armX(BSP_TICKS_PER_SEC/2, BSP_TICKS_PER_SEC/2);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_timeEvt.disarm();
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Sensor::processing);
        }
        case TIMEOUT_SIG: {
            // NOTE: this constant event is statically pre-allocated.
            // It can be posted/published as any other event.
            static const QEvt dataReadyEvt = { DATA_READY_SIG, 0U, 0U };

            ++me->m_pollCtr;
            printf("poll %3d\n", me->m_pollCtr);
            if ((me->m_pollCtr & 0x3U) == 0U) { // modulo 4
                me->POST(&dataReadyEvt, me);
            }
            return Q_HANDLED();
        }
        case TERMINATE_SIG: {
            return Q_TRAN(&Sensor::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Sensor::processing(Sensor * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_INIT_SIG: {
            return Q_TRAN(&Sensor::idle);
        }
    }
    return Q_SUPER(&Sensor::polling);
}
//............................................................................
QState Sensor::idle(Sensor * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> idle\n");
            return Q_HANDLED();
        }
        case DATA_READY_SIG: {
            return Q_TRAN(&Sensor::busy);
        }
    }
    return Q_SUPER(&Sensor::processing);
}
//............................................................................
QState Sensor::busy(Sensor * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> busy\n");
            return Q_HANDLED();
        }
        case TIMEOUT_SIG: {
            ++me->m_procCtr;
            printf("process %3d\n", me->m_procCtr);
            if ((me->m_procCtr & 0x1U) == 0U) { // modulo 2
                return Q_TRAN(&Sensor::idle);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Sensor::processing);
}

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static Sensor l_sensor; // the Sensor active object
static QEvt const *l_sensorQSto[10]; // Event queue storage for Sensor
static void *l_regPoolSto[100/sizeof(void *)]; // 100 bytes for the event pool

//............................................................................
int main(int argc, char *argv[]) {
    printf("Reminder state pattern\nQP version: %s\n"
           "Press ESC to quit...\n",
           QP::versionStr);

    BSP_init(argc, argv); // initialize the BSP

    QF::init(); // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QF::psInit()

    // initialize event pools...
    QF::poolInit(l_regPoolSto, sizeof(l_regPoolSto), sizeof(QEvt));

    // start the active objects...
    l_sensor.start(1U, l_sensorQSto, Q_DIM(l_sensorQSto),
                   (void *)0, 0, (QEvt *)0);

    return QF::run();  // run the QF application
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case '\33': {                                          // ESC pressed?
            l_sensor.POST(Q_NEW(QEvt, TERMINATE_SIG), (void *)0);
            break;
        }
    }
}
