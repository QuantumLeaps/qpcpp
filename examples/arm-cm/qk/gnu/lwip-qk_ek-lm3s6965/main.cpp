//****************************************************************************
// Product: DPP application with lwIP
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Oct 06, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

// Local-scope objects -------------------------------------------------------
static QEvt const *l_tableQueueSto[N_PHILO + 5];
static QEvt const *l_philoQueueSto[N_PHILO][N_PHILO];
static QEvt const *l_lwIPMgrQueueSto[10];
static QSubscrList   l_subscrSto[MAX_PUB_SIG];

static QF_MPOOL_EL(TableEvt) l_smlPoolSto[20]; // storage for small event pool
static QF_MPOOL_EL(TextEvt)  l_medPoolSto[4];  // storage for med.  event pool

//............................................................................
int main(void) {

    QF::init();       // initialize the framework and the underlying RT kernel

    BSP_init();                                          // initialize the BSP

                                                     // object dictionaries...
    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_medPoolSto);
    QS_OBJ_DICTIONARY(l_lwIPMgrQueueSto);
    QS_OBJ_DICTIONARY(l_philoQueueSto[0]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[1]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[2]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[3]);
    QS_OBJ_DICTIONARY(l_philoQueueSto[4]);
    QS_OBJ_DICTIONARY(l_tableQueueSto);

    QF::psInit(l_subscrSto, Q_DIM(l_subscrSto));     // init publish-subscribe

                                                  // initialize event pools...
    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QF::poolInit(l_medPoolSto, sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));

                                                // start the active objects...
    AO_LwIPMgr->start(1U,
                    l_lwIPMgrQueueSto, Q_DIM(l_lwIPMgrQueueSto),
                    (void *)0, 0U);

    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start(n + 2U,
                           l_philoQueueSto[n], Q_DIM(l_philoQueueSto[n]),
                           (void *)0, 0U);
    }
    AO_Table->start(N_PHILO + 2U,
                    l_tableQueueSto, Q_DIM(l_tableQueueSto),
                    (void *)0, 0U);

    return QF::run();                                // run the QF application
}

