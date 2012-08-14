//////////////////////////////////////////////////////////////////////////////
// Product:  Time Bomb Example with with "Nested Switch Statement"
// Last Updated for Version: 4.1.01
// Date of the Last Update:  Nov 04, 2009
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "bsp.h"

enum BombSignals {                             // all signals for the Bomb FSM
    UP_SIG,
    DOWN_SIG,
    ARM_SIG,
    TICK_SIG
};

enum BombStates {                               // all states for the Bomb FSM
    SETTING_STATE,
    TIMING_STATE
};

struct Event {
    uint16_t sig;                                       // signal of the event
    // add event parameters by subclassing the Event structure...
};

struct TickEvt : public Event {
    uint8_t fine_time;                              // the fine 1/10 s counter
};

class Bomb1 {                                                  // the Bomb FSM
public:
    Bomb1(uint8_t defuse) : m_defuse(defuse) {}
    void init(void);                               // the init() FSM interface
    void dispatch(Event const *e);             // the dispatch() FSM interface

private:
    void tran(uint8_t target)  { m_state = target; }

private:
    uint8_t m_state;                              // the scalar state-variable
    uint8_t m_timeout;                     // number of seconds till explosion
    uint8_t m_code;               // currently entered code to disarm the bomb
    uint8_t m_defuse;                 // secret defuse code to disarm the bomb
};

//............................................................................
                                           // the initial value of the timeout
#define INIT_TIMEOUT   10

//............................................................................
void Bomb1::init(void) {
    m_timeout = INIT_TIMEOUT;       // timeout is initialized in initial tran.
    tran(SETTING_STATE);
}
//............................................................................
void Bomb1::dispatch(Event const *e) {
    switch (m_state) {
        case SETTING_STATE: {
            switch (e->sig) {
                case UP_SIG: {
                    if (m_timeout < 60) {
                        ++m_timeout;
                        BSP_display(m_timeout);
                    }
                    break;
                }
                case DOWN_SIG: {
                    if (m_timeout > 1) {
                        --m_timeout;
                        BSP_display(m_timeout);
                    }
                    break;
                }
                case ARM_SIG: {
                    m_code = 0;
                    tran(TIMING_STATE);              // transition to "timing"
                    break;
                }
            }
            break;
        }
        case TIMING_STATE: {
            switch (e->sig) {
                case UP_SIG: {
                    m_code <<= 1;
                    m_code |= 1;
                    break;
                }
                case DOWN_SIG: {
                    m_code <<= 1;
                    break;
                }
                case ARM_SIG: {
                    if (m_code == m_defuse) {
                        tran(SETTING_STATE);        // transition to "setting"
                    }
                    break;
                }
                case TICK_SIG: {
                    if (((TickEvt const *)e)->fine_time == 0) {
                        --m_timeout;
                        BSP_display(m_timeout);
                        if (m_timeout == 0) {
                            BSP_boom();                    // destroy the bomb
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}

// Test harness --------------------------------------------------------------
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

static Bomb1 l_bomb(0x0D);  // time bomb FSM, the secret defuse code, 1101 bin

//............................................................................
int main() {

    cout << "Time Bomb (Nested switch)"  << endl
         << "Press 'u'   for UP   event" << endl
         << "Press 'd'   for DOWN event" << endl
         << "Press 'a'   for ARM  event" << endl
         << "Press <Esc> to quit."       << endl;

    l_bomb.init();                              // take the initial transition

    static TickEvt tick_evt;
    tick_evt.sig = TICK_SIG;
    tick_evt.fine_time = 0;
    for (;;) {                                                   // event loop

        delay(100);                                            // 100 ms delay

        if (++tick_evt.fine_time == 10) {
            tick_evt.fine_time = 0;
        }
        cout.width(1);
        cout << "T(" << (int)tick_evt.fine_time << ')'
             << ((tick_evt.fine_time == 0) ? '\n' : ' ');

        l_bomb.dispatch(&tick_evt);                     // dispatch TICK event

        if (_kbhit()) {
            static Event const up_evt   = { UP_SIG   };
            static Event const down_evt = { DOWN_SIG };
            static Event const arm_evt  = { ARM_SIG  };
            Event const *e = (Event *)0;

            switch (_getch()) {
                case 'u': {                                        // UP event
                    cout << endl << "UP  : ";
                    e = &up_evt;                      // generate the UP event
                    break;
                }
                case 'd': {                                      // DOWN event
                    cout << endl << "DOWN: ";
                    e = &down_evt;                  // generate the DOWN event
                    break;
                }
                case 'a': {                                       // ARM event
                    cout << endl << "ARM : ";
                    e = &arm_evt;                    // generate the ARM event
                    break;
                }
                case '\33': {                                     // <Esc> key
                    cout << endl << "ESC : Bye! Bye!" << flush << endl;
                    _exit(0);
                    break;
                }
            }
            if (e != (Event *)0) {                // keyboard event available?
                l_bomb.dispatch(e);                      // dispatch the event
            }
        }
    }

    return 0;
}
