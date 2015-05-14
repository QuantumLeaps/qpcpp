//****************************************************************************
// Product: Console-based BSP, MinGW,
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
#include "clock.h"

#include <stdio.h>
#include <conio.h>

using namespace std;
using namespace QP;

//............................................................................
void BSP_init(int /*argc*/, char * /*argv*/[]) {
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 'o': { // 'o': Alarm on event?
            APP_alarmClock->POST(Q_NEW(QEvt, ALARM_ON_SIG), (void *)0);
            break;
        }
        case 'f': { // 'f': Alarm off event?
            APP_alarmClock->POST(Q_NEW(QEvt, ALARM_OFF_SIG), (void *)0);
            break;
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            SetEvt *e = Q_NEW(SetEvt, ALARM_SET_SIG);
            e->digit = (uint8_t)(key - '0');
            APP_alarmClock->POST(e, (void *)0);
            break;
        }
        case '0': {
            SetEvt *e = Q_NEW(SetEvt, ALARM_SET_SIG);
            e->digit = 0;
            APP_alarmClock->POST(e, (void *)0);
            break;
        }
        case 'a': { // 'a': Clock 12H event?
            APP_alarmClock->POST(Q_NEW(QEvt, CLOCK_12H_SIG), (void *)0);
            break;
        }
        case 'b': { // 'b': Clock 24H event?
            APP_alarmClock->POST(Q_NEW(QEvt, CLOCK_24H_SIG), (void *)0);
            break;
        }
        case '\33': { // ESC pressed?
            APP_alarmClock->POST(Q_NEW(QEvt, TERMINATE_SIG), (void *)0);
            break;
        }
    }
}
//............................................................................
void BSP_showMsg(char_t const *str) {
    printf(str);
    printf("\n");
    fflush(stdout);
}
//............................................................................
void BSP_showTime24H(char_t const *str, uint32_t time, uint32_t base) {
    printf(str);
    printf("%02d:%02d\n", (int)(time / base), (int)(time % base));
    fflush(stdout);
}
//............................................................................
void BSP_showTime12H(char_t const *str, uint32_t time, uint32_t base) {
    uint32_t h = time / base;

    printf(str);
    printf("%02d:%02d %s\n", (h % 12) ? (h % 12) : 12,
           time % base, (h / 12) ? "PM" : "AM");
    fflush(stdout);
}
//............................................................................
void QF::onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC); // set the desired tick rate
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QP::QF_onClockTick(void) {
    QF::TICK_X(0U, &l_clock_tick); // perform the QF clock tick processing
    if (_kbhit()) { // any key pressed?
        BSP_onKeyboardInput(_getch());
    }
}
//............................................................................
extern "C" void Q_onAssert(char_t const * const file, int_t line) {
    printf("Assertion failed in %s at %d\n", file, line);
    QF::stop();
}
