//============================================================================
// DPP example for FreeRTOS kernel
// Last updated for version 6.0.4
// Last updated on  2018-01-07
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

//............................................................................
int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static StackType_t philoStack[N_PHILO][configMINIMAL_STACK_SIZE];
    static StackType_t tableStack[configMINIMAL_STACK_SIZE];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO]; // small pool

    QP::QF::init();  // initialize the framework and the underlying RT kernel

    // init publish-subscribe
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize the Board Support Package
    // NOTE: BSP::init() is called *after* initializing publish-subscribe and
    // event pools, to make the system ready to accept SysTick interrupts.
    // Unfortunately, the STM32Cube code that must be called from BSP,
    // configures and starts SysTick.
    //
    DPP::BSP::init();

    // start the extended Test1 thread
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->setAttr(QP::TASK_NAME_ATTR, "Philo");
        DPP::AO_Philo[n]->start(
            n + 1U,                  // QP priority of the AO
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            philoStack[n],           // stack storage
            sizeof(philoStack[n]));  // stack size [bytes]
    }

    DPP::AO_Table->setAttr(QP::TASK_NAME_ATTR, "Table");
    DPP::AO_Table->start(
        N_PHILO + 1U,            // QP priority of the AO
        tableQueueSto,           // event queue storage
        Q_DIM(tableQueueSto),    // queue length [events]
        tableStack,              // stack storage
        sizeof(tableStack));     // stack size [bytes]

    return QP::QF::run(); // run the QF application
}

