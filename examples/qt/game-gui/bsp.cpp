//============================================================================
// Product: "Fly'n'Shoot" game, BSP for Qt5
// Last updated for version 6.9.1
// Last updated on  2020-09-21
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include <QtWidgets>
#include "pixellabel.hpp"
#include "gui.hpp"
//-----------------
#include "qpcpp.hpp"
#include "game.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

//............................................................................
static quint8 const c_onColor[] = { 255U, 255U,   255U }; // white
static uint8_t l_ship_pos = GAME_SHIP_Y;

#ifdef Q_SPY
    enum QSUserRecords {
        PLAYER_TRIGGER = QP::QS_USER
    };

    // QS source IDs
    static QP::QSpyId const l_time_tick = { 0U };
    static QP::QSpyId const l_bsp = { 0U };
#endif

//............................................................................
void QP::QF_onClockTick(void) {
    QP::QTimeEvt::TICK_X(0U, &l_time_tick); // perform the QF clock tick processing

    static QP::QEvt const tickEvt(GAME::TIME_TICK_SIG);
    QP::QF::PUBLISH(&tickEvt, &l_time_tick); // publish the tick event
}
//............................................................................
void QP::QF::onStartup(void) {
    QP::QF_setTickRate(BSP_TICKS_PER_SEC);
}
//............................................................................
void QP::QF::onCleanup(void) {
}

//............................................................................
void BSP_init() {
    Q_ALLEGE(QS_INIT((char *)0));
    QS_OBJ_DICTIONARY(&l_time_tick);
    QS_OBJ_DICTIONARY(&l_bsp);
    QS_USR_DICTIONARY(PLAYER_TRIGGER);

    // set up the QS filters...
    QS_GLB_FILTER(QP::QS_QF_MPOOL_GET);
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
    qDebug("terminate");
    QP::QF::stop(); // stop the QF_run() thread
    qApp->quit(); // quit the Qt application *after* the QF_run() has stopped
}
//............................................................................
void BSP_drawBitmap(uint8_t const *bitmap) {
    PixelLabel *display = Gui::instance->m_display;
    for (unsigned y = 0U; y < BSP_SCREEN_HEIGHT; ++y) {
        for (unsigned x = 0U; x < BSP_SCREEN_WIDTH; ++x) {
            uint8_t bits = bitmap[x + (y/8)*BSP_SCREEN_WIDTH];
            if ((bits & (1U << (y & 0x07U))) != 0U) {
                display->setPixel(x, y, c_onColor);
            }
            else {
                display->clearPixel(x, y);
            }
        }
    }

    display->redraw();
}
//............................................................................
void BSP_drawNString(uint8_t x, uint8_t y, char const *str) {
    static uint8_t const font5x7[95][5] = {
        { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U }, // ' '
        { 0x00U, 0x00U, 0x4FU, 0x00U, 0x00U }, // !
        { 0x00U, 0x07U, 0x00U, 0x07U, 0x00U }, // "
        { 0x14U, 0x7FU, 0x14U, 0x7FU, 0x14U }, // #
        { 0x24U, 0x2AU, 0x7FU, 0x2AU, 0x12U }, // $
        { 0x23U, 0x13U, 0x08U, 0x64U, 0x62U }, // %
        { 0x36U, 0x49U, 0x55U, 0x22U, 0x50U }, // &
        { 0x00U, 0x05U, 0x03U, 0x00U, 0x00U }, // '
        { 0x00U, 0x1CU, 0x22U, 0x41U, 0x00U }, // (
        { 0x00U, 0x41U, 0x22U, 0x1CU, 0x00U }, // )
        { 0x14U, 0x08U, 0x3EU, 0x08U, 0x14U }, // *
        { 0x08U, 0x08U, 0x3EU, 0x08U, 0x08U }, // +
        { 0x00U, 0x50U, 0x30U, 0x00U, 0x00U }, // ,
        { 0x08U, 0x08U, 0x08U, 0x08U, 0x08U }, // -
        { 0x00U, 0x60U, 0x60U, 0x00U, 0x00U }, // .
        { 0x20U, 0x10U, 0x08U, 0x04U, 0x02U }, // /
        { 0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU }, // 0
        { 0x00U, 0x42U, 0x7FU, 0x40U, 0x00U }, // 1
        { 0x42U, 0x61U, 0x51U, 0x49U, 0x46U }, // 2
        { 0x21U, 0x41U, 0x45U, 0x4BU, 0x31U }, // 3
        { 0x18U, 0x14U, 0x12U, 0x7FU, 0x10U }, // 4
        { 0x27U, 0x45U, 0x45U, 0x45U, 0x39U }, // 5
        { 0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U }, // 6
        { 0x01U, 0x71U, 0x09U, 0x05U, 0x03U }, // 7
        { 0x36U, 0x49U, 0x49U, 0x49U, 0x36U }, // 8
        { 0x06U, 0x49U, 0x49U, 0x29U, 0x1EU }, // 9
        { 0x00U, 0x36U, 0x36U, 0x00U, 0x00U }, // :
        { 0x00U, 0x56U, 0x36U, 0x00U, 0x00U }, // ;
        { 0x08U, 0x14U, 0x22U, 0x41U, 0x00U }, // <
        { 0x14U, 0x14U, 0x14U, 0x14U, 0x14U }, // =
        { 0x00U, 0x41U, 0x22U, 0x14U, 0x08U }, // >
        { 0x02U, 0x01U, 0x51U, 0x09U, 0x06U }, // ?
        { 0x32U, 0x49U, 0x79U, 0x41U, 0x3EU }, // @
        { 0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU }, // A
        { 0x7FU, 0x49U, 0x49U, 0x49U, 0x36U }, // B
        { 0x3EU, 0x41U, 0x41U, 0x41U, 0x22U }, // C
        { 0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU }, // D
        { 0x7FU, 0x49U, 0x49U, 0x49U, 0x41U }, // E
        { 0x7FU, 0x09U, 0x09U, 0x09U, 0x01U }, // F
        { 0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU }, // G
        { 0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU }, // H
        { 0x00U, 0x41U, 0x7FU, 0x41U, 0x00U }, // I
        { 0x20U, 0x40U, 0x41U, 0x3FU, 0x01U }, // J
        { 0x7FU, 0x08U, 0x14U, 0x22U, 0x41U }, // K
        { 0x7FU, 0x40U, 0x40U, 0x40U, 0x40U }, // L
        { 0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU }, // M
        { 0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU }, // N
        { 0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU }, // O
        { 0x7FU, 0x09U, 0x09U, 0x09U, 0x06U }, // P
        { 0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU }, // Q
        { 0x7FU, 0x09U, 0x19U, 0x29U, 0x46U }, // R
        { 0x46U, 0x49U, 0x49U, 0x49U, 0x31U }, // S
        { 0x01U, 0x01U, 0x7FU, 0x01U, 0x01U }, // T
        { 0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU }, // U
        { 0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU }, // V
        { 0x3FU, 0x40U, 0x38U, 0x40U, 0x3FU }, // W
        { 0x63U, 0x14U, 0x08U, 0x14U, 0x63U }, // X
        { 0x07U, 0x08U, 0x70U, 0x08U, 0x07U }, // Y
        { 0x61U, 0x51U, 0x49U, 0x45U, 0x43U }, // Z
        { 0x00U, 0x7FU, 0x41U, 0x41U, 0x00U }, // [
        { 0x02U, 0x04U, 0x08U, 0x10U, 0x20U }, // '\'
        { 0x00U, 0x41U, 0x41U, 0x7FU, 0x00U }, // ]
        { 0x04U, 0x02U, 0x01U, 0x02U, 0x04U }, // ^
        { 0x40U, 0x40U, 0x40U, 0x40U, 0x40U }, // _
        { 0x00U, 0x01U, 0x02U, 0x04U, 0x00U }, // `
        { 0x20U, 0x54U, 0x54U, 0x54U, 0x78U }, // a
        { 0x7FU, 0x48U, 0x44U, 0x44U, 0x38U }, // b
        { 0x38U, 0x44U, 0x44U, 0x44U, 0x20U }, // c
        { 0x38U, 0x44U, 0x44U, 0x48U, 0x7FU }, // d
        { 0x38U, 0x54U, 0x54U, 0x54U, 0x18U }, // e
        { 0x08U, 0x7EU, 0x09U, 0x01U, 0x02U }, // f
        { 0x0CU, 0x52U, 0x52U, 0x52U, 0x3EU }, // g
        { 0x7FU, 0x08U, 0x04U, 0x04U, 0x78U }, // h
        { 0x00U, 0x44U, 0x7DU, 0x40U, 0x00U }, // i
        { 0x20U, 0x40U, 0x44U, 0x3DU, 0x00U }, // j
        { 0x7FU, 0x10U, 0x28U, 0x44U, 0x00U }, // k
        { 0x00U, 0x41U, 0x7FU, 0x40U, 0x00U }, // l
        { 0x7CU, 0x04U, 0x18U, 0x04U, 0x78U }, // m
        { 0x7CU, 0x08U, 0x04U, 0x04U, 0x78U }, // n
        { 0x38U, 0x44U, 0x44U, 0x44U, 0x38U }, // o
        { 0x7CU, 0x14U, 0x14U, 0x14U, 0x08U }, // p
        { 0x08U, 0x14U, 0x14U, 0x18U, 0x7CU }, // q
        { 0x7CU, 0x08U, 0x04U, 0x04U, 0x08U }, // r
        { 0x48U, 0x54U, 0x54U, 0x54U, 0x20U }, // s
        { 0x04U, 0x3FU, 0x44U, 0x40U, 0x20U }, // t
        { 0x3CU, 0x40U, 0x40U, 0x20U, 0x7CU }, // u
        { 0x1CU, 0x20U, 0x40U, 0x20U, 0x1CU }, // v
        { 0x3CU, 0x40U, 0x30U, 0x40U, 0x3CU }, // w
        { 0x44U, 0x28U, 0x10U, 0x28U, 0x44U }, // x
        { 0x0CU, 0x50U, 0x50U, 0x50U, 0x3CU }, // y
        { 0x44U, 0x64U, 0x54U, 0x4CU, 0x44U }, // z
        { 0x00U, 0x08U, 0x36U, 0x41U, 0x00U }, // {
        { 0x00U, 0x00U, 0x7FU, 0x00U, 0x00U }, // |
        { 0x00U, 0x41U, 0x36U, 0x08U, 0x00U }, // }
        { 0x02U, 0x01U, 0x02U, 0x04U, 0x02U }, // ~
    };

    PixelLabel *display = Gui::instance->m_display;
    while (*str != '\0') {
        uint8_t const *ch = &font5x7[*str - ' '][0];
        for (int dx = 0; dx < 5; ++dx) {
            for (int dy = 0; dy < 8; ++dy) {
                if ((ch[dx] & (1U << dy)) != 0U) {
                    display->setPixel(x + dx, y*8 + dy, c_onColor);
                }
                else {
                    display->clearPixel(x + dx, y*8 + dy);
                }
            }
        }
        ++str;
        x += 6;
    }
    display->redraw();
}
//............................................................................
void BSP_updateScore(uint16_t score) {
    Gui::instance->m_score->display((int)score);
}
//............................................................................
void BSP_displayOn(void) {
    Gui::instance->m_LED->setPixmap(QPixmap(":/res/LED_OFF.png"));
}
//............................................................................
void BSP_displayOff(void) {
    Gui::instance->m_display->clear();
    Gui::instance->m_display->redraw();
    Gui::instance->m_LED->setPixmap(QPixmap(":/res/LED_ON.png"));
}
//............................................................................
void BSP_moveShipUp(void) {
    if (l_ship_pos > 0U) {
        --l_ship_pos;
    }
    GAME::AO_Ship->POST(Q_NEW(GAME::ObjectPosEvt, GAME::PLAYER_SHIP_MOVE_SIG,
                        (uint8_t)GAME_SHIP_X, (uint8_t)l_ship_pos),
                 &l_bsp);
}
//............................................................................
void BSP_moveShipDown(void) {
    if (l_ship_pos < (GAME_SCREEN_HEIGHT - 3U)) {
        ++l_ship_pos;
    }
    GAME::AO_Ship->POST(Q_NEW(GAME::ObjectPosEvt, GAME::PLAYER_SHIP_MOVE_SIG,
                        (uint8_t)GAME_SHIP_X, (uint8_t)l_ship_pos),
                 &l_bsp);
}
//............................................................................
Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    QMessageBox::critical(0, "PROBLEM",
        QString("<p>Assertion failed in module <b>%1</b>,"
                "location <b>%2</b></p>")
            .arg(module)
            .arg(loc));
    QS_ASSERTION(module, loc, 10000); // send assertion info to the QS trace
    qFatal("Assertion failed in module %s, location %d", module, loc);
}

//============================================================================
#ifdef Q_SPY

#include "qspy.h"

static QTime l_time;

//............................................................................
static int custParserFun(QSpyRecord * const qrec) {
    int ret = 0; // do not perform standard QSPY parsing
    switch (qrec->rec) {
        case QP::QS_QF_MPOOL_GET: { // example record to parse
            int nFree;
            (void)QSpyRecord_getUint32(qrec, QS_TIME_SIZE);
            (void)QSpyRecord_getUint64(qrec, QS_OBJ_PTR_SIZE);
            nFree = (int)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE);
            (void)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE); // nMin
            if (QSpyRecord_OK(qrec)) {
                Gui::instance->m_epoolLabel->setText(QString::number(nFree));
                ret = 0; // don't perform standard QSPY parsing
            }
            break;
        }
    }
    return ret;
}
//............................................................................
bool QP::QS::onStartup(void const *) {
    static uint8_t qsBuf[4*1024];   // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    QSPY_config(QP_VERSION,         // version
                QS_OBJ_PTR_SIZE,    // objPtrSize
                QS_FUN_PTR_SIZE,    // funPtrSize
                QS_TIME_SIZE,       // tstampSize
                Q_SIGNAL_SIZE,      // sigSize,
                QF_EVENT_SIZ_SIZE,  // evtSize
                QF_EQUEUE_CTR_SIZE, // queueCtrSize
                QF_MPOOL_CTR_SIZE,  // poolCtrSize
                QF_MPOOL_SIZ_SIZE,  // poolBlkSize
                QF_TIMEEVT_CTR_SIZE,// tevtCtrSize
                nullptr,          // matFile,
                nullptr,
                &custParserFun);    // customized parser function

    l_time.start(); // start the time stamp

    return true; // success
}
//............................................................................
void QP::QS::onCleanup(void) {
    QSPY_stop();
}
//............................................................................
void QP::QS::onFlush(void) {
    uint16_t nBytes = 1024U;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != nullptr) {
        QSPY_parse(block, nBytes);
        nBytes = 1024U;
    }
}
//............................................................................
QP::QSTimeCtr QP::QS::onGetTime(void) {
    return (QSTimeCtr)l_time.elapsed();
}

//............................................................................
void QP::QS_onEvent(void) {
    uint16_t nBytes = 1024;
    uint8_t const *block;
    QF_CRIT_ENTRY(dummy);
    if ((block = QS::getBlock(&nBytes)) != nullptr) {
        QF_CRIT_EXIT(dummy);
        QSPY_parse(block, nBytes);
    }
    else {
        QF_CRIT_EXIT(dummy);
    }
}
//............................................................................
extern "C" void QSPY_onPrintLn(void) {
    qDebug(QSPY_line);
}

#endif // Q_SPY

