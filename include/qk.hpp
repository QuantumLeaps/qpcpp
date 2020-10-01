/// @file
/// @brief QK/C++ platform-independent public interface.
/// @ingroup qk
/// @cond
///***************************************************************************
/// Last updated for version 6.9.0
/// Last updated on  2020-08-11
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef QK_HPP
#define QK_HPP

#include "qequeue.hpp" // QK kernel uses the native QF event queue
#include "qmpool.hpp"  // QK kernel uses the native QF memory pool
#include "qpset.hpp"   // QK kernel uses the native QF priority set


//****************************************************************************
// QF configuration for QK -- data members of the QActive class...

// QK event-queue used for AOs
#define QF_EQUEUE_TYPE      QEQueue

// QK thread type used for AOs
// QK uses this member to store the private Thread-Local Storage pointer.
//
#define QF_THREAD_TYPE      void*


//****************************************************************************
namespace QP {
    class QActive; // forward declaration
} // namespace QP

//! attributes of the QK kernel (in C for easy access in assembly)
extern "C" {

struct QK_Attr {
    std::uint8_t volatile actPrio;    //!< prio of the active AO
    std::uint8_t volatile nextPrio;   //!< prio of the next AO to execute
    std::uint8_t volatile lockPrio;   //!< lock prio (0 == no-lock)
    std::uint8_t volatile lockHolder; //!< prio of the lock holder
    std::uint8_t volatile intNest;    //!< ISR nesting level
    QP::QPSet readySet; //!< QK ready-set of AOs and "naked" threads
};

//! global attributes of the QK kernel
extern QK_Attr QK_attr_;

//! QK scheduler finds the highest-priority thread ready to run
std::uint_fast8_t QK_sched_(void) noexcept;

//! QK activator activates the next active object. The activated AO preempts
// the currently executing AOs.
void QK_activate_(void) noexcept;

#ifdef QK_ON_CONTEXT_SW

    //! QK context switch callback (customized in BSPs for QK)
    ///
    /// @description
    /// This callback function provides a mechanism to perform additional
    /// custom operations when QK switches context from one thread to
    /// another.
    ///
    /// @param[in] prev   pointer to the previous thread (active object)
    ///                   (prev==0 means that @p prev was the QK idle loop)
    /// @param[in] next   pointer to the next thread (active object)
    ///                   (next==0) means that @p next is the QK idle loop)
    /// @attention
    /// QK_onContextSw() is invoked with interrupts **disabled** and must also
    /// return with interrupts **disabled**.
    ///
    /// @note
    /// This callback is enabled by defining the macro #QK_ON_CONTEXT_SW.
    ///
    /// @include qk_oncontextsw.cpp
    ///
    void QK_onContextSw(QP::QActive *prev, QP::QActive *next);

#endif // QK_ON_CONTEXT_SW

} // extern "C"


//****************************************************************************
namespace QP {

//! The scheduler lock status
using QSchedStatus = std::uint_fast16_t;

//****************************************************************************
//! QK services.
/// @description
/// This class groups together QK services. It has only static members and
/// should not be instantiated.
///
/// @note
/// The QK scheduler, QK priority, QK ready set, etc. belong conceptually
/// to the QK class (as static class members). However, to avoid potential
/// C++ name-mangling problems in assembly language, these elements are
/// defined outside of the QK class and use the extern "C" linkage.
class QK {
public:
    // QK scheduler locking...
    //! QK selective scheduler lock
    static QSchedStatus schedLock(std::uint_fast8_t const ceiling) noexcept;

    //! QK selective scheduler unlock
    static void schedUnlock(QSchedStatus const stat) noexcept;

    //! QK idle callback (customized in BSPs for QK)
    /// @description
    /// QP::QK::onIdle() is called continously by the QK idle loop. This
    /// callback gives the application an opportunity to enter a power-saving
    /// CPU mode, or perform some other idle processing.
    ///
    /// @note
    /// QP::QK::onIdle() is invoked with interrupts enabled and must
    /// also return with interrupts enabled.
    ///
    /// @sa QP::QF::onIdle()
    static void onIdle(void);

    //! get the current QK version number string of the form X.Y.Z
    static char_t const *getVersion(void) noexcept {
        return versionStr;
    }
};

} // namespace QP


//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    #ifndef QK_ISR_CONTEXT_
        //! Internal port-specific macro that reports the execution context
        // (ISR vs. thread).
        /// @returns true if the code executes in the ISR context and false
        /// otherwise
        #define QK_ISR_CONTEXT_()  (QK_attr_.intNest != 0U)
    #endif // QK_ISR_CONTEXT_

    // QK-specific scheduler locking
    //! Internal macro to represent the scheduler lock status
    // that needs to be preserved to allow nesting of locks.
    //
    #define QF_SCHED_STAT_ QSchedStatus lockStat_;

    //! Internal macro for selective scheduler locking.
    #define QF_SCHED_LOCK_(prio_) do {          \
        if (QK_ISR_CONTEXT_()) {                \
            lockStat_ = 0xFFU;                  \
        } else {                                \
            lockStat_ = QK::schedLock((prio_)); \
        }                                       \
    } while (false)

    //! Internal macro for selective scheduler unlocking.
    #define QF_SCHED_UNLOCK_() do {     \
        if (lockStat_ != 0xFFU) {       \
            QK::schedUnlock(lockStat_); \
        }                               \
    } while (false)

    // QK-specific native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT_ID(110, (me_)->m_eQueue.m_frontEvt != nullptr)

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do {                \
        QK_attr_.readySet.insert(                           \
            static_cast<std::uint_fast8_t>((me_)->m_prio)); \
        if (!QK_ISR_CONTEXT_()) {                           \
            if (QK_sched_() != 0U) {                        \
                QK_activate_();                             \
            }                                               \
        }                                                   \
    } while (false)

    // QK-specific native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

#endif // QP_IMPL

#endif // QK_HPP

