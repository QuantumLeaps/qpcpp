//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 19, 2012
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
#ifndef qvanilla_h
#define qvanilla_h

/// \file
/// \ingroup qf
/// \brief platform-independent interface to the cooperative "vanilla" kernel.

#include "qequeue.h"       // "Vanilla" kernel uses the native QF event queue
#include "qmpool.h"        // "Vanilla" kernel uses the native QF memory pool
#include "qpset.h"         // "Vanilla" kernel uses the native QF priority set

QP_BEGIN_
                              // the event queue type for the "Vanilla" kernel
#define QF_EQUEUE_TYPE     QEQueue
                                              // native event queue operations
#define QACTIVE_EQUEUE_WAIT_(me_) \
    Q_ASSERT((me_)->m_eQueue.m_frontEvt != static_cast<QEvt const *>(0))

#define QACTIVE_EQUEUE_SIGNAL_(me_) \
    (QF_readySet_.insert((me_)->m_prio))

#define QACTIVE_EQUEUE_ONEMPTY_(me_) \
    (QF_readySet_.remove((me_)->m_prio))

                                            // native QF event pool operations
#define QF_EPOOL_TYPE_           QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (p_).init((poolSto_), (poolSize_), static_cast<QMPoolSize>(evtSize_))
#define QF_EPOOL_EVENT_SIZE_(p_) \
    static_cast<uint32_t>((p_).getBlockSize())
#define QF_EPOOL_GET_(p_, e_)    ((e_) = static_cast<QEvt *>((p_).get()))
#define QF_EPOOL_PUT_(p_, e_)    ((p_).put(e_))

QP_END_

#if (QF_MAX_ACTIVE <= 8)
    extern "C" QP_ QPSet8  QF_readySet_;      ///< ready set of active objects
#else
    extern "C" QP_ QPSet64 QF_readySet_;      ///< ready set of active objects
#endif

extern "C" uint8_t QF_currPrio_;          ///< current task/interrupt priority
extern "C" uint8_t QF_intNest_;                   ///< interrupt nesting level

#endif                                                           // qvanilla_h
