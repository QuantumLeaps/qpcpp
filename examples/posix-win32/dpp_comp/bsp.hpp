//============================================================================
// Product: DPP example (console)
// Last Updated for Version: 7.3.0
// Date of the Last Update:  2023-08-13
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
#ifndef BSP_HPP_
#define BSP_HPP_

namespace BSP {

constexpr std::uint32_t TICKS_PER_SEC {100U};

void init(int argc, char **argv);
void start();
void displayPaused(std::uint8_t const paused);
void displayPhilStat(std::uint8_t const n, char const *stat);
void terminate(std::int16_t const result);

void randomSeed(std::uint32_t const seed); // random seed
std::uint32_t random(void); // pseudo-random generator

// for testing...
void wait4SW1(void);
void ledOn(void);
void ledOff(void);

} // namespace BSP

#endif // BSP_HPP_

