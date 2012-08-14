//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port to Altera Nios II, QK preemptive kernel
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

                    // The maximum number of active objects in the application
#define QF_MAX_ACTIVE               63
                                                        // QF critical section
#define QF_INT_KEY_TYPE             alt_irq_context
#define QF_INT_LOCK(key_)           ((key_) = alt_irq_disable_all())
#define QF_INT_UNLOCK(key_)         alt_irq_enable_all(key_)

#include "sys/alt_irq.h"     // for alt_irq_disable_all()/alt_irq_enable_all()

#include "qep_port.h"                                              // QEP port
#include "qk_port.h"                                                // QK port
#include "qf.h"                    // QF platform-independent public interface

#endif                                                            // qf_port_h
