//****************************************************************************
// DPP example for Windows
// Last updated for version 5.7.5
// Last updated on  2016-11-08
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

// "fudge factor" for Windows, see NOTE1
enum { WIN_FUDGE_FACTOR = 10 };

//............................................................................
int main() {
    static QP::QEvt const *tableQueueSto[N_PHILO*WIN_FUDGE_FACTOR];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO*WIN_FUDGE_FACTOR];
    static QF_MPOOL_EL(DPP::TableEvt) smlPoolSto[2*N_PHILO*WIN_FUDGE_FACTOR];

    static QP::QSubscrList subscrSto[DPP::MAX_PUB_SIG];

    QP::QF::init();  // initialize the framework and the underlying RT kernel
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
        DPP::AO_Philo[n]->start((uint_fast8_t)(n + 1U),
                           philoQueueSto[n], Q_DIM(philoQueueSto[n]),
                           (void *)0, 0U);
    }
    // leave the priority level (N_PHILO + 1) free for the mutex in BSP
    DPP::AO_Table->start((uint_fast8_t)(N_PHILO + 2U),
                    tableQueueSto, Q_DIM(tableQueueSto),
                    (void *)0, 0U);

    return QP::QF::run(); // run the QF application
}

//****************************************************************************
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
