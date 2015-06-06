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

#include <iostream>
#include <conio.h>

using namespace std;
using namespace QP;

//............................................................................
void BSP_init(int /*argc*/, char * /*argv*/[]) {
}
//............................................................................
void QF::onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC); // set the desired tick rate
}
//............................................................................
void QF::onCleanup(void) {
    cout << "\nBye!Bye!\n";
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
    cerr << "Assertion failed in " << file << " line " << line << endl;
    QF::stop();
}
