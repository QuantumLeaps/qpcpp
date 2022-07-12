//============================================================================
// Product: Console-based BSP
// Last Updated for Version: 6.9.1
// Date of the Last Update:  2020-09-22
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps, LLC. All rights reserved.
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

#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>

using namespace std;
using namespace QP;

Q_DEFINE_THIS_FILE

#ifdef Q_SPY
static uint8_t const l_QF::onClockTick = 0;
#endif

//............................................................................
void BSP_init(int argc, char * argv[]) {
    (void)argc;
    (void)argv;
    Q_ALLEGE(QS_INIT(argc > 1 ? argv[1] : nullptr));

    QS_OBJ_DICTIONARY(&l_QF::onClockTick);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);
    QS_GLB_FILTER(-QP::QS_QF_TICK);
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
    QTimeEvt::TICK_X(0U, &l_QF::onClockTick); // perform the QF clock tick processing

    QS_RX_INPUT(); // handle the QS-RX input
    QS_OUTPUT();   // handle the QS output

    int key = QF::consoleGetKey();
    if (key != 0) { /* any key pressed? */
        BSP_onKeyboardInput((uint8_t)key);
    }
}

//----------------------------------------------------------------------------
#ifdef Q_SPY

//............................................................................
//! callback function to execute a user command (to be implemented in BSP)
void QS::onCommand(uint8_t cmdId,
                   uint32_t param1, uint32_t param2, uint32_t param3)
{
    switch (cmdId) {
       case 0U: {
           break;
       }
       default:
           break;
    }

    // unused parameters
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif // Q_SPY
//----------------------------------------------------------------------------

//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    printf("Assertion failed in %s:%d\n", module, loc);
    FPRINTF_S(stderr, "Assertion failed in %s:%d", module, loc);
    QP::QF::onCleanup();
    exit(-1);
}

