//////////////////////////////////////////////////////////////////////////////
// Product: DPP example for ThreadX
// Last updated for version 5.6.2
// Last updated on  2016-03-10
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
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

//............................................................................
int main() {
    DPP::BSP::init();    // initialize the Board Support Package
    tx_kernel_enter();  // transfet control to the ThreadX RTOS
    return 0;           // tx_kernel_enter() does not return
}
//............................................................................
void tx_application_define(void *first_unused_memory) {
    // stacks...
    static ULONG philoStk[N_PHILO][128];
    static ULONG tableStk[256];

    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];

    static union SmallEvents {
        void   *e0;  // minimum event size
        uint8_t e1[sizeof(DPP::TableEvt)];
        // ... other event types to go into this pool
    } smlPoolSto[2*N_PHILO + 10]; // storage for the small event pool

    // initialize the framework and the underlying RT kernel...
    QP::QF::init();

    // init publish-subscribe
    QP::QF::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(tableQueueSto);
    QS_OBJ_DICTIONARY(philoQueueSto[0]);
    //QS_OBJ_DICTIONARY(philoQueueSto[1]);
    //QS_OBJ_DICTIONARY(philoQueueSto[2]);
    //QS_OBJ_DICTIONARY(philoQueueSto[3]);
    //QS_OBJ_DICTIONARY(philoQueueSto[4]);

    // start the active objects...
    for (uint8_t n = 0; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start(
            static_cast<uint_fast8_t>(n + 1),
            philoQueueSto[n], Q_DIM(philoQueueSto[n]),
            philoStk[n], sizeof(philoStk[n]));
    }
    DPP::AO_Table->start(
        static_cast<uint_fast8_t>(N_PHILO + 1),
        tableQueueSto, Q_DIM(tableQueueSto),
        tableStk, sizeof(tableStk));

    (void)QP::QF::run(); // run the QF application
}
