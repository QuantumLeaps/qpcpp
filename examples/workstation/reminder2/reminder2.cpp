//****************************************************************************
// Product: Reminder2 state pattern example
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// https://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.hpp"
#include "bsp.hpp"

#include <stdio.h>

Q_DEFINE_THIS_FILE

using namespace QP;

//............................................................................
enum ReminderSignals {
    CRUNCH_SIG = Q_USER_SIG, // the invented reminder signal
    ECHO_SIG,     // check the responsiveness of the system
    TERMINATE_SIG // terminate the application
};

struct ReminderEvt : public QEvt {
    uint32_t iter; // the next iteration to perform
};

//............................................................................
class Cruncher : public QActive { // the Cruncher active object
private:
    double m_sum; // internal variable

public:
    Cruncher()
        : QActive(&initial)
    {}

private:
    // state machine ...
    Q_STATE_DECL(initial);
    Q_STATE_DECL(processing);
    Q_STATE_DECL(final);
};

// HSM definition ------------------------------------------------------------
Q_STATE_DEF(Cruncher, initial) {
    (void)e; // unused parameter
    return tran(&processing);
}
//............................................................................
Q_STATE_DEF(Cruncher, processing) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            ReminderEvt *reminder = Q_NEW(ReminderEvt, CRUNCH_SIG);
            reminder->iter = 0;
            POST(reminder, me);
            m_sum = 0.0;
            status = Q_RET_HANDLED;
            break;
        }
        case CRUNCH_SIG: {
            uint32_t i = ((ReminderEvt const *)e)->iter;
            uint32_t n = i;
            i += 100U;
            for (; n < i; ++n) {
                if ((n & 1) == 0) {
                    m_sum += 1.0/(2*n + 1);
                }
                else {
                    m_sum -= 1.0/(2*n + 1);
                }
            }
            if (i < 0x07000000U) {
                ReminderEvt *reminder = Q_NEW(ReminderEvt, CRUNCH_SIG);
                reminder->iter = i;
                POST(reminder, me);
                status = Q_RET_HANDLED;
            }
            else {
                printf("pi=%16.14f\n", 4.0*m_sum);
                status = tran(&processing);
            }
            break;
        }
        case ECHO_SIG: {
            printf("Echo! pi=%16.14f\n", 4.0*m_sum);
            status = Q_RET_HANDLED;
            break;
        }
        case TERMINATE_SIG: {
            status = tran(&final);
            break;
        }
        default: {
            status = super(&QHsm::top);
            break;
        }
    }
    return status;
}
//............................................................................
Q_STATE_DEF(Cruncher, final) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            QF::stop(); // terminate the application
            status = Q_RET_HANDLED;
            break;
        }
        default: {
            status = super(&QHsm::top);
            break;
        }
    }
    return status;
}

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static Cruncher l_cruncher;     // the Cruncher active object
QEvt const *l_cruncherQSto[10]; // Event queue storage for Cruncher AO
static QF_MPOOL_EL(ReminderEvt) l_smlPoolSto[20]; // storage for small pool

//............................................................................
int main(int argc, char *argv[]) {
    printf("Reminder state pattern\nQP version: %s\n"
           "Press 'e' to echo the current value...\n"
           "Press ESC to quit...\n",
           QP::versionStr);

    BSP_init(argc, argv); // initialize the BSP
    QF::init(); // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QF_psInit()

    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

    // instantiate and start the active objects...
    l_cruncher.start(1U,
                     l_cruncherQSto, Q_DIM(l_cruncherQSto),
                     (void *)0, 0U, (QEvt *)0);

    return QF::run(); // run the QF application
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 'e': {
            static QEvt const echoEvt = { ECHO_SIG, 0U, 0U };
            l_cruncher.POST(&echoEvt, (void *)0);
            break;
        }
        case '\033': { // ESC pressed?
            // NOTE: this constant event is statically pre-allocated.
            // It can be posted/published as any other event.
            //
            static QEvt const terminateEvt = { TERMINATE_SIG, 0U, 0U };
            l_cruncher.POST(&terminateEvt, (void *)0);
            break;
        }
    }
}
