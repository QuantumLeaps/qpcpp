//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port TMS320C28x, TI-C2000 compiler
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
#ifndef qf_port_h
#define qf_port_h

        // The maximum number of active objects in the application, see NOTE01
#define QF_MAX_ACTIVE               8

#define QF_EVENT_SIZ_SIZE           2
#define QF_EQUEUE_CTR_SIZE          2
#define QF_MPOOL_SIZ_SIZE           2
#define QF_MPOOL_CTR_SIZE           2
#define QF_TIMEEVT_CTR_SIZE         2

                                             // QF critical section entry/exit
// QF_INT_KEY_TYPE not defined                                   // see NOTE02
#define QF_INT_LOCK(key_)           asm(" setc INTM")
#define QF_INT_UNLOCK(key_)         asm(" clrc INTM")

                                     // nesting of ISRs is allowed, see NOTE03
#define QF_ISR_NEST

#ifdef QF_ISR_NEST
                                        // ISR entry and exit code, see NOTE04
    #define QF_ISR_ENTRY(tempPIEIER_, gnum_, inum_) do { \
        IER |= M_INT##gnum_; \
        IER &= MINT##gnum_; \
        (tempPIEIER_) = PieCtrlRegs.PIEIER##gnum_.all; \
        PieCtrlRegs.PIEIER##gnum_.all &= MG##gnum_##inum_; \
        PieCtrlRegs.PIEACK.all = 0xFFFF; \
        QF_INT_UNLOCK(dummy); \
    } while (0)

    #define QF_ISR_EXIT(tempPIEIER_, gnum_)  do { \
        QF_INT_LOCK(dummy); \
        PieCtrlRegs.PIEIER##gnum_.all = (tempPIEIER_); \
    } while (0)

#else                       // when nesting of ISRs is NOT allowed, see NOTE04

    #define QF_ISR_ENTRY()  do { \
        IER = 0x0000; \
        QF_INT_UNLOCK(dummy); \
    } while (0)

    #define QF_ISR_EXIT()  do { \
        QF_INT_LOCK(dummy); \
        PieCtrlRegs.PIEACK.all = 0xFFFF; \
    } while (0)

#endif                                                          // QF_ISR_NEST

#include "qep_port.h"                                              // QEP port
#include "qvanilla.h"                          // "Vanilla" cooperative kernel
#include "qf.h"                    // QF platform-independent public interface

void QF_zero(void);                              // zero the .bss QF variables
void bzero(uint8_t *ptr, uint16_t len);       // helper to clear other objects

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The maximum number of active objects QF_MAX_ACTIVE could be increased to
// 63 inclusive. Here, the lower limit is used only to save some RAM.
//
//
// NOTE02:
// The QF_INT_KEY_TYPE macro is not defined, which means that this QP port
// uses the policy of "unconditional interrupt locking and unlocking".
// This interrupt locking policy does NOT allow nesting of critical sections.
//
// NOTE03:
// By default, this port allows nesting of ISRs. However, you can disallow
// ISR nesting by commening out the definition of the macro QF_ISR_NEST.
//
//
// NOTE04:
// The TMS320C28x automatically disables interrupts upon the entry to an ISR
// by setting the INTM mask. This means that the whole body of an ISR is a
// critical section. To avoid nesting of critical sections inside ISRs (so
// that you can call QF functions), you need to clear the INTM mask. This does
// not mean, however, that you must allow interrupt nesting.
//
// By defining the macro QF_ISR_NEST (see NOTE03) you allow interrupt nesting.
// You need to call the macro QF_ISR_ENTRY(tempPIEIER_, gnum_, inum_) upon the
// entry to every ISR and QF_ISR_EXIT(tempPIEIER_, gnum_) upon the exit from
// every ISR. These macros are defined to allow prioritization of interrupts
// multiplexed in the PIE, as described in the Texas Instruments Application
// Note "Software Prioritized Interrupts"
//
//
// NOTE05:
// Alternativey, when you disallow ISR nesting, you need use a dfferent
// version of the QF_ISR_ENTRY()/QF_ISR_EXIT() macros, becasue you still must
// prevent nesting of critical sections inside ISRs. In this case the
// QF_ISR_ENTRY()/QF_ISR_EXIT() macros don't take any parameters and use
// the IER register to disable interrupts before clearing the INTM mask.
///

#endif                                                            // qf_port_h
