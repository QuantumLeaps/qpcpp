//============================================================================
// Product: DPP example for ThreadX
// Last updated for: @ref qpcpp_7_3_0
// Last updated on  2023-08-22
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
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
#include "dpp.hpp"
#include "bsp.hpp"

//............................................................................
int main() {
    tx_kernel_enter(); // transfer control to the ThreadX RTOS
    return 0; // tx_kernel_enter() does not return
}
//............................................................................
void tx_application_define(void * /*first_unused_memory*/) {
    QP::QF::init(); // initialize the framework
    BSP::init();    // initialize the Board Support Package

    // init publish-subscribe
    static QP::QSubscrList l_subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(l_subscrSto, Q_DIM(l_subscrSto));

    // init event pools...
    static QF_MPOOL_EL(APP::TableEvt) smlPoolSto[2*APP::N_PHILO];
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...

    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10];
    static ULONG philoStk[APP::N_PHILO][200]; // stacks for the Philos
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
        APP::AO_Philo[n]->setAttr(QP::THREAD_NAME_ATTR, "Philo");
        APP::AO_Philo[n]->start(
            n + 1U,
            philoQueueSto[n], Q_DIM(philoQueueSto[n]),
            philoStk[n], sizeof(philoStk[n]));
    }

    static QP::QEvt const *tableQueueSto[APP::N_PHILO];
    static ULONG tableStk[200]; // stack for the Table
    APP::AO_Table->setAttr(QP::THREAD_NAME_ATTR, "Table");
    APP::AO_Table->start(
        APP::N_PHILO + 1U,
        tableQueueSto, Q_DIM(tableQueueSto),
        tableStk, sizeof(tableStk));

    (void)QP::QF::run(); // run the QF application
}
