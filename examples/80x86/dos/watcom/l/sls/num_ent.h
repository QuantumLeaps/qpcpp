//////////////////////////////////////////////////////////////////////////////
// Product: Number data entry
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
#ifndef num_ent_h
#define num_ent_h

using namespace QP;

#define NUM_STR_WIDTH  13

class NumEntry : public QHsm {
    char    m_str[NUM_STR_WIDTH + 1];   // string representation of the number
    uint8_t m_len;                           // number of displayed characters
    uint8_t m_x;             // x-coordinate of the number entry on the screen
    uint8_t m_y;             // y-coordinate of the number entry on the screen
    uint8_t m_color;     // foreground color of the number entry on the screen

public:
    NumEntry(void);                                                    // ctor
    ~NumEntry();                                                       // xtor
    void config(uint8_t x, uint8_t y, uint8_t color);
    void insert(uint8_t keyId);
    double get(void);      // return the numerical value from the number entry

private:
    static QState initial (NumEntry *me, QEvt const *e);
    static QState top     (NumEntry *me, QEvt const *e);
    static QState negative(NumEntry *me, QEvt const *e);
    static QState zero    (NumEntry *me, QEvt const *e);
    static QState integer (NumEntry *me, QEvt const *e);
    static QState fraction(NumEntry *me, QEvt const *e);
};

#endif                                                            // num_ent_h
