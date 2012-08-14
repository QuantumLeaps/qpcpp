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

#include <stdio.h>
#include <math.h>

#include <new.h>                                          // for placement new

// Local objects -------------------------------------------------------------
static QEvt const l_clear_evt = { C_SIG, 0 };

#define NUM_ENTRY_X      13
#define NUM_ENTRY_Y      13
#define NUM_ENTRY_COLOR  (Video::FGND_YELLOW)

//............................................................................
QState UI_num::handler(UI_num *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
                                    // send object dictionaries for UI objects
            QS_OBJ_DICTIONARY(&me->m_num_entry);

                                        // instantiate the state-local objects
            new(&me->m_num_entry) NumEntry();
                               // take the initial transition in the component
            me->m_num_entry.init();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_history = me->state();                 // store the history
                                        // destroy the state-local objects...
            me->m_num_entry.NumEntry::~NumEntry();
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&UI_num_sd::handler);
        }
        case HELP_SIG: {
            return Q_TRAN(&UI_help::handler);                   // Help screen
        }
    }
    return Q_SUPER(&UI_top::handler);
}
//............................................................................
QState UI_num_sd::handler(UI_num_sd *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            uint8_t c;
            static char const * const help_sd[] = {
                "Standard Deviation:         ",
                "Find the mean value and the ",
                " root-mean-square (RMS)     ",
                "deviation of n data samples ",
                "xi, where i = 1..n.         ",
                "Mean value <x> is calculated",
                "as follows:                 ",
                "<x> = Sum(xi)/n;            ",
                "Two RMS estimatators are    ",
                "provided:                   ",
                "sig(n) =                    ",
                "   sqrt(Sum(xi-<x>)**2 / n);",
                "sig(n-1) =                  ",
                "sqrt(Sum(xi-<x>)**2 / (n-1))"
            };
                                        // instantiate the state-local objects
            me->m_help_text = help_sd;
            me->m_help_len  = Q_DIM(help_sd);
            me->m_n      = 0.0;
            me->m_sum    = 0.0;
            me->m_sum_sq = 0.0;

            Video::printStrAt(2, 10, Video::FGND_BLACK,
                "Screen 1: Standard Deviation     ");
            Video::clearRect( 0, 11, 35, 23, Video::BGND_BLUE);
            Video::clearRect(35, 11, 80, 23, Video::BGND_BLACK);

            c = Video::FGND_LIGHT_GRAY;
            Video::printStrAt(36, 12, c,
                "Press '-'        to enter a negative number");
            Video::printStrAt(36, 13, c,
                "Press '0' .. '9' to enter a digit");
            Video::printStrAt(36, 14, c,
                "Press '.'        to enter the decimal point");
            Video::printStrAt(36, 15, c,
                "Press <Enter>    to enter the data sample");
            Video::printStrAt(36, 16, c,
                "Press 'e' or 'E' to Cancel last entry");
            Video::printStrAt(36, 17, c,
                "Press 'c' or 'C' to Cancel the data set");

            c = Video::FGND_WHITE;
            Video::printStrAt(36, 20, c,
                "Press UP-arrow   for previous screen");
            Video::printStrAt(36, 21, c,
                "Press DOWN-arrow for next screen");
            Video::printStrAt(36, 22, c,
                "Press F1         for help");

            Video::clearRect(NUM_ENTRY_X, NUM_ENTRY_Y,
                NUM_ENTRY_X + NUM_STR_WIDTH, NUM_ENTRY_Y + 1,
                Video::BGND_BLACK);
            Video::drawRect(NUM_ENTRY_X - 1, NUM_ENTRY_Y - 1,
                NUM_ENTRY_X + NUM_STR_WIDTH + 1, NUM_ENTRY_Y + 2,
                Video::FGND_WHITE, 2);

            me->m_num_entry.config(NUM_ENTRY_X, NUM_ENTRY_Y, NUM_ENTRY_COLOR);
            me->m_num_entry.dispatch(&l_clear_evt);

            c = Video::FGND_WHITE;                                   // labels
            Video::printStrAt(NUM_ENTRY_X - 1, NUM_ENTRY_Y + 4, c,
                 "n        =");
            Video::printStrAt(NUM_ENTRY_X - 1, NUM_ENTRY_Y + 5, c,
                 "<x>      =");
            Video::printStrAt(NUM_ENTRY_X - 1, NUM_ENTRY_Y + 6, c,
                "sig(n)   =");
            Video::printStrAt(NUM_ENTRY_X - 1, NUM_ENTRY_Y + 7, c,
                "sig(n-1) =");

            c = Video::FGND_YELLOW;                                  // values
            Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 4, c,
                "0           ");
            Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 5, c,
                "N/A         ");
            Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 6, c,
                "N/A         ");
            Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 7, c,
                "N/A         ");

            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
                                         // destroy the state-local objects...
            // noting to destroy
            return Q_HANDLED();
        }
        case C_SIG: {
            return Q_TRAN(&UI_num_sd::handler);          // transition-to-self
        }
        case CE_SIG: {
            me->m_num_entry.dispatch(&l_clear_evt);
            return Q_HANDLED();
        }
        case UP_SIG: {
            return Q_TRAN(&UI_num_lr::handler);             // previous Screen
        }
        case DOWN_SIG: {
            return Q_TRAN(&UI_num_lr::handler);                 // next Screen
        }
        case NEG_SIG:
        case DIGIT_0_SIG:
        case DIGIT_1_9_SIG:
        case POINT_SIG: {
            me->m_num_entry.dispatch(e);        // dispatch to the number comp
            return Q_HANDLED();
        }
        case ENTER_SIG: {
            double tmp = me->m_num_entry.get();
            char buf[14];

            me->m_n      += 1.0;
            me->m_sum    += tmp;
            me->m_sum_sq += tmp*tmp;

            sprintf(buf, "%-12.6g", me->m_n);
            Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 4,
                              Video::FGND_YELLOW, buf);

            tmp = me->m_sum / me->m_n;                                  // <x>
            sprintf(buf, "%-12.6g", tmp);
            Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 5,
                              Video::FGND_YELLOW, buf);

            tmp = me->m_sum_sq / me->m_n - tmp*tmp;
            if (tmp >= 0.0) {                                      // sigma(n)
                tmp = sqrt(tmp);
                sprintf(buf, "%-12.6g", tmp);
                Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 6,
                                  Video::FGND_YELLOW, buf);
                if (me->m_n > 1.0) {                             // sigma(n-1)
                     tmp *= sqrt(me->m_n/(me->m_n - 1.0));
                     sprintf(buf, "%-12.6g", tmp);
                     Video::printStrAt(NUM_ENTRY_X + 10, NUM_ENTRY_Y + 7,
                                       Video::FGND_YELLOW, buf);
                }
            }
            me->m_num_entry.dispatch(&l_clear_evt);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&UI_num::handler);
}
