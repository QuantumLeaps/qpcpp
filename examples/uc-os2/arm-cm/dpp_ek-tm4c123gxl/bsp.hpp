//============================================================================
// Product: BSP for DPP with uC/OS-II RTOS
// Last Updated for Version: 6.3.8
// Date of the Last Update:  2019-01-30
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef BSP_HPP
#define BSP_HPP

namespace DPP {

class BSP {
public:
    enum { TICKS_PER_SEC = OS_TICKS_PER_SEC };

    static void init(void);
    static void displayPaused(uint8_t const paused);
    static void displayPhilStat(uint8_t const n, char const *stat);
    static void terminate(int16_t const result);

    static void randomSeed(uint32_t const seed); // random seed
    static uint32_t random(void); // pseudo-random generator

    // for testing...
    static void wait4SW1(void);
    static void ledOn(void);
    static void ledOff(void);
};

} // namespace DPP

#endif // BSP_HPP
