//============================================================================
// Product: QP/C++ example for Qt with GUI
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
#include "game.hpp"
#include "bsp.hpp"

//............................................................................
int main(int argc, char *argv[]) {
    static QP::QEvt const * missileQueueSto[2];
    static QP::QEvt const * shipQueueSto[3];

    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[10];
    static QF_MPOOL_EL(GAME::ObjectImageEvt) medPoolSto[2*GAME_MINES_MAX+10];

    static QP::QSubscrList subscrSto[GAME::MAX_PUB_SIG];

    QP::GuiApp app(argc, argv);
    Gui gui;

    gui.show();

    QP::QF::init(); // initialize the framework and the underlying RT kernel
    BSP_init();     // initialize the Board Support Package

    // initialize the event pools...
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
    QP::QF::poolInit(medPoolSto, sizeof(medPoolSto), sizeof(medPoolSto[0]));

    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // send signal dictionaries for globally published events...
    QS_SIG_DICTIONARY(GAME::TIME_TICK_SIG,      nullptr);
    QS_SIG_DICTIONARY(GAME::PLAYER_TRIGGER_SIG, nullptr);
    QS_SIG_DICTIONARY(GAME::PLAYER_QUIT_SIG,    nullptr);
    QS_SIG_DICTIONARY(GAME::GAME_OVER_SIG,      nullptr);

    // start the active objects...
    // NOTE: AO_Tunnel is the GUI Active Object
    GAME::AO_Tunnel ->start(1U, // priority
                      (QP::QEvt const **)0, 0U,    // no evt queue
                      nullptr, 0U); // no stack for GUI thread

    GAME::AO_Missile->start(2U,                    // priority
                      missileQueueSto, Q_DIM(missileQueueSto), // evt queue
                      nullptr, 0U); // default stack size
    GAME::AO_Ship   ->start(3U,                    // priority
                      shipQueueSto, Q_DIM(shipQueueSto), // evt queue
                      nullptr, 0U); // default stack size

    return QP::QF::run(); // run the QF application
}

