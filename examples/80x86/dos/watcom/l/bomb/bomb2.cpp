//////////////////////////////////////////////////////////////////////////////
// Product:  Time Bomb Example with "State Table"
// Last Updated for Version: 4.5.03
// Date of the Last Update:  Nov 22, 2009
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
#include "statetbl.h"

enum BombSignals {                             // all signals for the Bomb FSM
    UP_SIG,
    DOWN_SIG,
    ARM_SIG,
    TICK_SIG,

    MAX_SIG                                           // the number of signals
};

enum BombStates {                               // all states for the Bomb FSM
    SETTING_STATE,
    TIMING_STATE,

    MAX_STATE                                          // the number of states
};

struct TickEvt : public Event {
    uint8_t fine_time;                              // the fine 1/10 s counter
};

class Bomb2 : public StateTable {                              // the Bomb FSM
public:
    Bomb2(uint8_t defuse);                                      // public ctor

    static void initial     (Bomb2 *me, Event const *e); // initial transition

    static void setting_UP  (Bomb2 *me, Event const *e);   // transition func.
    static void setting_DOWN(Bomb2 *me, Event const *e);   // transition func.
    static void setting_ARM (Bomb2 *me, Event const *e);   // transition func.
    static void timing_UP   (Bomb2 *me, Event const *e);   // transition func.
    static void timing_DOWN (Bomb2 *me, Event const *e);   // transition func.
    static void timing_ARM  (Bomb2 *me, Event const *e);   // transition func.
    static void timing_TICK (Bomb2 *me, Event const *e);   // transition func.

private:
    static Tran const state_table[MAX_STATE][MAX_SIG];

    uint8_t m_timeout;                     // number of seconds till explosion
    uint8_t m_defuse;                 // secret defuse code to disarm the bomb
    uint8_t m_code;               // currently entered code to disarm the bomb
};

                                           // the initial value of the timeout
#define INIT_TIMEOUT   10

//............................................................................
Tran const Bomb2::state_table[MAX_STATE][MAX_SIG] = {
    { (Tran)&Bomb2::setting_UP,  (Tran)&Bomb2::setting_DOWN,
      (Tran)&Bomb2::setting_ARM, &StateTable::empty },
    { (Tran)&Bomb2::timing_UP,   (Tran)&Bomb2::timing_DOWN,
      (Tran)&Bomb2::timing_ARM,  (Tran)&Bomb2::timing_TICK  }
};
//............................................................................
Bomb2::Bomb2(uint8_t defuse)
    : StateTable(&Bomb2::state_table[0][0], MAX_STATE, MAX_SIG,
                 (Tran)&Bomb2::initial),
      m_defuse(defuse)
{}
//............................................................................
void Bomb2::initial(Bomb2 *me, Event const *) {
    me->m_timeout = INIT_TIMEOUT;
    TRAN(SETTING_STATE);
}
//............................................................................
void Bomb2::setting_UP(Bomb2 *me, Event const *) {
    if (me->m_timeout < 60) {
        ++me->m_timeout;
        BSP_display(me->m_timeout);
    }
}
//............................................................................
void Bomb2::setting_DOWN(Bomb2 *me, Event const *) {
    if (me->m_timeout > 1) {
        --me->m_timeout;
        BSP_display(me->m_timeout);
    }
}
//............................................................................
void Bomb2::setting_ARM(Bomb2 *me, Event const *) {
    me->m_code = 0;
    TRAN(TIMING_STATE);                              // transition to "timing"
}
//............................................................................
void Bomb2::timing_UP(Bomb2 *me, Event const *) {
    me->m_code <<= 1;
    me->m_code |= 1;
}
//............................................................................
void Bomb2::timing_DOWN(Bomb2 *me, Event const *) {
    me->m_code <<= 1;
}
//............................................................................
void Bomb2::timing_ARM(Bomb2 *me, Event const *) {
    if (me->m_code == me->m_defuse) {
        TRAN(SETTING_STATE);                        // transition to "setting"
    }
}
//............................................................................
void Bomb2::timing_TICK(Bomb2 *me, Event const *e) {
    if (((TickEvt const *)e)->fine_time == 0) {
        --me->m_timeout;
        BSP_display(me->m_timeout);
        if (me->m_timeout == 0) {
            BSP_boom();                                    // destroy the bomb
        }
    }
}

// Test harness --------------------------------------------------------------
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

static Bomb2 l_bomb(0x0D);  // time bomb FSM, the secret defuse code, 1101 bin

//............................................................................
int main() {

    cout << "Time Bomb (State-Table)"    << endl
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

        if (kbhit()) {
            static Event const up_evt   = { UP_SIG   };
            static Event const down_evt = { DOWN_SIG };
            static Event const arm_evt  = { ARM_SIG  };
            Event const *e = (Event *)0;

            switch (getch()) {
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
                    cout << endl << "ESC : Bye! Bye!";
                    exit(0);
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
