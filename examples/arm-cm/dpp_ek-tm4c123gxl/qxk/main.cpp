//****************************************************************************
// DPP example for QXK
// Last updated for version 5.6.2
// Last updated on  2016-03-31
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
#include "bsp.h"

//............................................................................
int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static uint64_t tableStackSto[64];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static uint64_t philoStackSto[N_PHILO][64];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO]; // small pool

    // stack for the "naked" test thread
    static QP::QEvt const *testQueueSto[5];
    static uint64_t testStackSto[64];

    // stack for the QXK's idle thread
    static uint64_t idleStackSto[32];


    QP::QF::init();  // initialize the framework and the underlying RT kernel
    QP::QXK::init(idleStackSto, sizeof(idleStackSto)); // initialize QXK
    DPP::BSP::init(); // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(tableQueueSto);
    QS_OBJ_DICTIONARY(philoQueueSto[0]);
    QS_OBJ_DICTIONARY(philoQueueSto[1]);
    QS_OBJ_DICTIONARY(philoQueueSto[2]);
    QS_OBJ_DICTIONARY(philoQueueSto[3]);
    QS_OBJ_DICTIONARY(philoQueueSto[4]);

    QP::QF::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start(
            static_cast<uint_fast8_t>(n + 1), // QP priority of the AO
            philoQueueSto[n],          // event queue storage
            Q_DIM(philoQueueSto[n]),   // queue length [events]
            philoStackSto[n],          // stack storage
            sizeof(philoStackSto[n]),  // stack size [bytes]
            static_cast<QP::QEvt *>(0));   // initialization event
    }

    DPP::AO_Table->start(
            static_cast<uint_fast8_t>(N_PHILO + 1), // QP priority of the AO
            tableQueueSto,           // event queue storage
            Q_DIM(tableQueueSto),    // queue length [events]
            tableStackSto,           // stack storage
            sizeof(tableStackSto),   // stack size [bytes]
            static_cast<QP::QEvt *>(0)); // initialization event

    // start the "naked" test thread
    DPP::XT_Test->start(
            static_cast<uint_fast8_t>(10), // QP priority of the AO
            testQueueSto,            // event queue storage
            Q_DIM(testQueueSto),     // queue length [events]
            testStackSto,            // stack storage
            sizeof(testStackSto),    // stack size [bytes]
            static_cast<QP::QEvt *>(0)); // initialization event

    return QP::QF::run(); // run the QF application
}
