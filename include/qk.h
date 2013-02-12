//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++
// Last Updated for Version: 4.5.04
// Date of the Last Update:  Jan 16, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
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
#ifndef qk_h
#define qk_h

/// \file
/// \ingroup qep qf qk qs
/// \brief QK/C++ platform-independent public interface.
///
/// This header file must be included directly or indirectly
/// in all modules (*.cpp files) that use QK/C++.

#include "qequeue.h"          // The QK kernel uses the native QF event queue
#include "qmpool.h"           // The QK kernel uses the native QF memory pool
#include "qpset.h"            // The QK kernel uses the native QF priority set

//////////////////////////////////////////////////////////////////////////////
// QF configuration for QK

/// \brief This macro defines the type of the event queue used for the
/// active objects.
///
/// \note This is just an example of the macro definition. Typically, you need
/// to define it in the specific QF port file (qf_port.h). In case of QK,
/// which always depends on the native QF queue, this macro is defined at the
/// level of the platform-independent interface qk.h.
#define QF_EQUEUE_TYPE             QEQueue

#if defined(QK_TLS) || defined(QK_EXT_SAVE)
    /// \brief This macro defines the type of the OS-Object used for blocking
    /// the native QF event queue when the queue is empty
    ///
    /// In QK, the OS object is used to hold the per-thread flags, which might
    /// be used, for example, to remember the thread attributes (e.g.,
    /// if the thread uses a floating point co-processor). The OS object value
    /// is set on per-thread basis in QActive::start(). Later, the extended
    /// context switch macros (QK_EXT_SAVE() and QK_EXT_RESTORE()) might use
    /// the per-thread flags to determine what kind of extended context switch
    /// this particular thread needs (e.g., the thread might not be using the
    /// coprocessor or might be using a different one).
    #define QF_OS_OBJECT_TYPE      uint8_t

    /// \brief This macro defines the type of the thread handle used for the
    /// active objects.
    ///
    /// The thread type in QK is the pointer to the thread-local storage (TLS)
    /// This thread-local storage can be set on per-thread basis in
    /// QActive::start(). Later, the QK scheduler, passes the pointer to the
    /// thread-local storage to the macro #QK_TLS.
    #define QF_THREAD_TYPE         void *
#endif                                                // QK_TLS || QK_EXT_SAVE

#if (QF_MAX_ACTIVE <= 8)
    extern "C" QP_ QPSet8  QK_readySet_;                  ///< ready set of QK
#else
    extern "C" QP_ QPSet64 QK_readySet_;                  ///< ready set of QK
#endif

extern "C" uint8_t QK_currPrio_;          ///< current task/interrupt priority
extern "C" uint8_t QK_intNest_;                   ///< interrupt nesting level

// QK active object queue implementation .....................................

/// \brief Platform-dependent macro defining how QF should block the calling
/// task when the QF native queue is empty
///
/// \note This is just an example of #QACTIVE_EQUEUE_WAIT_ for the QK-port
/// of QF. QK never activates a task that has no events to process, so in this
/// case the macro asserts that the queue is not empty. In other QF ports you
// need to define the macro appropriately for the underlying kernel/OS you're
/// using.
#define QACTIVE_EQUEUE_WAIT_(me_) \
    Q_ASSERT((me_)->m_eQueue.m_frontEvt != static_cast<QEvt const *>(0))

/// \brief Platform-dependent macro defining how QF should signal the
/// active object task that an event has just arrived.
///
/// The macro is necessary only when the native QF event queue is used.
/// The signaling of task involves unblocking the task if it is blocked.
///
/// \note #QACTIVE_EQUEUE_SIGNAL_ is called from a critical section.
/// It might leave the critical section internally, but must restore
/// the critical section before exiting to the caller.
///
/// \note This is just an example of #QACTIVE_EQUEUE_SIGNAL_ for the QK-port
/// of QF. In other QF ports you need to define the macro appropriately for
/// the underlying kernel/OS you're using.
#define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    QK_readySet_.insert((me_)->m_prio); \
    if (QK_intNest_ == static_cast<uint8_t>(0)) { \
        uint8_t p = QK_schedPrio_(); \
        if (p != static_cast<uint8_t>(0)) { \
            QK_sched_(p); \
        } \
    } \
} while (false)

/// \brief Platform-dependent macro defining the action QF should take
/// when the native QF event queue becomes empty.
///
/// \note #QACTIVE_EQUEUE_ONEMPTY_ is called from a critical section.
/// It should not leave the critical section.
///
/// \note This is just an example of #QACTIVE_EQUEUE_ONEMPTY_ for the QK-port
/// of QF. In other QF ports you need to define the macro appropriately for
/// the underlying kernel/OS you're using.
#define QACTIVE_EQUEUE_ONEMPTY_(me_) \
    QK_readySet_.remove((me_)->m_prio)

// QK event pool operations ..................................................

/// \brief This macro defines the type of the event pool used in this QF port.
///
/// \note This is a specific implementation for the QK-port of QF.
/// In other QF ports you need to define the macro appropriately for
/// the underlying kernel/OS you're using.
#define QF_EPOOL_TYPE_              QMPool

/// \brief Platform-dependent macro defining the event pool initialization
///
/// \note This is a specific implementation for the QK-port of QF.
/// In other QF ports you need to define the macro appropriately for
/// the underlying kernel/OS you're using.
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (p_).init((poolSto_), (poolSize_), \
        static_cast<QMPoolSize>(evtSize_))

/// \brief Platform-dependent macro defining how QF should obtain the
/// event pool block-size
///
/// \note This is a specific implementation for the QK-port of QF.
/// In other QF ports you need to define the macro appropriately for
/// the underlying kernel/OS you're using.
#define QF_EPOOL_EVENT_SIZE_(p_)    \
    static_cast<uint32_t>((p_).getBlockSize())

/// \brief Platform-dependent macro defining how QF should obtain an event
/// \a e_ from the event pool \a p_
///
/// \note This is a specific implementation for the QK-port of QF.
/// In other QF ports you need to define the macro appropriately for
/// the underlying kernel/OS you're using.
#define QF_EPOOL_GET_(p_, e_)       ((e_) = static_cast<QEvt *>((p_).get()))

/// \brief Platform-dependent macro defining how QF should return an event
/// \a e_ to the event pool \a p_
///
/// \note This is a specific implementation for the QK-port of QF.
/// In other QF ports you need to define the macro appropriately for
/// the underlying kernel/OS you're using.
#define QF_EPOOL_PUT_(p_, e_)       ((p_).put(e_))

//////////////////////////////////////////////////////////////////////////////
QP_BEGIN_

#ifndef QK_NO_MUTEX
    //////////////////////////////////////////////////////////////////////////
    /// \brief QK Mutex type.
    ///
    /// QMutex represents the priority-ceiling mutex available in QK.
    /// \sa QK::mutexLock()
    /// \sa QK::mutexUnlock()
    typedef uint8_t QMutex;
#endif                                                          // QK_NO_MUTEX

//////////////////////////////////////////////////////////////////////////////
/// \brief QK services.
///
/// This class groups together QK services. It has only static members and
/// should not be instantiated.
///
// \note The QK scheduler, QK priority, QK ready set, etc. belong conceptually
/// to the QK class (as static class members). However, to avoid C++ potential
/// name-mangling problems in assembly language, these elements are defined
/// outside of the QK class and use the extern "C" linkage specification.
///
class QK {
public:

    /// \brief get the current QK version number string
    ///
    /// \return version of the QK as a constant 6-character string of the
    /// form x.y.zz, where x is a 1-digit major version number, y is a
    /// 1-digit minor version number, and zz is a 2-digit release number.
    static char_t const Q_ROM * Q_ROM_VAR getVersion(void);

    /// \brief QK idle callback (customized in BSPs for QK)
    ///
    /// QK::onIdle() is called continously by the QK idle loop. This callback
    /// gives the application an opportunity to enter a power-saving CPU mode,
    /// or perform some other idle processing.
    ///
    /// \note QK::onIdle() is invoked with interrupts unlocked and must also
    /// return with interrupts unlocked.
    ///
    /// \sa QF::onIdle()
    static void onIdle(void);

#ifndef QK_NO_MUTEX

    /// \brief QK priority-ceiling mutex lock
    ///
    /// Lock the QK scheduler up to the priority level \a prioCeiling.
    ///
    // \note This function should be always paired with QK::mutexUnlock().
    /// The code between QK::mutexLock() and QK::mutexUnlock() should be
    /// kept to the minimum.
    ///
    /// \include qk_mux.cpp
    static QMutex mutexLock(uint8_t const prioCeiling);

    /// \brief QK priority-ceiling mutex unlock
    ///
    /// \note This function should be always paired with QK::mutexLock().
    /// The code between QK::mutexLock() and QK::mutexUnlock() should be
    /// kept to the minimum.
    ///
    /// \include qk_mux.cpp
    static void mutexUnlock(QMutex const mutex);

#endif                                                          // QK_NO_MUTEX

};

QP_END_

//////////////////////////////////////////////////////////////////////////////
extern "C" {

/// \brief QK initialization
///
/// QK_init() is called from QF_init() in qk.cpp. This function is
/// defined in the QK ports.
void QK_init(void);

/// \brief The QK scheduler
///
/// \note The QK scheduler must be always called with the interrupts
/// disabled and enables interrupts internally.
///
/// \sa QK_schedPrio_()
void QK_sched_(uint8_t p);

/// \brief The QK extended scheduler for interrupt context
///
/// \note The QK extended scheduler must be always called with the
/// interrupts disabled and enables interrupts internally.
///
/// \sa QK_schedPrio_()
void QK_schedExt_(uint8_t p);

/// \brief Find the highest-priority task ready to run
///
/// \note QK_schedPrio_() must be always called with interrupts disabled
/// and returns with interrupts disabled.
uint8_t QK_schedPrio_(void);

}                                                                // extern "C"

#endif                                                                 // qk_h
