//============================================================================
// DPP example for QXK
// Last updated for version 6.7.0
// Last updated on  2019-12-26
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "test.hpp"
#include "bsp.hpp"

//............................................................................
int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static QP::QEvt const *testQueueSto[5];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO]; // small pool

    // stack for the QXK extended thread
    static uint64_t testStackSto[64];

    QP::QF::init();  // initialize the framework
    DPP::BSP::init(); // initialize the BSP

    // initialize publish-subscribe...
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start(
            static_cast<uint_fast8_t>(n + 1), // QP priority
            philoQueueSto[n],          // event queue storage
            Q_DIM(philoQueueSto[n]),   // queue length [events]
            nullptr,    // no stack storage
            static_cast<uint_fast16_t>(0)); // stack size [bytes]
    }

    DPP::AO_Table->start(
            static_cast<uint_fast8_t>(N_PHILO + 2U), // QP priority
            tableQueueSto,           // event queue storage
            Q_DIM(tableQueueSto),    // queue length [events]
            nullptr,    // no stack storage
            static_cast<uint_fast16_t>(0)); // stack size [bytes]

    // start the extended thread for testing
    DPP::XT_Test->start(
            static_cast<uint_fast8_t>(N_PHILO + 3U), // QP priority
            testQueueSto,            // event queue storage
            Q_DIM(testQueueSto),     // queue length [events]
            testStackSto,            // stack storage
            sizeof(testStackSto));   // stack size [bytes]


    return QP::QF::run(); // run the QF application
}
