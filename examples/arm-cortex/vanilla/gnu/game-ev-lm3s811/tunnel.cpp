//////////////////////////////////////////////////////////////////////////////
// Product: Product: "Fly'n'Shoot" game example
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
#include "game.h"
#include "bsp.h"

#include <string.h>                              // for memmove() and memcpy()

Q_DEFINE_THIS_FILE

// Tunnel Active Object ------------------------------------------------------
class Tunnel : public QActive {                    // extend the QActive class

    QTimeEvt m_blinkTimeEvt;                        // time event for blinking
    QTimeEvt m_screenTimeEvt;                 // time event for screen changes

    QHsm *m_mines[GAME_MINES_MAX];                             // active mines
    QHsm *m_mine1_pool[GAME_MINES_MAX];
    QHsm *m_mine2_pool[GAME_MINES_MAX];

    uint8_t m_blink_ctr;                                      // blink counter

    uint8_t m_last_mine_x;
    uint8_t m_last_mine_y;

    uint8_t m_wall_thickness_top;
    uint8_t m_wall_thickness_bottom;
    uint8_t m_minimal_gap;

public:
    Tunnel(void);

private: // HSM
    static QState initial          (Tunnel *me, QEvt const *e);
    static QState final            (Tunnel *me, QEvt const *e);
    static QState active           (Tunnel *me, QEvt const *e);
    static QState playing          (Tunnel *me, QEvt const *e);
    static QState demo             (Tunnel *me, QEvt const *e);
    static QState game_over        (Tunnel *me, QEvt const *e);
    static QState screen_saver     (Tunnel *me, QEvt const *e);
    static QState screen_saver_hide(Tunnel *me, QEvt const *e);
    static QState screen_saver_show(Tunnel *me, QEvt const *e);

private: // Helper functions
    void advance(void);
    void plantMine(void);
    void addImageAt(uint8_t bmp, uint8_t x, int8_t y);
    void dispatchToAllMines(QEvt const *e);
    uint8_t isWallHit(uint8_t bmp, uint8_t x_pos, uint8_t y_pos);
};

static void randomSeed(uint32_t seed);                          // random seed
static uint32_t random(void);                       // pseudo-random generator

static Tunnel l_tunnel;       // the sole instance of the Tunnel active object

// global objects ------------------------------------------------------------
QActive * const AO_Tunnel = &l_tunnel;             // opaque pointer to Tunnel

// local objects -------------------------------------------------------------
static uint32_t l_rnd;                                          // random seed
static uint8_t l_walls[GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8];
static uint8_t l_frame[GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8];


//............................................................................
Tunnel::Tunnel(void) : QActive((QStateHandler)&Tunnel::initial),
                   m_blinkTimeEvt(BLINK_TIMEOUT_SIG),
                   m_screenTimeEvt(SCREEN_TIMEOUT_SIG),
                   m_last_mine_x(0),
                   m_last_mine_y(0)
{
    for (uint8_t n = 0; n < GAME_MINES_MAX; ++n) {
        m_mine1_pool[n] = Mine1_getInst(n);      // initialize mine1-type pool
        m_mine2_pool[n] = Mine2_getInst(n);      // initialize mine2-type pool
        m_mines[n] = (QHsm *)0;                          // mine 'n' is unused
    }
}

// HSM definition ------------------------------------------------------------
QState Tunnel::initial(Tunnel *me, QEvt const *) {

    for (uint8_t n = 0; n < GAME_MINES_MAX; ++n) {
        me->m_mine1_pool[n]->init();       // take the initial tran. for Mine1
        me->m_mine2_pool[n]->init();       // take the initial tran. for Mine2
    }

    randomSeed(1234);                      // seed the pseudo-random generator

    me->subscribe(TIME_TICK_SIG);
    me->subscribe(PLAYER_TRIGGER_SIG);
    me->subscribe(PLAYER_QUIT_SIG);

    QS_OBJ_DICTIONARY(&l_tunnel);       // object dictionary for Tunnel object
    QS_OBJ_DICTIONARY(&l_tunnel.m_blinkTimeEvt);
    QS_OBJ_DICTIONARY(&l_tunnel.m_screenTimeEvt);

    QS_FUN_DICTIONARY(&Tunnel::initial);   // fun. dictionaries for Tunnel HSM
    QS_FUN_DICTIONARY(&Tunnel::final);
    QS_FUN_DICTIONARY(&Tunnel::active);
    QS_FUN_DICTIONARY(&Tunnel::playing);
    QS_FUN_DICTIONARY(&Tunnel::demo);
    QS_FUN_DICTIONARY(&Tunnel::game_over);
    QS_FUN_DICTIONARY(&Tunnel::screen_saver);
    QS_FUN_DICTIONARY(&Tunnel::screen_saver_hide);
    QS_FUN_DICTIONARY(&Tunnel::screen_saver_show);

    QS_SIG_DICTIONARY(BLINK_TIMEOUT_SIG,  &l_tunnel);         // local signals
    QS_SIG_DICTIONARY(SCREEN_TIMEOUT_SIG, &l_tunnel);
    QS_SIG_DICTIONARY(SHIP_IMG_SIG,       &l_tunnel);
    QS_SIG_DICTIONARY(MISSILE_IMG_SIG,    &l_tunnel);
    QS_SIG_DICTIONARY(MINE_IMG_SIG,       &l_tunnel);
    QS_SIG_DICTIONARY(MINE_DISABLED_SIG,  &l_tunnel);
    QS_SIG_DICTIONARY(EXPLOSION_SIG,      &l_tunnel);
    QS_SIG_DICTIONARY(SCORE_SIG,          &l_tunnel);

    return Q_TRAN(&Tunnel::demo);
}
//............................................................................
QState Tunnel::final(Tunnel *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // clear the screen
            memset(l_frame, (uint8_t)0,
                   (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
            BSP_drawBitmap(l_frame, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
            QF::stop();                                 // stop QF and cleanup
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Tunnel::active(Tunnel *me, QEvt const *e) {
    switch (e->sig) {
        case MINE_DISABLED_SIG: {
            Q_ASSERT((((MineEvt const *)e)->id < GAME_MINES_MAX)
                     && (me->m_mines[((MineEvt const *)e)->id] != (QHsm *)0));
            me->m_mines[((MineEvt const *)e)->id] = (QHsm *)0;
            return Q_HANDLED();
        }
        case PLAYER_QUIT_SIG: {
            return Q_TRAN(&Tunnel::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Tunnel::demo(Tunnel *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_last_mine_x = 0;    // last mine at right edge of the tunnel
            me->m_last_mine_y = 0;
                                               // set the tunnel properties...
            me->m_wall_thickness_top = 0;
            me->m_wall_thickness_bottom = 0;
            me->m_minimal_gap = GAME_SCREEN_HEIGHT - 3;

            // erase the tunnel walls
            memset(l_walls, (uint8_t)0,
                   (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));


            me->m_blinkTimeEvt.postEvery(me, BSP_TICKS_PER_SEC/2);  // 1/2 sec

            me->m_screenTimeEvt.postIn(me, BSP_TICKS_PER_SEC*20);    // 20 sec

            me->m_blink_ctr = 0;                     // init the blink counter
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_blinkTimeEvt.disarm();
            me->m_screenTimeEvt.disarm();
            return Q_HANDLED();
        }
        case BLINK_TIMEOUT_SIG: {
            me->m_blink_ctr ^= 1;                   // toggle the blink cunter
            return Q_HANDLED();
        }
        case SCREEN_TIMEOUT_SIG: {
            return Q_TRAN(&Tunnel::screen_saver);
        }
        case TIME_TICK_SIG: {
            me->advance();
            if (me->m_blink_ctr != 0) {
                // add the text bitmap into the frame buffer
                me->addImageAt(PRESS_BUTTON_BMP,
                               (GAME_SCREEN_WIDTH - 55)/2,
                               (GAME_SCREEN_HEIGHT - 8)/2);
            }
            BSP_drawBitmap(l_frame, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
            return Q_HANDLED();
        }
        case PLAYER_TRIGGER_SIG: {
            return Q_TRAN(&Tunnel::playing);
        }
    }
    return Q_SUPER(&Tunnel::active);
}
//............................................................................
QState Tunnel::game_over(Tunnel *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_blinkTimeEvt.postEvery(me, BSP_TICKS_PER_SEC/2);  // 1/2 sec
            me->m_screenTimeEvt.postIn(me,
                                       BSP_TICKS_PER_SEC*5);          // 5 sec
            me->m_blink_ctr = 0;
            BSP_drawNString((GAME_SCREEN_WIDTH - 6*9)/2, 0, "Game Over");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_blinkTimeEvt.disarm();
            me->m_screenTimeEvt.disarm();
            BSP_updateScore(0);             // update the score on the display
            return Q_HANDLED();
        }
        case BLINK_TIMEOUT_SIG: {
            me->m_blink_ctr ^= 1;                   // toggle the blink couner
            BSP_drawNString((GAME_SCREEN_WIDTH - 6*9)/2, 0,
                             ((me->m_blink_ctr == 0)
                             ? "Game Over"
                             : "         "));
            return Q_HANDLED();
        }
        case SCREEN_TIMEOUT_SIG: {
            return Q_TRAN(&Tunnel::demo);
        }
    }
    return Q_SUPER(&Tunnel::active);
}
//............................................................................
QState Tunnel::playing(Tunnel *me, QEvt const *e) {
    uint8_t x;
    int8_t y;
    uint8_t bmp;

    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_minimal_gap = GAME_SCREEN_HEIGHT - 3;

            // erase the walls
            memset(l_walls, (uint8_t)0,
                   (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));

            static QEvt const takeoff = { TAKE_OFF_SIG, 0 };
            AO_Ship->POST(&takeoff, me);               // post the TAKEOFF sig
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            QEvt recycle;
            recycle.sig = MINE_RECYCLE_SIG;
            me->dispatchToAllMines(&recycle);             // recycle all Mines
            return Q_HANDLED();
        }
        case TIME_TICK_SIG: {
            // render this frame on the display
            BSP_drawBitmap(l_frame, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

            me->advance();
            me->plantMine();
            me->dispatchToAllMines(e);

            return Q_HANDLED();
        }
        case SHIP_IMG_SIG:
        case MISSILE_IMG_SIG: {
            x   = (uint8_t)((ObjectImageEvt const *)e)->x;
            y   =  (int8_t)((ObjectImageEvt const *)e)->y;
            bmp = (uint8_t)((ObjectImageEvt const *)e)->bmp;

                                  // did the Ship/Missile hit the tunnel wall?
            if (me->isWallHit(bmp, x, y)) {
                static QEvt const hit = { HIT_WALL_SIG, 0};
                if (e->sig == SHIP_IMG_SIG) {
                    AO_Ship->POST(&hit, me);
                }
                else {
                    AO_Missile->POST(&hit, me);
                }
            }
            me->addImageAt(bmp, x, y);
            me->dispatchToAllMines(e);             // let Mines check for hits
            return Q_HANDLED();
        }
        case MINE_IMG_SIG:
        case EXPLOSION_SIG: {
            x   = (uint8_t)((ObjectImageEvt const *)e)->x;
            y   =  (int8_t)((ObjectImageEvt const *)e)->y;
            bmp = (uint8_t)((ObjectImageEvt const *)e)->bmp;

            me->addImageAt(bmp, x, y);
            return Q_HANDLED();
        }
        case SCORE_SIG: {
            BSP_updateScore(((ScoreEvt const *)e)->score);

            // increase difficulty of the game:
            // the tunnel gets narrower as the score goes up
            me->m_minimal_gap = GAME_SCREEN_HEIGHT - 3
                              - ((ScoreEvt const *)e)->score/2000;
            return Q_HANDLED();
        }
        case GAME_OVER_SIG: {
            uint16_t score = ((ScoreEvt const *)e)->score;
            char str[5];

            BSP_updateScore(score);

            // clear the screen
            memset(l_frame, (uint8_t)0,
                   (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
            BSP_drawBitmap(l_frame, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

            // Output the final score to the screen
            BSP_drawNString((GAME_SCREEN_WIDTH - 6*10)/2, 1, "Score:");
            str[4] = '\0';                        // zero-terminate the string
            str[3] = '0' + (score % 10); score /= 10;
            str[2] = '0' + (score % 10); score /= 10;
            str[1] = '0' + (score % 10); score /= 10;
            str[0] = '0' + (score % 10);
            BSP_drawNString((GAME_SCREEN_WIDTH - 6*10)/2 + 6*6, 1, str);

            return Q_TRAN(&Tunnel::game_over);
        }
    }
    return Q_SUPER(&Tunnel::active);
}
//............................................................................
// A random-pixel screen saver to avoid damage to the display
QState Tunnel::screen_saver(Tunnel *me, QEvt const *e) {
    switch (e->sig) {
        case Q_INIT_SIG: {
            return Q_TRAN(&Tunnel::screen_saver_hide);
        }
        case PLAYER_TRIGGER_SIG: {
            return Q_TRAN(&Tunnel::demo);
        }
    }
    return Q_SUPER(&Tunnel::active);
}
//............................................................................
QState Tunnel::screen_saver_hide(Tunnel *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_displayOff();                        // power down the display
            me->m_screenTimeEvt.postIn(me, BSP_TICKS_PER_SEC*3); // 3s timeout
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_screenTimeEvt.disarm();
            BSP_displayOn();                           // power up the display
            return Q_HANDLED();
        }
        case SCREEN_TIMEOUT_SIG: {
            return Q_TRAN(&Tunnel::screen_saver_show);
        }
    }
    return Q_SUPER(&Tunnel::screen_saver);
}
//............................................................................
QState Tunnel::screen_saver_show(Tunnel *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // clear the screen frame buffer
            memset(l_frame, (uint8_t)0,
                   (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
            uint32_t rnd = random();
            me->addImageAt(PRESS_BUTTON_BMP,
                    (uint8_t)(rnd % (GAME_SCREEN_WIDTH - 55)),
                    (int8_t) (rnd % (GAME_SCREEN_HEIGHT - 8)));
            BSP_drawBitmap(l_frame, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
            me->m_screenTimeEvt.postIn(me, BSP_TICKS_PER_SEC/3);    // 1/3 sec
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_screenTimeEvt.disarm();
            // clear the screen frame buffer
            memset(l_frame, (uint8_t)0,
                   (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
            BSP_drawBitmap(l_frame, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
            return Q_HANDLED();
        }
        case SCREEN_TIMEOUT_SIG: {
            return Q_TRAN(&Tunnel::screen_saver_hide);
        }
    }
    return Q_SUPER(&Tunnel::screen_saver);
}

// helper functions ----------------------------------------------------------
//
// The bitmap for the "Press Button" text:
//
//     xxx.........................xxx........x...x...........
//     x..x........................x..x.......x...x...........
//     x..x.x.xx..xx...xxx..xxx....x..x.x..x.xxx.xxx..xx..xxx.
//     xxx..xx...x..x.x....x.......xxx..x..x..x...x..x..x.x..x
//     x....x....xxxx..xx...xx.....x..x.x..x..x...x..x..x.x..x
//     x....x....x.......x....x....x..x.x..x..x...x..x..x.x..x
//     x....x.....xxx.xxx..xxx.....xxx...xxx...x...x..xx..x..x
//     .......................................................
///
static uint8_t const press_button_bits[] = {
    0x7F, 0x09, 0x09, 0x06, 0x00, 0x7C, 0x08, 0x04, 0x04, 0x00,
    0x38, 0x54, 0x54, 0x58, 0x00, 0x48, 0x54, 0x54, 0x24, 0x00,
    0x48, 0x54, 0x54, 0x24, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x49,
    0x49, 0x36, 0x00, 0x3C, 0x40, 0x40, 0x7C, 0x00, 0x04, 0x3F,
    0x44, 0x00, 0x04, 0x3F, 0x44, 0x00, 0x38, 0x44, 0x44, 0x38,
    0x00, 0x7C, 0x04, 0x04, 0x78
};

// bitmap of the Ship:
//
//     x....
//     xxx..
//     xxxxx
///
static uint8_t const ship_bits[] = {
    0x07, 0x06, 0x06, 0x04, 0x04
};

// bitmap of the Missile:
//
//     xxx
///
static uint8_t const missile_bits[] = {
    0x01, 0x01, 0x01
};

// bitmap of the Mine type-1:
//
//     .x.
//     xxx
//     .x.
///
static uint8_t const mine1_bits[] = {
    0x02, 0x07, 0x02
};

// bitmap of the Mine type-2:
//
//     x..x
//     .xx.
//     .xx.
//     x..x
///
static uint8_t const mine2_bits[] = {
    0x09, 0x06, 0x06, 0x09
};

// Mine type-2 is nastier than Mine type-1. The type-2 mine can
// hit the Ship with any of its "tentacles". However, it can be
// destroyed by the Missile only by hitting its center, defined as
// the following bitmap:
//
//     ....
//     .xx.
//     .xx.
//     ....
///
static uint8_t const mine2_missile_bits[] = {
    0x00, 0x06, 0x06, 0x00
};

// The bitmap of the explosion stage 0:
//
//     .......
//     .......
//     ...x...
//     ..x.x..
//     ...x...
//     .......
//     .......
///
static uint8_t const explosion0_bits[] = {
    0x00, 0x00, 0x08, 0x14, 0x08, 0x00, 0x00
};

// The bitmap of the explosion stage 1:
//
//     .......
//     .......
//     ..x.x..
//     ...x...
//     ..x.x..
//     .......
//     .......
///
static uint8_t const explosion1_bits[] = {
    0x00, 0x00, 0x14, 0x08, 0x14, 0x00, 0x00
};

// The bitmap of the explosion stage 2:
//
//     .......
//     .x...x.
//     ..x.x..
//     ...x...
//     ..x.x..
//     .x...x.
//     .......
///
static uint8_t const explosion2_bits[] = {
    0x00, 0x22, 0x14, 0x08, 0x14, 0x22, 0x00
};

// The bitmap of the explosion stage 3:
//
//     x..x..x
//     .x.x.x.
//     ..x.x..
//     xx.x.xx
//     ..x.x..
//     .x.x.x.
//     x..x..x
///
static uint8_t const explosion3_bits[] = {
    0x49, 0x2A, 0x14, 0x6B, 0x14, 0x2A, 0x49
};

struct Bitmap {               // the auxiliary structure to hold const bitmaps
    uint8_t const *bits;                             // the bits in the bitmap
    uint8_t width;                                  // the width of the bitmap
};

static Bitmap const l_bitmap[MAX_BMP] = {
    { press_button_bits,  Q_DIM(press_button_bits)  },
    { ship_bits,          Q_DIM(ship_bits)          },
    { missile_bits,       Q_DIM(missile_bits)       },
    { mine1_bits,         Q_DIM(mine1_bits)         },
    { mine2_bits,         Q_DIM(mine2_bits)         },
    { mine2_missile_bits, Q_DIM(mine2_missile_bits) },
    { explosion0_bits,    Q_DIM(explosion0_bits)    },
    { explosion1_bits,    Q_DIM(explosion1_bits)    },
    { explosion2_bits,    Q_DIM(explosion2_bits)    },
    { explosion3_bits,    Q_DIM(explosion3_bits)    }
};

//............................................................................
uint32_t random(void) {         // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    l_rnd = l_rnd * (3*7*11*13*23);
    return l_rnd >> 8;
}
//............................................................................
void randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
void Tunnel::advance(void) {
    uint32_t rnd;
    uint32_t bmp1;                 // bimap representing 1 column of the image

    rnd = (random() & 0xFF);

    // reduce the top wall thickness 18.75% of the time
    if ((rnd < 48) && (m_wall_thickness_top > 0)) {
        --m_wall_thickness_top;
    }

    // reduce the bottom wall thickness 18.75% of the time
    if ((rnd > 208) && (m_wall_thickness_bottom > 0)) {
        --m_wall_thickness_bottom;
    }

    rnd = (random() & 0xFF);

    // grow the top wall thickness 18.75% of the time
    if ((rnd < 48)
        && ((GAME_SCREEN_HEIGHT
             - m_wall_thickness_top
             - m_wall_thickness_bottom) > m_minimal_gap)
        && ((m_last_mine_x < (GAME_SCREEN_WIDTH - 5))
            || (m_last_mine_y > (m_wall_thickness_top + 1))))
    {
        ++m_wall_thickness_top;
    }

    // grow the bottom wall thickness 18.75% of the time
    if ((rnd > 208)
        && ((GAME_SCREEN_HEIGHT
             - m_wall_thickness_top
             - m_wall_thickness_bottom) > m_minimal_gap)
        && ((m_last_mine_x < (GAME_SCREEN_WIDTH - 5))
             || (m_last_mine_y + 1
                < (GAME_SCREEN_HEIGHT - m_wall_thickness_bottom))))
    {
        ++m_wall_thickness_bottom;
    }

    // advance the Tunnel by 1 game step to the left
    memmove(l_walls, l_walls + GAME_SPEED_X,
            (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8) - GAME_SPEED_X);

    bmp1 = (~(~0 << m_wall_thickness_top))
            | (~0 << (GAME_SCREEN_HEIGHT
                        - m_wall_thickness_bottom));

    l_walls[GAME_SCREEN_WIDTH - 1] = (uint8_t)bmp1;
    l_walls[GAME_SCREEN_WIDTH + GAME_SCREEN_WIDTH - 1]
          = (uint8_t)(bmp1 >> 8);

    // copy the Tunnel layer to the main frame buffer
    memcpy(l_frame, l_walls, (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT/8));
}
//............................................................................
void Tunnel::plantMine(void) {
    uint32_t rnd = (random() & 0xFF);

    if (m_last_mine_x > 0) {
        --m_last_mine_x;         // shift the last Mine 1 position to the left
    }
                                                      // last mine far enough?
    if ((m_last_mine_x + GAME_MINES_DIST_MIN < GAME_SCREEN_WIDTH)
        && (rnd < 8))                   // place the mines only 5% of the time
    {
        uint8_t n;
        for (n = 0; n < Q_DIM(m_mines); ++n) {      // look for disabled mines
            if (m_mines[n] == (QHsm *)0) {
                break;
            }
        }
        if (n < Q_DIM(m_mines)) {                    // a disabled Mine found?
            ObjectPosEvt ope;                 // event to dispatch to the Mine

            rnd = (random() & 0xFFFF);

            if ((rnd & 1) == 0) {               // choose the type of the mine
                m_mines[n] = m_mine1_pool[n];
            }
            else {
                m_mines[n] = m_mine2_pool[n];
            }

            // new Mine is planted in the last column of the tunnel
            m_last_mine_x = GAME_SCREEN_WIDTH;

            // choose a random y-position for the Mine in the Tunnel
            rnd %= (GAME_SCREEN_HEIGHT
                    - m_wall_thickness_top
                    - m_wall_thickness_bottom - 4);
            m_last_mine_y = m_wall_thickness_top + 2 + rnd;

            ope.sig = MINE_PLANT_SIG;
            ope.x = m_last_mine_x;
            ope.y = m_last_mine_y;
            m_mines[n]->dispatch(&ope);                     // direct dispatch
        }
    }
}
//............................................................................
void Tunnel::dispatchToAllMines(QEvt const *e) {
    uint8_t n;
    for (n = 0; n < GAME_MINES_MAX; ++n) {
        if (m_mines[n] != (QHsm *)0) {                    // is the mine used?
            m_mines[n]->dispatch(e);
        }
    }
}
//............................................................................
void Tunnel::addImageAt(uint8_t bmp, uint8_t x_pos, int8_t y_pos) {
    uint8_t x;                                // the x-index of the ship image
    uint8_t w;                                       // the width of the image

    Q_REQUIRE(bmp < Q_DIM(l_bitmap));

    w = l_bitmap[bmp].width;
    if (w > GAME_SCREEN_WIDTH - x_pos) {
        w = GAME_SCREEN_WIDTH - x_pos;
    }
    for (x = 0; x < w; ++x) {
        uint32_t bmp1;
        if (y_pos >= 0) {
            bmp1 = (l_bitmap[bmp].bits[x] << (uint8_t)y_pos);
        }
        else {
            bmp1 = (l_bitmap[bmp].bits[x] >> (uint8_t)(-y_pos));
        }
        l_frame[x_pos + x] |= (uint8_t)bmp1;
        l_frame[x_pos + x + GAME_SCREEN_WIDTH] |= (uint8_t)(bmp1 >> 8);
    }
}
//............................................................................
uint8_t Tunnel::isWallHit(uint8_t bmp, uint8_t x_pos, uint8_t y_pos) {
    uint8_t x;
    uint8_t w;                                       // the width of the image

    Q_REQUIRE(bmp < Q_DIM(l_bitmap));

    w = l_bitmap[bmp].width;
    if (w > GAME_SCREEN_WIDTH - x_pos) {
        w = GAME_SCREEN_WIDTH - x_pos;
    }
    for (x = 0; x < w; ++x) {
        uint32_t bmp1 = ((uint32_t)l_bitmap[bmp].bits[x] << y_pos);
        if (((l_walls[x_pos + x] & (uint8_t)bmp1) != 0)
            || ((l_walls[x_pos + x + GAME_SCREEN_WIDTH]
                 & (uint8_t)(bmp1 >> 8)) != 0))
        {
            return (uint8_t)1;
        }
    }
    return (uint8_t)0;
}
//............................................................................
uint8_t do_bitmaps_overlap(uint8_t bmp_id1, uint8_t x1, uint8_t y1,
                           uint8_t bmp_id2, uint8_t x2, uint8_t y2)
{
    uint8_t x;
    uint8_t x0;
    uint8_t w;
    uint32_t bits1;
    uint32_t bits2;
    Bitmap const *bmp1;
    Bitmap const *bmp2;

    Q_REQUIRE((bmp_id1 < Q_DIM(l_bitmap)) && (bmp_id2 < Q_DIM(l_bitmap)));

    bmp1 = &l_bitmap[bmp_id1];
    bmp2 = &l_bitmap[bmp_id2];

                // is the incoming object starting to overlap the Mine bitmap?
    if ((x1 <= x2) && (x1 + bmp2->width > x2)) {
        x0 = x2 - x1;
        w  = x1 + bmp2->width - x2;
        if (w > bmp1->width) {
            w = bmp1->width;
        }
        for (x = 0; x < w; ++x) {         // scan over the overlapping columns
            bits1 = ((uint32_t)bmp2->bits[x + x0] << y2);
            bits2 = ((uint32_t)bmp1->bits[x] << y1);
            if ((bits1 & bits2) != 0) {                // do the bits overlap?
                return (uint8_t)1;                                     // yes!
            }
        }
    }
    else {
        if ((x1 > x2) && (x2 + bmp1->width > x1)) {
            x0 = x1 - x2;
            w  = x2 + bmp1->width - x1;
            if (w > bmp2->width) {
                w = bmp2->width;
            }
            for (x = 0; x < w; ++x) {     // scan over the overlapping columns
                bits1 = ((uint32_t)bmp1->bits[x + x0] << y1);
                bits2 = ((uint32_t)bmp2->bits[x] << y2);
                if ((bits1 & bits2) != 0) {            // do the bits overlap?
                    return (uint8_t)1;                                 // yes!
                }
            }
        }
    }
    return (uint8_t)0;                           // the bitmaps do not overlap
}
