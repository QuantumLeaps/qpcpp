//****************************************************************************
// Product: QP-lwIP demonstration
// Last Updated for Version: 5.4.0
// Date of the Last Update:  2015-05-12
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
// Web  : http://www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

//............................................................................
int main(void) {
    static QEvt const *tableQueueSto[N_PHILO + 5];
    static QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static QEvt const *lwIPMgrQueueSto[10];
    static QSubscrList subscrSto[MAX_PUB_SIG];

    static QF_MPOOL_EL(TableEvt) smlPoolSto[20]; // storage for small pool
    static QF_MPOOL_EL(TextEvt)  medPoolSto[4];  // storage for med.  pool

    QF::init();  // initialize the framework and the underlying RT kernel
    BSP_init();  // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(medPoolSto);
    QS_OBJ_DICTIONARY(lwIPMgrQueueSto);
    QS_OBJ_DICTIONARY(philoQueueSto[0]);
    QS_OBJ_DICTIONARY(philoQueueSto[1]);
    QS_OBJ_DICTIONARY(philoQueueSto[2]);
    QS_OBJ_DICTIONARY(philoQueueSto[3]);
    QS_OBJ_DICTIONARY(philoQueueSto[4]);
    QS_OBJ_DICTIONARY(tableQueueSto);

    // initialize publish-subscribe...
    QF::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
    QF::poolInit(medPoolSto, sizeof(medPoolSto), sizeof(medPoolSto[0]));

    // start the active objects...
    AO_LwIPMgr->start(1U,
                    lwIPMgrQueueSto, Q_DIM(lwIPMgrQueueSto),
                    (void *)0, 0U);

    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start(n + 2U,
                           philoQueueSto[n], Q_DIM(philoQueueSto[n]),
                           (void *)0, 0U);
    }
    AO_Table->start(N_PHILO + 2U,
                    tableQueueSto, Q_DIM(tableQueueSto),
                    (void *)0, 0U);

    return QF::run(); // run the application
}

