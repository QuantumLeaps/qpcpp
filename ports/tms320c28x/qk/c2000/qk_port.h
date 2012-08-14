//////////////////////////////////////////////////////////////////////////////
// Product: QK/C++ port TMS320C28x, TI-C2000 compiler
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
                                        // nesting of ISRs allowed, see NOTE01
#define QF_ISR_NEST

#ifdef QF_ISR_NEST
                                 // ISR entry and exit code for QK, see NOTE02
    #define QK_ISR_ENTRY(tempPIEIER_, gnum_, inum_) do { \
        ++QK_intNest_; \
        IER |= M_INT##gnum_; \
        IER &= MINT##gnum_; \
        (tempPIEIER_) = PieCtrlRegs.PIEIER##gnum_.all; \
        PieCtrlRegs.PIEIER##gnum_.all &= MG##gnum_##inum_; \
        PieCtrlRegs.PIEACK.all = 0xFFFF; \
        QF_INT_UNLOCK(dummy); \
    } while (0)

    #define QK_ISR_EXIT(tempPIEIER_, gnum_)  do { \
        QF_INT_LOCK(dummy); \
        PieCtrlRegs.PIEIER##gnum_.all = (tempPIEIER_); \
        --QK_intNest_; \
        if (QK_intNest_ == (uint8_t)0) { \
            QK_schedule_(); \
        } \
    } while (0)

#else                       // when nesting of ISRs is NOT allowed, see NOTE03

    #define QK_ISR_ENTRY()  do { \
        ++QK_intNest_; \
        IER = 0x0000; \
        QF_INT_UNLOCK(dummy); \
    } while (0)

    #define QK_ISR_EXIT()  do { \
        QF_INT_LOCK(dummy); \
        PieCtrlRegs.PIEACK.all = 0xFFFF; \
        --QK_intNest_; \
        if (QK_intNest_ == (uint8_t)0) { \
            QK_schedule_(); \
        } \
    } while (0)

#endif                                                          // QF_ISR_NEST

#include "qk.h"                    // QK platform-independent public interface

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// By default, this port allows nesting of ISRs. However, you can disallow
// ISR nesting by commening out the definition of the macro QF_ISR_NEST.
//
// NOTE02:
// The TMS320C28x automatically disables interrupts upon the entry to an ISR
// by setting the INTM mask. This means that the whole body of an ISR is a
// critical section. To avoid nesting of critical sections inside ISRs (so tha
// you can call QF functions), you need to clear the INTM mask. This does not
// mean, however, that you must allow interrupt nesting.
//
// By defining the macro QF_ISR_NEST (see NOTE03) you allow interrupt nesting.
// You need to call the macro QF_ISR_ENTRY(tempPIEIER_, gnum_, inum_) upon the
// entry to every ISR and QF_ISR_EXIT(tempPIEIER_, gnum_) upon the exit from
// every ISR. These macros are defined to allow prioritization of interrupts
// multiplexed in the PIE, as described in the Texas Instruments Application
// Note "Software Prioritized Interrupts"
//
// NOTE03:
// Alternativey, when you disallow ISR nesting, you need use a dfferent
// version of the QF_ISR_ENTRY()/QF_ISR_EXIT() macros, becasue you still must
// prevent nesting of critical sections inside ISRs. In this case the
// QF_ISR_ENTRY()/QF_ISR_EXIT() macros don't take any parameters and use
// the IER register to disable interrupts before clearing the INTM mask.
//

#endif                                                            // qk_port_h

