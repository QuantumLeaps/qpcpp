//****************************************************************************
// Product: QF/C++ port to embOS
// Last updated for version 5.3.0
// Last updated on  2014-06-27
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, www.state-machine.com.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#ifndef qf_port_h
#define qf_port_h

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE         32

// embOS message mailbox and thread types
#define QF_EQUEUE_TYPE        OS_MAILBOX
#define QF_THREAD_TYPE        OS_TASK
#define QF_OS_OBJECT_TYPE     uint_fast8_t

// QF critical section for embOS, see NOTE2
//#define QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy)  OS_IncDI()
#define QF_CRIT_EXIT(dummy)   OS_DecRI()

#define QF_TASK_USES_FPU      (static_cast<uint_fast8_t>(1))

#include "RTOS.h"      // embOS API

#include "qep_port.h"  // QEP port
#include "qequeue.h"   // used for event deferral
#include "qmpool.h"    // this QP port uses the native QF memory pool
#include "qpset.h"     // native QF set for publish/subscribe
#include "qf.h"        // QF platform-independent public interface


//****************************************************************************
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL

    // native QF event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_fast16_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif // ifdef QP_IMPL

//****************************************************************************
// NOTE1:
// The maximum number of active objects in QP can be increased to 63,
// inclusive, but it can be reduced to save some memory. Also, the number of
// active objects cannot exceed the number of embOS thread priorities,
// because each QP active object requires a unique priority level.
//
// NOTE2:
// The embOS critical section does not use the status variable, because the
// implementation uses a global up-down counter, which allows the embOS
// critical section to nest. Nesting of critical sections is needed in this
// QP-embOS port.
//

#endif // qf_port_h
