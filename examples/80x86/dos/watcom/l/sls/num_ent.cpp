//////////////////////////////////////////////////////////////////////////////
// Product: Number data entry
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 20, 2012
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
#include "qp_port.h"                           // the port of the QP framework
#include "num_ent.h"
#include "ui.h"
#include "video.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//............................................................................
NumEntry::NumEntry(void)                                               // ctor
    : QHsm((QStateHandler)&NumEntry::initial)
{}
//............................................................................
NumEntry::~NumEntry() {                                                // xtor
}
//............................................................................
void NumEntry::config(uint8_t x, uint8_t y, uint8_t color) {
    m_x     = x;
    m_y     = y;
    m_color = color;
}
//............................................................................
void NumEntry::insert(uint8_t keyId) {
    if (m_len == 0) {
        m_str[NUM_STR_WIDTH - 1] = (char)keyId;
    }
    else if (m_len < (NUM_STR_WIDTH - 1)) {
        memmove(&m_str[0], &m_str[1], NUM_STR_WIDTH - 1);
        m_str[NUM_STR_WIDTH - 1] = (char)keyId;
    }
    ++m_len;
    Video::printStrAt(m_x, m_y, m_color, m_str);
}
//............................................................................
double NumEntry::get(void) {
    return strtod(m_str, (char **)0);
}

// HSM definition ------------------------------------------------------------
QState NumEntry::initial(NumEntry *me, QEvt const * /* e */) {
                         // send functions dictionaries for NumEntry states...
    QS_FUN_DICTIONARY(&NumEntry::top);
    QS_FUN_DICTIONARY(&NumEntry::negative);
    QS_FUN_DICTIONARY(&NumEntry::zero);
    QS_FUN_DICTIONARY(&NumEntry::integer);
    QS_FUN_DICTIONARY(&NumEntry::fraction);

               // send signal dictionaries for signals specific to NumEntry...
    QS_SIG_DICTIONARY(C_SIG,         me);
    QS_SIG_DICTIONARY(DIGIT_0_SIG,   me);
    QS_SIG_DICTIONARY(DIGIT_1_9_SIG, me);
    QS_SIG_DICTIONARY(POINT_SIG,     me);
    QS_SIG_DICTIONARY(NEG_SIG,       me);


    return Q_TRAN(&NumEntry::zero);
}
//............................................................................
QState NumEntry::top(NumEntry *me, QEvt const *e) {
    switch (e->sig) {
        case C_SIG: {
            memset(me->m_str, ' ', NUM_STR_WIDTH - 1);
            me->m_str[NUM_STR_WIDTH - 1] = '0';
            me->m_str[NUM_STR_WIDTH] = '\0';
            me->m_len = 0;
            Video::printStrAt(me->m_x, me->m_y, me->m_color, me->m_str);
            return Q_TRAN(&NumEntry::zero);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState NumEntry::zero(NumEntry *me, QEvt const *e) {
    switch (e->sig) {
        case DIGIT_0_SIG: {
            ;                                             // explicitly ignore
            return Q_HANDLED();
        }
        case NEG_SIG: {
            me->m_str[NUM_STR_WIDTH - 2] = '-';
            Video::printStrAt(me->m_x, me->m_y, me->m_color, me->m_str);
            return Q_TRAN(&NumEntry::negative);
        }
        case DIGIT_1_9_SIG: {
            me->insert(((KeyboardEvt const *)e)->key_code);
            return Q_TRAN(&NumEntry::integer);
        }
        case POINT_SIG: {
            me->insert(((KeyboardEvt const *)e)->key_code);
            return Q_TRAN(&NumEntry::fraction);
        }
    }
    return Q_SUPER(&NumEntry::top);
}
//............................................................................
QState NumEntry::negative(NumEntry *me, QEvt const *e) {
    switch (e->sig) {
        case NEG_SIG:
        case DIGIT_0_SIG: {
            ;                                             // explicitly ignore
            return Q_HANDLED();
        }
        case DIGIT_1_9_SIG: {
            me->insert(((KeyboardEvt const *)e)->key_code);
            return Q_TRAN(&NumEntry::integer);
        }
        case POINT_SIG: {
            me->insert(((KeyboardEvt const *)e)->key_code);
            return Q_TRAN(&NumEntry::fraction);
        }
    }
    return Q_SUPER(&NumEntry::top);
}
//............................................................................
QState NumEntry::integer(NumEntry *me, QEvt const *e) {
    switch (e->sig) {
        case DIGIT_0_SIG:                        // intentionally fall through
        case DIGIT_1_9_SIG: {
            me->insert(((KeyboardEvt const *)e)->key_code);
            return Q_HANDLED();
        }
        case POINT_SIG: {
            me->insert(((KeyboardEvt const *)e)->key_code);
            return Q_TRAN(&NumEntry::fraction);
        }
    }
    return Q_SUPER(&NumEntry::top);
}
//............................................................................
QState NumEntry::fraction(NumEntry *me, QEvt const *e) {
    switch (e->sig) {
        case DIGIT_0_SIG:                        // intentionally fall through
        case DIGIT_1_9_SIG: {
            me->insert(((KeyboardEvt const *)e)->key_code);
            return Q_HANDLED();
        }
        case POINT_SIG: {
            ;                                             // explicitly ignore
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&NumEntry::top);
}

