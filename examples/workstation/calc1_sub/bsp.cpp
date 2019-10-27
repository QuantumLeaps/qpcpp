//****************************************************************************
// Product:  Board Support Package (BSP) for the Calculator example
// Last Updated for Version: 6.5.0
// Date of the Last Update:  2019-03-21
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// https://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.hpp"
#include "bsp.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace QP;
using namespace std;

#define DISP_WIDTH  9

static char l_display[DISP_WIDTH + 1]; // the calculator display
static int  l_len; // number of displayed characters

//............................................................................
void BSP_clear(void) {
    memset(l_display, ' ', DISP_WIDTH - 1);
    l_display[DISP_WIDTH - 1] = '0';
    l_display[DISP_WIDTH] = '\0';
    l_len = 0;
}
//............................................................................
void BSP_insert(int keyId) {
    if (l_len == 0) {
        l_display[DISP_WIDTH - 1] = (char)keyId;
        ++l_len;
    }
    else if (l_len < (DISP_WIDTH - 1)) {
        memmove(&l_display[0], &l_display[1], DISP_WIDTH - 1);
        l_display[DISP_WIDTH - 1] = (char)keyId;
        ++l_len;
    }
}
//............................................................................
void BSP_display(double value) {
    sprintf(l_display, "%9.6g", value);
}
//............................................................................
void BSP_display_error(char const *err) {
    strcpy(l_display, err);
}
//............................................................................
void BSP_negate(void) {
    BSP_clear();
    l_display[DISP_WIDTH - 2] = '-';
}
//............................................................................
void BSP_show_display(void) {
    cout << endl << '[' << l_display << "] ";
}
//............................................................................
void BSP_exit(void) {
    cout << endl << "Bye! Bye" << endl;
    QF::onCleanup();
    exit(0);
}
//............................................................................
double BSP_get_value(void) {
    return strtod(l_display, (char **)0);
}
//............................................................................
void BSP_message(char const *msg) {
    cout << msg;
}

namespace QP {
/*..........................................................................*/
void QF::onStartup(void) {
    QF_consoleSetup();
}
/*..........................................................................*/
void QF::onCleanup(void) {
    QF_consoleCleanup();
}
/*..........................................................................*/
void QF_onClockTick(void) {
}

} // namespace QP

//............................................................................
// this function is used by the QP embedded systems-friendly assertions
extern "C" void Q_onAssert(char const * const module, int loc) {
    cout << "Assertion failed in " << module << ':' << loc
         << flush << endl;
    QF::onCleanup();
    exit(-1);
}
