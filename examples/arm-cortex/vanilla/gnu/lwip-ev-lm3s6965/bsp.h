//////////////////////////////////////////////////////////////////////////////
// Product: BSP for DPP application with lwIP on EV-LM3S9665 board
// Last Updated for Version: 4.0.03
// Date of the Last Update:  Mar 24, 2009
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
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

#define BSP_TICKS_PER_SEC    100

void BSP_init(void);

                                 // RITEK 128x96x4 OLED used in Rev C-D boards
#define RITEK_OLED
                                 // OSRAM 128x64x4 OLED used in REV A-B boards
//#define OSRAM_OLED

#ifdef __cplusplus
extern "C" {
#endif

void ISR_SysTick(void);
void ISR_Ethernet(void);
void ISR_Nmi(void);
void ISR_Fault(void);
void ISR_DefaultHandler(void);

#ifdef __cplusplus
}
#endif

#endif                                                                // bsp_h
