//////////////////////////////////////////////////////////////////////////////
// Product: DPP example
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 20, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

namespace DPP {

//............................................................................
extern "C" int_t main(void) {
    static QP::QEvt const *tableQueueSto[N_PHILO];
    static QP::QEvt const *philoQueueSto[N_PHILO][N_PHILO];
    static QP::QSubscrList   subscrSto[MAX_PUB_SIG];
    static QF_MPOOL_EL(TableEvt) smlPoolSto[2U*N_PHILO];         // small pool

    BSP_init();                                          // initialize the BSP

    QP::QF::init();   // initialize the framework and the underlying RT kernel

                                                     // object dictionaries...
    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(tableQueueSto);
    QS_OBJ_DICTIONARY(philoQueueSto[0]);
    QS_OBJ_DICTIONARY(philoQueueSto[1]);
    QS_OBJ_DICTIONARY(philoQueueSto[2]);
    QS_OBJ_DICTIONARY(philoQueueSto[3]);
    QS_OBJ_DICTIONARY(philoQueueSto[4]);

    QP::QF::psInit(&subscrSto[0], Q_DIM(subscrSto)); // init publish-subscribe

                                                  // initialize event pools...
    QP::QF::poolInit(&smlPoolSto[0], sizeof(smlPoolSto),
                                     sizeof(smlPoolSto[0]));

                                                // start the active objects...
    for (uint8_t n = 0U; n < N_PHILO; ++n) {
        AO_Philo[n]->start(static_cast<uint8_t>(n + 1U),
                           &philoQueueSto[n][0], Q_DIM(philoQueueSto[n]),
                           static_cast<void *>(0), 0U);
    }
    AO_Table->start(static_cast<uint8_t>(N_PHILO + 1U),
                    &tableQueueSto[0], Q_DIM(tableQueueSto),
                    static_cast<void *>(0), 0U);


    return QP::QF::run();                            // run the QF application
}

}                                                             // namespace DPP
