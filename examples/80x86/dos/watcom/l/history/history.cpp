//////////////////////////////////////////////////////////////////////////////
// Product: History state pattern example
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 15, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
//////////////////////////////////////////////////////////////////////////////
#include "qep_port.h"
#include "qassert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Q_DEFINE_THIS_FILE

using namespace QP;

//............................................................................
enum ToasterOvenSignals {
    OPEN_SIG = Q_USER_SIG,
    CLOSE_SIG,
    TOAST_SIG,
    BAKE_SIG,
    OFF_SIG,
    TERMINATE_SIG                                 // terminate the application
};
//............................................................................
class ToasterOven : public QHsm {
private:
    QStateHandler m_doorClosed_history;

public:
    ToasterOven() : QHsm((QStateHandler)&ToasterOven::initial) {    }

private:
    static QState initial   (ToasterOven *me, QEvt const *e);
    static QState doorClosed(ToasterOven *me, QEvt const *e);
    static QState off       (ToasterOven *me, QEvt const *e);
    static QState heating   (ToasterOven *me, QEvt const *e);
    static QState toasting  (ToasterOven *me, QEvt const *e);
    static QState baking    (ToasterOven *me, QEvt const *e);
    static QState doorOpen  (ToasterOven *me, QEvt const *e);
    static QState final     (ToasterOven *me, QEvt const *e);
};
//............................................................................
QState ToasterOven::initial(ToasterOven *me, QEvt const *) {
    me->m_doorClosed_history = Q_STATE_CAST(&ToasterOven::off);
    return Q_TRAN(&ToasterOven::doorClosed);
}
//............................................................................
QState ToasterOven::final(ToasterOven *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> final\n");
            printf("\nBye!Bye!\n");
            _exit(0);                                  // exit the application
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState ToasterOven::doorClosed(ToasterOven *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("door-Closed;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_doorClosed_history = me->state();       //<-save the HISTORY
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&ToasterOven::off);
        }
        case OPEN_SIG: {
            return Q_TRAN(&ToasterOven::doorOpen);
        }
        case TOAST_SIG: {
            return Q_TRAN(&ToasterOven::toasting);
        }
        case BAKE_SIG: {
            return Q_TRAN(&ToasterOven::baking);
        }
        case OFF_SIG: {
            return Q_TRAN(&ToasterOven::off);
        }
        case TERMINATE_SIG: {
            return Q_TRAN(&ToasterOven::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState ToasterOven::off(ToasterOven *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("toaster-Off;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&ToasterOven::doorClosed);
}
//............................................................................
QState ToasterOven::heating(ToasterOven *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("heater-On;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            printf("heater-Off;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&ToasterOven::doorClosed);
}
//............................................................................
QState ToasterOven::toasting(ToasterOven *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("toasting;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&ToasterOven::heating);
}
//............................................................................
QState ToasterOven::baking(ToasterOven *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("baking;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&ToasterOven::heating);
}
//............................................................................
QState ToasterOven::doorOpen(ToasterOven *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("door-Open,lamp-On;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            printf("lamp-Off;");
            return Q_HANDLED();
        }
        case CLOSE_SIG: {
            return Q_TRAN(me->m_doorClosed_history);//<-tranisition to HISTORY
        }
    }
    return Q_SUPER(&QHsm::top);
}
// test harness ==============================================================

#include <conio.h>

static ToasterOven l_test;

int main() {
    printf("History state pattern\nQEP version: %s\n"
           "Press 'o' to OPEN  the door\n"
           "Press 'c' to CLOSE the door\n"
           "Press 't' to start TOASTING\n"
           "Press 'b' to start BAKING\n"
           "Press 'f' to turn the oven OFF\n"
           "Press ESC to quit...\n",
           QEP::getVersion());

    l_test.init(); // trigger the initial transition before dispatching events

    for (;;) {
        printf("\n");
        uint8_t c = (uint8_t)_getch();  // read one character from the console
        printf("%c: ", c);

        QEvt e;
        switch (c) {
            case 'o':  e.sig = OPEN_SIG;        break;
            case 'c':  e.sig = CLOSE_SIG;       break;
            case 't':  e.sig = TOAST_SIG;       break;
            case 'b':  e.sig = BAKE_SIG;        break;
            case 'f':  e.sig = OFF_SIG;         break;
            case 0x1B: e.sig = TERMINATE_SIG;   break;
        }
        l_test.dispatch(&e);      // dispatch the event into the state machine
    }
    return 0;
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    _exit(-1);
}
