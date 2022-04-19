//============================================================================
// Product: Console-based BSP, MinGW
// Last Updated for Version: 5.1.1
// Date of the Last Update:  Oct 09, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          https://state-machine.com
// e-mail:                  info@quantum-leaps.com
//============================================================================
#ifndef BSP_HPP
#define BSP_HPP

void BSP_init(int argc, char *argv[]);
void BSP_onKeyboardInput(uint8_t ch);        // process the keyboard character

#define BSP_TICKS_PER_SEC    50U

#endif                                                                // BSP_HPP
