//============================================================================
// DPP example, uC/OS-II kernel
// Last updated for version 6.1.0
// Last updated on  2018-02-11
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
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
#include "test.h"
#include "bsp.hpp"
#include "assert.h"

Q_DEFINE_THIS_FILE

//............................................................................
int main() {
    // stacks for all uC/OS-II threads (grouped toghether for ease of testing)
    static OS_STK philoStackSto[N_PHILO][128];
    static OS_STK tableStackSto[256];
    static OS_STK testStackSto[128];

    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
    static DPP::TableEvt smlPoolSto[2*N_PHILO]; // small pool

    QP::QF::init();  // initialize the framework and the underlying RT kernel
    DPP::BSP::init(); // initialize the BSP

    // initialize publish-subscribe...
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        // NOTE: provide uC/OS-II task attributes for the AO's task
        DPP::AO_Philo[n]->setAttr(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK, 0);
        DPP::AO_Philo[n]->start(
            static_cast<uint_fast8_t>(n + 1), // QP priority
            philoQueueSto[n],          // event queue storage
            Q_DIM(philoQueueSto[n]),   // queue length [events]
            philoStackSto[n],          // stack storage
            sizeof(philoStackSto[n])); // stack size [bytes]
    }

    // NOTE: leave QF priority (N_PHILO + 1U) for the mutex to
    // protect the random number generator shared among Philo AOs

    // NOTE: provide uC/OS-II task attributes for the AO's task
    DPP::AO_Table->setAttr(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK, 0);
    DPP::AO_Table->start(
        static_cast<uint_fast8_t>(N_PHILO + 2U), // QP priority
        tableQueueSto,           // event queue storage
        Q_DIM(tableQueueSto),    // queue length [events]
        tableStackSto,           // stack storage
        sizeof(tableStackSto));  // stack size [bytes]

    // start a "naked" uC/OS-II task for testing...
    Q_ALLEGE(OS_ERR_NONE == OSTaskCreateExt(
        &test_thread, // the test thread function
        nullptr,    // the 'pdata' parameter
        &testStackSto[(sizeof(testStackSto)/sizeof(OS_STK)) - 1],
        static_cast<INT8U>(QF_MAX_ACTIVE - (N_PHILO + 3U)), // uC/OS-II prio
        static_cast<INT16U>(N_PHILO + 2U),       // the unique task id
        static_cast<OS_STK *>(&testStackSto[0]), // pbos
        static_cast<INT32U>(sizeof(testStackSto)/sizeof(OS_STK)),
        nullptr,                  // pext
        static_cast<INT16U>(OS_TASK_OPT_STK_CLR))); // task attributes

    return QP::QF::run(); // run the QF application
}
