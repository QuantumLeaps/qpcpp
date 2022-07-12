//============================================================================
// Product: BSP for emWin/uC/GUI, Win32 simulation, NO Window Manager
// Last updated for version 6.8.0
// Last updated on  2020-01-22
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
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
#include "dpp.hpp"
#include "bsp.hpp"

extern "C" {
    #include "GUI.h"
    #include "GUI_SIM.h"
    #include "DIALOG.h"
}

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities
#include <time.h>
#include <windows.h>

Q_DEFINE_THIS_FILE

// local variables -----------------------------------------------------------
#ifdef Q_SPY
    static uint8_t l_running;
    static SOCKET  l_sock = INVALID_SOCKET;

    static uint8_t const l_clock_tick = 0U;
    static uint8_t const l_simHardKey = 0U;
    static uint8_t const l_MOUSE_StoreState = 0U;
#endif

//............................................................................
static void simHardKey(int keyIndex, int keyState) {
    static const QEvt keyEvt[] = {
        { KEY_UP_REL_SIG,       0 }, // hardkey UP released
        { KEY_UP_PRESS_SIG,     0 }, // hardkey UP pressed
        { KEY_RIGHT_REL_SIG,    0 }, // hardkey RIGHT released
        { KEY_RIGHT_PRESS_SIG,  0 }, // hardkey RIGHT pressed
        { KEY_CENTER_REL_SIG,   0 }, // hardkey CENTER released
        { KEY_CENTER_PRESS_SIG, 0 }, // hardkey CENTER pressed
        { KEY_LEFT_REL_SIG,     0 }, // hardkey LEFT released
        { KEY_LEFT_PRESS_SIG,   0 }, // hardkey LEFT pressed
        { KEY_DOWN_REL_SIG,     0 }, // hardkey DOWN released
        { KEY_DOWN_PRESS_SIG,   0 }, // hardkey DOWN pressed
        { KEY_POWER_REL_SIG,    0 }, // hardkey POWER released
        { KEY_POWER_PRESS_SIG,  0 }  // hardkey POWER pressed
    };

    // do not overrun the array
    Q_REQUIRE((keyIndex * 2) + keyState < Q_DIM(keyEvt));

    // post the hardkey event to the Table active object (GUI manager)
    AO_Table->POST(&keyEvt[(keyIndex * 2) + keyState], &l_simHardKey);

    if ((keyIndex == 5) && (keyState == 0)) { // hardkey POWER released?
        QF::stop(); // terminate the simulation
    }
}
//............................................................................
extern "C" void GUI_MOUSE_StoreState(const GUI_PID_STATE *pState) {
    MouseEvt *pe = Q_NEW(MouseEvt, MOUSE_CHANGE_SIG);
    pe->x = pState->x;
    pe->y = pState->y;
    pe->Pressed = pState->Pressed;
    pe->Layer = pState->Layer;
    AO_Table->POST(pe, &l_MOUSE_StoreState);
}
//............................................................................
#ifdef Q_SPY
static DWORD WINAPI idleThread(LPVOID par) { // signature for CreateThread()
    (void)par;
    l_running = (uint8_t)1;
    while (l_running) {
        uint16_t nBytes = 1024;
        uint8_t const *block;
        QF_CRIT_ENTRY(dummy);
        block = QS::getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);
        if (block != (uint8_t *)0) {
            send(l_sock, (char const *)block, nBytes, 0);
        }
        Sleep(10); // wait for a while
    }
    return 0; // return success
}
#endif
//............................................................................
void BSP_init(void) {
    int n;

    GUI_Init(); // initialize the embedded GUI

    n = SIM_HARDKEY_GetNum();
    for (n = n - 1; n >= 0; --n) {
        SIM_HARDKEY_SetCallback(n, &simHardKey);
    }

    QF_setTickRate(BSP_TICKS_PER_SEC, 30); // set the desired tick rate
}
//............................................................................
void QF::onStartup(void) {
}
//............................................................................
void QF::onCleanup(void) {
#if Q_SPY
    l_running = (uint8_t)0;
#endif
}
//............................................................................
void QP::QF_onClockTick(void) {
    QTimeEvt::TICK(&l_clock_tick); // perform the QF clock tick processing
}

//............................................................................
Q_NORETURN Q_onAssert(char const * const file, int_t const loc) {
    char str[256];

    QF_CRIT_ENTRY(dummy); // make sure nothing else is running
    SNPRINTF_S(str, sizeof(str), "%s:%d", file, loc);
    MessageBox(NULL, str, "Assertion Failure", MB_TASKMODAL | MB_OK);
    QF::stop(); // terminate the QF, causes termination of the MainTask()
}

