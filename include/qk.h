/// @file
/// @brief QK/C++ platform-independent public interface.
/// @ingroup qk
/// @cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2014-05-08
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qk_h
#define qk_h

#include "qequeue.h" // QK kernel uses the native QF event queue
#include "qmpool.h"  // QK kernel uses the native QF memory pool
#include "qpset.h"   // QK kernel uses the native QF priority set


//****************************************************************************
// QF configuration for QK

//! This macro defines the type of the event queue used for active objects.
/// @note
/// This is just an example of the macro definition. Typically, you need
/// to define it in the specific QF port file (qf_port.h). In case of QK,
/// which always depends on the native QF queue, this macro is defined at the
/// level of the platform-independent interface qk.h.
#define QF_EQUEUE_TYPE             QEQueue

#if defined(QK_TLS)
    #define QF_OS_OBJECT_TYPE      uint8_t
    #define QF_THREAD_TYPE         void *
#endif  // QK_TLS

//****************************************************************************
namespace QP {

#ifndef QK_NO_MUTEX
    //! QK Mutex type.
    /// @description
    /// QMutex represents the priority-ceiling mutex available in QK.
    /// @sa QP::QK::mutexLock() QP::QK::mutexUnlock()
    typedef uint_fast8_t QMutex;
#endif  // QK_NO_MUTEX

//****************************************************************************
//! QK services.
/// @description
/// This class groups together QK services. It has only static members and
/// should not be instantiated.
///
// @note The QK scheduler, QK priority, QK ready set, etc. belong conceptually
/// to the QK class (as static class members). However, to avoid C++ potential
/// name-mangling problems in assembly language, these elements are defined
/// outside of the QK class and use the extern "C" linkage specification.
class QK {
public:

    //! get the current QK version number string of the form X.Y.Z
    static char_t const Q_ROM *getVersion(void) {
        return versionStr;
    }

    //! QK idle callback (customized in BSPs for QK)
    /// @description
    /// QP::QK::onIdle() is called continously by the QK idle loop. This
    /// callback gives the application an opportunity to enter a power-saving
    /// CPU mode, or perform some other idle processing.
    ///
    /// @note QP::QK::onIdle() is invoked with interrupts enabled and must
    /// also return with interrupts enabled.
    ///
    /// @sa QP::QF::onIdle()
    static void onIdle(void);

#ifndef QK_NO_MUTEX

    //! QK priority-ceiling mutex lock
    static QMutex mutexLock(uint_fast8_t const prioCeiling);

    //! QK priority-ceiling mutex unlock
    static void mutexUnlock(QMutex const mutex);

#endif // QK_NO_MUTEX

};

} // namespace QP

//****************************************************************************
extern "C" {

//! QK initialization
void QK_init(void);

//! The QK scheduler
void QK_sched_(uint_fast8_t p);

//! Find the highest-priority task ready to run
uint_fast8_t QK_schedPrio_(void);

#if (QF_MAX_ACTIVE <= 8)
    extern QP::QPSet8  QK_readySet_;  //!< ready set of AOs
#else
    extern QP::QPSet64 QK_readySet_;  //!< ready set of AOs
#endif

extern uint_fast8_t volatile QK_currPrio_; //!< current task/ISR priority
extern uint_fast8_t volatile QK_intNest_;  //!< interrupt nesting level

} // extern "C"

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL
    #ifndef QK_ISR_CONTEXT_
        //! Internal port-specific macro that reports the execution context
        // (ISR vs. thread).
        /// @returns true if the code executes in the ISR context and false
        /// otherwise
        #define QK_ISR_CONTEXT_() (QK_intNest_ != static_cast<uint_fast8_t>(0))
    #endif // QK_ISR_CONTEXT_

    // native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT_ID(0, (me_)->m_eQueue.m_frontEvt != static_cast<QEvt const *>(0))

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
        QK_readySet_.insert((me_)->m_prio); \
        if (!QK_ISR_CONTEXT_()) { \
            uint_fast8_t p = QK_schedPrio_(); \
            if (p != static_cast<uint_fast8_t>(0)) { \
                QK_sched_(p); \
            } \
        } \
    } while (false)
    #define QACTIVE_EQUEUE_ONEMPTY_(me_) \
        QK_readySet_.remove((me_)->m_prio)

    // native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_fast16_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif // QP_IMPL

#endif // qk_h
