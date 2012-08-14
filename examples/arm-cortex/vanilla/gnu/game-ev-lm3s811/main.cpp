//////////////////////////////////////////////////////////////////////////////
// Product: Product: "Fly'n'Shoot" game
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
#include "bsp.h"
#include "game.h"

// Local-scope objects -------------------------------------------------------
static QEvt const * l_missileQueueSto[2];
static QEvt const * l_shipQueueSto[3];
static QEvt const * l_tunnelQueueSto[GAME_MINES_MAX + 5];

static union SmallEvents {
    void   *e0;                                          // minimum event size
    uint8_t e1[sizeof(QEvt)];
    // ... other event types to go into this pool
} l_smlPoolSto[10];                        // storage for the small event pool

static union MediumEvents {
    void   *e0;                                          // minimum event size
    uint8_t e1[sizeof(ObjectPosEvt)];
    uint8_t e2[sizeof(ObjectImageEvt)];
    uint8_t e3[sizeof(MineEvt)];
    uint8_t e4[sizeof(ScoreEvt)];
    // ... other event types to go into this pool
} l_medPoolSto[2*GAME_MINES_MAX + 8];     // storage for the medium event pool

static QSubscrList    l_subscrSto[MAX_PUB_SIG];

//............................................................................
int main(void) {

    BSP_init();                        // initialize the Board Support Package

    QF::init();       // initialize the framework and the underlying RT kernel

                                              // initialize the event pools...
    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    QF::poolInit(l_medPoolSto, sizeof(l_medPoolSto), sizeof(l_medPoolSto[0]));

    QF::psInit(l_subscrSto, Q_DIM(l_subscrSto));     // init publish-subscribe

                               // send object dictionaries for event queues...
    QS_OBJ_DICTIONARY(l_missileQueueSto);
    QS_OBJ_DICTIONARY(l_shipQueueSto);
    QS_OBJ_DICTIONARY(l_tunnelQueueSto);

                                // send object dictionaries for event pools...
    QS_OBJ_DICTIONARY(l_smlPoolSto);
    QS_OBJ_DICTIONARY(l_medPoolSto);

                  // send signal dictionaries for globally published events...
    QS_SIG_DICTIONARY(TIME_TICK_SIG,      static_cast<void *>(0));
    QS_SIG_DICTIONARY(PLAYER_TRIGGER_SIG, static_cast<void *>(0));
    QS_SIG_DICTIONARY(PLAYER_QUIT_SIG,    static_cast<void *>(0));
    QS_SIG_DICTIONARY(GAME_OVER_SIG,      static_cast<void *>(0));

                                                // start the active objects...
    AO_Missile->start(1U,                                          // priority
                      l_missileQueueSto, Q_DIM(l_missileQueueSto),// evt queue
                      static_cast<void *>(0), 0U);      // no per-thread stack
    AO_Ship   ->start(2U,                                          // priority
                      l_shipQueueSto, Q_DIM(l_shipQueueSto),      // evt queue
                      static_cast<void *>(0), 0U);      // no per-thread stack
    AO_Tunnel ->start(3U,                                          // priority
                      l_tunnelQueueSto, Q_DIM(l_tunnelQueueSto),  // evt queue
                      static_cast<void *>(0), 0U);      // no per-thread stack

    return QF::run();                                // run the QF application
}
