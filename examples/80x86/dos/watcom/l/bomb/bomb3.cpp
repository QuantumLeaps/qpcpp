//////////////////////////////////////////////////////////////////////////////
// Product:  Time Bomb Example with the State design pattern (GoF)
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

class Bomb3;
class BombState {
public:
    virtual void onUP  (Bomb3 *) const {}
    virtual void onDOWN(Bomb3 *) const {}
    virtual void onARM (Bomb3 *) const {}
    virtual void onTICK(Bomb3 *, uint8_t) const {}
};

class SettingState : public BombState {
public:
    virtual void onUP  (Bomb3 *context) const;
    virtual void onDOWN(Bomb3 *context) const;
    virtual void onARM (Bomb3 *context) const;
};

class TimingState : public BombState {
public:
    virtual void onUP  (Bomb3 *context) const;
    virtual void onDOWN(Bomb3 *context) const;
    virtual void onARM (Bomb3 *context) const;
    virtual void onTICK(Bomb3 *context, uint8_t fine_time) const;
};

class Bomb3 {
public:
    Bomb3(uint8_t defuse) : m_defuse(defuse)
    {}

    void init();                                   // the init() FSM interface

    void onUP  ()                  { m_state->onUP  (this); }
    void onDOWN()                  { m_state->onDOWN(this); }
    void onARM ()                  { m_state->onARM (this); }
    void onTICK(uint8_t fine_time) { m_state->onTICK(this, fine_time); }

private:
    void tran(BombState const *target) { m_state = target; }

private:
    BombState const *m_state;                            // the state variable
    uint8_t m_timeout;                     // number of seconds till explosion
    uint8_t m_code;               // currently entered code to disarm the bomb
    uint8_t m_defuse;                 // secret defuse code to disarm the bomb

private:
    static SettingState const setting;
    static TimingState  const timing;

    friend class SettingState;
    friend class TimingState;
};

//............................................................................
                                           // the initial value of the timeout
#define INIT_TIMEOUT   10

SettingState const Bomb3::setting;
TimingState  const Bomb3::timing;

void Bomb3::init() {
    m_timeout = INIT_TIMEOUT;
    tran(&Bomb3::setting);
}
//............................................................................
void SettingState::onUP(Bomb3 *context) const {
    if (context->m_timeout < 60) {
        ++context->m_timeout;
        BSP_display(context->m_timeout);
    }
}
void SettingState::onDOWN(Bomb3 *context) const {
    if (context->m_timeout > 1) {
        --context->m_timeout;
        BSP_display(context->m_timeout);
    }
}
void SettingState::onARM(Bomb3 *context) const {
    context->m_code = 0;
    context->tran(&Bomb3::timing);                   // transition to "timing"
}

//............................................................................
void TimingState::onUP(Bomb3 *context) const {
    context->m_code <<= 1;
    context->m_code |= 1;
}
void TimingState::onDOWN(Bomb3 *context) const {
    context->m_code <<= 1;
}
void TimingState::onARM(Bomb3 *context) const {
    if (context->m_code == context->m_defuse) {
        context->tran(&Bomb3::setting);             // transition to "setting"
    }
}
void TimingState::onTICK(Bomb3 *context, uint8_t fine_time) const {
    if (fine_time == 0) {
        --context->m_timeout;
        BSP_display(context->m_timeout);
        if (context->m_timeout == 0) {
            BSP_boom();                                    // destroy the bomb
        }
    }
}

// Test harness --------------------------------------------------------------
#include <iostream.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

static Bomb3 bomb(0x0D);    // time bomb FSM, the secret defuse code, 1101 bin

//............................................................................
int main() {

    cout << "Time Bomb (GoF State-Pattern)" << endl
         << "Press 'u'   for UP   event" << endl
         << "Press 'd'   for DOWN event" << endl
         << "Press 'a'   for ARM  event" << endl
         << "Press <Esc> to quit."       << endl;

    bomb.init();                                // take the initial transition

    for (;;) {                                                   // event loop
        static int fine_time = 0;

        delay(100);                                            // 100 ms delay

        if (++fine_time == 10) {
            fine_time = 0;
        }
        cout.width(1);
        cout << "T(" << fine_time << ')' << ((fine_time == 0) ? '\n' : ' ');

        bomb.onTICK(fine_time);                         // dispatch TICK event

        if (kbhit()) {
            switch (getch()) {
                case 'u': {                                        // UP event
                    cout << endl << "UP  : ";
                    bomb.onUP();                          // dispatch UP event
                    break;
                }
                case 'd': {                                      // DOWN event
                    cout << endl << "DOWN: ";
                    bomb.onDOWN();                      // dispatch DOWN event
                    break;
                }
                case 'a': {                                       // ARM event
                    cout << endl << "ARM : ";
                    bomb.onARM();                        // dispatch ARM event
                    break;
                }
                case '\33': {                                     // <Esc> key
                    cout << endl << "ESC : Bye! Bye!";
                    _exit(0);
                    break;
                }
            }
        }
    }

    return 0;
}
