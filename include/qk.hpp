//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QK/C++ platform-independent public interface.

#ifndef QK_HPP
#define QK_HPP

#include "qequeue.hpp" // QK kernel uses the native QF event queue
#include "qmpool.hpp"  // QK kernel uses the native QF memory pool
#include "qpset.hpp"   // QK kernel uses the native QF priority set


//============================================================================
// QF configuration for QK -- data members of the QActive class...

// QK event-queue used for AOs
#define QF_EQUEUE_TYPE  QEQueue

// QK thread type used for AOs
// QK uses this member to store the private Thread-Local Storage pointer.
//
#define QF_THREAD_TYPE  void*


//============================================================================
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

//! global attributes of the QK kernel (extern "C" to be accessible from C)
extern QK_Attr QK_attr_;

//! QK scheduler finds the highest-priority thread ready to run
//!
//! @description
//! The QK scheduler finds out the priority of the highest-priority AO
//! that (1) has events to process and (2) has priority that is above the
//! current priority.
//!
//! @returns the 1-based priority of the the active object, or zero if
//! no eligible active object is ready to run.
//!
//! @attention
//! QK_sched_() must be always called with interrupts **disabled** and
//! returns with interrupts **disabled**.
//!
std::uint_fast8_t QK_sched_(void) noexcept;

//! QK activator activates the next active object. The activated AO preempts
//! the currently executing AOs
//!
//! @description
//! QK_activate_() activates ready-to run AOs that are above the initial
//! active priority (QK_attr_.actPrio).
//!
//! @note
//! The activator might enable interrupts internally, but always returns with
//! interrupts **disabled**.
//!
void QK_activate_(void) noexcept;

#ifdef QK_ON_CONTEXT_SW

    //! QK context switch callback (customized in BSPs for QK)
    //!
    //! @description
    //! This callback function provides a mechanism to perform additional
    //! custom operations when QK switches context from one thread to
    //! another.
    //!
    //! @param[in] prev   pointer to the previous thread (active object)
    //!                   (prev==0 means that @p prev was the QK idle loop)
    //! @param[in] next   pointer to the next thread (active object)
    //!                   (next==0) means that @p next is the QK idle loop)
    //! @attention
    //! QK_onContextSw() is invoked with interrupts **disabled** and must also
    //! return with interrupts **disabled**.
    //!
    //! @note
    //! This callback is enabled by defining the macro #QK_ON_CONTEXT_SW.
    //!
    //! @include qk_oncontextsw.cpp
    //!
    void QK_onContextSw(QP::QActive *prev, QP::QActive *next);

#endif // QK_ON_CONTEXT_SW

} // extern "C"

//============================================================================
namespace QP {

//! The scheduler lock status
using QSchedStatus = std::uint_fast16_t;

//============================================================================
//! QK preemptive run-to-completion (non-blocking) kernel
//!
//! @description
//! This class groups together QK services. It has only static members and
//! should not be instantiated.
//!
//! @note
//! The QK scheduler, QK priority, QK ready set, etc. belong conceptually
//! to the QK class (as static class members). However, to avoid potential
//! C++ name-mangling problems in assembly language, these elements are
//! defined outside of the QK class and use the extern "C" linkage.
class QK {
public:
    // QK scheduler locking...
    //! QK selective scheduler lock
    //!
    //! @description
    //! This function locks the QK scheduler to the specified ceiling.
    //!
    //! @param[in]   ceiling    priority ceiling to which the QK scheduler
    //!                         needs to be locked
    //!
    //! @returns
    //! The previous QK Scheduler lock status, which is to be used to unlock
    //! the scheduler by restoring its previous lock status in
    //! QP::QK::schedUnlock().
    //!
    //! @note
    //! QP::QK::schedLock() must be always followed by the corresponding
    //! QP::QK::schedUnlock().
    //!
    //! @sa QK_schedUnlock()
    //!
    //! @usage
    //! The following example shows how to lock and unlock the QK scheduler:
    //! @include qk_lock.cpp
    //!
    static QSchedStatus schedLock(std::uint_fast8_t const ceiling) noexcept;

    //! QK selective scheduler unlock
    //!
    //! @description
    //! This function unlocks the QK scheduler to the previous status.
    //!
    //! @param[in]   stat       previous QK Scheduler lock status returned from
    //!                         QP::QK::schedLock()
    //! @note
    //! QP::QK::schedUnlock() must always follow the corresponding
    //! QP::QK::schedLock().
    //!
    //! @sa QP::QK::schedLock()
    //!
    //! @usage
    //! The following example shows how to lock and unlock the QK scheduler:
    //! @include qk_lock.cpp
    //!
    static void schedUnlock(QSchedStatus const stat) noexcept;

    //! QK idle callback (customized in BSPs for QK)
    //!
    //! @description
    //! QP::QK::onIdle() is called continously by the QK idle loop. This
    //! callback gives the application an opportunity to enter a power-saving
    //! CPU mode, or perform some other idle processing.
    //!
    //! @note
    //! QP::QK::onIdle() is invoked with interrupts enabled and must
    //! also return with interrupts enabled.
    //!
    //! @sa QP::QF::onIdle()
    static void onIdle(void);

    //! get the current QK version number string of the form X.Y.Z
    static char const *getVersion(void) noexcept {
        return versionStr;
    }
};

} // namespace QP

//============================================================================
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    #ifndef QK_ISR_CONTEXT_
        //! Internal port-specific macro that reports the execution context
        // (ISR vs. thread).
        //! @returns true if the code executes in the ISR context and false
        //! otherwise
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
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).m_blockSize)
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

#endif // QP_IMPL

#endif // QK_HPP
