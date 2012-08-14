//////////////////////////////////////////////////////////////////////////////
// Product: QK/C++ port, AVR, IAR compiler
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
// by the Free Software Foundation, either version 2 of the License, or
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#ifndef qk_port_h
#define qk_port_h

                                               // QK interrupt service routine
#define QK_ISR(name_) \
__interrupt void name_(void) { \
    uint8_t pmic_sta; \
    void name_##_ISR(void); \
    QK_intNest_ = 1; \
    name_##_ISR(); \
    __disable_interrupt(); \
    pmic_sta = PMIC.STATUS; \
    if ((pmic_sta == 0x01) || (pmic_sta == 0x02) || (pmic_sta == 0x04)) { \
        QK_intNest_ = 0; \
        if (QK_readySet_.notEmpty()) { \
            QK_end_of_interrupt(); \
            QK_schedule_(); \
            __asm("sei\n" \
                  "out 0x3C,r4\n" \
                  "out 0x3B,r27\n" \
                  "out 0x39,r26\n" \
                  "out 0x38,r25\n" \
                  "out 0x3F,r24\n" \
                  "ld  r16,y+\n" \
                  "ld  r17,y+\n" \
                  "ld  r18,y+\n" \
                  "ld  r19,y+\n" \
                  "ld  r20,y+\n" \
                  "ld  r21,y+\n" \
                  "ld  r22,y+\n" \
                  "ld  r23,y+\n" \
                  "ld  r0,y+\n" \
                  "ld  r1,y+\n" \
                  "ld  r2,y+\n" \
                  "ld  r3,y+\n" \
                  "ld  r30,y+\n" \
                  "ld  r31,y+\n" \
                  "ld  r24,y+\n" \
                  "ld  r25,y+\n" \
                  "ld  r26,y+\n" \
                  "ld  r27,y+\n" \
                  "ld  r4,y+\n" \
                  "ret"); \
        } \
    } \
    else { \
        __enable_interrupt(); \
    } \
} \
void name_##_ISR()

extern "C" void QK_end_of_interrupt  (void);

#include "qk.h"                    // QK platform-independent public interface

#endif                                                            // qk_port_h
