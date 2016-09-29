/// @file
/// @brief QP/C++ port to Qt
/// @cond
///***************************************************************************
/// Last Updated for Version: QP 5.7.2/Qt 5.x
/// Last updated on  2016-09-28
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qf_port_h
#define qf_port_h

// event queue and thread types for the Qt port
#define QF_EQUEUE_TYPE         QEQueue
#define QF_OS_OBJECT_TYPE      QWaitCondition *
#define QF_THREAD_TYPE         QThread *

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE          63

// The number of system clock tick rates
#define QF_MAX_TICK_RATE       2

// various QF object sizes configuration for this port
#define QF_EVENT_SIZ_SIZE      4
#define QF_EQUEUE_CTR_SIZE     4
#define QF_MPOOL_SIZ_SIZE      4
#define QF_MPOOL_CTR_SIZE      4
#define QF_TIMEEVT_CTR_SIZE    4

// QF interrupt disable/enable, see NOTE1
#define QF_INT_DISABLE()       (QP::QF_enterCriticalSection_())
#define QF_INT_ENABLE()        (QP::QF_leaveCriticalSection_())

// Qt critical section, see NOTE1
// QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy)   QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)    QF_INT_ENABLE()

class QWaitCondition; // forward declaration
class QThread;        // forward declaration
class QMutex;         // forward declaration

#include "qep_port.h" // QEP port
#include "qequeue.h"  // Qt port uses event-queue
#include "qmpool.h"   // Qt port uses memory-pool
#include "qpset.h"    // Qt port uses priority set
#include "qf.h"       // QF platform-independent public interface

#ifdef QT_GUI_LIB
    #include "guiapp.h"    // GUI QF application interface (only when needed)
    #include "guiactive.h" // GUI active objects interface (only when needed)
#endif

namespace QP {

void QF_enterCriticalSection_(void);
void QF_leaveCriticalSection_(void);

// set Qt thread priority;
// can be called either before or after QMActive::START()
//
void QF_setQtPrio(QMActive *act, int_t qtPrio);

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

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    // Qt-specific scheduler locking (not used at this point)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) ((void)0)
    #define QF_SCHED_UNLOCK_()    ((void)0)

    // Qt-specific event queue customization
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        while ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt const *>(0)) \
           static_cast<QWaitCondition*>((me_)->m_osObject)->wait(&QF_qtMutex_)

    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        static_cast<QWaitCondition*>((me_)->m_osObject)->wakeOne()

    #define QACTIVE_EQUEUE_ONEMPTY_(me_) ((void)0)

    // Qt-specific event pool operations
    #define QF_EPOOL_TYPE_            QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init(poolSto_, poolSize_, evtSize_)
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_)     ((p_).put(e_))

    #include <QMutex>
    #include <QWaitCondition>

#endif // QP_IMPL

// undefine the conflicting Q_ASSERT definition from Qt
#ifdef Q_ASSERT
    #undef Q_ASSERT
#endif

//****************************************************************************
// NOTE01:
// QF, like all real-time frameworks, needs to execute certain sections of
// code indivisibly to avoid data corruption. The most straightforward way of
// protecting such critical sections of code is disabling and enabling
// interrupts, which Win32 does not allow.
//
// This QF port uses therefore a single package-scope Qt QMutex object
// QF_qtMutex_ to protect all critical sections.
//

#endif // qf_port_h
