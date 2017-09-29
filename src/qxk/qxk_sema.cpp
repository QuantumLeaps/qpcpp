/// @file
/// @brief QXK/C++ preemptive kernel counting semaphore implementation
/// @ingroup qxk
/// @cond
////**************************************************************************
/// Last updated for version 5.9.9
/// Last updated on  2017-09-29
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
////**************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qxk_pkg.h"      // QXK package-scope internal interface
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

Q_DEFINE_THIS_MODULE("qxk_sema")

//****************************************************************************
/// @description
/// Initializes a semaphore with the specified count and maximum count.
/// If the semaphore is used for resource sharing, both the initial count
/// and maximum count should be set to the number of identical resources
/// guarded by the semaphore. If the semaphore is used as a signaling
/// mechanism, the initial count should set to 0 and maximum count to 1
/// (binary semaphore).
///
/// @param[in]     count  initial value of the semaphore counter
/// @param[in]     max_count  maximum value of the semaphore counter.
///                The purpose of the max_count is to limit the counter
///                so that the semaphore cannot unblock more times than
///                the maximum.
/// @note
/// QXSemaphore::init() must be called **before** the semaphore can be used
/// (signaled or waited on).
///
void QXSemaphore::init(uint_fast16_t const count,
                       uint_fast16_t const max_count)
{
    Q_REQUIRE_ID(100, max_count > static_cast<uint_fast16_t>(0));

    m_waitSet.setEmpty();
    m_count     = static_cast<uint16_t>(count);
    m_max_count = static_cast<uint16_t>(max_count);
}

//****************************************************************************
/// @description
/// When an extended thread calls QXSemaphore::wait() and the value of the
/// semaphore counter is greater than 0, QXSemaphore_wait() decrements the
/// semaphore counter and returns (true) to its caller. However, if the value
/// of the semaphore counter is 0, the function places the calling thread in
/// the waiting list for the semaphore. The thread waits until the semaphore
/// is signaled by calling QXSemaphore::signal(), or the specified timeout
/// expires. If the semaphore is signaled before the timeout expires, QXK
/// resumes the highest-priority extended thread waiting for the semaphore.
///
/// @param[in]  nTicks    number of clock ticks (at the associated rate)
///                       to wait for the semaphore. The value of
///                       QXTHREAD_NO_TIMEOUT indicates that no timeout will
///                       occur and the semaphore will wait indefinitely.
/// @returns
/// true if the semaphore has been signaled, and false if the timeout occured.
///
/// @note
/// Multiple extended threads can wait for a given semahpre.
///
bool QXSemaphore::wait(uint_fast16_t const nTicks) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QXThread *curr = static_cast<QXThread *>(QXK_attr_.curr);

    /// @pre this function must:
    /// (1) NOT be called from an ISR; (2) be called from an extended thread;
    /// (3) the thread must NOT be holding a mutex and
    /// (4) the thread must NOT be already blocked on any object.
    ///
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_()) /* can't block inside an ISR */
        && (curr != static_cast<QXThread *>(0)) /* curr must be extended */
        && (QXK_attr_.lockPrio == static_cast<uint_fast8_t>(0)) /* no lock */
        && (curr->m_temp.obj == static_cast<QMState *>(0))); // not blocked

    if (m_count > static_cast<uint16_t>(0)) {
        --m_count;
        curr->m_timeEvt.sig = static_cast<QSignal>(QXK_SEMA_SIG); // non-zero
    }
    else {
        // remember the blocking object
        curr->m_temp.obj = reinterpret_cast<QMState const *>(this);
        curr->teArm_(static_cast<enum_t>(QXK_SEMA_SIG), nTicks);
        m_waitSet.insert(curr->m_prio);
        QXK_attr_.readySet.remove(curr->m_prio);
        (void)QXK_sched_();
        QF_CRIT_EXIT_();
        QF_CRIT_EXIT_NOP(); // BLOCK here

        QF_CRIT_ENTRY_();
        // the blocking object must be this semaphore
        Q_ASSERT_ID(210, curr->m_temp.obj
                         == reinterpret_cast<QMState const *>(this));
        curr->m_temp.obj = static_cast<QMState *>(0); // clear
    }
    QF_CRIT_EXIT_();

    // signal of non-zero means that the time event has not expired
    return (curr->m_timeEvt.sig != static_cast<QSignal>(0));
}

//****************************************************************************
///
/// @description
/// This operation checks if the semaphore counter is greater than 0,
/// in which case the counter is decremented.
///
/// @returns
/// 'true' if the semaphore has count available and 'false' NOT available.
///
/// @note
/// This function can be called from any context, including ISRs and basic
/// threds (active objects).
///
bool QXSemaphore::tryWait(void) {
    bool isAvailable;
    QF_CRIT_STAT_

    /// @pre the semaphore must be initialized
    Q_REQUIRE_ID(300, (m_max_count > static_cast<uint16_t>(0)));

    QF_CRIT_ENTRY_();
    // is the semaphore available?
    if (m_count > static_cast<uint16_t>(0)) {
        --m_count;
        isAvailable = true;
    }
    else { // the semaphore is NOT available (would block)
        isAvailable = false;
    }
    QF_CRIT_EXIT_();

    return isAvailable;
}

//****************************************************************************
/// @description
/// If the semaphore counter value is 0 or more, it is incremented, and this
/// function returns to its caller. If the extended threads are waiting for
/// the semaphore to be signaled, QXSemaphore_signal() removes the highest-
/// priority thread waiting for the semaphore from the waiting list and makes
/// this thread ready-to-run. The QXK scheduler is then called to determine
/// if the awakened thread is now the highest-priority thread that is
/// ready-to-run.
///
/// @returns true when the semaphore gets signaled and false when
/// the semaphore count exceeded the maximum.
///
/// @note
/// A semaphore can be signaled from many places, including from ISRs, basic
/// threads (AOs), and extended threads.
///
bool QXSemaphore::signal(void) {
    bool signaled = true; // assume that the semaphore will be signaled
    QF_CRIT_STAT_

    /// @pre the semaphore must be initialized
    Q_REQUIRE_ID(400, m_max_count > static_cast<uint16_t>(0));

    QF_CRIT_ENTRY_();
    if (m_waitSet.notEmpty()) {
        uint_fast8_t p = m_waitSet.findMax();
        QXK_attr_.readySet.insert(p);
        m_waitSet.remove(p);

        QXThread *thr = static_cast<QXThread *>(QF::active_[p]);

        // the thread must be extended and the semaphore count must be zero
        Q_ASSERT_ID(410, (thr != static_cast<QXThread *>(0)) /* registered */
             && (thr->m_osObject != static_cast<void *>(0)) /* extended */
             && (m_count == static_cast<uint16_t>(0))); // not signaled

        // disarm the internal time event
        (void)thr->teDisarm_();

        if (!QXK_ISR_CONTEXT_()) { // not inside ISR?
            (void)QXK_sched_();
        }
    }
    else {
        if (m_count < m_max_count) {
            ++m_count;
        }
        else {
            signaled = false; // semaphore NOT signaled
        }
    }
    QF_CRIT_EXIT_();

    return signaled;
}

} // namespace QP
