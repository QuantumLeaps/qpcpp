//============================================================================
// Product: Console-based BSP
// Last Updated for Version: 6.3.6
// Date of the Last Update:  2018-10-14
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
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
#include "clock.hpp"
#include "bsp.hpp"

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>

using namespace std;
using namespace QP;

//............................................................................
void BSP_init(int /*argc*/, char * /*argv*/[]) {
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 'o': { // 'o': Alarm on event?
            APP_alarmClock->POST(Q_NEW(QEvt, ALARM_ON_SIG), nullptr);
            break;
        }
        case 'f': { // 'f': Alarm off event?
            APP_alarmClock->POST(Q_NEW(QEvt, ALARM_OFF_SIG), nullptr);
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
            APP_alarmClock->POST(e, nullptr);
            break;
        }
        case '0': {
            SetEvt *e = Q_NEW(SetEvt, ALARM_SET_SIG);
            e->digit = 0;
            APP_alarmClock->POST(e, nullptr);
            break;
        }
        case 'a': { // 'a': Clock 12H event?
            APP_alarmClock->POST(Q_NEW(QEvt, CLOCK_12H_SIG), nullptr);
            break;
        }
        case 'b': { // 'b': Clock 24H event?
            APP_alarmClock->POST(Q_NEW(QEvt, CLOCK_24H_SIG), nullptr);
            break;
        }
        case '\33': { // ESC pressed?
            APP_alarmClock->POST(Q_NEW(QEvt, TERMINATE_SIG), nullptr);
            break;
        }
    }
}
//............................................................................
void BSP_showMsg(char const *str) {
    PRINTF_S("%s\n", str);
    fflush(stdout);
}
//............................................................................
void BSP_showTime24H(char const *str, uint32_t time, uint32_t base) {
    PRINTF_S("%s %02d:%02d\n", str, (int)(time / base), (int)(time % base));
    fflush(stdout);
}
//............................................................................
void BSP_showTime12H(char const *str, uint32_t time, uint32_t base) {
    uint32_t h = time / base;

    PRINTF_S("%s %02d:%02d %s\n", str, (h % 12) ? (h % 12) : 12,
           time % base, (h / 12) ? "PM" : "AM");
    fflush(stdout);
}
//............................................................................
void QF::onStartup(void) {
    QF::setTickRate(BSP_TICKS_PER_SEC, 30); // set the desired tick rate
    QF::consoleSetup();
}
//............................................................................
void QF::onCleanup(void) {
    QF::consoleCleanup();
}
//............................................................................
void QP::QF::onClockTick(void) {
    QTimeEvt::TICK_X(0U, &l_clock_tick); // perform the QF clock tick processing
    int key = QF::consoleGetKey();
    if (key != 0) { /* any key pressed? */
        BSP_onKeyboardInput((uint8_t)key);
    }
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const file, int_t const line) {
    FPRINTF_S(stderr, "Assertion failed in %s at %d\n", file, line);
    exit(-1);

}
