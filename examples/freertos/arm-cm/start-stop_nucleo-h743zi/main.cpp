//============================================================================
// start-stop example for FreeRTOS kernel
// Last updated for version 6.6.0
// Last updated on  2019-07-30
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "worker.hpp"
#include "bsp.hpp"

//............................................................................
int main() {
    static QP::QEvt const *launcherQueueSto[10];
    static StackType_t launcherStackSto[configMINIMAL_STACK_SIZE];

    static QP::QSubscrList subscrSto[MAX_PUB_SIG];
    // event pools -- not used

    QP::QF::init();  // initialize the framework and the underlying RT kernel

    // init publish-subscribe
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    //QP::QF::poolInit(smlPoolSto,
    //                 sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize the Board Support Package
    // NOTE: BSP::init() is called *after* initializing publish-subscribe and
    // event pools, to make the system ready to accept SysTick interrupts.
    // Unfortunately, the STM32Cube code that must be called from BSP,
    // configures and starts SysTick.
    //
    BSP::init();

    AO_Launcher->setAttr(QP::TASK_NAME_ATTR, "Launcher");
    AO_Launcher->start(
        2U,                           // QP priority of the AO
        launcherQueueSto,             // event queue storage
        Q_DIM(launcherQueueSto),      // queue length [events]
        launcherStackSto,             // stack storage
        sizeof(launcherStackSto));    // stack size [bytes]

    return QP::QF::run(); // run the QF application
}

