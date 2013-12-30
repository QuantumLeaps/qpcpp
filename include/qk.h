//****************************************************************************
// Product: QP/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 02, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
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


//****************************************************************************
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
    #define QF_OS_OBJECT_TYPE      uint8_t
    #define QF_THREAD_TYPE         void *
#endif                                                // QK_TLS || QK_EXT_SAVE

//****************************************************************************
namespace QP {

#ifndef QK_NO_MUTEX
    //////////////////////////////////////////////////////////////////////////
    /// \brief QK Mutex type.
    ///
    /// QMutex represents the priority-ceiling mutex available in QK.
    /// \sa QK::mutexLock()
    /// \sa QK::mutexUnlock()
    typedef uint8_t QMutex;
#endif                                                          // QK_NO_MUTEX

//****************************************************************************
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
    /// \return version of QK as a constant 5-character string of the
    /// form X.Y.Z, where X is a 1-digit major version number, Y is a
    /// 1-digit minor version number, and Z is a 1-digit release number.
    static char_t const Q_ROM *getVersion(void) {
        return QP_VERSION_STR;
    }

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

}                                                              // namespace QP

//****************************************************************************
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

#if (QF_MAX_ACTIVE <= 8)
    extern QP::QPSet8  QK_readySet_;                     ///< ready set of AOs
#else
    extern QP::QPSet64 QK_readySet_;                     ///< ready set of AOs
#endif

extern uint8_t volatile QK_currPrio_;     ///< current task/interrupt priority
extern uint_t  volatile QK_intNest_;              ///< interrupt nesting level

}                                                                // extern "C"

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef qf_pkg_h

    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT((me_)->m_eQueue.m_frontEvt != static_cast<QEvt const *>(0))
    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
        QK_readySet_.insert((me_)->m_prio); \
        if (QK_intNest_ == static_cast<uint_t>(0)) { \
            uint8_t p = QK_schedPrio_(); \
            if (p != static_cast<uint8_t>(0)) { \
                QK_sched_(p); \
            } \
        } \
    } while (false)
    #define QACTIVE_EQUEUE_ONEMPTY_(me_) \
        QK_readySet_.remove((me_)->m_prio)

                                            // native QF event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif                                                      // #ifdef qf_pkg_h

#endif                                                                 // qk_h
