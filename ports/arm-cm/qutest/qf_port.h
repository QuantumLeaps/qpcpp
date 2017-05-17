/// @file
/// @brief QF/C++ port for QUTEST Unit Test, ARM Cortex-M with GNU or Visual C++
/// @ingroup qutest
/// @cond
///***************************************************************************
/// Last updated for version 5.9.0
/// Last updated on  2017-05-04
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) 2005-2017 Quantum Leaps, LLC. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qf_port_h
#define qf_port_h

// QUTEST event queue and thread types
#define QF_EQUEUE_TYPE QEQueue
//#define QF_OS_OBJECT_TYPE
//#define QF_THREAD_TYPE

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE        64

// The number of system clock tick rates
#define QF_MAX_TICK_RATE     2

// QF interrupt disable/enable
#define QF_INT_DISABLE()     (++QF_intNest)
#define QF_INT_ENABLE()      (--QF_intNest)

// QF critical section
// QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy) QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)  QF_INT_ENABLE()

// QF_LOG2 not defined -- use the internal LOG2() implementation

#include "qep_port.h"  // QEP port
#include "qequeue.h"   // QUTEST port uses QEQueue event-queue
#include "qmpool.h"    // QUTEST port uses QMPool memory-pool
#include "qf.h"        // QF platform-independent public interface

namespace QP {
// interrupt nesting up-down counter
extern uint8_t volatile QF_intNest;
}

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    /* QUTEST scheduler locking (not used) */
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) ((void)0)
    #define QF_SCHED_UNLOCK_()    ((void)0)

    /* QUTEST active object event queue customization (not used) */
    #define QACTIVE_EQUEUE_WAIT_(me_)   ((void)0)
    #define QACTIVE_EQUEUE_SIGNAL_(me_) ((void)0)

    // QUTEST-specific event pool operations
    #define QF_EPOOL_TYPE_  QMPool

    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))

    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_)     ((p_).put(e_))

#endif // QP_IMPL

//****************************************************************************
// NOTE1:
// This QF "port" provides dummy declaration for the QF stub that provides
// empty definitions of the QF facilities.
//

#endif // qf_port_h
