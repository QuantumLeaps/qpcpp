/// @file
/// @brief Priority-ceiling blocking mutex QP::QXMutex class definition
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.3.8
/// Last updated on  2019-01-14
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qxk_pkg.h"      // QXK package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef qxk_h
    #error "Source file included in a project NOT based on the QXK kernel"
#endif // qxk_h

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
/// @include qxk_mux.cpp
///
void QXMutex::init(uint_fast8_t const ceiling) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    /// @pre the celiling priority of the mutex must:
    /// - cannot exceed the maximum #QF_MAX_ACTIVE;
    /// - the ceiling priority of the mutex must not be already in use;
    /// (QF requires priority to be **unique**).
    Q_REQUIRE_ID(100,
        (ceiling <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
        && ((ceiling == static_cast<uint_fast8_t>(0))
            || (QF::active_[ceiling] == static_cast<QActive *>(0))));

    m_ceiling    = static_cast<uint8_t>(ceiling);
    m_lockNest   = static_cast<uint8_t>(0);
    m_holderPrio = static_cast<uint8_t>(0);
    QF::bzero(&m_waitSet, static_cast<uint_fast16_t>(sizeof(m_waitSet)));

    if (ceiling != static_cast<uint_fast8_t>(0)) {
        // reserve the ceiling priority level for this mutex
        QF::active_[ceiling] = reinterpret_cast<QActive *>(this);
    }
    QF_CRIT_EXIT_();
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
/// @include qxk_mux.cpp
///
bool QXMutex::lock(uint_fast16_t const nTicks) {
    bool locked = true; // assume that the mutex will be locked
    QXThread *curr;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    curr = static_cast<QXThread *>(QXK_attr_.curr);

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - be called from an extended thread;
    /// - the ceiling priority must not be used; or if used
    ///   - the thread priority must be below the ceiling of the mutex;
    /// - the thread must NOT be holding a scheduler lock;
    /// - the thread must NOT be already blocked on any object.
    ///
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
        && (curr != static_cast<QXThread *>(0)) /* curr must be extended */
        && ((m_ceiling == static_cast<uint8_t>(0)) /* below ceiling */
            || (curr->m_startPrio < m_ceiling))
        && (QXK_attr_.lockHolder != curr->m_prio) /* not holding a lock */
        && curr->isBlockedOn()); // not blocked

    // is the mutex available?
    if (m_lockNest == static_cast<uint8_t>(0)) {
        m_lockNest = static_cast<uint8_t>(1);

        if (m_ceiling != static_cast<uint8_t>(0)) {
            // the priority slot must be occupied by this mutex
            Q_ASSERT_ID(210,
                QF::active_[m_ceiling] == reinterpret_cast<QActive *>(this));

            // boost the priority of this thread to the mutex ceiling
            curr->m_prio = m_ceiling;
            QF::active_[m_ceiling] = curr;
            QXK_attr_.readySet.remove(
                static_cast<uint_fast8_t>(curr->m_startPrio));
            QXK_attr_.readySet.insert(
                static_cast<uint_fast8_t>(curr->m_prio));
        }

        // make the curr thread the new mutex holder
        m_holderPrio = static_cast<uint8_t>(curr->m_startPrio);

        QS_BEGIN_NOCRIT_(QS_MUTEX_LOCK,
            static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_();  // timestamp
            QS_2U8_(curr->m_startPrio, /* start prio */
                    m_ceiling); // current ceiling
        QS_END_NOCRIT_()
    }
    // is the mutex locked by this thread already (nested locking)?
    else if (m_holderPrio == curr->m_startPrio) {

        // the nesting level must not exceed 0xFF
        Q_ASSERT_ID(220, m_lockNest < static_cast<uint8_t>(0xFF));

        ++m_lockNest;
    }
    else { // the mutex is alredy locked by a different thread

        // the ceiling holder priority must be valid
        Q_ASSERT_ID(230, m_holderPrio != static_cast<uint8_t>(0));

        if (m_ceiling != static_cast<uint8_t>(0)) {
            // the prio slot must be occupied by the thr. holding the mutex
            Q_ASSERT_ID(240,
                        QF::active_[m_ceiling] != static_cast<QActive *>(0));
        }

        // remove this curr prio from the ready set (block)
        // and insert to the waiting set on this mutex
        uint_fast8_t p = static_cast<uint_fast8_t>(curr->m_prio);
        QXK_attr_.readySet.remove(p);
        m_waitSet.insert(p);

        // store the blocking object (this mutex)
        curr->m_temp.obj = reinterpret_cast<QMState *>(this);
        curr->teArm_(static_cast<enum_t>(QXK_SEMA_SIG), nTicks);

        // schedule the next thread if multitasking started
        (void)QXK_sched_();
        QF_CRIT_EXIT_();
        QF_CRIT_EXIT_NOP(); // BLOCK here !!!

        QF_CRIT_ENTRY_();   // AFTER unblocking...
        // the blocking object must be this mutex
        Q_ASSERT_ID(240, curr->isBlockedOn(this));

        // did the blocking time-out? (signal of zero means that it did)
        if (curr->m_timeEvt.sig == static_cast<QSignal>(0)) {
            if (m_waitSet.hasElement(p)) { // still waiting?
                m_waitSet.remove(p); // remove the unblocked thread
                locked = false; /* the mutex was NOT locked */
            }
        }
        else { // blocking did NOT time out
            // the thread must NOT be waiting on this mutex
            Q_ASSERT_ID(250, !m_waitSet.hasElement(p));
        }
        curr->m_temp.obj = static_cast<QMState *>(0); // clear blocking obj.
    }
    QF_CRIT_EXIT_();

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
bool QXMutex::tryLock(void) {
    QActive *curr;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    curr = QXK_attr_.curr;
    if (curr == static_cast<QActive *>(0)) { // called from a basic thread?
        curr = QF::active_[QXK_attr_.actPrio];
    }

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - the calling thread must be valid;
    /// - the ceiling must be not used; or
    ///   - the thread priority must be below the ceiling of the mutex;
    /// - the thread must NOT be holding a scheduler lock;
    Q_REQUIRE_ID(300, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
        && (curr != static_cast<QActive *>(0)) /* curr thread must be valid */
        && ((m_ceiling == static_cast<uint8_t>(0)) /* below ceiling */
            || (curr->m_startPrio < m_ceiling))
        && (curr->m_prio != QXK_attr_.lockHolder)); // not holding a lock

    // is the mutex available?
    if (m_lockNest == static_cast<uint8_t>(0)) {
        m_lockNest = static_cast<uint8_t>(1);

        if (m_ceiling != static_cast<uint8_t>(0)) {
            // the priority slot must be set to this mutex
            Q_ASSERT_ID(310,
                QF::active_[m_ceiling] == reinterpret_cast<QActive *>(this));

            // boost the priority of this thread to the mutex ceiling
            curr->m_prio = m_ceiling;
            QF::active_[m_ceiling] = curr;
            QXK_attr_.readySet.remove(
                static_cast<uint_fast8_t>(curr->m_startPrio));
            QXK_attr_.readySet.insert(
                static_cast<uint_fast8_t>(curr->m_prio));
        }

        // make curr thread the new mutex holder
        m_holderPrio = static_cast<uint8_t>(curr->m_startPrio);

        QS_BEGIN_NOCRIT_(QS_MUTEX_LOCK, static_cast<void *>(0), curr)
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(curr->m_startPrio), /* start prio */
                    m_ceiling);  // current ceiling
        QS_END_NOCRIT_()
    }
    // is the mutex held by this thread already (nested locking)?
    else if (m_holderPrio == curr->m_startPrio) {
        // the nesting level must not exceed 0xFF
        Q_ASSERT_ID(320, m_lockNest < static_cast<uint8_t>(0xFF));

        ++m_lockNest;
    }
    else { // the mutex is alredy locked by a different thread
        if (m_ceiling != static_cast<uint8_t>(0)) {
            // the prio slot must be claimed by the mutex holder
            Q_ASSERT_ID(330, (m_holderPrio != static_cast<uint8_t>(0))
                && (QF::active_[m_ceiling] == QF::active_[m_holderPrio]));
        }
        curr = static_cast<QActive *>(0); // means that mutex is NOT available
    }
    QF_CRIT_EXIT_();

    return curr != static_cast<QActive *>(0);
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
/// @include qxk_mux.cpp
///
void QXMutex::unlock(void) {
    QActive *curr;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    curr = static_cast<QActive *>(QXK_attr_.curr);
    if (curr == static_cast<QActive *>(0)) { // called from a basic thread?
        curr = QF::active_[QXK_attr_.actPrio];
    }

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - the calling thread must be valid;
    /// - the mutex must be held by this thread;
    /// - the ceiling must not be used or
    ///    - the current thread must have priority equal to the mutex ceiling;
    /// - the mutex must be already locked at least once.
    ///
    Q_REQUIRE_ID(400, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
        && (curr != static_cast<QActive *>(0)) /* curr must be valid */
        && (curr->m_startPrio == static_cast<uint8_t>(m_holderPrio))
        && ((m_ceiling == static_cast<uint8_t>(0)) /* curr at ceiling prio */
            || (curr->m_prio == m_ceiling))
        && (m_lockNest > static_cast<uint8_t>(0))); // locked at least once

    // is this the last nesting level?
    if (m_lockNest == static_cast<uint8_t>(1)) {

        if (m_ceiling != static_cast<uint8_t>(0)) {
            // restore the holding thread's priority to the original
            QXK_attr_.readySet.remove(
                static_cast<uint_fast8_t>(m_ceiling));
            QXK_attr_.readySet.insert(
                static_cast<uint_fast8_t>(curr->m_startPrio));
            curr->m_prio = curr->m_startPrio;

            // put the mutex at the priority ceiling slot
            QF::active_[m_ceiling] = reinterpret_cast<QActive *>(this);
        }

        // the mutex no longer held by a thread
        m_holderPrio = static_cast<uint8_t>(0);

        QS_BEGIN_NOCRIT_(QS_MUTEX_UNLOCK, static_cast<void *>(0), curr)
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(curr->m_startPrio), /* start prio */
                    m_ceiling);  // the mutex ceiling
        QS_END_NOCRIT_()

        // are any other threads waiting for this mutex?
        if (m_waitSet.notEmpty()) {

            // find the highest-priority thread waiting on this mutex
            uint_fast8_t p = m_waitSet.findMax();
            QXThread *thr = static_cast<QXThread *>(QF::active_[p]);

            // the waiting thread must:
            // - the ceiling must not be used; or if used
            //   - the thread must have priority below the ceiling
            // - be registered in QF
            // - have still the start priority
            // - be blocked on this mutex
            Q_ASSERT_ID(410,
                ((m_ceiling == static_cast<uint8_t>(0)) /* below ceiling */
                   || (p < static_cast<uint_fast8_t>(m_ceiling)))
                && (thr != static_cast<QXThread *>(0)) /* registered in QF */
                && (thr->m_prio == thr->m_startPrio)
                && thr->isBlockedOn(this)); // blocked on this mutex

            // disarm the internal time event
            (void)thr->teDisarm_();

            if (m_ceiling != static_cast<uint8_t>(0)) {
                // boost the priority of this thread to the mutex ceiling
                thr->m_prio = m_ceiling;
                QF::active_[m_ceiling] = thr;
            }

            // make the thread the new mutex holder
            m_holderPrio = static_cast<uint8_t>(p);

            // make the thread ready to run (at the ceiling prio)
            // and remove from the waiting list
            QXK_attr_.readySet.insert(static_cast<uint_fast8_t>(thr->m_prio));
            m_waitSet.remove(p);

            QS_BEGIN_NOCRIT_(QS_MUTEX_LOCK, static_cast<void *>(0), thr)
                QS_TIME_();  // timestamp
                QS_2U8_(thr->m_startPrio, /* start priority */
                        m_ceiling);  // ceiling priority
            QS_END_NOCRIT_()
        }
        else { // no threads are waiting for this mutex
            m_lockNest = static_cast<uint8_t>(0);

            if (m_ceiling != static_cast<uint8_t>(0)) {
                // put the mutex at the priority ceiling slot
                QF::active_[m_ceiling] = reinterpret_cast<QActive *>(this);
            }
        }

        // schedule the next thread if multitasking started
        if (QXK_sched_() != static_cast<uint_fast8_t>(0)) {
            QXK_activate_(); // activate a basic thread
        }
    }
    else { // releasing the mutex
        --m_lockNest; // release one level
    }
    QF_CRIT_EXIT_();
}

} // namespace QP
