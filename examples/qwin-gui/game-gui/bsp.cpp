//============================================================================
// Product: "Fly 'n' Shoot" game example for Win32-GUI
// Last updated for: @qpcpp_7_3_0
// Last updated on  2023-07-19
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps. <www.state-machine.com>
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
#include "qpcpp.hpp"
#include "game.hpp"
#include "bsp.hpp"

#include "qwin_gui.h"  // QWIN GUI
#include "resource.h"  // GUI resource IDs generated by the resource editor

#include "safe_std.h"  // portable "safe" <stdio.h>/<string.h> facilities

#ifdef Q_SPY
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>  // Win32 API for multithreading
    #include <winsock2.h> // for Windows network facilities
#endif

//============================================================================
namespace { // unnamed local namespace

Q_DEFINE_THIS_FILE

// local variables -----------------------------------------------------------
static HINSTANCE l_hInst;   // this application instance
static HWND      l_hWnd;    // main window handle
static LPSTR     l_cmdLine; // the command line string

static GraphicDisplay   l_lcd;        // LCD display on EFM32-SLSTK3401A
static SegmentDisplay   l_userLED0;   // USER LED0 on EFM32-SLSTK3401A
static SegmentDisplay   l_userLED1;   // USER LED1 on EFM32-SLSTK3401A
static SegmentDisplay   l_scoreBoard; // segment display for the score
static OwnerDrawnButton l_userBtn0;   // USER Button0 on EFM32-SLSTK3401A
static OwnerDrawnButton l_userBtn1;   // USER Button1 on EFM32-SLSTK3401A

// (R,G,B) colors for the LCD display
static BYTE const c_onColor[3] = { 0x07U, 0x07U, 0x07U }; // dark grey
static BYTE const c_offColor[3] = { 0xA0U, 0xA0U, 0xA0U }; // light grey

// LCD geometry and frame buffer
static uint32_t l_fb[BSP::SCREEN_HEIGHT + 1][BSP::SCREEN_WIDTH / 32U];

// the walls buffer
static uint32_t l_walls[GAME_TUNNEL_HEIGHT + 1][BSP::SCREEN_WIDTH / 32U];

static unsigned l_rnd;  // random seed

static void paintBits(uint8_t x, uint8_t y, uint8_t const *bits, uint8_t h);
static void paintBitsClear(uint8_t x, uint8_t y,
                           uint8_t const *bits, uint8_t h);
static void playerTrigger(void);

#ifdef Q_SPY
    enum QSUserRecords {
        PLAYER_TRIGGER = QP::QS_USER,
        COMMAND_STAT
    };
    static SOCKET l_sock = INVALID_SOCKET;

    // QS source IDs
    static QP::QSpyId const l_clock_tick = { 0U };
    static QP::QSpyId const l_mouse      = { 0U };
#endif

} // unnamed namespace

//============================================================================
extern "C" {

Q_NORETURN Q_onError(char const* const module, int_t const loc) {
    QP::QF::stop();  // stop ticking
    QS_ASSERTION(module, loc, 10000U); // report assertion to QS

    char message[80];
    SNPRINTF_S(message, Q_DIM(message) - 1,
        "Assertion failed in module %s location %d", module, loc);
    MessageBox(l_hWnd, message, "!!! ASSERTION !!!",
        MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    PostQuitMessage(-1);
}

// Local functions -----------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
    WPARAM wParam, LPARAM lParam);

} // extern "C"

//============================================================================
// thread function for running the application main()
static DWORD WINAPI appThread(LPVOID par) {
    (void)par;         // unused parameter
    return main_gui(); // run the QF application
}

//============================================================================
namespace BSP {

//............................................................................
void init() {
    if (!QS_INIT(l_cmdLine)) { // QS initialization failed?
        MessageBox(l_hWnd,
            "Cannot connect to QSPY via TCP/IP\n"
            "Please make sure that 'qspy -t' is running",
            "QS_INIT() Error",
            MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }
    QS_OBJ_DICTIONARY(&l_clock_tick); // must be called *after* QF::init()
    QS_USR_DICTIONARY(PLAYER_TRIGGER);
    QS_USR_DICTIONARY(COMMAND_STAT);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);
    QS_GLB_FILTER(-QP::QS_QF_TICK);
}
//............................................................................
void terminate(std::int16_t result) {
#ifdef Q_SPY
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
#endif
    QP::QF::stop(); // stop the main QF application and the ticker thread

    // cleanup all QWIN resources...
    OwnerDrawnButton_xtor(&l_userBtn0); // cleanup the l_userBtn0 resources
    OwnerDrawnButton_xtor(&l_userBtn1); // cleanup the l_userBtn1 resources
    SegmentDisplay_xtor(&l_userLED0);   // cleanup the l_userLED0 resources
    SegmentDisplay_xtor(&l_userLED1);   // cleanup the l_userLED1 resources
    SegmentDisplay_xtor(&l_scoreBoard); // cleanup the scoreBoard resources
    GraphicDisplay_xtor(&l_lcd);        // cleanup the l_lcd resources

    // end the main dialog
    EndDialog(l_hWnd, result);
}
//............................................................................
void updateScreen(void) {
    UINT x, y;

    // turn LED1 on
    SegmentDisplay_setSegment(&l_userLED1, 0U, 1U);

    // map the LCD pixels to the GraphicDisplay pixels...
    for (y = 0; y < SCREEN_HEIGHT; ++y) {
        for (x = 0; x < SCREEN_WIDTH; ++x) {
            uint32_t bits = l_fb[y][x >> 5];
            if ((bits & (1U << (x & 0x1FU))) != 0U) {
                GraphicDisplay_setPixel(&l_lcd, x, y, c_onColor);
            }
            else {
                GraphicDisplay_clearPixel(&l_lcd, x, y);
            }
        }
    }

    GraphicDisplay_redraw(&l_lcd); // redraw the updated display

    // turn LED1 off
    SegmentDisplay_setSegment(&l_userLED1, 0U, 0U);
}
//............................................................................
void clearFB() {
    uint_fast8_t y;
    for (y = 0U; y < SCREEN_HEIGHT; ++y) {
        l_fb[y][0] = 0U;
        l_fb[y][1] = 0U;
        l_fb[y][2] = 0U;
        l_fb[y][3] = 0U;
    }
}
//............................................................................
void clearWalls() {
    uint_fast8_t y;
    for (y = 0U; y < GAME_TUNNEL_HEIGHT; ++y) {
        l_walls[y][0] = 0U;
        l_walls[y][1] = 0U;
        l_walls[y][2] = 0U;
        l_walls[y][3] = 0U;
    }
}
//............................................................................
bool isThrottle(void) { // is the throttle button depressed?
    return OwnerDrawnButton_isDepressed(&l_userBtn1) != 0;
}
//............................................................................
void paintString(uint8_t x, uint8_t y, char const *str) {
    static uint8_t const font5x7[95][7] = {
        { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U }, //
        { 0x04U, 0x04U, 0x04U, 0x04U, 0x00U, 0x00U, 0x04U }, // !
        { 0x0AU, 0x0AU, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U }, // "
        { 0x0AU, 0x0AU, 0x1FU, 0x0AU, 0x1FU, 0x0AU, 0x0AU }, // #
        { 0x04U, 0x1EU, 0x05U, 0x0EU, 0x14U, 0x0FU, 0x04U }, // $
        { 0x03U, 0x13U, 0x08U, 0x04U, 0x02U, 0x19U, 0x18U }, // %
        { 0x06U, 0x09U, 0x05U, 0x02U, 0x15U, 0x09U, 0x16U }, // &
        { 0x06U, 0x04U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U }, // '
        { 0x08U, 0x04U, 0x02U, 0x02U, 0x02U, 0x04U, 0x08U }, // (
        { 0x02U, 0x04U, 0x08U, 0x08U, 0x08U, 0x04U, 0x02U }, // )
        { 0x00U, 0x04U, 0x15U, 0x0EU, 0x15U, 0x04U, 0x00U }, // *
        { 0x00U, 0x04U, 0x04U, 0x1FU, 0x04U, 0x04U, 0x00U }, // +
        { 0x00U, 0x00U, 0x00U, 0x00U, 0x06U, 0x04U, 0x02U }, // ,
        { 0x00U, 0x00U, 0x00U, 0x1FU, 0x00U, 0x00U, 0x00U }, // -
        { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x06U, 0x06U }, // .
        { 0x00U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U, 0x00U }, // /
        { 0x0EU, 0x11U, 0x19U, 0x15U, 0x13U, 0x11U, 0x0EU }, // 0
        { 0x04U, 0x06U, 0x04U, 0x04U, 0x04U, 0x04U, 0x0EU }, // 1
        { 0x0EU, 0x11U, 0x10U, 0x08U, 0x04U, 0x02U, 0x1FU }, // 2
        { 0x1FU, 0x08U, 0x04U, 0x08U, 0x10U, 0x11U, 0x0EU }, // 3
        { 0x08U, 0x0CU, 0x0AU, 0x09U, 0x1FU, 0x08U, 0x08U }, // 4
        { 0x1FU, 0x01U, 0x0FU, 0x10U, 0x10U, 0x11U, 0x0EU }, // 5
        { 0x0CU, 0x02U, 0x01U, 0x0FU, 0x11U, 0x11U, 0x0EU }, // 6
        { 0x1FU, 0x10U, 0x08U, 0x04U, 0x02U, 0x02U, 0x02U }, // 7
        { 0x0EU, 0x11U, 0x11U, 0x0EU, 0x11U, 0x11U, 0x0EU }, // 8
        { 0x0EU, 0x11U, 0x11U, 0x1EU, 0x10U, 0x08U, 0x06U }, // 9
        { 0x00U, 0x06U, 0x06U, 0x00U, 0x06U, 0x06U, 0x00U }, // :
        { 0x00U, 0x06U, 0x06U, 0x00U, 0x06U, 0x04U, 0x02U }, // ;
        { 0x08U, 0x04U, 0x02U, 0x01U, 0x02U, 0x04U, 0x08U }, // <
        { 0x00U, 0x00U, 0x1FU, 0x00U, 0x1FU, 0x00U, 0x00U }, // =
        { 0x02U, 0x04U, 0x08U, 0x10U, 0x08U, 0x04U, 0x02U }, // >
        { 0x0EU, 0x11U, 0x10U, 0x08U, 0x04U, 0x00U, 0x04U }, // ?
        { 0x0EU, 0x11U, 0x10U, 0x16U, 0x15U, 0x15U, 0x0EU }, // @
        { 0x0EU, 0x11U, 0x11U, 0x11U, 0x1FU, 0x11U, 0x11U }, // A
        { 0x0FU, 0x11U, 0x11U, 0x0FU, 0x11U, 0x11U, 0x0FU }, // B
        { 0x0EU, 0x11U, 0x01U, 0x01U, 0x01U, 0x11U, 0x0EU }, // C
        { 0x07U, 0x09U, 0x11U, 0x11U, 0x11U, 0x09U, 0x07U }, // D
        { 0x1FU, 0x01U, 0x01U, 0x0FU, 0x01U, 0x01U, 0x1FU }, // E
        { 0x1FU, 0x01U, 0x01U, 0x0FU, 0x01U, 0x01U, 0x01U }, // F
        { 0x0EU, 0x11U, 0x01U, 0x1DU, 0x11U, 0x11U, 0x1EU }, // G
        { 0x11U, 0x11U, 0x11U, 0x1FU, 0x11U, 0x11U, 0x11U }, // H
        { 0x0EU, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x0EU }, // I
        { 0x1CU, 0x08U, 0x08U, 0x08U, 0x08U, 0x09U, 0x06U }, // J
        { 0x11U, 0x09U, 0x05U, 0x03U, 0x05U, 0x09U, 0x11U }, // K
        { 0x01U, 0x01U, 0x01U, 0x01U, 0x01U, 0x01U, 0x1FU }, // L
        { 0x11U, 0x1BU, 0x15U, 0x15U, 0x11U, 0x11U, 0x11U }, // M
        { 0x11U, 0x11U, 0x13U, 0x15U, 0x19U, 0x11U, 0x11U }, // N
        { 0x0EU, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x0EU }, // O
        { 0x0FU, 0x11U, 0x11U, 0x0FU, 0x01U, 0x01U, 0x01U }, // P
        { 0x0EU, 0x11U, 0x11U, 0x11U, 0x15U, 0x09U, 0x16U }, // Q
        { 0x0FU, 0x11U, 0x11U, 0x0FU, 0x05U, 0x09U, 0x11U }, // R
        { 0x1EU, 0x01U, 0x01U, 0x0EU, 0x10U, 0x10U, 0x0FU }, // S
        { 0x1FU, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U }, // T
        { 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x0EU }, // U
        { 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x0AU, 0x04U }, // V
        { 0x11U, 0x11U, 0x11U, 0x15U, 0x15U, 0x15U, 0x0AU }, // W
        { 0x11U, 0x11U, 0x0AU, 0x04U, 0x0AU, 0x11U, 0x11U }, // X
        { 0x11U, 0x11U, 0x11U, 0x0AU, 0x04U, 0x04U, 0x04U }, // Y
        { 0x1FU, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U, 0x1FU }, // Z
        { 0x0EU, 0x02U, 0x02U, 0x02U, 0x02U, 0x02U, 0x0EU }, // [
        { 0x00U, 0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x00U }, // '\'
        { 0x0EU, 0x08U, 0x08U, 0x08U, 0x08U, 0x08U, 0x0EU }, // ]
        { 0x04U, 0x0AU, 0x11U, 0x00U, 0x00U, 0x00U, 0x00U }, // ^
        { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x1FU }, // _
        { 0x02U, 0x04U, 0x08U, 0x00U, 0x00U, 0x00U, 0x00U }, // `
        { 0x00U, 0x00U, 0x0EU, 0x10U, 0x1EU, 0x11U, 0x1EU }, // a
        { 0x01U, 0x01U, 0x0DU, 0x13U, 0x11U, 0x11U, 0x0FU }, // b
        { 0x00U, 0x00U, 0x0EU, 0x01U, 0x01U, 0x11U, 0x0EU }, // c
        { 0x10U, 0x10U, 0x16U, 0x19U, 0x11U, 0x11U, 0x1EU }, // d
        { 0x00U, 0x00U, 0x0EU, 0x11U, 0x1FU, 0x01U, 0x0EU }, // e
        { 0x0CU, 0x12U, 0x02U, 0x07U, 0x02U, 0x02U, 0x02U }, // f
        { 0x00U, 0x1EU, 0x11U, 0x11U, 0x1EU, 0x10U, 0x0EU }, // g
        { 0x01U, 0x01U, 0x0DU, 0x13U, 0x11U, 0x11U, 0x11U }, // h
        { 0x04U, 0x00U, 0x06U, 0x04U, 0x04U, 0x04U, 0x0EU }, // i
        { 0x08U, 0x00U, 0x0CU, 0x08U, 0x08U, 0x09U, 0x06U }, // j
        { 0x01U, 0x01U, 0x09U, 0x05U, 0x03U, 0x05U, 0x09U }, // k
        { 0x06U, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x0EU }, // l
        { 0x00U, 0x00U, 0x0BU, 0x15U, 0x15U, 0x11U, 0x11U }, // m
        { 0x00U, 0x00U, 0x0DU, 0x13U, 0x11U, 0x11U, 0x11U }, // n
        { 0x00U, 0x00U, 0x0EU, 0x11U, 0x11U, 0x11U, 0x0EU }, // o
        { 0x00U, 0x00U, 0x0FU, 0x11U, 0x0FU, 0x01U, 0x01U }, // p
        { 0x00U, 0x00U, 0x16U, 0x19U, 0x1EU, 0x10U, 0x10U }, // q
        { 0x00U, 0x00U, 0x0DU, 0x13U, 0x01U, 0x01U, 0x01U }, // r
        { 0x00U, 0x00U, 0x0EU, 0x01U, 0x0EU, 0x10U, 0x0FU }, // s
        { 0x02U, 0x02U, 0x07U, 0x02U, 0x02U, 0x12U, 0x0CU }, // t
        { 0x00U, 0x00U, 0x11U, 0x11U, 0x11U, 0x19U, 0x16U }, // u
        { 0x00U, 0x00U, 0x11U, 0x11U, 0x11U, 0x0AU, 0x04U }, // v
        { 0x00U, 0x00U, 0x11U, 0x11U, 0x15U, 0x15U, 0x0AU }, // w
        { 0x00U, 0x00U, 0x11U, 0x0AU, 0x04U, 0x0AU, 0x11U }, // x
        { 0x00U, 0x00U, 0x11U, 0x11U, 0x1EU, 0x10U, 0x0EU }, // y
        { 0x00U, 0x00U, 0x1FU, 0x08U, 0x04U, 0x02U, 0x1FU }, // z
        { 0x08U, 0x04U, 0x04U, 0x02U, 0x04U, 0x04U, 0x08U }, // {
        { 0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U, 0x04U }, // |
        { 0x02U, 0x04U, 0x04U, 0x08U, 0x04U, 0x04U, 0x02U }, // }
        { 0x02U, 0x15U, 0x08U, 0x00U, 0x00U, 0x00U, 0x00U }, // ~
    };

    for (; *str != '\0'; ++str, x += 6) {
        uint8_t const *ch = &font5x7[*str - ' '][0];
        paintBitsClear(x, y, ch, 7);
    }
}

//============================================================================
typedef struct { // the auxiliary structure to hold const bitmaps
    uint8_t const *bits; // the bits in the bitmap
    uint8_t height;      // the height of the bitmap
} Bitmap;

// bitmap of the Ship:
//
//     x....
//     xxx..
//     xxxxx
//
static uint8_t const ship_bits[] = {
    0x01U, 0x07U, 0x1FU
};

// bitmap of the Missile:
//
//     xxxx
//
static uint8_t const missile_bits[] = {
    0x0FU
};

// bitmap of the Mine type-1:
//
//     .x.
//     xxx
//     .x.
//
static uint8_t const mine1_bits[] = {
    0x02U, 0x07U, 0x02U
};

// bitmap of the Mine type-2:
//
//     x..x
//     .xx.
//     .xx.
//     x..x
//
static uint8_t const mine2_bits[] = {
    0x09U, 0x06U, 0x06U, 0x09U
};

// Mine type-2 is nastier than Mine type-1. The type-2 mine can
// hit the Ship with any of its "tentacles". However, it can be
// destroyed by the Missile only by hitting its center, defined as
// the following bitmap:
//
//     ....
//     .xx.
//     .xx.
//
static uint8_t const mine2_missile_bits[] = {
    0x00U, 0x06U, 0x06U
};

//
// The bitmap of the explosion stage 0:
//
//     .......
//     ...x...
//     ..x.x..
//     ...x...
//
static uint8_t const explosion0_bits[] = {
    0x00U, 0x08U, 0x14U, 0x08U
};

//
// The bitmap of the explosion stage 1:
//
//     .......
//     ..x.x..
//     ...x...
//     ..x.x..
//
static uint8_t const explosion1_bits[] = {
    0x00U, 0x14U, 0x08U, 0x14U
};

//
// The bitmap of the explosion stage 2:
//
//     .x...x.
//     ..x.x..
//     ...x...
//     ..x.x..
//     .x...x.
//
static uint8_t const explosion2_bits[] = {
    0x11U, 0x0AU, 0x04U, 0x0AU, 0x11U
};

//
// The bitmap of the explosion stage 3:
//
//     x..x..x
//     .x.x.x.
//     ..x.x..
//     xx.x.xx
//     ..x.x..
//     .x.x.x.
//     x..x..x
//
static uint8_t const explosion3_bits[] = {
    0x49, 0x2A, 0x14, 0x6B, 0x14, 0x2A, 0x49
};

static Bitmap const l_bitmap[GAME::MAX_BMP] = {
    { ship_bits,       Q_DIM(ship_bits) },
    { missile_bits,    Q_DIM(missile_bits) },
    { mine1_bits,      Q_DIM(mine1_bits) },
    { mine2_bits,      Q_DIM(mine2_bits) },
    { mine2_missile_bits, Q_DIM(mine2_missile_bits) },
    { explosion0_bits, Q_DIM(explosion0_bits) },
    { explosion1_bits, Q_DIM(explosion1_bits) },
    { explosion2_bits, Q_DIM(explosion2_bits) },
    { explosion3_bits, Q_DIM(explosion3_bits) }
};

//............................................................................
void paintBitmap(uint8_t x, uint8_t y, uint8_t bmp_id) {
    Bitmap const *bmp = &l_bitmap[bmp_id];
    paintBits(x, y, bmp->bits, bmp->height);
}
//............................................................................
void advanceWalls(uint8_t top, uint8_t bottom) {
    uint_fast8_t y;
    for (y = 0U; y < GAME_TUNNEL_HEIGHT; ++y) {
        // shift the walls one pixel to the left
        l_walls[y][0] = (l_walls[y][0] >> 1) | (l_walls[y][1] << 31);
        l_walls[y][1] = (l_walls[y][1] >> 1) | (l_walls[y][2] << 31);
        l_walls[y][2] = (l_walls[y][2] >> 1) | (l_walls[y][3] << 31);
        l_walls[y][3] = (l_walls[y][3] >> 1);

        // add new column of walls at the end
        if (y <= top) {
            l_walls[y][3] |= (1U << 31);
        }
        if (y >= (GAME_TUNNEL_HEIGHT - bottom)) {
            l_walls[y][3] |= (1U << 31);
        }

        // copy the walls to the frame buffer
        l_fb[y][0] = l_walls[y][0];
        l_fb[y][1] = l_walls[y][1];
        l_fb[y][2] = l_walls[y][2];
        l_fb[y][3] = l_walls[y][3];
    }
}
//............................................................................
bool doBitmapsOverlap(uint8_t bmp_id1, uint8_t x1, uint8_t y1,
                          uint8_t bmp_id2, uint8_t x2, uint8_t y2)
{
    uint8_t y;
    uint8_t y0;
    uint8_t h;
    uint32_t bits1;
    uint32_t bits2;
    Bitmap const *bmp1;
    Bitmap const *bmp2;

    Q_REQUIRE((bmp_id1 < Q_DIM(l_bitmap)) && (bmp_id2 < Q_DIM(l_bitmap)));

    // are the bitmaps close enough in x?
    if (x1 >= x2) {
        if (x1 > x2 + 8U) {
            return false;
        }
        x1 -= x2;
        x2 = 0U;
    }
    else {
        if (x2 > x1 + 8U) {
            return false;
        }
        x2 -= x1;
        x1 = 0U;
    }

    bmp1 = &l_bitmap[bmp_id1];
    bmp2 = &l_bitmap[bmp_id2];
    if ((y1 <= y2) && (y1 + bmp1->height > y2)) {
        y0 = y2 - y1;
        h = y1 + bmp1->height - y2;
        if (h > bmp2->height) {
            h = bmp2->height;
        }
        for (y = 0; y < h; ++y) { // scan over the overlapping rows
            bits1 = ((uint32_t)bmp1->bits[y + y0] << x1);
            bits2 = ((uint32_t)bmp2->bits[y] << x2);
            if ((bits1 & bits2) != 0U) { // do the bits overlap?
                return true; // yes!
            }
        }
    }
    else {
        if ((y1 > y2) && (y2 + bmp2->height > y1)) {
            y0 = y1 - y2;
            h = y2 + bmp2->height - y1;
            if (h > bmp1->height) {
                h = bmp1->height;
            }
            for (y = 0; y < h; ++y) {  // scan over the overlapping rows
                bits1 = ((uint32_t)bmp1->bits[y] << x1);
                bits2 = ((uint32_t)bmp2->bits[y + y0] << x2);
                if ((bits1 & bits2) != 0U) { // do the bits overlap?
                                       return true; // yes!
                }
            }
        }
    }
    return false; // the bitmaps do not overlap
}
//............................................................................
bool isWallHit(uint8_t bmp_id, uint8_t x, uint8_t y) {
    Bitmap const *bmp = &l_bitmap[bmp_id];
    uint32_t shft = (x & 0x1FU);
    uint32_t *walls = &l_walls[y][x >> 5];
    for (y = 0; y < bmp->height; ++y, walls += (SCREEN_WIDTH >> 5)) {
        if (*walls & ((uint32_t)bmp->bits[y] << shft)) {
            return true;
        }
        if (shft > 24U) {
            if (*(walls + 1) & ((uint32_t)bmp->bits[y] >> (32U - shft))) {
                return true;
            }
        }
    }
    return false;
}

//............................................................................
void updateScore(uint16_t score) {
    uint8_t seg[5];
    char str[5];

    if (score == 0U) {
        paintString(1U, SCREEN_HEIGHT - 8U, "SCORE:");
    }

    seg[0] = score % 10U; score /= 10U;
    seg[1] = score % 10U; score /= 10U;
    seg[2] = score % 10U; score /= 10U;
    seg[3] = score % 10U;

    // update the SCORE area on the screeen
    str[0] = seg[3] + '0';
    str[1] = seg[2] + '0';
    str[2] = seg[1] + '0';
    str[3] = seg[0] + '0';
    str[4] = '\0';
    paintString(6U*6U, SCREEN_HEIGHT - 8U, str);

    // update the score in the l_scoreBoard SegmentDisplay
    SegmentDisplay_setSegment(&l_scoreBoard, 0U, (UINT)seg[0]);
    SegmentDisplay_setSegment(&l_scoreBoard, 1U, (UINT)seg[1]);
    SegmentDisplay_setSegment(&l_scoreBoard, 2U, (UINT)seg[2]);
    SegmentDisplay_setSegment(&l_scoreBoard, 3U, (UINT)seg[3]);
}
//............................................................................
void displayOn() {
    SegmentDisplay_setSegment(&l_userLED0, 0U, 1U);
}
//............................................................................
void displayOff() {
    SegmentDisplay_setSegment(&l_userLED0, 0U, 0U);
    GraphicDisplay_clear(&l_lcd);
    GraphicDisplay_redraw(&l_lcd);
}
//............................................................................
uint32_t random(void) {  // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);
    return l_rnd >> 8;
}
//............................................................................
void randomSeed(std::uint32_t seed) {
    l_rnd = seed;
}

} // namespace BSP

//============================================================================
namespace { // unnamed local namespace

static void paintBits(uint8_t x, uint8_t y, uint8_t const *bits, uint8_t h) {
    uint32_t *fb = &l_fb[y][x >> 5];
    uint32_t shft = (x & 0x1FU);
    for (y = 0; y < h; ++y, fb += (BSP::SCREEN_WIDTH >> 5)) {
        *fb |= ((uint32_t)bits[y] << shft);
        if (shft > 24U) {
            *(fb + 1) |= ((uint32_t)bits[y] >> (32U - shft));
        }
    }
}
//............................................................................
static void paintBitsClear(uint8_t x, uint8_t y,
                           uint8_t const *bits, uint8_t h)
{
    uint32_t *fb = &l_fb[y][x >> 5];
    uint32_t shft = (x & 0x1FU);
    uint32_t mask1 = ~((uint32_t)0xFFU << shft);
    uint32_t mask2;
    if (shft > 24U) {
        mask2 = ~(0xFFU >> (32U - shft));
    }
    for (y = 0; y < h; ++y, fb += (BSP::SCREEN_WIDTH >> 5)) {
        *fb = ((*fb & mask1) | ((uint32_t)bits[y] << shft));
        if (shft > 24U) {
            *(fb + 1) = ((*(fb + 1) & mask2)
                | ((uint32_t)bits[y] >> (32U - shft)));
        }
    }
}

//............................................................................
static void playerTrigger(void) {
    static QP::QEvt const fireEvt(GAME::PLAYER_TRIGGER_SIG);
#ifdef Q_SPY
    static QP::QSpyId const sender = { QP::QS_AP_ID };
#endif
    QP::QF::PUBLISH(&fireEvt, &sender);
}

} // unnamed local namespace

//============================================================================
extern "C" {

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE /*hPrevInst*/,
                              LPSTR cmdLine, int iCmdShow)
{
    l_hInst   = hInst;   // save the application instance
    l_cmdLine = cmdLine; // save the command line string

    // create the main custom dialog window
    HWND hWnd = CreateCustDialog(hInst, IDD_APPLICATION, NULL,
                                 &WndProc, "MY_CLASS");
    ShowWindow(hWnd, iCmdShow); // show the main window

    // enter the message loop...
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    BSP::terminate(0);

    return msg.wParam;
}
//............................................................................
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
                                WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {

        // Perform initialization upon creation of the main dialog window
        // NOTE: Any child-windows are NOT created yet at this time, so
        // the GetDlgItem() function can't be used (it will return NULL).
        //
        case WM_CREATE: {
            l_hWnd = hWnd; // save the window handle

            // initialize the owner-drawn buttons...
            // NOTE: must be done *before* the first drawing of the buttons,
            // so WM_INITDIALOG is too late.
            //
            OwnerDrawnButton_init(&l_userBtn0, IDC_USER0,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_UP)),
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_DWN)),
                LoadCursor(NULL, IDC_HAND));
            OwnerDrawnButton_init(&l_userBtn1, IDC_USER1,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_UP)),
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_DWN)),
                LoadCursor(NULL, IDC_HAND));
            return 0;
        }

        // Perform initialization after all child windows have been created
        case WM_INITDIALOG: {
            GraphicDisplay_init(&l_lcd, 128, 128, IDC_LCD, c_offColor);

            SegmentDisplay_init(&l_userLED0,
                1U,  // 1 "segment" (the LED0 itself)
                2U); // 2 bitmaps (for LED0 OFF/ON states)
            SegmentDisplay_initSegment(&l_userLED0, 0U, IDC_LED0);
            SegmentDisplay_initBitmap(&l_userLED0,
                0U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_OFF)));
            SegmentDisplay_initBitmap(&l_userLED0,
                1U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_ON)));

            SegmentDisplay_init(&l_userLED1,
                1U,  // 1 "segment" (the LED1 itself)
                2U); // 2 bitmaps (for LED1 OFF/ON states)
            SegmentDisplay_initSegment(&l_userLED1, 0U, IDC_LED1);
            SegmentDisplay_initBitmap(&l_userLED1,
                0U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_OFF)));
            SegmentDisplay_initBitmap(&l_userLED1,
                1U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_LED_ON)));

            SegmentDisplay_init(&l_scoreBoard,
                4U,   // 4 "segments" (digits 0-3)
                10U); // 10 bitmaps (for 0-9 states)
            SegmentDisplay_initSegment(&l_scoreBoard, 0U, IDC_SEG0);
            SegmentDisplay_initSegment(&l_scoreBoard, 1U, IDC_SEG1);
            SegmentDisplay_initSegment(&l_scoreBoard, 2U, IDC_SEG2);
            SegmentDisplay_initSegment(&l_scoreBoard, 3U, IDC_SEG3);
            SegmentDisplay_initBitmap(&l_scoreBoard,
                0U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG0)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                1U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG1)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                2U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG2)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                3U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG3)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                4U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG4)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                5U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG5)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                6U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG6)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                7U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG7)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                8U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG8)));
            SegmentDisplay_initBitmap(&l_scoreBoard,
                9U, LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_SEG9)));

            BSP::updateScore(0U);

            // --> QP: spawn the application thread to run main_gui()
            HANDLE hThr = CreateThread(NULL, 0, &appThread, NULL, 0, NULL);
            Q_ASSERT(hThr != (HANDLE)0);
            return 0;
        }

        case WM_DESTROY: {
            OutputDebugString("DESTROY\n");
            PostQuitMessage(0);
            return 0;
        }

        // commands from child controls and menus...
        case WM_COMMAND: {
            switch (wParam) {
                case IDOK:
                case IDCANCEL: {
                    OutputDebugString("QUIT\n");
                    PostQuitMessage(0);
                    break;
                }
                case IDC_USER0:    // owner-drawn buttons...
                case IDC_USER1: {
                    SetFocus(hWnd);
                    break;
                }
            }
            return 0;
        }

        // drawing of owner-drawn buttons...
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            switch (pdis->CtlID) {
                case IDC_USER0: {  // owner-drawn Button0
                    OutputDebugString("USER0\n");
                    switch (OwnerDrawnButton_draw(&l_userBtn0, pdis)) {
                        case BTN_DEPRESSED: {
                            playerTrigger();
                            SegmentDisplay_setSegment(&l_userLED0, 0U, 1U);
                            break;
                        }
                        case BTN_RELEASED: {
                            SegmentDisplay_setSegment(&l_userLED0, 0U, 0U);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
                case IDC_USER1: {  // owner-drawn Button1
                    OutputDebugString("USER1\n");
                    switch (OwnerDrawnButton_draw(&l_userBtn1, pdis)) {
                        case BTN_DEPRESSED: {
                             break;
                        }
                        case BTN_RELEASED: {
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
            }
            return 0;
        }

        // mouse wheel input...
        case WM_MOUSEWHEEL: {
            OutputDebugString("MOUSEWHEEL\n");
            return 0;
        }

        // keyboard input...
        case WM_KEYDOWN: {
            OutputDebugString("KEYDOWN\n");
            switch (wParam) {
                case VK_SPACE:
                    playerTrigger();
                    OwnerDrawnButton_set(&l_userBtn0, 1);
                    break;
                }
                return 0;
            }

        case WM_KEYUP: {
            OutputDebugString("KEYUP\n");
            switch (wParam) {
                case VK_SPACE:
                    OwnerDrawnButton_set(&l_userBtn0, 0);
                    break;
            }
            return 0;
        }
    }
    return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

} // extern "C"

//============================================================================

namespace QP {

//............................................................................
void QF::onStartup(void) {
    setTickRate(BSP::TICKS_PER_SEC, 30); // set the desired tick rate
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QF::onClockTick(void) {
    static QP::QEvt const tickEvt(GAME::TIME_TICK_SIG);
    QTimeEvt::TICK_X(0U, &l_clock_tick); // time events at rate 0
    QF::PUBLISH(&tickEvt, &l_clock_tick); // publish the tick event

    QS_RX_INPUT(); // handle the QS-RX input
    QS_OUTPUT();   // handle the QS output
}

//----------------------------------------------------------------------------
#ifdef Q_SPY // define QS callbacks

//............................................................................
void QS::onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

