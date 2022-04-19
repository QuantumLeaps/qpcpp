//============================================================================
// Product: BSP for DPP application with lwIP on EV-LM3S9665 board
// Last Updated for Version: 4.4.00
// Date of the Last Update:  Apr 19, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
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

#define BSP_TICKS_PER_SEC    100U

void BSP_init(void);

                                 // RITEK 128x96x4 OLED used in Rev C-D boards
#define RITEK_OLED
                                 // OSRAM 128x64x4 OLED used in REV A-B boards
//#define OSRAM_OLED

extern "C" {

void ISR_SysTick(void);
void ISR_Ethernet(void);
void ISR_Nmi(void);
void ISR_Fault(void);
void ISR_DefaultHandler(void);

}

#endif                                                                // BSP_HPP
