//////////////////////////////////////////////////////////////////////////////
// Product: UI with State-Local Storage Example
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 15, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#ifndef ui_h
#define ui_h

using namespace QP;

enum UserSignals {
    QUIT_SIG = Q_USER_SIG,

    // insert other published signals here ...
    MAX_PUB_SIG,                                  // the last published signal

    C_SIG,
    CE_SIG,
    DIGIT_0_SIG,
    DIGIT_1_9_SIG,
    POINT_SIG,
    NEG_SIG,
    ENTER_SIG,
    UP_SIG,
    DOWN_SIG,
    HELP_SIG,

    MAX_SIG                              // the last signal (keep always last)
};

struct KeyboardEvt : public QEvt {
    uint8_t key_code;                                       // code of the key
};

//............................................................................
class UI_top : public QActive {
public:
    QStateHandler m_history;
    char const * const *m_help_text;
    uint16_t m_help_len;

public:
    UI_top(void) : QActive((QStateHandler)&UI_top::initial) {
    }

public:
    static QState initial(UI_top *me, QEvt const *e); // initial pseudostate
    static QState handler(UI_top *me, QEvt const *e);
    static QState final  (UI_top *me, QEvt const *e);
};
//............................................................................
class UI_mem : public UI_top {
private:
    uint8_t m_mem[100];   // maximum size of any substate (subclass) of UI_top
};
//............................................................................
class UI_num : public UI_top {
public:
    NumEntry m_num_entry;

public:
    static QState handler(UI_num *me, QEvt const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_num) < sizeof(UI_mem));

//............................................................................
class UI_num_sd : public UI_num {                        // standard deviation
public:
    double m_n;
    double m_sum;
    double m_sum_sq;

public:
    static QState handler(UI_num_sd *me, QEvt const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_num_sd) < sizeof(UI_mem));

//............................................................................
class UI_num_lr : public UI_num {                         // linear regression
public:
    double m_x;
    double m_n;
    double m_xsum;
    double m_xsum_sq;
    double m_ysum;
    double m_ysum_sq;
    double m_xysum;

public:
    static QState handler(UI_num_lr *me, QEvt const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_num_lr) < sizeof(UI_mem));

//............................................................................
class UI_help : public UI_top {
public:
    uint16_t m_help_line;

public:
    static QState handler(UI_help *me, QEvt const *e);
};
Q_ASSERT_COMPILE(sizeof(UI_help) < sizeof(UI_mem));

//----------------------------------------------------------------------------
extern QActive * const AO_UI;          // "opaque" pointer to UI Active Object

#endif
