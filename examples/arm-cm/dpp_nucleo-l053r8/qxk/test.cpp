//****************************************************************************
// DPP example for QXK
// Last updated for version 5.6.0
// Last updated on  2015-12-28
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, www.state-machine.com.
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
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

namespace DPP {

//............................................................................
static void thread_function(void *par) {
    (void)par;

    for (;;) {

        (void)QP::QXThread::queueGet(BSP::TICKS_PER_SEC*2U, 0U);
        BSP::ledOn();

        QP::QXThread::delay(BSP::TICKS_PER_SEC/4U, 0U);
        BSP::ledOff();

        QP::QXThread::delay(BSP::TICKS_PER_SEC*3U/4U, 0U);
    }
}

// local "naked" thread object ...............................................
static QP::QXThread l_test(&thread_function, 0U);

// global pointer to the test thread .........................................
QP::QXThread * const XT_Test = &l_test;

} // namespace DPP
