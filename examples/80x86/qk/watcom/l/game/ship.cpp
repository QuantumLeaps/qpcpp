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

//Q_DEFINE_THIS_FILE

#define SHIP_WIDTH  5
#define SHIP_HEIGHT 3

// local objects -------------------------------------------------------------
class Ship : public QActive {                      // extend the QActive class
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_exp_ctr;
    uint16_t m_score;

public:
    Ship(void) : QActive((QStateHandler)&Ship::initial),
                 m_x(GAME_SHIP_X), m_y(GAME_SHIP_Y) {}
private:
    static QState initial  (Ship *me, QEvt const *e);
    static QState active   (Ship *me, QEvt const *e);
    static QState parked   (Ship *me, QEvt const *e);
    static QState flying   (Ship *me, QEvt const *e);
    static QState exploding(Ship *me, QEvt const *e);
};

static Ship l_ship;             // the sole instance of the Ship active object

// global objects ------------------------------------------------------------
QActive * const AO_Ship = &l_ship;                // opaque pointer to Ship AO

// HSM definition ------------------------------------------------------------
//............................................................................
QState Ship::initial(Ship *me, QEvt const *) {

    me->subscribe(TIME_TICK_SIG);
    me->subscribe(PLAYER_TRIGGER_SIG);


    QS_OBJ_DICTIONARY(&l_ship);           // object dictionary for Ship object

    QS_FUN_DICTIONARY(&Ship::initial);   // function dictionaries for Ship HSM
    QS_FUN_DICTIONARY(&Ship::active);
    QS_FUN_DICTIONARY(&Ship::parked);
    QS_FUN_DICTIONARY(&Ship::flying);
    QS_FUN_DICTIONARY(&Ship::exploding);

    QS_SIG_DICTIONARY(PLAYER_SHIP_MOVE_SIG, &l_ship);         // local signals
    QS_SIG_DICTIONARY(TAKE_OFF_SIG,         &l_ship);
    QS_SIG_DICTIONARY(HIT_WALL_SIG,         &l_ship);
    QS_SIG_DICTIONARY(HIT_MINE_SIG,         &l_ship);
    QS_SIG_DICTIONARY(DESTROYED_MINE_SIG,   &l_ship);

    return Q_TRAN(&Ship::active);               // top-most initial transition
}
//............................................................................
QState Ship::active(Ship *me, QEvt const *e) {
    switch (e->sig) {
        case Q_INIT_SIG: {                        // nested initial transition
            return Q_TRAN(&Ship::parked);
        }
        case PLAYER_SHIP_MOVE_SIG: {
            me->m_x = ((ObjectPosEvt const *)e)->x;
            me->m_y = ((ObjectPosEvt const *)e)->y;
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Ship::parked(Ship *me, QEvt const *e) {
    switch (e->sig) {
        case TAKE_OFF_SIG: {                  // permition to take off granted
            return Q_TRAN(&Ship::flying);
        }
    }
    return Q_SUPER(&Ship::active);
}
//............................................................................
QState Ship::flying(Ship *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            ScoreEvt *sev;

            me->m_score = 0;                                // reset the score
            sev = Q_NEW(ScoreEvt, SCORE_SIG);
            sev->score = me->m_score;
            AO_Tunnel->POST(sev, me);
            return Q_HANDLED();
        }
        case TIME_TICK_SIG: {
            // tell the Tunnel to draw the Ship and test for hits
            ObjectImageEvt *oie = Q_NEW(ObjectImageEvt, SHIP_IMG_SIG);
            oie->x   = me->m_x;
            oie->y   = me->m_y;
            oie->bmp = SHIP_BMP;
            AO_Tunnel->POST(oie, me);

            ++me->m_score;   // increment the score for surviving another tick

            if ((me->m_score % 10) == 0) {            // is the score "round"?
                ScoreEvt *sev = Q_NEW(ScoreEvt, SCORE_SIG);
                sev->score = me->m_score;
                AO_Tunnel->POST(sev, me);
            }

            return Q_HANDLED();
        }
        case PLAYER_TRIGGER_SIG: {                      // trigger the Missile
            ObjectPosEvt *ope = Q_NEW(ObjectPosEvt, MISSILE_FIRE_SIG);
            ope->x = me->m_x;
            ope->y = me->m_y + SHIP_HEIGHT - 1;
            AO_Missile->POST(ope, me);
            return Q_HANDLED();
        }
        case DESTROYED_MINE_SIG: {
            me->m_score += ((ScoreEvt const *)e)->score;
            // the score will be sent to the Tunnel by the next TIME_TICK
            return Q_HANDLED();
        }
        case HIT_WALL_SIG:
        case HIT_MINE_SIG: {
            return Q_TRAN(&Ship::exploding);
        }
    }
    return Q_SUPER(&Ship::active);
}
//............................................................................
QState Ship::exploding(Ship *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_exp_ctr = 0;
            return Q_HANDLED();
        }
        case TIME_TICK_SIG: {
            if (me->m_exp_ctr < 15) {
                ObjectImageEvt *oie;

                ++me->m_exp_ctr;

                     // tell the Tunnel to draw the current stage of Explosion
                oie = Q_NEW(ObjectImageEvt, EXPLOSION_SIG);
                oie->bmp = EXPLOSION0_BMP + (me->m_exp_ctr >> 2);
                oie->x   = me->m_x;                          // x of explosion
                oie->y   = (int8_t)((int)me->m_y - 4 + SHIP_HEIGHT);
                AO_Tunnel->POST(oie, me);
            }
            else {
                ScoreEvt *gameOver = Q_NEW(ScoreEvt, GAME_OVER_SIG);
                gameOver->score = me->m_score;
                AO_Tunnel->POST(gameOver, me);
                return Q_TRAN(&Ship::parked);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Ship::active);
}
