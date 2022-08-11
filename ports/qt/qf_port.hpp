//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-30
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QP/C++ port to Qt

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// event queue and thread types for the Qt port
#define QF_EQUEUE_TYPE         QEQueue
#define QF_OS_OBJECT_TYPE      QWaitCondition *
#define QF_THREAD_TYPE         QThread *

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE          64U

// The number of system clock tick rates
#define QF_MAX_TICK_RATE       2U

// various QF object sizes configuration for this port
#define QF_EVENT_SIZ_SIZE      4U
#define QF_EQUEUE_CTR_SIZE     4U
#define QF_MPOOL_SIZ_SIZE      4U
#define QF_MPOOL_CTR_SIZE      4U
#define QF_TIMEEVT_CTR_SIZE    4U

// QF interrupt disable/enable, see NOTE1
#define QF_INT_DISABLE()       (QP::QF_enterCriticalSection_())
#define QF_INT_ENABLE()        (QP::QF_leaveCriticalSection_())

// Qt critical section, see NOTE1
// QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy)   QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)    QF_INT_ENABLE()

class QWaitCondition;   // forward declaration
class QThread;          // forward declaration
class QMutex;           // forward declaration

#include "qep_port.hpp" // QEP port
#include "qequeue.hpp"  // Qt port uses event-queue
#include "qmpool.hpp"   // Qt port uses memory-pool
#include "qf.hpp"       // QF platform-independent public interface

#ifdef QT_GUI_LIB
    #include "guiapp.hpp"    // GUI QF application (only when needed)
    #include "guiactive.hpp" // GUI active objects (only when needed)
#else
    #include <QCoreApplication>
#endif

// fix the conflict between the Qt's Q_NORETURN and QP's Q_NORETURN...
#ifdef Q_NORETURN
#undef Q_NORETURN
#define Q_NORETURN void
#endif

namespace QP {

void QF_enterCriticalSection_(void);
void QF_leaveCriticalSection_(void);

// set Qt thread priority;
// can be called either before or after QActive::START()
//
void QF_setQtPrio(QActive *act, int_t qtPrio);

// set clock tick rate
void QF_setTickRate(uint32_t ticksPerSec);

// clock tick callback (provided in the app)
void QF_onClockTick(void);

#ifdef QP_IMPL
extern QMutex QF_qtMutex_;
#endif

#ifdef Q_SPY
void QS_onEvent(void);
#endif

} // namespace QP

//============================================================================
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    // Qt-specific scheduler locking (not used at this point)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) ((void)0)
    #define QF_SCHED_UNLOCK_()    ((void)0)

    // Qt-specific event queue customization
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        while ((me_)->m_eQueue.m_frontEvt == nullptr) \
           static_cast<QWaitCondition*>((me_)->m_osObject)->wait(&QF_qtMutex_)

    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        static_cast<QWaitCondition*>((me_)->m_osObject)->wakeOne()

    // Qt-specific event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

    #include <QMutex>
    #include <QWaitCondition>

#endif // QP_IMPL

// undefine the conflicting Q_ASSERT definition from Qt
#ifdef Q_ASSERT
    #undef Q_ASSERT
#endif

//============================================================================
// NOTE1:
// QF, like all real-time frameworks, needs to execute certain sections of
// code indivisibly to avoid data corruption. The most straightforward way of
// protecting such critical sections of code is disabling and enabling
// interrupts, which Win32 does not allow.
//
// This QF port uses therefore a single package-scope Qt QMutex object
// QF_qtMutex_ to protect all critical sections.
//

#endif // QF_PORT_HPP

