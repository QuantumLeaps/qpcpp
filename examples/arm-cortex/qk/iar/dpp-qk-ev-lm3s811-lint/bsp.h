//////////////////////////////////////////////////////////////////////////////
// Product: "Dining Philosophers Problem" example
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 08, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
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
#ifndef bsp_h
#define bsp_h

namespace DPP {

uint32_t const BSP_TICKS_PER_SEC = static_cast<uint32_t>(50);
uint32_t const BSP_SCREEN_WIDTH  = static_cast<uint32_t>(96);
uint32_t const BSP_SCREEN_HEIGHT = static_cast<uint32_t>(16);

void BSP_init(void);
void BSP_displayPaused(uint8_t const paused);
void BSP_displayPhilStat(uint8_t const n, char_t const *stat);
void BSP_terminate(int16_t const result);

void BSP_randomSeed(uint32_t const seed);                       // random seed
uint32_t BSP_random(void);                          // pseudo-random generator

}                                                             // namespace DPP

#endif                                                                // bsp_h
