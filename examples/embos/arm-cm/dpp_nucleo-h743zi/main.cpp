//============================================================================
// DPP example, embOS kernel
// Last updated for version 6.9.3
// Last updated on  2021-04-09
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

//............................................................................
int main() {
    // stacks
    static OS_STACKPTR int tableStack[128];
    static OS_STACKPTR int philoStack[N_PHILO][128];

    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];

    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO];


    QP::QF::init();  // initialize the framework and the underlying RT kernel

    DPP::BSP::init(); // initialize the BSP

    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->setAttr(QP::TASK_NAME_ATTR, "Philo");
        DPP::AO_Philo[n]->start(
            static_cast<uint_fast8_t>(n + 1U), // QP priority of the AO
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            philoStack[n],           // stack storage
            sizeof(philoStack[n]));  // stack size [bytes]
    }

    // set the embOS task attributes BEFORE starting the AO
    DPP::AO_Table->setAttr(QP::TASK_USES_FPU, 0);
    DPP::AO_Table->setAttr(QP::TASK_NAME_ATTR, "Table");
    DPP::AO_Table->start(
        static_cast<uint_fast8_t>(N_PHILO + 1U), // QP priority of the AO
        tableQueueSto,         // event queue storage
        Q_DIM(tableQueueSto),  // queue length [events]
        tableStack,            // stack storage
        sizeof(tableStack));   // stack size [bytes]

    return QP::QF::run(); // run the QF application
}
