//============================================================================
// DPP example for Windows
// Last updated for: @qpcpp_7_3_0
// Last updated on  2023-07-19
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps. <www.state-machine.com>
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

// "fudge factor" for Windows, see NOTE1
enum { WIN_FUDGE_FACTOR = 10 };

//............................................................................
int main() {
    QP::QF::init();  // initialize the framework and the underlying RT kernel
    BSP::init(); // initialize the BSP

    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    static QF_MPOOL_EL(APP::TableEvt) smlPoolSto[2*APP::N_PHILO*WIN_FUDGE_FACTOR];
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the active objects...
    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10*WIN_FUDGE_FACTOR];
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
        APP::AO_Philo[n]->start(n + 1U,
                           philoQueueSto[n], Q_DIM(philoQueueSto[n]),
                           nullptr, 0U);
    }
    // leave the priority level (N_PHILO + 1) free for the mutex in BSP
    static QP::QEvt const *tableQueueSto[APP::N_PHILO*WIN_FUDGE_FACTOR];
    APP::AO_Table->start(APP::N_PHILO + 2U,
                    tableQueueSto, Q_DIM(tableQueueSto),
                    nullptr, 0U);

    return QP::QF::run(); // run the QF application
}

//============================================================================
// NOTE1:
// Windows is not a deterministic real-time system, which means that the
// system can occasionally and unexpectedly "choke and freeze" for a number
// of seconds. The designers of Windows have dealt with these sort of issues
// by massively oversizing the resources available to the applications. For
// example, the default Windows GUI message queues size is 10,000 entries,
// which can dynamically grow to an even larger number. Also the stacks of
// Win32 threads can dynamically grow to several megabytes.
//
// In contrast, the event queues, event pools, and stack size inside the
// real-time embedded (RTE) systems can be (and must be) much smaller,
// because you typically can put an upper bound on the real-time behavior
// and the resulting delays.
//
// To be able to run the unmodified applications designed originally for
// RTE systems on Windows, and to reduce the odds of resource shortages in
// this case, the generous WIN_FUDGE_FACTOR is used to oversize the
// event queues and event pools.
//
