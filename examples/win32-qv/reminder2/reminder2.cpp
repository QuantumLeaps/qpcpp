//****************************************************************************
// Product: Reminder2 state pattern example
// Last Updated for Version: 5.4.0
// Date of the Last Update:  2015-05-04
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
#include "qpcpp.h"
#include "bsp.h"

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
        : QActive(Q_STATE_CAST(&Cruncher::initial))
    {}

private:
    // state machine ...
    static QState initial   (Cruncher * const me, QEvt const * const e);
    static QState processing(Cruncher * const me, QEvt const * const e);
    static QState final     (Cruncher * const me, QEvt const * const e);
};

// HSM definition ------------------------------------------------------------
QState Cruncher::initial(Cruncher * const me, QEvt const * const e) {
    (void)e; // unused parameter
    return Q_TRAN(&Cruncher::processing);
}
//............................................................................
QState Cruncher::processing(Cruncher * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            ReminderEvt *reminder = Q_NEW(ReminderEvt, CRUNCH_SIG);
            reminder->iter = 0;
            me->POST(reminder, me);
            me->m_sum = 0.0;
            status = Q_HANDLED();
            break;
        }
        case CRUNCH_SIG: {
            uint32_t i = ((ReminderEvt const *)e)->iter;
            uint32_t n = i;
            i += 100U;
            for (; n < i; ++n) {
                if ((n & 1) == 0) {
                    me->m_sum += 1.0/(2*n + 1);
                }
                else {
                    me->m_sum -= 1.0/(2*n + 1);
                }
            }
            if (i < 0x07000000U) {
                ReminderEvt *reminder = Q_NEW(ReminderEvt, CRUNCH_SIG);
                reminder->iter = i;
                me->POST(reminder, me);
                status = Q_HANDLED();
            }
            else {
                printf("pi=%16.14f\n", 4.0*me->m_sum);
                status = Q_TRAN(&Cruncher::processing);
            }
            break;
        }
        case ECHO_SIG: {
            printf("Echo! pi=%16.14f\n", 4.0*me->m_sum);
            status = Q_HANDLED();
            break;
        }
        case TERMINATE_SIG: {
            status = Q_TRAN(&Cruncher::final);
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
QState Cruncher::final(Cruncher * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            QF::stop(); // terminate the application
            status = Q_HANDLED();
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
