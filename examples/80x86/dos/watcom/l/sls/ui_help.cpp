//////////////////////////////////////////////////////////////////////////////
// Product: UI with State-Local Storage Example
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

#define HELP_X      3
#define HELP_Y      14
#define HELP_DX     28
#define HELP_DY     5

//............................................................................
static void printHelp(char const * const *txt) {
    uint8_t y;
    for (y = 0; y < HELP_DY; ++y) {
        Video::printStrAt(HELP_X, HELP_Y + y, Video::FGND_YELLOW, txt[y]);
    }
}

//............................................................................
QState UI_help::handler(UI_help *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
                                        // instantiate the state-local objects
            Video::printStrAt(2, 10, Video::FGND_BLACK,
                "Screen 0: Help                   ");
            Video::clearRect( 0, 11, 35, 23, Video::BGND_BLUE);
            Video::clearRect(35, 11, 80, 23, Video::BGND_BLACK);

            Video::printStrAt(36, 12, Video::FGND_LIGHT_GRAY,
                "Press DOWN-Arrow to scroll down");
            Video::printStrAt(36, 13, Video::FGND_LIGHT_GRAY,
                "Press UP-Arrow   to scroll up");

            Video::printStrAt(36, 20, Video::FGND_WHITE,
                "Press F1         to return to last screen");

            Video::clearRect(HELP_X - 1, HELP_Y,
                HELP_X + HELP_DX + 1, HELP_Y + HELP_DY, Video::BGND_BLACK);
            Video::drawRect (HELP_X - 2, HELP_Y - 1,
                HELP_X + HELP_DX + 2, HELP_Y + HELP_DY + 1,
                Video::FGND_WHITE,2);

            me->m_help_line = 0;
            printHelp(me->m_help_text + me->m_help_line);

            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
                                         // destroy the state-local objects...
            return Q_HANDLED();
        }
        case DOWN_SIG: {
            if (me->m_help_line + HELP_DY < me->m_help_len) {
                ++me->m_help_line;
                printHelp(me->m_help_text + me->m_help_line);
            }
            return Q_HANDLED();
        }
        case UP_SIG: {
            if (me->m_help_line > 0) {
                --me->m_help_line;
                printHelp(me->m_help_text + me->m_help_line);
            }
            return Q_HANDLED();
        }
        case HELP_SIG: {
            return Q_TRAN(me->m_history);        // go back to the last screen
        }
    }
    return Q_SUPER(&UI_top::handler);
}
