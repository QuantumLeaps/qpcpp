/// @file
/// @brief QXK/C++ preemptive extended (blocking) kernel, platform-independent
/// public interface.
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 5.6.0
/// Last updated on  2015-12-29
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps. All rights reserved.
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

#ifndef qxk_h
#define qxk_h

#include "qequeue.h"  // QXK kernel uses the native QF event queue
#include "qmpool.h"   // QXK kernel uses the native QF memory pool
#include "qpset.h"    // QXK kernel uses the native QF priority set

//****************************************************************************
// QF configuration for QXK: data members of the ::QMActive class...

//! This macro defines the type of the event queue used for the AOs
/// @description
/// QXK uses the native QF event queue QEQueue.
///
#define QF_EQUEUE_TYPE      QEQueue

//! OS-dependent per-thread operating-system object
/// @description
/// The use of this member depends on the CPU. For example, in port to
/// ARM Cortex-M with FPU this member is used to store the LR.
///
#define QF_OS_OBJECT_TYPE   void*

//! OS-dependent representation of the private thread
/// @description
/// QXK uses this member to store thread attributes.
///
#define QF_THREAD_TYPE      QXK_ThreadType

//****************************************************************************
extern "C" {

//! attributes of the QXK kernel
struct QXK_Attr {
    void *curr;  //!< currently executing thread
    void *next;  //!< next thread to execute
#ifndef QXK_ISR_CONTEXT_
    uint_fast8_t volatile intNest; //!< ISR nesting level
#endif // QXK_ISR_CONTEXT_
#if (QF_MAX_ACTIVE <= 8)
    QP::QPSet8  readySet; //!< QXK ready-set of AOs and "naked" threads
#else
    QP::QPSet64 readySet; //!< QXK ready-set of AOs and "naked" threads
#endif
};

//! global attributes of the QXK kernel
extern QXK_Attr QXK_attr_;

} // extern "C"

//****************************************************************************
namespace QP {

//****************************************************************************
//! Type of the QMActive.m_thread member for the QXK kernel
struct QXK_ThreadType {
    void *m_stack;            //!< top of the per-thread stack
    uint_fast8_t m_startPrio; //!< start priority of the thread
};

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

    //! QXK initialization
    /// @description
    /// QP::QXK::init() must be called from the application before
    /// QP::QF::run() to initialize the stack for the QXK idle thread.
    static void init(void *idleStkSto, uint_fast16_t idleStkSize);

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

    //! get the current QXK version number string of the form X.Y.Z
    static char_t const *getVersion(void) {
        return versionStr;
    }
};

//****************************************************************************
//! Priority Ceiling Mutex the QXK preemptive kernel
class QXMutex {
public:

    //! initialize the QXK priority-ceiling mutex
    void init(uint_fast8_t const prioCeiling);

    //! lock the QXK priority-ceiling mutex
    void lock(void);

    //! unlock the QXK priority-ceiling mutex
    void unlock(void);

private:
    uint_fast8_t m_prioCeiling;
    uint_fast8_t m_lockNest;
#if (QF_MAX_ACTIVE <= 8)
    QP::QPSet8  m_waitSet; //!< set of "naked" threads waiting on this mutex
#else
    QP::QPSet64 m_waitSet; //!< set of "naked" threads waiting on this mutex
#endif
};

} // namespace QP


//****************************************************************************
extern "C" {

//! The QXK scheduler for internal use only (might be in assembly)
void QXK_sched_(void);

} // extern "C"

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    #ifndef QXK_ISR_CONTEXT_
        //! Internal port-specific macro that reports the execution context
        // (ISR vs. thread).
        /// @returns true if the code executes in the ISR context and false
        /// otherwise
        #define QXK_ISR_CONTEXT_() \
            (QXK_attr_.intNest != static_cast<uint_fast8_t>(0))
    #endif // QXK_ISR_CONTEXT_

    #define QACTIVE_EQUEUE_WAIT_(me_) \
        if ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt *>(0)) { \
            QXK_attr_.readySet.remove((me_)->m_prio); \
            QXK_sched_(); \
            QF_CRIT_EXIT_(); \
            QF_CRIT_EXIT_NOP(); \
            QF_CRIT_ENTRY_(); \
        }

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
        QXK_attr_.readySet.insert((me_)->m_prio); \
        if (!QXK_ISR_CONTEXT_()) { \
            QXK_sched_(); \
        } \
    } while (false)

    #define QACTIVE_EQUEUE_ONEMPTY_(me_) ((void)0)

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

#endif // qxk_h
