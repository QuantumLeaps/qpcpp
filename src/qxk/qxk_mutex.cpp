/// @file
/// @brief Priority-ceiling blocking mutex QP::QXMutex class definition
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Product: QK/C++
/// Last updated for version 5.9.7
/// Last updated on  2017-08-19
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
/// https://state-machine.com
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

//****************************************************************************/
/// @description
/// Initialize the QXK priority ceiling mutex.
///
/// @param[in]     ceiling ceiling priotity of the mutex
///
/// @note
/// The ceiling priority must be unused by any thread. The ceiling
/// priority must be higher than priority of any thread that uses the
/// protected resource.
///
/// @usage
/// @include qxk_mux.cpp
///
void QXMutex::init(uint_fast8_t ceiling) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    /// @pre the celiling priority of the mutex must:
    /// - not be zero;
    /// - cannot exceed the maximum #QF_MAX_ACTIVE;
    /// - the ceiling priority of the mutex must not be already in use;
    /// (QF requires priority to be **unique**).
    Q_REQUIRE_ID(100, (static_cast<uint_fast8_t>(0) < ceiling)
              && (ceiling <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
              && (QF::active_[ceiling] == static_cast<QActive *>(0)));

    m_ceiling  = ceiling;
    m_lockNest = static_cast<uint_fast8_t>(0);
    QF::bzero(&m_waitSet, static_cast<uint_fast16_t>(sizeof(m_waitSet)));

    // reserve the ceiling priority level for this mutex
    QF::active_[ceiling] = reinterpret_cast<QActive *>(this);

    QF_CRIT_EXIT_();
}

//****************************************************************************
///
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
/// can lock the same mutex multiple times (<= 225). However, each call to
/// QXMutex::lock() must be ballanced by the matching call to
/// QXMutex::unlock().
///
/// @usage
/// @include qxk_mux.cpp
///
bool QXMutex::lock(uint_fast16_t const nTicks) {
    QXThread *thr;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    thr = static_cast<QXThread *>(QXK_attr_.curr);

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - be called from an extended thread;
    /// - the thread priority must be below the ceiling of the mutex;
    /// - the thread must NOT be holding a scheduler lock;
    /// - the thread must NOT be already blocked on any object.
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
        && (thr != static_cast<QXThread *>(0)) /* thr must be extended */
        && (thr->m_startPrio < m_ceiling) /* prio below the ceiling */
        && (QXK_attr_.lockHolder != thr->m_prio) /* not holding a lock */
        && (thr->m_temp.obj == static_cast<QMState *>(0))); // not blocked

    // is the mutex available?
    if (m_lockNest == static_cast<uint_fast8_t>(0)) {
        m_lockNest = static_cast<uint_fast8_t>(1);

        // the priority slot must be set to this mutex */
        Q_ASSERT_ID(210,
            QF::active_[m_ceiling] == reinterpret_cast<QActive *>(this));

        // switch the priority of this thread to the mutex ceiling
        thr->m_prio = m_ceiling;
        QF::active_[m_ceiling] = thr;

        QXK_attr_.readySet.remove(thr->m_startPrio);
        QXK_attr_.readySet.insert(thr->m_prio);

        QS_BEGIN_NOCRIT_(QS_MUTEX_LOCK,
            static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(thr->m_startPrio), /* start prio */
                    static_cast<uint8_t>(thr->m_prio)); // current ceiling
        QS_END_NOCRIT_()
    }
    // is the mutex locked by this thread already (nested locking)?
    else if (QF::active_[m_ceiling] == thr) {

        // the nesting level must not exceed 0xFF
        Q_ASSERT_ID(220, m_lockNest < static_cast<uint_fast8_t>(0xFF));

        ++m_lockNest;
    }
    else { // the mutex is alredy locked by a different thread

        // the prio slot must be claimed by the thread holding the mutex
        Q_ASSERT_ID(230, QF::active_[m_ceiling] != static_cast<QActive *>(0));

        // remove this thr prio from the ready set (block)
        // and insert to the waiting set on this mutex
        QXK_attr_.readySet.remove(thr->m_prio);
        m_waitSet.insert(thr->m_prio);

        // store the blocking object (this mutex)
        thr->m_temp.obj = reinterpret_cast<QMState *>(this);
        thr->teArm_(static_cast<enum_t>(QXK_SEMA_SIG), nTicks);

        // schedule the next thread if multitasking started
        (void)QXK_sched_();
        QF_CRIT_EXIT_();
        QF_CRIT_EXIT_NOP(); // BLOCK here

        QF_CRIT_ENTRY_();
        // the blocking object must be this mutex
        Q_ASSERT_ID(240, thr->m_temp.obj == reinterpret_cast<QMState *>(this));
        thr->m_temp.obj = static_cast<QMState *>(0); // clear
    }
    QF_CRIT_EXIT_();

    // signal of non-zero means that the time event has not expired
    return thr->m_timeEvt.sig != static_cast<QSignal>(0);
}

//****************************************************************************
///
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
/// can lock the same mutex multiple times (<= 225). However, each successful
/// call to QXMutex_tryLock() must be ballanced by the matching call to
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
    /// - the QXK kernel must be running;
    /// - the calling thread must be valid;
    /// - the thread priority must be below the ceiling of the mutex;
    /// - the thread must NOT be holding a scheduler lock;
    Q_REQUIRE_ID(300, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
        && (QXK_attr_.lockPrio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
        && (curr != static_cast<QActive *>(0)) /* curr thread must be valid */
        && (curr->m_startPrio < m_ceiling) /* prio below the ceiling */
        && (curr->m_prio != QXK_attr_.lockHolder)); // not holding a lock

    // is the mutex available?
    if (m_lockNest == static_cast<uint_fast8_t>(0)) {
        m_lockNest = static_cast<uint_fast8_t>(1);

        // the priority slot must be set to this mutex
        Q_ASSERT_ID(310,
            QF::active_[m_ceiling] == reinterpret_cast<QActive *>(this));

        // switch the priority of this thread to the mutex ceiling
        curr->m_prio = m_ceiling;
        QF::active_[m_ceiling] = curr;

        QXK_attr_.readySet.remove(curr->m_startPrio);
        QXK_attr_.readySet.insert(curr->m_prio);

        QS_BEGIN_NOCRIT_(QS_MUTEX_LOCK,
            static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(curr->m_startPrio), /* start prio */
                    static_cast<uint8_t>(curr->m_prio));  // current ceiling
        QS_END_NOCRIT_()
    }
    // is the mutex locked by this thread already (nested locking)?
    else if (QF::active_[m_ceiling] == curr) {
        // the nesting level must not exceed 0xFF
        Q_ASSERT_ID(320, m_lockNest < static_cast<uint_fast8_t>(0xFF));
        ++m_lockNest;
    }
    else { // the mutex is alredy locked by a different thread
        // the prio slot must be claimed by the thread holding the mutex
        Q_ASSERT_ID(330, QF::active_[m_ceiling] != static_cast<QActive *>(0));
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
/// can lock the same mutex multiple times (<= 225). However, each call to
/// QXMutex::lock() or a _successfull_ call to QXMutex_tryLock() must be
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
    /// - the holding thread must have priority equal to the mutex ceiling;
    /// - the mutex must be already locked at least once.
    Q_REQUIRE_ID(400, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
        && (curr != static_cast<QActive *>(0)) /* curr must be valid */
        && (curr == QF::active_[m_ceiling]) /* held by this thr */
        && (curr->m_prio == m_ceiling) /* thr prio at the ceiling */
        && (m_lockNest > static_cast<uint_fast8_t>(0)));//locked at least once

    // is this the last nesting level?
    if (m_lockNest == static_cast<uint_fast8_t>(1)) {

        // restore the holding thread's priority to the original
        curr->m_prio = curr->m_startPrio;

        // remove the boosted priority and insert the original priority
        QXK_attr_.readySet.remove(m_ceiling);
        QXK_attr_.readySet.insert(curr->m_startPrio);

        QS_BEGIN_NOCRIT_(QS_MUTEX_UNLOCK,
            static_cast<void *>(0), static_cast<void *>(0))
            QS_TIME_();  // timestamp
            QS_2U8_(static_cast<uint8_t>(curr->m_startPrio),  /* start prio */
                    static_cast<uint8_t>(m_ceiling));  // the mutex ceiling
        QS_END_NOCRIT_()

        // are any other threads waiting for this mutex?
        if (m_waitSet.notEmpty()) {

            // find the highest-priority waiting thread
            uint_fast8_t p = m_waitSet.findMax();
            QXThread *thr = static_cast<QXThread *>(QF::active_[p]);

            // the waiting thread must:
            // (1) have priority below the ceiling
            // (2) be extended
            // (3) not be redy to run
            // (4) have still the start priority
            // (5) be blocked on this mutex
            Q_ASSERT_ID(410, (p < m_ceiling)
                && (thr != static_cast<QXThread *>(0)) /* extended thread */
                && (!QXK_attr_.readySet.hasElement(p))
                && (thr->m_prio == thr->m_startPrio)
                && (thr->m_temp.obj == reinterpret_cast<QMState *>(this)));

            // disarm the internal time event
            (void)thr->teDisarm_();

            // this thread is no longer waiting for the mutex
            m_waitSet.remove(p);

            // switch the priority of this thread to the mutex ceiling
            thr->m_prio = m_ceiling;
            QF::active_[m_ceiling] = thr;

            // make the thread ready to run (at the ceiling prio)
            QXK_attr_.readySet.insert(thr->m_prio);

            QS_BEGIN_NOCRIT_(QS_MUTEX_LOCK,
                static_cast<void *>(0), static_cast<void *>(0))
                QS_TIME_();  // timestamp
                QS_2U8_(static_cast<uint8_t>(thr->m_startPrio),/*start prio*/
                        static_cast<uint8_t>(thr->m_prio));  // ceiling prio
            QS_END_NOCRIT_()
        }
        else { // no threads are waiting for this mutex
            m_lockNest = static_cast<uint_fast8_t>(0);

            // put the mutex at the priority ceiling slot
            QF::active_[m_ceiling] = reinterpret_cast<QActive *>(this);
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
