//////////////////////////////////////////////////////////////////////////////
// Product: Product: "Fly'n'Shoot" game
// Last Updated for Version: 4.2.00
// Date of the Last Update:  Jul 16, 2011
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

// local objects -------------------------------------------------------------
class Mine2 : public Mine {                           // extend the Mine class
public:
    Mine2(void);

protected:
    virtual void onInitial(void);
    virtual void onDrawMine(void);
    virtual uint8_t onShipCollision(ObjectImageEvt const *e);
    virtual uint8_t onMissileCollision(ObjectImageEvt const *e);
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
Mine2::Mine2(void) : Mine(MINE_ID(this)) {                         // the ctor
}
//............................................................................
void Mine2::onInitial() {
                                           // obj. dictionaries for Mine2 pool
    if (m_id == 0) {
        QS_OBJ_DICTIONARY(&l_mine2[0]);
    }
    if (m_id == 1) {
        QS_OBJ_DICTIONARY(&l_mine2[1]);
    }
    if (m_id == 2) {
        QS_OBJ_DICTIONARY(&l_mine2[2]);
    }
    if (m_id == 3) {
        QS_OBJ_DICTIONARY(&l_mine2[3]);
    }
    if (m_id == 4) {
        QS_OBJ_DICTIONARY(&l_mine2[4]);
    }
}
//............................................................................
void Mine2::onDrawMine(void) {
    ObjectImageEvt *oie = Q_NEW(ObjectImageEvt, MINE_IMG_SIG);
    oie->x   = m_x;
    oie->y   = m_y;
    oie->bmp = MINE2_BMP;
    AO_Tunnel->POST(oie, this);
}
//............................................................................
uint8_t Mine2::onShipCollision(ObjectImageEvt const *e) {
    uint8_t x   = (uint8_t)e->x;
    uint8_t y   = (uint8_t)e->y;
    uint8_t bmp = (uint8_t)e->bmp;

    // test for incoming Ship hitting this mine
    if (do_bitmaps_overlap(MINE2_BMP, m_x, m_y, bmp, x, y)) {
                                       // Hit event with the type of the Mine1
        static MineEvt const mine2_hit(HIT_MINE_SIG, 2);
        AO_Ship->POST(&mine2_hit, this);

        return 1;                            // report collision with the Ship
    }
    else {
        return 0;                                // no collision with the Ship
    }
}
//............................................................................
uint8_t Mine2::onMissileCollision(ObjectImageEvt const *e) {
    uint8_t x   = (uint8_t)e->x;
    uint8_t y   = (uint8_t)e->y;
    uint8_t bmp = (uint8_t)e->bmp;


    // test for incoming Missile hitting this mine
    // NOTE: Mine type-2 is nastier than Mine type-1.
    // The type-2 mine can hit the Ship with any of its
    // "tentacles". However, it can be destroyed by the
    // Missile only by hitting its center, defined as
    // a smaller bitmap MINE2_MISSILE_BMP.
    //
    if (do_bitmaps_overlap(MINE2_MISSILE_BMP, m_x, m_y, bmp, x, y)) {
                            // Score event with the score for destroying Mine2
        static ScoreEvt const mine2_destroyed(DESTROYED_MINE_SIG, 45);
        AO_Missile->POST(&mine2_destroyed, this);

        return 1;                         // report collision with the Missile
    }
    else {
        return 0;                             // no collision with the Missile
    }
}

