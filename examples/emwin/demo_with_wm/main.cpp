//============================================================================
// Product: main task for emWin/uC/GUI, Win32 simulation
// Last updated for version 7.3.0
// Last updated on  2023-07-20
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
#include "bsp.hpp"
#include "dpp.hpp"

// GUI includes
Q_DEFINE_THIS_FILE

//............................................................................
extern "C" void MainTask(void) {
    QF::init(); // initialize the framework and the underlying RT kernel
    BSP_init(); // initialize the BSP

    // initialize event pools...
    static union SmallEvents {
        void* min_size;
        TableEvt te;
        MouseEvt me;
        // other event types to go into this pool
    } l_smlPoolSto[2 * N_PHILO]; // storage for the small event pool
    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

    // initialize publish-subscribe
    static QSubscrList l_subscrSto[MAX_PUB_SIG];
    QActive::psInit(l_subscrSto, Q_DIM(l_subscrSto));

    // start the active objects...
    static QEvt const *l_philoQueueSto[N_PHILO][10];
    for (uint8_t n = 0; n < N_PHILO; ++n) {
        AO_Philo[n]->start(n + 1U,
                           l_philoQueueSto[n], Q_DIM(l_philoQueueSto[n]),
                           nullptr, 1024, nullptr);
    }

    static QEvt const *l_tableQueueSto[N_PHILO];
    AO_Table->start(N_PHILO + 1U,
                    l_tableQueueSto, Q_DIM(l_tableQueueSto),
                    nullptr, 1024, nullptr);

    QF::run(); // run the QF application
}

