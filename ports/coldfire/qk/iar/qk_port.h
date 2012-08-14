//////////////////////////////////////////////////////////////////////////////
// Product: QK/C++ port, ColdFire, QK kernel, IAR Compiler
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

                                                // QK interrupt entry and exit
#define QK_ISR_ENTRY(level_) do { \
    ++QK_intNest_; \
    __set_status_register(__get_status_register() \
                          & (0xF8FF | ((level_) << 8))); \
} while (0)

#define QK_ISR_EXIT()  do { \
    __asm("move.w #0x2700,sr"); \
    --QK_intNest_; \
    if (QK_intNest_ == (uint8_t)0) { \
        QK_schedule_(0x2000); \
    } \
} while (0)

                                  // allow using the QK priority ceiling mutex
#define QK_MUTEX  1

#include "qk.h"                    // QK platform-independent public interface

#endif                                                            // qk_port_h
