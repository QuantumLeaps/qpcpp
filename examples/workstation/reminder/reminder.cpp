//============================================================================
// Product: Reminder state pattern example
// Last Updated for Version: 6.5.0
// Date of the Last Update:  2019-03-25
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

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities

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
        : QActive(&initial),
          m_timeEvt(this, TIMEOUT_SIG, 0U)
    {}

private:
    // hierarchical state machine ...
    Q_STATE_DECL(initial);
    Q_STATE_DECL(polling);
    Q_STATE_DECL(processing);
    Q_STATE_DECL(idle);
    Q_STATE_DECL(busy);
    Q_STATE_DECL(final);
};

// HSM definition ------------------------------------------------------------
Q_STATE_DEF(Sensor, initial) {
    m_pollCtr = 0;
    m_procCtr = 0;

    // NOTE: don't forget to subscribe to any signals of interest,
    // if you're using publish-subscribe...
    //
    // subscribe(...);
    (void)e; // unused parameter
    return tran(&polling);
}
//............................................................................
Q_STATE_DEF(Sensor, final) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            PRINTF_S("%s\n", "-> final");
            QF::stop(); // stop QF and cleanup
            return Q_RET_HANDLED;
        }
    }
    return super(&QHsm::top);
}
//............................................................................
Q_STATE_DEF(Sensor, polling) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // periodic timeout in 1/2 second and every 1/2 second
            m_timeEvt.armX(BSP_TICKS_PER_SEC/2, BSP_TICKS_PER_SEC/2);
            return Q_RET_HANDLED;
        }
        case Q_EXIT_SIG: {
            m_timeEvt.disarm();
            return Q_RET_HANDLED;
        }
        case Q_INIT_SIG: {
            return tran(&processing);
        }
        case TIMEOUT_SIG: {
            // NOTE: this constant event is statically pre-allocated.
            // It can be posted/published as any other event.
            static const QEvt dataReadyEvt = { DATA_READY_SIG, 0U, 0U };

            ++m_pollCtr;
            PRINTF_S("poll %3d\n", m_pollCtr);
            if ((m_pollCtr & 0x3U) == 0U) { // modulo 4
                POST(&dataReadyEvt, me);
            }
            return Q_RET_HANDLED;
        }
        case TERMINATE_SIG: {
            return tran(&final);
        }
    }
    return super(&QHsm::top);
}
//............................................................................
Q_STATE_DEF(Sensor, processing) {
    switch (e->sig) {
        case Q_INIT_SIG: {
            return tran(&idle);
        }
    }
    return super(&polling);
}
//............................................................................
Q_STATE_DEF(Sensor, idle) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            PRINTF_S("%s\n", "-> idle");
            return Q_RET_HANDLED;
        }
        case DATA_READY_SIG: {
            return tran(&busy);
        }
    }
    return super(&processing);
}
//............................................................................
Q_STATE_DEF(Sensor, busy) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            PRINTF_S("%s\n", "-> busy");
            return Q_RET_HANDLED;
        }
        case TIMEOUT_SIG: {
            ++m_procCtr;
            PRINTF_S("process %3d\n", m_procCtr);
            if ((m_procCtr & 0x1U) == 0U) { // modulo 2
                return tran(&idle);
            }
            return Q_RET_HANDLED;
        }
    }
    return super(&processing);
}

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static Sensor l_sensor; // the Sensor active object
static QEvt const *l_sensorQSto[10]; // storage for event queue for Sensor
static void *l_regPoolSto[100/sizeof(void *)]; // storage for the event pool

//............................................................................
int main(int argc, char *argv[]) {
    PRINTF_S("Reminder state pattern\nQP version: %s\n"
           "Press ESC to quit...\n",
           QP::versionStr);

    BSP_init(argc, argv); // initialize the BSP

    QF::init(); // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QActive::psInit()

    // initialize event pools...
    QF::poolInit(l_regPoolSto, sizeof(l_regPoolSto), sizeof(QEvt));

    // start the active objects...
    l_sensor.start(1U, l_sensorQSto, Q_DIM(l_sensorQSto),
                   nullptr, 0, (QEvt *)0);

    return QF::run();  // run the QF application
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case '\33': { // ESC pressed?
            l_sensor.POST(Q_NEW(QEvt, TERMINATE_SIG), nullptr);
            break;
        }
    }
}
