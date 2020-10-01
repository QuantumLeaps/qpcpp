/// @file
/// @brief QXK/C++ preemptive extended (blocking) kernel, platform-independent
/// public interface.
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-15
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

#ifndef QXK_HPP
#define QXK_HPP

#include "qequeue.hpp"  // QXK kernel uses the native QF event queue
#include "qmpool.hpp"   // QXK kernel uses the native QF memory pool
#include "qpset.hpp"    // QXK kernel uses the native QF priority set

//****************************************************************************
// QF configuration for QXK -- data members of the QActive class...

// QXK event-queue used for AOs
#define QF_EQUEUE_TYPE      QEQueue

// QXK OS-object used to store the private stack poiner for extended threads.
// (The private stack pointer is NULL for basic-threads).
//
#define QF_OS_OBJECT_TYPE   void*

// QXK thread type used to store the private Thread-Local Storage pointer.
#define QF_THREAD_TYPE      void*

//! Access Thread-Local Storage (TLS) and cast it on the given @p type_
#define QXK_TLS(type_) (static_cast<type_>(QXK_current()->m_thread))


//****************************************************************************
namespace QP {
    class QActive; // forward declaration
} // namespace QP

//****************************************************************************
extern "C" {

//! attributes of the QXK kernel
struct QXK_Attr {
    QP::QActive * volatile curr;      //!< currently executing thread
    QP::QActive * volatile next;      //!< next thread to execute
    std::uint8_t volatile actPrio;    //!< prio of the active basic thread
    std::uint8_t volatile lockPrio;   //!< lock prio (0 == no-lock)
    std::uint8_t volatile lockHolder; //!< prio of the lock holder
    std::uint8_t volatile intNest;    //!< ISR nesting level
    QP::QActive * idleThread;         //!< pointer to the idle thread
    QP::QPSet readySet;               //!< ready-set of all threads
};

//! global attributes of the QXK kernel
extern QXK_Attr QXK_attr_;

//! QXK scheduler finds the highest-priority thread ready to run
std::uint_fast8_t QXK_sched_(void) noexcept;

//! QXK activator activates the next active object. The activated AO preempts
// the currently executing AOs.
//
void QXK_activate_(void);

//! return the currently executing active-object/thread
QP::QActive *QXK_current(void) noexcept;

#ifdef QXK_ON_CONTEXT_SW

    //! QXK context switch callback (customized in BSPs for QXK)
    ///
    /// @description
    /// This callback function provides a mechanism to perform additional
    /// custom operations when QXK switches context from one thread to
    /// another.
    ///
    /// @param[in] prev   pointer to the previous thread (active object)
    ///                   (prev==0 means that @p prev was the QXK idle thread)
    /// @param[in] next   pointer to the next thread (active object)
    ///                   (next==0) means that @p next is the QXK idle thread)
    /// @attention
    /// QXK_onContextSw() is invoked with interrupts **disabled** and must also
    /// return with interrupts **disabled**.
    ///
    /// @note
    /// This callback is enabled by defining the macro #QXK_ON_CONTEXT_SW.
    ///
    /// @include qxk_oncontextsw.cpp
    ///
    void QXK_onContextSw(QP::QActive *prev, QP::QActive *next);

#endif // QXK_ON_CONTEXT_SW

} // extern "C"

//****************************************************************************
namespace QP {

//! The scheduler lock status
using QSchedStatus = std::uint_fast16_t;

//****************************************************************************
//! QXK services.
/// @description
/// This class groups together QXK services. It has only static members and
/// should not be instantiated.
///
/// @note The QXK initialization and the QXK scheduler belong conceptually
/// to the QXK class (as static class members). However, to avoid C++
/// potential name-mangling problems in assembly language, these elements
/// are defined outside of the QXK class and outside the QP namespace with
/// the extern "C" linkage specification.
class QXK {
public:
    //! QXK selective scheduler lock
    static QSchedStatus schedLock(std::uint_fast8_t const ceiling) noexcept;

    //! QXK selective scheduler unlock
    static void schedUnlock(QSchedStatus const stat) noexcept;

    //! QXK idle callback (customized in BSPs for QXK)
    /// @description
    /// QP::QXK::onIdle() is called continously by the QXK idle loop. This
    /// callback gives the application an opportunity to enter a power-saving
    /// CPU mode, or perform some other idle processing.
    ///
    /// @note QP::QXK::onIdle() is invoked with interrupts enabled and must
    /// also return with interrupts enabled.
    ///
    /// @sa QP::QF::onIdle()
    static void onIdle(void);
};

} // namespace QP


//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    #ifndef QXK_ISR_CONTEXT_
        //! Internal port-specific macro that reports the execution context
        // (ISR vs. thread).
        /// @returns true if the code executes in the ISR context and false
        /// otherwise
        #define QXK_ISR_CONTEXT_() \
            (QXK_attr_.intNest != 0U)
    #endif // QXK_ISR_CONTEXT_

    // QXK-specific scheduler locking
    //! Internal macro to represent the scheduler lock status
    // that needs to be preserved to allow nesting of locks.
    //
    #define QF_SCHED_STAT_ QSchedStatus lockStat_;

    //! Internal macro for selective scheduler locking.
    #define QF_SCHED_LOCK_(prio_) do {           \
        if (QXK_ISR_CONTEXT_()) {                \
            lockStat_ = 0xFFU;                   \
        } else {                                 \
            lockStat_ = QXK::schedLock((prio_)); \
        }                                        \
    } while (false)

    //! Internal macro for selective scheduler unlocking.
    #define QF_SCHED_UNLOCK_() do {      \
        if (lockStat_ != 0xFFU) {        \
            QXK::schedUnlock(lockStat_); \
        }                                \
    } while (false)

    // QXK-specific native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT_ID(110, (me_)->m_eQueue.m_frontEvt != nullptr)

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do {                   \
        QXK_attr_.readySet.insert(                             \
            static_cast<std::uint_fast8_t>((me_)->m_dynPrio)); \
        if (!QXK_ISR_CONTEXT_()) {                             \
            if (QXK_sched_() != 0U) {                          \
                QXK_activate_();                               \
            }                                                  \
        }                                                      \
    } while (false)

    // QXK-specific native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

#endif // QP_IMPL

#endif // QXK_HPP

