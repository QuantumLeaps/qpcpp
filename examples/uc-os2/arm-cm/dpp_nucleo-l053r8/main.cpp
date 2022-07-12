//============================================================================
// DPP example, uC/OS-II kernel
// Last updated for version 6.9.3
// Last updated on  2021-04-08
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
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

//............................................................................
int main() {
    // stacks for all uC/OS-II threads (grouped toghether for ease of testing)
    static OS_STK philoStkSto[N_PHILO][100];
    static OS_STK tableStkSto[128];

    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];

    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO];

    QP::QF::init();  // initialize the framework and the underlying RT kernel
    DPP::BSP::init(); // initialize the BSP

    // initialize publish-subscribe...
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    // CAUTION:
    // Make sure to configure **SUFFICIENT** number of uC/OS-II tasks and
    // message queues in "os_cfg.h" for all your active objects!!!
    //
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        // NOTE: provide uC/OS-II task attributes for the AO's task
        DPP::AO_Philo[n]->setAttr(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK, 0);
        DPP::AO_Philo[n]->setAttr(QP::TASK_NAME_ATTR, "Philo");
        DPP::AO_Philo[n]->start(
            n + 1U,                  // QP priority
            philoQueueSto[n],        // storage for the AO's queue
            Q_DIM(philoQueueSto[n]), // queue's length [events]
            philoStkSto[n],          // stack storage
            sizeof(philoStkSto[n])); // stack size [bytes]
    }

    // NOTE: provide uC/OS-II task attributes for the AO's task
    DPP::AO_Table->setAttr(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK, 0);
    DPP::AO_Table->setAttr(QP::TASK_NAME_ATTR, "Table");
    DPP::AO_Table->start(
        N_PHILO + 1U,            // QP priority
        tableQueueSto,           // storage for the AO's queue
        Q_DIM(tableQueueSto),    // queue's length [events]
        tableStkSto,             // stack storage
        sizeof(tableStkSto));    // stack size [bytes]

    return QP::QF::run(); // run the QF application
}

