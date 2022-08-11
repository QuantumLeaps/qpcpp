//============================================================================
// DPP example
// Last updated for: @qpcpp_7_0_0
// Last updated on  2022-02-28
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
namespace {

static QP::QEvt const *tableQueueSto[10];
K_THREAD_STACK_DEFINE(tableStack, 1024); /* stack storage */

static QP::QEvt const *philoQueueSto[N_PHILO][10];
K_THREAD_STACK_DEFINE(philoStack[N_PHILO], 512); /* stack storage */

static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];
static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO]; /* small pool */

}

//............................................................................
int main() {
    QP::QF::init();   // initialize the framework and the underlying RT kernel
    DPP::BSP::init(); // initialize the Board Support Package

    // init publish-subscribe
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->setAttr(0,        // thread opions
                                  "Philo"); // thread name
        DPP::AO_Philo[n]->start((uint_fast8_t)(n + 1U), // priority
            philoQueueSto[n],
            Q_DIM(philoQueueSto[n]),
            (void *)philoStack[n], /* stack storage */
            K_THREAD_STACK_SIZEOF(philoStack[n]), /* stack size [bytes] */
            nullptr);         // no initialization param
    }

    DPP::AO_Table->setAttr(0,        // thread opions
                           "Table"); // thread name
    DPP::AO_Table->start((uint_fast8_t)(N_PHILO + 1U), // priority
        tableQueueSto,
        Q_DIM(tableQueueSto),
        (void *)tableStack,        /* stack storage */
        K_THREAD_STACK_SIZEOF(tableStack), /* stack size [bytes] */
        nullptr);         // no initialization param

    return QP::QF::run(); // run the QF application
}
