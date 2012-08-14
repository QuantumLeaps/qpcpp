//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++ example
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 13, 2012
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
#include "gui.h"
#include "qp_app.h"
//-----------------
#include "qp_port.h"
#include "game.h"
#include "bsp.h"

// Local-scope objects -------------------------------------------------------
static QF_MPOOL_EL(QP::QEvt)       l_smlPoolSto[10];
static QF_MPOOL_EL(GAME::ObjectImageEvt) l_medPoolSto[2*GAME_MINES_MAX + 10];

static QP::QSubscrList    l_subscrSto[GAME::MAX_PUB_SIG];

//............................................................................
int main(int argc, char *argv[]) {
    QPApp app(argc, argv);
    Gui gui;

    gui.show();

    QP::QF::init();                                // initialize the framework
    BSP_init();                                          // initialize the BSP

                                              // initialize the event pools...
    QP::QF::poolInit(l_smlPoolSto,
        sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QP::QF::poolInit(l_medPoolSto,
        sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));

    QP::QF::psInit(l_subscrSto, Q_DIM(l_subscrSto)); // init publish-subscribe

                               // send object dictionaries for event queues...

                                // send object dictionaries for event pools...
    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_medPoolSto);

                  // send signal dictionaries for globally published events...
    QS_SIG_DICTIONARY(GAME::TIME_TICK_SIG,      static_cast<void *>(0));
    QS_SIG_DICTIONARY(GAME::PLAYER_TRIGGER_SIG, static_cast<void *>(0));
    QS_SIG_DICTIONARY(GAME::PLAYER_QUIT_SIG,    static_cast<void *>(0));
    QS_SIG_DICTIONARY(GAME::GAME_OVER_SIG,      static_cast<void *>(0));

                                                // start the active objects...
    GAME::AO_Missile->start(1U,                                    // priority
                            (QP::QEvt const **)0, (uint32_t)0,     // no queue
                            (void *)0, (uint32_t)0);               // no stack
    GAME::AO_Ship   ->start(2U,                                    // priority
                            (QP::QEvt const **)0, (uint32_t)0,     // no queue
                            (void *)0, (uint32_t)0);               // no stack
    GAME::AO_Tunnel ->start(3U,                                    // priority
                            (QP::QEvt const **)0, (uint32_t)0,     // no queue
                            (void *)0, (uint32_t)0);               // no stack

    return QP::QF::run();                                // calls qApp->exec()
}
