//============================================================================
// Product: BSP for emWin/uC/GUI, Win32 simulation, NO Window Manager
// Last updated for version 7.3.0
// Last updated on  2023-07-20
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
    #include "LCD_SIM.h"
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
        QP::QEvt(KEY_UP_REL_SIG),      // hardkey UP released
        QP::QEvt(KEY_UP_PRESS_SIG),    // hardkey UP pressed
        QP::QEvt(KEY_RIGHT_REL_SIG),   // hardkey RIGHT released
        QP::QEvt(KEY_RIGHT_PRESS_SIG), // hardkey RIGHT pressed
        QP::QEvt(KEY_CENTER_REL_SIG),  // hardkey CENTER released
        QP::QEvt(KEY_CENTER_PRESS_SIG),// hardkey CENTER pressed
        QP::QEvt(KEY_LEFT_REL_SIG),    // hardkey LEFT released
        QP::QEvt(KEY_LEFT_PRESS_SIG),  // hardkey LEFT pressed
        QP::QEvt(KEY_DOWN_REL_SIG),    // hardkey DOWN released
        QP::QEvt(KEY_DOWN_PRESS_SIG),  // hardkey DOWN pressed
        QP::QEvt(KEY_POWER_REL_SIG),   // hardkey POWER released
        QP::QEvt(KEY_POWER_PRESS_SIG)  // hardkey POWER pressed
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
void BSP_init(void) {
    int n;

    GUI_Init(); // initialize the embedded GUI

    n = SIM_HARDKEY_GetNum();
    for (n = n - 1; n >= 0; --n) {
        SIM_HARDKEY_SetCallback(n, &simHardKey);
    }

    QF::setTickRate(BSP_TICKS_PER_SEC, 30); // set the desired tick rate
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
void QF::onClockTick(void) {
    QTimeEvt::TICK(&l_clock_tick); // perform the QF clock tick processing
}

extern "C" {
//............................................................................
Q_NORETURN Q_onError(char const * const file, int_t const loc) {
    char str[256];
    SNPRINTF_S(str, sizeof(str), "%s:%d", file, loc);
    MessageBox(NULL, str, "Assertion Failure", MB_TASKMODAL | MB_OK);
    QF::stop(); // terminate the QF, causes termination of the MainTask()
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

} // extern "C"
