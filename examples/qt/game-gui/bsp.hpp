//============================================================================
// Product: "Fly'n'Shoot" game, BSP for Qt5
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
#ifndef BSP_HPPPP
#define BSP_HPPPP

#define BSP_TICKS_PER_SEC    33U
#define BSP_SCREEN_WIDTH     96U
#define BSP_SCREEN_HEIGHT    16U

void BSP_init();
void BSP_terminate(int16_t result);
void BSP_drawBitmap(uint8_t const *bitmap);
void BSP_drawNString(uint8_t x,    // x in pixels
                     uint8_t y,    // y position in chars
                     char const *str);
void BSP_updateScore(uint16_t score);

void BSP_displayOn(void);
void BSP_displayOff(void);

void BSP_moveShipUp(void);
void BSP_moveShipDown(void);

#endif // BSP_HPPPP
