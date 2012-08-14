//////////////////////////////////////////////////////////////////////////////
// Product: Product: "Fly'n'Shoot" game
// Last Updated for Version: 4.2.00
// Date of the Last Update:  Jul 15, 2011
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2011 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "bsp.h"
#include "game.h"
#include "mine.h"

Q_DEFINE_THIS_FILE

//............................................................................
QState Mine::initial(Mine *me, QEvent const *) {
    static uint8_t dict_sent;
    if (!dict_sent) {

        QS_FUN_DICTIONARY(&Mine::initial); // fun. dictionaries for Mine HSM
        QS_FUN_DICTIONARY(&Mine::unused);
        QS_FUN_DICTIONARY(&Mine::used);
        QS_FUN_DICTIONARY(&Mine::planted);
        QS_FUN_DICTIONARY(&Mine::exploding);

        dict_sent = 1;
    }

    QS_SIG_DICTIONARY(MINE_PLANT_SIG,    me);                 // local signals
    QS_SIG_DICTIONARY(MINE_DISABLED_SIG, me);
    QS_SIG_DICTIONARY(MINE_RECYCLE_SIG,  me);
    QS_SIG_DICTIONARY(SHIP_IMG_SIG,      me);
    QS_SIG_DICTIONARY(MISSILE_IMG_SIG,   me);

    me->onInitial();               // customized initialization for subclasses

    return Q_TRAN(&Mine::unused);
}
//............................................................................
QState Mine::unused(Mine *me, QEvent const *e) {
    switch (e->sig) {
        case MINE_PLANT_SIG: {
            me->m_x = ((ObjectPosEvt const *)e)->x;
            me->m_y = ((ObjectPosEvt const *)e)->y;
            return Q_TRAN(&Mine::planted);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Mine::used(Mine *me, QEvent const *e) {
    switch (e->sig) {
        case Q_EXIT_SIG: {
            // tell the Tunnel that this mine is becoming disabled
            MineEvt *mev = Q_NEW(MineEvt, MINE_DISABLED_SIG);
            mev->id = me->m_id;
            AO_Tunnel->POST(mev, me);
            return Q_HANDLED();
        }
        case MINE_RECYCLE_SIG: {
            return Q_TRAN(&Mine::unused);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Mine::planted(Mine *me, QEvent const *e) {
    uint8_t x;
    uint8_t y;
    uint8_t bmp;

    switch (e->sig) {
        case TIME_TICK_SIG: {
            if (me->m_x >= GAME_SPEED_X) {
                me->m_x -= GAME_SPEED_X;               // move the mine 1 step
                                           // tell the Tunnel to draw the Mine
                me->onDrawMine();                  // customized in subclasses
            }
            else {
                return Q_TRAN(&Mine::unused);
            }
            return Q_HANDLED();
        }
        case SHIP_IMG_SIG: {
            if (me->onShipCollision((ObjectImageEvt const *)e)) {
                // go straight to 'disabled' and let the Ship do the exploding
                return Q_TRAN(&Mine::unused);
            }
            return Q_HANDLED();
        }
        case MISSILE_IMG_SIG: {
            if (me->onMissileCollision((ObjectImageEvt const *)e)) {
                return Q_TRAN(&Mine::exploding);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Mine::used);
}
//............................................................................
QState Mine::exploding(Mine *me, QEvent const *e) {
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
                return Q_TRAN(&Mine::unused);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Mine::used);
}
