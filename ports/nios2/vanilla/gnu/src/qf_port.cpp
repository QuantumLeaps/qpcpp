//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port to Altera Nios II, Vanilla kernel
// Last Updated for Version: 4.1.02
// Date of the Last Update:  Feb 10, 2010
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2010 Quantum Leaps, LLC. All rights reserved.
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

#include "qf_port.h"                                        // QF port include

//............................................................................
extern "C" void QF_tick_(void) {
       /* enable lower-priority interrupts if the SYS_CLK_TIMER_IRQ is not
       * already the highest-priority IRQ in the system */
#if !defined(ALT_ENHANCED_INTERRUPT_API_PRESENT) && (SYS_CLK_TIMER_IRQ > 0)
    alt_u32 prio_mask = alt_irq_interruptible(SYS_CLK_TIMER_IRQ);
#endif

    QF::tick();

#if !defined(ALT_ENHANCED_INTERRUPT_API_PRESENT) && (SYS_CLK_TIMER_IRQ > 0)
    alt_irq_non_interruptible(prio_mask);
#endif
}
