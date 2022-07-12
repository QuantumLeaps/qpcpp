//============================================================================
// Product: Transition to History Example
// Last Updated for Version: 6.9.1
// Date of the Last Update:  2020-09-22
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2002-2018 Quantum Leaps, LLC. All rights reserved.
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
#include "qpcpp.hpp"   // QP API
#include "history.hpp"

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>

using namespace QP;
using namespace std;

//............................................................................
int main() {

    QF::init();
    QF::onStartup();

    PRINTF_S("History state pattern\nQEP version: %s\n"
           "Press 'o' to OPEN  the door\n"
           "Press 'c' to CLOSE the door\n"
           "Press 't' to start TOASTING\n"
           "Press 'b' to start BAKING\n"
           "Press 'f' to turn the oven OFF\n"
           "Press ESC to quit...\n",
           QP_VERSION_STR);

    // instantiate the ToastOven HSM and trigger the initial transition
    the_oven->init(0U);

    for (;;) {

        PRINTF_S("\n", "");

        uint8_t c = (uint8_t)QF::consoleWaitForKey();
        PRINTF_S("%c: ", c);

        QP::QEvt e;
        switch (c) {
            case 'o':  e.sig = OPEN_SIG;        break;
            case 'c':  e.sig = CLOSE_SIG;       break;
            case 't':  e.sig = TOAST_SIG;       break;
            case 'b':  e.sig = BAKE_SIG;        break;
            case 'f':  e.sig = OFF_SIG;         break;
            case 0x1B: e.sig = TERMINATE_SIG;   break;
        }

        // dispatch the event into the state machine
        the_oven->dispatch(&e, 0U);
    }

    QF::onCleanup();
    return 0;
}

namespace QP {
/*..........................................................................*/
void QF::onStartup(void) {
    QF::consoleSetup();
}
/*..........................................................................*/
void QF::onCleanup(void) {
    QF::consoleCleanup();
}
/*..........................................................................*/
void QF::onClockTick(void) {
}

} // namespace QP

//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const file, int_t const  line) {
    FPRINTF_S(stderr, "Assertion failed in %s, line %d", file, line);
    QF::onCleanup();
    exit(-1);
}
