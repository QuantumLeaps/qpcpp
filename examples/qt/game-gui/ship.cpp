//$file${.::ship.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: game.qm
// File:  ${.::ship.cpp}
//
// This code has been generated by QM 5.2.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${.::ship.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include "bsp.hpp"
#include "game.hpp"

//Q_DEFINE_THIS_FILE

#define SHIP_WIDTH  5U
#define SHIP_HEIGHT 3U

// encapsulated delcaration of the Ship active object ------------------------
//$declare${AOs::Ship} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace GAME {

//${AOs::Ship} ...............................................................
class Ship : public QP::QActive {
private:
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_exp_ctr;
    uint16_t m_score;

public:
    Ship();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(active);
    Q_STATE_DECL(parked);
    Q_STATE_DECL(flying);
    Q_STATE_DECL(exploding);
}; // class Ship

} // namespace GAME
//$enddecl${AOs::Ship} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

namespace GAME {

// local objects -------------------------------------------------------------
static Ship l_ship; // the sole instance of the Ship active object

// Public-scope objects ------------------------------------------------------
QP::QActive * const AO_Ship = &l_ship; // opaque pointer
} // namespace GAME

// Active object definition --------------------------------------------------
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::Ship} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace GAME {

//${AOs::Ship} ...............................................................

//${AOs::Ship::Ship} .........................................................
Ship::Ship()
 : QActive(Q_STATE_CAST(&Ship::initial)),
   m_x(GAME_SHIP_X),
   m_y(GAME_SHIP_Y)
{}

//${AOs::Ship::SM} ...........................................................
Q_STATE_DEF(Ship, initial) {
    //${AOs::Ship::SM::initial}
    subscribe(TIME_TICK_SIG);
    subscribe(PLAYER_TRIGGER_SIG);
    // object dictionaries...
    QS_OBJ_DICTIONARY(&l_ship);
    // function dictionaries for Ship HSM...
    QS_FUN_DICTIONARY(&Ship::initial);
    QS_FUN_DICTIONARY(&Ship::active);
    QS_FUN_DICTIONARY(&Ship::parked);
    QS_FUN_DICTIONARY(&Ship::flying);
    QS_FUN_DICTIONARY(&Ship::exploding);
    // local signals...
    QS_SIG_DICTIONARY(PLAYER_SHIP_MOVE_SIG, &l_ship);
    QS_SIG_DICTIONARY(TAKE_OFF_SIG,         &l_ship);
    QS_SIG_DICTIONARY(HIT_WALL_SIG,         &l_ship);
    QS_SIG_DICTIONARY(HIT_MINE_SIG,         &l_ship);
    QS_SIG_DICTIONARY(DESTROYED_MINE_SIG,   &l_ship);

    (void)e; // unused parameter

    QS_FUN_DICTIONARY(&Ship::active);
    QS_FUN_DICTIONARY(&Ship::parked);
    QS_FUN_DICTIONARY(&Ship::flying);
    QS_FUN_DICTIONARY(&Ship::exploding);

    return tran(&active);
}

//${AOs::Ship::SM::active} ...................................................
Q_STATE_DEF(Ship, active) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Ship::SM::active::initial}
        case Q_INIT_SIG: {
            status_ = tran(&parked);
            break;
        }
        //${AOs::Ship::SM::active::PLAYER_SHIP_MOVE}
        case PLAYER_SHIP_MOVE_SIG: {
            m_x = Q_EVT_CAST(ObjectPosEvt)->x;
            m_y = Q_EVT_CAST(ObjectPosEvt)->y;
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

//${AOs::Ship::SM::active::parked} ...........................................
Q_STATE_DEF(Ship, parked) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Ship::SM::active::parked::TAKE_OFF}
        case TAKE_OFF_SIG: {
            status_ = tran(&flying);
            break;
        }
        default: {
            status_ = super(&active);
            break;
        }
    }
    return status_;
}

//${AOs::Ship::SM::active::flying} ...........................................
Q_STATE_DEF(Ship, flying) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Ship::SM::active::flying}
        case Q_ENTRY_SIG: {
            m_score = 0U; /* reset the score */
            AO_Tunnel->POST(Q_NEW(ScoreEvt, SCORE_SIG, m_score), this);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Ship::SM::active::flying::TIME_TICK}
        case TIME_TICK_SIG: {
            // tell the Tunnel to draw the Ship and test for hits
            AO_Tunnel->POST(Q_NEW(ObjectImageEvt, SHIP_IMG_SIG,
                                  m_x, m_y, SHIP_BMP),
                            this);
            ++m_score; // increment the score for surviving another tick

            if ((m_score % 10U) == 0U) { // is the score "round"?
                AO_Tunnel->POST(Q_NEW(ScoreEvt, SCORE_SIG, m_score), this);
            }
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Ship::SM::active::flying::PLAYER_TRIGGER}
        case PLAYER_TRIGGER_SIG: {
            AO_Missile->POST(Q_NEW(ObjectPosEvt, MISSILE_FIRE_SIG,
                                   m_x, m_y + SHIP_HEIGHT - 1U),
                             this);
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Ship::SM::active::flying::DESTROYED_MINE}
        case DESTROYED_MINE_SIG: {
            m_score += Q_EVT_CAST(ScoreEvt)->score;
            // the score will be sent to the Tunnel by the next TIME_TICK
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Ship::SM::active::flying::HIT_WALL}
        case HIT_WALL_SIG: {
            status_ = tran(&exploding);
            break;
        }
        //${AOs::Ship::SM::active::flying::HIT_MINE}
        case HIT_MINE_SIG: {
            status_ = tran(&exploding);
            break;
        }
        default: {
            status_ = super(&active);
            break;
        }
    }
    return status_;
}

//${AOs::Ship::SM::active::exploding} ........................................
Q_STATE_DEF(Ship, exploding) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Ship::SM::active::exploding}
        case Q_ENTRY_SIG: {
            m_exp_ctr = 0U;
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Ship::SM::active::exploding::TIME_TICK}
        case TIME_TICK_SIG: {
            //${AOs::Ship::SM::active::exploding::TIME_TICK::[m_exp_ctr<15U]}
            if (m_exp_ctr < 15U) {
                ++m_exp_ctr;
                // tell the Tunnel to draw the current stage of Explosion
                AO_Tunnel->POST(Q_NEW(ObjectImageEvt, EXPLOSION_SIG,
                                      m_x, (int8_t)((int)m_y - 4U + SHIP_HEIGHT),
                                      EXPLOSION0_BMP + (m_exp_ctr >> 2)),
                                this);
                status_ = Q_RET_HANDLED;
            }
            //${AOs::Ship::SM::active::exploding::TIME_TICK::[else]}
            else {
                AO_Tunnel->POST(Q_NEW(ScoreEvt, GAME_OVER_SIG, m_score), this);
                status_ = tran(&parked);
            }
            break;
        }
        default: {
            status_ = super(&active);
            break;
        }
    }
    return status_;
}

} // namespace GAME
//$enddef${AOs::Ship} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
