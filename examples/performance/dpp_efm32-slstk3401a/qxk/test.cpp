//****************************************************************************
// DPP example for QXK
// Last updated for version 5.8.0
// Last updated on  2016-11-30
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
// http://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "test.h"
#include "bsp.h"

namespace DPP {

//............................................................................
static void thread_function(QP::QXThread * const me) {
    (void)me; // unused parameter
    XT_Sema.init(1U); // 1 count
    for (;;) {
        (void)XT_Sema.wait(0U, 0U); // wait forever
        BSP::ledOn();
        QP::QXThread::delay(1, 0U);  // 1 cycle
        BSP::ledOff();
    }
}

// local extended thread object ..............................................
static QP::QXThread l_test(&thread_function, 0U);

// global pointer to the test thread .........................................
QP::QXThread * const XT_Test = &l_test;
QP::QXSemaphore XT_Sema;

} // namespace DPP
