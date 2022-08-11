//============================================================================
// DPP example for QXK
// Last updated for version 6.3.3
// Last updated on  2018-06-23
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

static QP::QTicker l_ticker0(0); // ticker for tick rate 0
QP::QActive *DPP::the_Ticker0 = &l_ticker0;

//............................................................................
int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO]; // small pool

    // stacks and queues for the extended test threads
    static QP::QEvt const *test1QueueSto[5];
    static uint64_t test1StackSto[64];
    static QP::QEvt const *test2QueueSto[5];
    static uint64_t test2StackSto[64];

    QP::QF::init();  // initialize the framework and the underlying RT kernel
    DPP::BSP::init(); // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(&l_ticker0);

    // init publish-subscribe
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the extended Test1 thread
    DPP::XT_Test1->start(
            1U,                      // QP prio of the thread
            test1QueueSto,           // event queue storage
            Q_DIM(test1QueueSto),    // queue length [events]
            test1StackSto,           // stack storage
            sizeof(test1StackSto));  // stack size

    // NOTE: leave priority 2 free for a mutex

    // start the Philo active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start(
            (n + 3U),                // QP priority of the AO
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            nullptr, 0U);            // no stack storage
    }

    // example of prioritizing the Ticker0 active object
    DPP::the_Ticker0->start(N_PHILO + 3U, // priority
                            0, 0, 0, 0);

    // NOTE: leave priority (N_PHILO + 4) free for mutex

    // start the extended Test2 thread
    DPP::XT_Test2->start(
            N_PHILO + 5U,            // QP prio of the thread
            test2QueueSto,           // event queue storage
            Q_DIM(test2QueueSto),    // queue length [events]
            test2StackSto,           // stack storage
            sizeof(test2StackSto));  // stack size [bytes]

    // NOTE: leave priority (N_PHILO + 6) free for mutex

    DPP::AO_Table->start(
            N_PHILO + 7U,            // QP priority of the AO
            tableQueueSto,           // event queue storage
            Q_DIM(tableQueueSto),    // queue length [events]
            nullptr, 0U);            // no stack storage

    return QP::QF::run(); // run the QF application
}
