/// @file
/// @brief Priority-ceiling blocking mutex QP::QXMutex class definition
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-18
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

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qxk_pkg.hpp"      // QXK package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QXK_HPP
    #error "Source file included in a project NOT based on the QXK kernel"
#endif // QXK_HPP

namespace QP {

Q_DEFINE_THIS_MODULE("qxk_mutex")

//****************************************************************************
/// @description
/// Initialize the QXK priority ceiling mutex.
///
/// @param[in]  ceiling    the ceiling-priotity of this mutex or zero.
///
/// @note
/// `ceiling == 0` means that the priority-ceiling protocol shall __not__
/// be used by this mutex. Such mutex will __not__ change (boost) the
/// priority of the holding thread.
///
/// @note
/// `ceiling > 0` means that the priority-ceiling protocol shall be used
/// by this mutex. Such mutex __will__ boost the priority of the holding
/// thread to the `ceiling` level for as long as the thread holds this mutex.
///
/// @attention
/// When the priority-ceiling protocol is used (`ceiling > 0`), the
/// `ceiling` priority must be unused by any other thread or mutex.
/// Also, the `ceiling` priority must be higher than priority of any thread
/// that uses this mutex.
///
/// @usage
/// @include qxk_mutex.cpp
///
void QXMutex::init(std::uint_fast8_t const ceiling) noexcept {
    QF_CRIT_STAT_

    QF_CRIT_E_();
    /// @pre the celiling priority of the mutex must:
    /// - cannot exceed the maximum #QF_MAX_ACTIVE;
    /// - the ceiling priority of the mutex must not be already in use;
    /// (QF requires priority to be **unique**).
    Q_REQUIRE_ID(100,
        (ceiling <= QF_MAX_ACTIVE)
        && ((ceiling == 0U)
            || (QF::active_[ceiling] == nullptr)));

    m_ceiling    = static_cast<std::uint8_t>(ceiling);
    m_lockNest   = 0U;
    m_holderPrio = 0U;
    QF::bzero(&m_waitSet, sizeof(m_waitSet));

    if (ceiling != 0U) {
        // reserve the ceiling priority level for this mutex
        QF::active_[ceiling] = QXK_PTR_CAST_(QActive*, this);
    }
    QF_CRIT_X_();
}

//****************************************************************************
/// @description
/// Lock the QXK priority ceiling mutex QP::QXMutex.
///
/// @param[in]  nTicks    number of clock ticks (at the associated rate)
///                       to wait for the semaphore. The value of
///                       #QXTHREAD_NO_TIMEOUT indicates that no timeout will
///                       occur and the semaphore will wait indefinitely.
/// @returns
/// 'true' if the mutex has been acquired and 'false' if a timeout occured.
///
/// @note
/// The mutex locks are allowed to nest, meaning that the same extended thread
/// can lock the same mutex multiple times (< 255). However, each call to
/// QXMutex::lock() must be ballanced by the matching call to
/// QXMutex::unlock().
///
/// @usage
/// @include qxk_mutex.cpp
///
bool QXMutex::lock(std::uint_fast16_t const nTicks) noexcept {
    bool locked = true; // assume that the mutex will be locked
    QXThread *curr;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    curr = QXK_PTR_CAST_(QXThread*, QXK_attr_.curr);

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - be called from an extended thread;
    /// - the ceiling priority must not be used; or if used
    ///   - the thread priority must be below the ceiling of the mutex;
    /// - the ceiling must be in range
    /// - the thread must NOT be already blocked on any object.
    ///
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_())
        && (curr != nullptr)
        && ((m_ceiling == 0U)
            || (curr->m_prio < m_ceiling))
        && (m_ceiling <= QF_MAX_ACTIVE)
        && (curr->m_temp.obj == nullptr)); // not blocked
    /// @pre also: the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_ID(201, QXK_attr_.lockHolder != curr->m_prio);

    // is the mutex available?
    if (m_lockNest == 0U) {
        m_lockNest = 1U;

        if (m_ceiling != 0U) {
            // the priority slot must be occupied by this mutex
            Q_ASSERT_ID(210, QF::active_[m_ceiling]
                             == QXK_PTR_CAST_(QActive*, this));

            // boost the dynamic priority of this thread to the ceiling
            QXK_attr_.readySet.rmove(
                static_cast<std::uint_fast8_t>(curr->m_dynPrio));
            curr->m_dynPrio = m_ceiling;
            QXK_attr_.readySet.insert(
                static_cast<std::uint_fast8_t>(curr->m_dynPrio));
            QF::active_[m_ceiling] = curr;
        }

        // make the curr thread the new mutex holder
        m_holderPrio = static_cast<std::uint8_t>(curr->m_prio);

        QS_BEGIN_NOCRIT_PRE_(QS_MUTEX_LOCK, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            // start prio & current ceiling
            QS_2U8_PRE_(curr->m_prio, m_ceiling);
        QS_END_NOCRIT_PRE_()
    }
    // is the mutex locked by this thread already (nested locking)?
    else if (m_holderPrio == curr->m_prio) {

        // the nesting level must not exceed the dynamic range of uint8_t
        Q_ASSERT_ID(220, m_lockNest < 0xFFU);

        ++m_lockNest;
    }
    else { // the mutex is alredy locked by a different thread

        // the ceiling holder priority must be valid
        Q_ASSERT_ID(230, 0U < m_holderPrio);
        Q_ASSERT_ID(231, m_holderPrio <= QF_MAX_ACTIVE);

        if (m_ceiling != 0U) {
            // the prio slot must be occupied by the thr. holding the mutex
            Q_ASSERT_ID(240, QF::active_[m_ceiling]
                             == QF::active_[m_holderPrio]);
        }

        // remove the curr dynamic prio from the ready set (block)
        // and insert it to the waiting set on this mutex
        std::uint_fast8_t const p =
            static_cast<std::uint_fast8_t>(curr->m_dynPrio);
        QXK_attr_.readySet.rmove(p);
        m_waitSet.insert(p);

        // store the blocking object (this mutex)
        curr->m_temp.obj = QXK_PTR_CAST_(QMState*, this);
        curr->teArm_(static_cast<enum_t>(QXK_SEMA_SIG), nTicks);

        // schedule the next thread if multitasking started
        static_cast<void>(QXK_sched_());
        QF_CRIT_X_();
        QF_CRIT_EXIT_NOP(); // BLOCK here !!!

        QF_CRIT_E_();   // AFTER unblocking...
        // the blocking object must be this mutex
        Q_ASSERT_ID(240, curr->m_temp.obj == QXK_PTR_CAST_(QMState *, this));

        // did the blocking time-out? (signal of zero means that it did)
        if (curr->m_timeEvt.sig == 0U) {
            if (m_waitSet.hasElement(p)) { // still waiting?
                m_waitSet.rmove(p); // remove the unblocked thread
                locked = false; // the mutex was NOT locked
            }
        }
        else { // blocking did NOT time out
            // the thread must NOT be waiting on this mutex
            Q_ASSERT_ID(250, !m_waitSet.hasElement(p));
        }
        curr->m_temp.obj = nullptr; // clear blocking obj.
    }
    QF_CRIT_X_();

    return locked;
}

//****************************************************************************
/// @description
/// Try to lock the QXK priority ceiling mutex QP::QXMutex.
///
/// @returns
/// 'true' if the mutex was successfully locked and 'false' if the mutex was
/// unavailable and was NOT locked.
///
/// @note
/// This function **can** be called from both basic threads (active objects)
/// and extended threads.
///
/// @note
/// The mutex locks are allowed to nest, meaning that the same extended thread
/// can lock the same mutex multiple times (< 255). However, each successful
/// call to QXMutex::tryLock() must be ballanced by the matching call to
/// QXMutex::unlock().
///
bool QXMutex::tryLock(void) noexcept {
    QActive *curr;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    curr = QXK_attr_.curr;
    if (curr == nullptr) { // called from a basic thread?
        curr = QF::active_[QXK_attr_.actPrio];
    }

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - the calling thread must be valid;
    /// - the ceiling must be not used; or
    ///   - the thread priority must be below the ceiling of the mutex;
    /// - the ceiling must be in range
    Q_REQUIRE_ID(300, (!QXK_ISR_CONTEXT_())
        && (curr != nullptr)
        && ((m_ceiling == 0U)
            || (curr->m_prio < m_ceiling))
        && (m_ceiling <= QF_MAX_ACTIVE));
    /// @pre also: the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_ID(301, QXK_attr_.lockHolder != curr->m_prio);

    // is the mutex available?
    if (m_lockNest == 0U) {
        m_lockNest = 1U;

        if (m_ceiling != 0U) {
            // the priority slot must be set to this mutex
            Q_ASSERT_ID(310, QF::active_[m_ceiling]
                             == QXK_PTR_CAST_(QActive *, this));

            // boost the dynamic priority of this thread to the ceiling
            QXK_attr_.readySet.rmove(
                static_cast<std::uint_fast8_t>(curr->m_dynPrio));
            curr->m_dynPrio = m_ceiling;
            QXK_attr_.readySet.insert(
                static_cast<std::uint_fast8_t>(curr->m_dynPrio));
            QF::active_[m_ceiling] = curr;
        }

        // make curr thread the new mutex holder
        m_holderPrio = curr->m_prio;

        QS_BEGIN_NOCRIT_PRE_(QS_MUTEX_LOCK, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            // start prio & current ceiling
            QS_2U8_PRE_(curr->m_prio, m_ceiling);
        QS_END_NOCRIT_PRE_()
    }
    // is the mutex held by this thread already (nested locking)?
    else if (m_holderPrio == curr->m_prio) {
        // the nesting level must not exceed  the dynamic range of uint8_t
        Q_ASSERT_ID(320, m_lockNest < 0xFFU);

        ++m_lockNest;
    }
    else { // the mutex is alredy locked by a different thread
        if (m_ceiling != 0U) {
            // the prio slot must be claimed by the mutex holder
            Q_ASSERT_ID(330, QF::active_[m_ceiling]
                             == QF::active_[m_holderPrio]);
        }
        curr = nullptr; // means that mutex is NOT available
    }
    QF_CRIT_X_();

    return curr != nullptr;
}

//****************************************************************************
/// @description
/// Unlock the QXK priority ceiling mutex.
///
/// @note
/// This function **can** be called from both basic threads (active objects)
/// and extended threads.
///
/// @note
/// The mutex locks are allowed to nest, meaning that the same extended thread
/// can lock the same mutex multiple times (< 255). However, each call to
/// QXMutex::lock() or a _successfull_ call to QXMutex::tryLock() must be
/// ballanced by the matching call to QXMutex::unlock().
///
/// @usage
/// @include qxk_mutex.cpp
///
void QXMutex::unlock(void) noexcept {
    QActive *curr;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    curr = static_cast<QActive *>(QXK_attr_.curr);
    if (curr == nullptr) { // called from a basic thread?
        curr = QF::active_[QXK_attr_.actPrio];
    }

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - the calling thread must be valid;
    /// - the ceiling must not be used or
    ///    - the current thread must have priority equal to the mutex ceiling;
    /// - the ceiling must be in range
    ///
    Q_REQUIRE_ID(400, (!QXK_ISR_CONTEXT_())
        && (curr != nullptr)
        && ((m_ceiling == 0U)
            || (curr->m_dynPrio == m_ceiling))
        && (m_ceiling <= QF_MAX_ACTIVE));
    /// @pre also: the mutex must be already locked at least once.
    Q_REQUIRE_ID(401, m_lockNest > 0U);
    /// @pre also: the mutex must be held by this thread.
    Q_REQUIRE_ID(402, m_holderPrio == curr->m_prio);

    // is this the last nesting level?
    if (m_lockNest == 1U) {

        if (m_ceiling != 0U) {
            // restore the holding thread's priority to the original
            QXK_attr_.readySet.rmove(
                static_cast<std::uint_fast8_t>(curr->m_dynPrio));
            curr->m_dynPrio = curr->m_prio;
            QXK_attr_.readySet.insert(
                static_cast<std::uint_fast8_t>(curr->m_dynPrio));

            // put the mutex at the priority ceiling slot
            QF::active_[m_ceiling] = QXK_PTR_CAST_(QActive*, this);
        }

        // the mutex no longer held by a thread
        m_holderPrio = 0U;

        QS_BEGIN_NOCRIT_PRE_(QS_MUTEX_UNLOCK, curr->m_prio)
            QS_TIME_PRE_();  // timestamp
            // start prio & the mutex ceiling
            QS_2U8_PRE_(curr->m_prio, m_ceiling);
        QS_END_NOCRIT_PRE_()

        // are any other threads waiting for this mutex?
        if (m_waitSet.notEmpty()) {

            // find the highest-priority thread waiting on this mutex
            std::uint_fast8_t const p = m_waitSet.findMax();
            QXThread * const thr = QXK_PTR_CAST_(QXThread*, QF::active_[p]);

            // the waiting thread must:
            // - the ceiling must not be used; or if used
            //   - the thread must have priority below the ceiling
            // - be registered in QF
            // - have the dynamic priority the same as initial priority
            // - be blocked on this mutex
            Q_ASSERT_ID(410,
                ((m_ceiling == 0U)
                   || (p < static_cast<std::uint_fast8_t>(m_ceiling)))
                && (thr != nullptr)
                && (thr->m_dynPrio == thr->m_prio)
                && (thr->m_temp.obj == QXK_PTR_CAST_(QMState*, this)));

            // disarm the internal time event
            static_cast<void>(thr->teDisarm_());

            if (m_ceiling != 0U) {
                // boost the dynamic priority of this thread to the ceiling
                thr->m_dynPrio = m_ceiling;
                QF::active_[m_ceiling] = thr;
            }

            // make the thread the new mutex holder
            m_holderPrio = static_cast<std::uint8_t>(thr->m_prio);

            // make the thread ready to run (at the ceiling prio)
            // and remove from the waiting list
            QXK_attr_.readySet.insert(thr->m_dynPrio);
            m_waitSet.rmove(p);

            QS_BEGIN_NOCRIT_PRE_(QS_MUTEX_LOCK, thr->m_prio)
                QS_TIME_PRE_();  // timestamp
                // start priority & ceiling priority
                QS_2U8_PRE_(thr->m_prio, m_ceiling);
            QS_END_NOCRIT_PRE_()
        }
        else { // no threads are waiting for this mutex
            m_lockNest = 0U;

            if (m_ceiling != 0U) {
                // put the mutex at the priority ceiling slot
                QF::active_[m_ceiling] = QXK_PTR_CAST_(QActive*, this);
            }
        }

        // schedule the next thread if multitasking started
        if (QXK_sched_() != 0U) {
            QXK_activate_(); // activate a basic thread
        }
    }
    else { // releasing the mutex
        --m_lockNest; // release one level
    }
    QF_CRIT_X_();
}

} // namespace QP
