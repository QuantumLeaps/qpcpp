//////////////////////////////////////////////////////////////////////////////
// Product: QK/C++ port, AVR, GNU compiler
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
ISR(name_) { \
void name_##_ISR(void); \
    uint8_t pmic_sta; \
    void name_##_ISR(void); \
    QK_intNest_ = 1; \
    name_##_ISR(); \
    cli(); \
    pmic_sta = PMIC.STATUS; \
    if ((pmic_sta == 0x01) || (pmic_sta == 0x02) || (pmic_sta == 0x04)) { \
        QK_intNest_ = 0; \
        if (QK_readySet_.notEmpty()) { \
            QK_end_of_interrupt(); \
            QK_schedule_(); \
            __asm__ __volatile__ ("sei\n\t" \
                  "pop r31\n\t" \
                  "pop r30\n\t" \
                  "pop r27\n\t" \
                  "pop r26\n\t" \
                  "pop r25\n\t" \
                  "pop r24\n\t" \
                  "pop r23\n\t" \
                  "pop r22\n\t" \
                  "pop r21\n\t" \
                  "pop r20\n\t" \
                  "pop r19\n\t" \
                  "pop r18\n\t" \
                  "pop r0\n\t"  \
                  "out 0x3b,r0\n\t" \
                  "pop r0\n\t" \
                  "out 0x39,r0\n\t" \
                  "pop r0\n\t" \
                  "out 0x38,r0\n\t" \
                  "pop r0\n\t" \
                  "out 0x3f,r0\n\t" \
                  "pop r0\n\t" \
                  "pop r1\n\t" \
                  "ret"); \
        } \
    } \
    else { \
        sei(); \
    } \
} \
void name_##_ISR()

extern "C" void QK_end_of_interrupt(void);

#include "qk.h"                    // QK platform-independent public interface

#endif                                                            // qk_port_h
