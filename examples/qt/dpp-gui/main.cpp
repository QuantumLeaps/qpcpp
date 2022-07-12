//============================================================================
// Product: QP/C++ example for Qt5
// Last updated for version 6.6.0
// Last updated on  2019-07-30
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#include "gui.hpp"
//-----------------
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

//............................................................................
static QP::QEvt const *l_philoQueueSto[N_PHILO][N_PHILO];
static QP::QSubscrList l_subscrSto[DPP::MAX_PUB_SIG];

// storage for event pools...
static QF_MPOOL_EL(DPP::TableEvt) l_smlPoolSto[2*N_PHILO];

//............................................................................
int main(int argc, char *argv[]) {
    QP::GuiApp app(argc, argv);
    Gui gui;

    gui.show();

    QP::QF::init(); // initialize the framework
    BSP_init();     // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(l_smlPoolSto);

    QP::QActive::psInit(l_subscrSto, Q_DIM(l_subscrSto)); // init publish-subscribe

    // initialize event pools...
    QP::QF::poolInit(l_smlPoolSto,
                     sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

    // start the active objects...
    for (uint_fast8_t n = 0U; n < N_PHILO; ++n) {
        DPP::AO_Philo[n]->start((uint_fast8_t)(n + 1),
                   l_philoQueueSto[n], Q_DIM(l_philoQueueSto[n]),
                   nullptr, 0U); // default stack size
    }
    // NOTE: Table is the GUI Thread in this app
    DPP::AO_Table->start((uint_fast8_t)(N_PHILO + 1),
               (QP::QEvt const **)0, 0U,// no evt queue for GUI thread
               nullptr, 0U);          // no stack for GUI thread

    return QP::QF::run(); // calls qApp->exec()
}
