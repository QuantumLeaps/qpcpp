//============================================================================
// Product:  Calculator Example
// Last Updated for Version: 6.9.3
// Date of the Last Update:  2021-03-18
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2021 Quantum Leaps, LLC. All rights reserved.
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
#include "qpcpp.hpp" // QP API
#include "bsp.hpp"   // board support package
#include "calc1.hpp" // application

#include <iostream>

using namespace QP;
using namespace std;

//............................................................................
int main() {

    QF::init();
    QF::onStartup();

    cout << "Calculator example, QEP version: "
         << QP_VERSION_STR << endl
         << "Press '0' .. '9'     to enter a digit\n"
            "Press '.'            to enter the decimal point\n"
            "Press '+' or '#'     to add\n"
            "Press '-'            to subtract or negate a number\n"
            "Press '*'            to multiply\n"
            "Press '/'            to divide\n"
            "Press '=' or <Enter> to get the result\n"
            "Press 'c' or 'C'     to Cancel\n"
            "Press 'e' or 'E'     to Cancel Entry\n"
            "Press <Esc>          to quit.\n\n";

    the_calc->init(0U); // trigger initial transition

    for (;;) { // event loop
        CalcEvt e; // Calculator event

        BSP_show_display(); // show the display

        cout << ": ";

        e.key_code = static_cast<std::uint8_t>(QF::consoleWaitForKey());
        cout << static_cast<char>(e.key_code) << ' ';

        switch (e.key_code) {
            case 'c': // intentionally fall through
            case 'C': {
                static_cast<QP::QEvt *>(&e)->sig = C_SIG;
                break;
            }
            case 'e': // intentionally fall through
            case 'E': {
                static_cast<QP::QEvt *>(&e)->sig = CE_SIG;
                break;
            }
            case '0': {
                static_cast<QP::QEvt *>(&e)->sig = DIGIT_0_SIG;
                break;
            }
            case '1': // intentionally fall through
            case '2': // intentionally fall through
            case '3': // intentionally fall through
            case '4': // intentionally fall through
            case '5': // intentionally fall through
            case '6': // intentionally fall through
            case '7': // intentionally fall through
            case '8': // intentionally fall through
            case '9': {
                static_cast<QP::QEvt *>(&e)->sig = DIGIT_1_9_SIG;
                break;
            }
            case '.': {
                static_cast<QP::QEvt *>(&e)->sig = POINT_SIG;
                break;
            }
            case '+': // intentionally fall through
            case '-': // intentionally fall through
            case '*': // intentionally fall through
            case '/': {
                static_cast<QP::QEvt *>(&e)->sig = OPER_SIG;
                break;
            }
            case '#': { // alternative '+'
                static_cast<QP::QEvt *>(&e)->sig = OPER_SIG;
                e.key_code = '+';
                break;
            }
            case '=': // intentionally fall through
            case '\r': { // Enter key
                static_cast<QP::QEvt *>(&e)->sig = EQUALS_SIG;
                break;
            }
            case '\33': { // ESC key
                static_cast<QP::QEvt *>(&e)->sig = OFF_SIG;
                break;
            }
            default: {
                static_cast<QP::QEvt *>(&e)->sig = 0; // invalid event
                break;
            }
        }

        if (static_cast<QP::QEvt *>(&e)->sig != 0) { // valid event generated?
            the_calc->dispatch(&e, 0U); // dispatch event
        }
    }

    QF::onCleanup();
    return 0;
}
