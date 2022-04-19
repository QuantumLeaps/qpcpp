//============================================================================
// Product: "Low-Power" example, dual-mode QXK kernel
// Last Updated for Version: 6.7.0
// Date of the Last Update:  2019-12-27
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
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "low_power.hpp"
#include "bsp.hpp"

//Q_DEFINE_THIS_FILE

// local objects -------------------------------------------------------------
static void XBlinky1_run(QP::QXThread * const me);

// global objects ------------------------------------------------------------
QP::QXThread XT_Blinky1(&XBlinky1_run, // thread routine
                        1U); // associate the thread with tick rate-1
QP::QXSemaphore XSEM_sw1;

//............................................................................
static void XBlinky1_run(QP::QXThread * const me) {
    (void)me; // unused parameter
    bool isActive = false;
    for (;;) {
        if (!isActive) {
            XSEM_sw1.wait();
            isActive = true;
        }
        else {
            BSP_tickRate1_on(); // turn on the tick rate-1 !
            for (uint8_t count = 13U; count > 0U; --count) {
                BSP_led1_on();
                QP::QXThread::delay(2U);
                BSP_led1_off();
                QP::QXThread::delay(2U);
            }
            isActive = false;
        }
    }
}
