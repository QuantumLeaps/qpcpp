//============================================================================
// Product: "Fly 'n' Shoot" game example on EFM32-SLSTK3401A board
// Last updated for version 7.3.0
// Last updated on  2023-09-06
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// https://state-machine.com
// <info@state-machine.com>
//============================================================================
#ifndef BSP_HPP_
#define BSP_HPP_

namespace BSP {

constexpr uint32_t TICKS_PER_SEC = static_cast<uint32_t>(33);
constexpr uint32_t SCREEN_WIDTH  = static_cast<uint32_t>(128);
constexpr uint32_t SCREEN_HEIGHT = static_cast<uint32_t>(128);

void init(void);
void terminate(int16_t result);

void updateScreen(void);
void clearFB(void);
void clearWalls(void);
void paintString(uint8_t x, uint8_t y, char const *str);
void paintBitmap(uint8_t x, uint8_t y, uint8_t bmp_id);
void advanceWalls(uint8_t top, uint8_t bottom);
void updateScore(uint16_t score);

bool isThrottle(void); // is the throttle button depressed?
bool doBitmapsOverlap(uint8_t bmp_id1, uint8_t x1, uint8_t y1,
                          uint8_t bmp_id2, uint8_t x2, uint8_t y2);
bool isWallHit(uint8_t bmp_id, uint8_t x, uint8_t y);
void displayOn(void);
void displayOff(void);

void randomSeed(uint32_t seed);   // random seed
uint32_t random(void);            // pseudo-random generator

extern QP::QTicker *the_Ticker0;

} // namespace BSP

#endif // BSP_HPP_

