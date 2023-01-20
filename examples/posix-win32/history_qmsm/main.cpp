//============================================================================
// Product: Transition to History Example
// Last updated for version: 7.3.0
// Last updated on: 2023-09-02
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"    // QP API
#include "history.hpp"  // application

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

        QP::QSignal sig = 0U;
        switch (c) {
            case 'o':  sig = OPEN_SIG;        break;
            case 'c':  sig = CLOSE_SIG;       break;
            case 't':  sig = TOAST_SIG;       break;
            case 'b':  sig = BAKE_SIG;        break;
            case 'f':  sig = OFF_SIG;         break;
            case 0x1B: sig = TERMINATE_SIG;   break;
        }

        // dispatch the event into the state machine
        QEvt const e(sig);
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
extern "C" Q_NORETURN Q_onError(char const * const file, int_t const  line) {
    FPRINTF_S(stderr, "Assertion failed in %s, line %d", file, line);
    QF::onCleanup();
    exit(-1);
}
