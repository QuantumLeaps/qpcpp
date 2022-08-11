//============================================================================
// Product: "Orthogonal Component" state pattern example
// Last Updated for Version: 6.3.6
// Date of the Last Update:  2018-10-14
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
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
#include "bsp.hpp"
#include "alarm.hpp"
#include "clock.hpp"

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities

//............................................................................
int main(int argc, char *argv[]) {
    static QEvt const *alarmClockQSto[10]; // queue storage for AlarmClock
    static QF_MPOOL_EL(TimeEvt) smlPoolSto[10]; // storage for small pool


    PRINTF_S("Orthogonal Component pattern\nQP version: %s\n"
           "Press 'o' to turn the Alarm ON\n"
           "Press 'f' to turn the Alarm OFF\n"
           "Press '0'..'9' to set the Alarm time\n"
           "Press 'a' to set the Clock in 12-hour mode\n"
           "Press 'b' to set the Clock in 24-hour mode\n"
           "Press ESC to quit...\n",
           QP_VERSION_STR);

    BSP_init(argc, argv); // initialize the BSP
    QF::init(); // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QActive::psInit()

    // initialize event pools...
    QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    APP_alarmClock->start(1U,
                  alarmClockQSto, Q_DIM(alarmClockQSto),
                  nullptr, 0);

    return QF::run(); // run the QF application
}
