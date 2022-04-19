//============================================================================
// Product: DPP example, uC/OS-II kernel
// Last Updated for Version: 5.6.5
// Date of the Last Update:  2016-07-02
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
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// https://state-machine.com
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "test.h"
#include "bsp.hpp"
#include "assert.h"

Q_DEFINE_THIS_FILE

extern "C" {

OS_EVENT *Sema;  // uC/OS-II semaphore for testing

//............................................................................
void test_thread(void *pdata) { // uC/OS-II task signature
    (void)pdata;
    Sema = OSSemCreate(1);
    Q_ASSERT(Sema != static_cast<OS_EVENT *>(0)); // semaphore must be created
    for (;;) {
        INT8U err;
        OSSemPend(Sema, 0, &err); // wait forever
        DPP::BSP::ledOn();
        OSTimeDly(1);  // 1 clock cycle
        DPP::BSP::ledOff();
    }
}

} // extern "C"
