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

Q_DEFINE_THIS_FILE

// local objects -------------------------------------------------------------
class Mine2 : public QHsm {                           // extend the QHsm class
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_exp_ctr;

public:
    Mine2(void) : QHsm((QStateHandler)&Mine2::initial) {}

private:
    static QState initial  (Mine2 *me, QEvt const *e);
    static QState unused   (Mine2 *me, QEvt const *e);
    static QState used     (Mine2 *me, QEvt const *e);
    static QState planted  (Mine2 *me, QEvt const *e);
    static QState exploding(Mine2 *me, QEvt const *e);
};


static Mine2 l_mine2[GAME_MINES_MAX];                // a pool of type-2 mines

                                // helper macro to provide the ID of this mine
#define MINE_ID(me_)    ((me_) - l_mine2)

//............................................................................
QHsm *Mine2_getInst(uint8_t id) {
    Q_REQUIRE(id < GAME_MINES_MAX);
    return &l_mine2[id];
}
//............................................................................
QState Mine2::initial(Mine2 *me, QEvt const *) {
    static uint8_t dict_sent;
    if (!dict_sent) {
        QS_OBJ_DICTIONARY(&l_mine2[0]);    // obj. dictionaries for Mine2 pool
        QS_OBJ_DICTIONARY(&l_mine2[1]);
        QS_OBJ_DICTIONARY(&l_mine2[2]);
        QS_OBJ_DICTIONARY(&l_mine2[3]);
        QS_OBJ_DICTIONARY(&l_mine2[4]);

        QS_FUN_DICTIONARY(&Mine2::initial); // fun. dictionaries for Mine2 HSM
        QS_FUN_DICTIONARY(&Mine2::unused);
        QS_FUN_DICTIONARY(&Mine2::used);
        QS_FUN_DICTIONARY(&Mine2::planted);
        QS_FUN_DICTIONARY(&Mine2::exploding);

        dict_sent = 1;
    }

    QS_SIG_DICTIONARY(MINE_PLANT_SIG,    me);                 // local signals
    QS_SIG_DICTIONARY(MINE_DISABLED_SIG, me);
    QS_SIG_DICTIONARY(MINE_RECYCLE_SIG,  me);
    QS_SIG_DICTIONARY(SHIP_IMG_SIG,      me);
    QS_SIG_DICTIONARY(MISSILE_IMG_SIG,   me);

    return Q_TRAN(&Mine2::unused);
}
//............................................................................
QState Mine2::unused(Mine2 *me, QEvt const *e) {
    switch (e->sig) {
        case MINE_PLANT_SIG: {
            me->m_x = ((ObjectPosEvt const *)e)->x;
            me->m_y = ((ObjectPosEvt const *)e)->y;
            return Q_TRAN(&Mine2::planted);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Mine2::used(Mine2 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_EXIT_SIG: {
            // tell the Tunnel that this mine is becoming disabled
            MineEvt *mev = Q_NEW(MineEvt, MINE_DISABLED_SIG);
            mev->id = MINE_ID(me);
            AO_Tunnel->POST(mev, me);
            return Q_HANDLED();
        }
        case MINE_RECYCLE_SIG: {
            return Q_TRAN(&Mine2::unused);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Mine2::planted(Mine2 *me, QEvt const *e) {
    uint8_t x;
    uint8_t y;
    uint8_t bmp;

    switch (e->sig) {
        case TIME_TICK_SIG: {
            if (me->m_x >= GAME_SPEED_X) {
                ObjectImageEvt *oie;

                me->m_x -= GAME_SPEED_X;               // move the mine 1 step

                // tell the Tunnel to draw the Mine
                oie = Q_NEW(ObjectImageEvt, MINE_IMG_SIG);
                oie->x   = me->m_x;
                oie->y   = me->m_y;
                oie->bmp = MINE2_BMP;
                AO_Tunnel->POST(oie, me);
            }
            else {
                return Q_TRAN(&Mine2::unused);
            }
            return Q_HANDLED();
        }
        case SHIP_IMG_SIG: {
            x   = (uint8_t)((ObjectImageEvt const *)e)->x;
            y   = (uint8_t)((ObjectImageEvt const *)e)->y;
            bmp = (uint8_t)((ObjectImageEvt const *)e)->bmp;

            // test for incoming Ship hitting this mine
            if (do_bitmaps_overlap(MINE2_BMP, me->m_x, me->m_y, bmp, x, y)) {
                                       // Hit event with the type of the Mine2
                static MineEvt const mine2_hit(HIT_MINE_SIG, 2);
                AO_Ship->POST(&mine2_hit, me);

                // go straight to 'disabled' and let the Ship do the exploding
                return Q_TRAN(&Mine2::unused);
            }
            return Q_HANDLED();
        }
        case MISSILE_IMG_SIG: {
            x   = (uint8_t)((ObjectImageEvt const *)e)->x;
            y   = (uint8_t)((ObjectImageEvt const *)e)->y;
            bmp = (uint8_t)((ObjectImageEvt const *)e)->bmp;

            // test for incoming Missile hitting this mine
            // NOTE: Mine type-2 is nastier than Mine type-1.
            // The type-2 mine can hit the Ship with any of its
            // "tentacles". However, it can be destroyed by the
            // Missile only by hitting its center, defined as
            // a smaller bitmap MINE2_MISSILE_BMP.
            if (do_bitmaps_overlap(MINE2_MISSILE_BMP,
                                   me->m_x, me->m_y, bmp, x, y))
            {
                            // Score event with the score for destroying Mine2
                static ScoreEvt const mine2_destroyed(DESTROYED_MINE_SIG, 45);
                AO_Missile->POST(&mine2_destroyed, me);
                return Q_TRAN(&Mine2::exploding);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Mine2::used);
}
//............................................................................
QState Mine2::exploding(Mine2 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_exp_ctr = 0;
            return Q_HANDLED();
        }
        case TIME_TICK_SIG: {
            if ((me->m_x >= GAME_SPEED_X) && (me->m_exp_ctr < 15)) {
                ObjectImageEvt *oie;

                ++me->m_exp_ctr;              // advance the explosion counter
                me->m_x -= GAME_SPEED_X;           // move explosion by 1 step

                // tell the Game to render the current stage of Explosion
                oie = Q_NEW(ObjectImageEvt, EXPLOSION_SIG);
                oie->x   = me->m_x + 1;                      // x of explosion
                oie->y   = (int8_t)((int)me->m_y - 4 + 2);   // y of explosion
                oie->bmp = EXPLOSION0_BMP + (me->m_exp_ctr >> 2);
                AO_Tunnel->POST(oie, me);
            }
            else {
                return Q_TRAN(&Mine2::unused);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Mine2::used);
}
