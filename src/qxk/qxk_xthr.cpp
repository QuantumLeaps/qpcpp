/// @file
/// @brief QXK/C++ preemptive kernel extended (blocking) thread implementation
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-19
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
#include "qxk_pkg.hpp"      // QXK package-scope internal interface
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

Q_DEFINE_THIS_MODULE("qxk_xthr")

//! intertnal macro to encapsulate casting of pointers for MISRA deviations */
///
/// @description
/// This macro is specifically and exclusively used for downcasting pointers
/// to QActive to pointers to QXThread in situations when it is known
/// that such downcasting is correct.
/// However, such pointer casting is not compliant with
/// MISRA C++ Rule 5-2-7 as well as other messages (e.g.,
/// PC-Lint-Plus warning 826). Defining this specific macro for this purpose
/// allows to selectively disable the warnings for this particular case.
///
#define QXTHREAD_CAST_(ptr_) (static_cast<QP::QXThread *>(ptr_))

namespace QP {

//****************************************************************************
QXThread::QXThread(QXThreadHandler const handler,
                   std::uint_fast8_t const tickRate) noexcept
  : QActive(Q_STATE_CAST(handler)),
    m_timeEvt(this, static_cast<enum_t>(QXK_DELAY_SIG),
                    static_cast<std::uint_fast8_t>(tickRate))
{
    m_state.act = nullptr; // mark as extended thread
}

//****************************************************************************
// QXThread virtual function implementations...
void QXThread::init(void const * const e,
                    std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(e); // unused parameter
    static_cast<void>(qs_id); // unused parameter
    Q_ERROR_ID(110);
}

//****************************************************************************
void QXThread::dispatch(QEvt const * const e,
                        std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(e); // unused parameter
    static_cast<void>(qs_id); // unused parameter
    Q_ERROR_ID(120);
}

//****************************************************************************
///
/// @description
/// Starts execution of an extended thread and registers it with the framework.
/// The extended thread becomes ready-to-run immediately and is scheduled
/// if the QXK is already running.
///
/// @param[in]     prio    priority at which to start the extended thread
/// @param[in]     qSto    pointer to the storage for the ring buffer of the
///                        event queue. This cold be NULL, if this extended
///                        thread does not use the built-in event queue.
/// @param[in]     qLen    length of the event queue [in events],
///                        or zero if queue not used
/// @param[in]     stkSto  pointer to the stack storage (must be provided)
/// @param[in]     stkSize stack size [in bytes] (must not be zero)
/// @param[in]     par     pointer to an extra parameter (might be NULL)
///
/// @usage
/// The following example shows starting an extended thread:
/// @include qxk_start.cpp
///
void QXThread::start(std::uint_fast8_t const prio,
                     QEvt const * * const qSto, std::uint_fast16_t const qLen,
                     void * const stkSto, std::uint_fast16_t const stkSize,
                     void const * const par)
{
    static_cast<void>(par); // unused parameter

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - the thread priority cannot exceed #QF_MAX_ACTIVE;
    /// - the stack storage must be provided;
    /// - the thread must be instantiated (see QP::QXThread::QXThread()).
    Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_())
        && (prio <= QF_MAX_ACTIVE)
        && (stkSto != nullptr)
        && (stkSize != 0U)
        && (m_state.act == nullptr));

    // is storage for the queue buffer provided?
    if (qSto != nullptr) {
        m_eQueue.init(qSto, qLen);
    }

    // extended threads provide their thread function in place of
    // the top-most initial transition 'm_temp.act'
    QXK_stackInit_(this, m_temp.thr, stkSto, stkSize);

    m_prio    = static_cast<std::uint8_t>(prio);
    m_dynPrio = static_cast<std::uint8_t>(prio);

    // the new thread is not blocked on any object
    m_temp.obj = nullptr;

    QF::add_(this); // make QF aware of this extended thread

    QF_CRIT_STAT_
    QF_CRIT_E_();
    // extended-thread becomes ready immediately
    QXK_attr_.readySet.insert(static_cast<std::uint_fast8_t>(m_dynPrio));

    // see if this thread needs to be scheduled in case QXK is running
    static_cast<void>(QXK_sched_());
    QF_CRIT_X_();
}

#ifdef Q_SPY
//****************************************************************************
/// @description
/// Direct event posting is the simplest asynchronous communication method
/// available in QF. The following example illustrates how the Philo active
/// object posts directly the HUNGRY event to the Table active object.@n
/// @n
/// The parameter @p margin specifies the minimum number of free slots in
/// the queue that must be available for posting to succeed. The function
/// returns 1 (success) if the posting succeeded (with the provided margin)
/// and 0 (failure) when the posting fails.
///
/// @param[in] e      pointer to the event to be posted
/// @param[in] margin number of required free slots in the queue
///                   after posting the event. The special value
///                   QP::QF_NO_MARGIN means that this function will assert
///                   if posting fails.
/// @param[in] sender  pointer to a sender object (used in QS only)
///
/// @returns
/// 'true' (success) if the posting succeeded (with the provided margin) and
/// 'false' (failure) when the posting fails.
///
/// @attention
/// Should be called only via the macro POST() or POST_X().
///
/// @note
/// The QP::QF_NO_MARGIN value of the @p margin parameter is special and
/// denotes situation when the post() operation is assumed to succeed
/// (event delivery guarantee). An assertion fires, when the event cannot
/// be delivered in this case.
/// delivered in this case.
///
bool QXThread::post_(QEvt const * const e,
                     std::uint_fast16_t const margin,
                     void const * const sender) noexcept
#else
bool QXThread::post_(QEvt const * const e,
                     std::uint_fast16_t const margin) noexcept
#endif
{
    bool status;
    QF_CRIT_STAT_
    QS_TEST_PROBE_DEF(&QXThread::post_)

    // is it the private time event?
    if (e == &m_timeEvt) {
        QF_CRIT_E_();
        // the private time event is disarmed and not in any queue,
        // so it is safe to change its signal. The signal of 0 means
        // that the time event has expired.
        m_timeEvt.sig = 0U;

        unblock_();
        QF_CRIT_X_();

        status = true;
    }
    // is the event queue provided?
    else if (m_eQueue.m_end != 0U) {

        /// @pre event pointer must be valid
        Q_REQUIRE_ID(300, e != nullptr);

        QF_CRIT_E_();
        QEQueueCtr nFree = m_eQueue.m_nFree; // get volatile into temporary

        // test-probe#1 for faking queue overflow
        QS_TEST_PROBE_ID(1,
            nFree = 0U;
        )

        if (margin == QF_NO_MARGIN) {
            if (nFree > 0U) {
                status = true; // can post
            }
            else {
                status = false; // cannot post
                Q_ERROR_CRIT_(310); // must be able to post the event
            }
        }
        else if (nFree > static_cast<QEQueueCtr>(margin)) {
            status = true; // can post
        }
        else {
            status = false; // cannot post, but don't assert
        }

        // is it a dynamic event?
        if (e->poolId_ != 0U) {
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        if (status) { // can post the event?

            --nFree;  // one free entry just used up
            m_eQueue.m_nFree = nFree; // update the volatile
            if (m_eQueue.m_nMin > nFree) {
                m_eQueue.m_nMin = nFree; // update minimum so far
            }

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
                QS_TIME_PRE_();        // timestamp
                QS_OBJ_PRE_(sender);   // the sender object
                QS_SIG_PRE_(e->sig);   // the signal of the event
                QS_OBJ_PRE_(this);     // this active object
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & refCtr
                QS_EQC_PRE_(nFree);    // number of free entries
                QS_EQC_PRE_(m_eQueue.m_nMin); // min number of free entries
            QS_END_NOCRIT_PRE_()

            // queue empty?
            if (m_eQueue.m_frontEvt == nullptr) {
                m_eQueue.m_frontEvt = e;  // deliver event directly

                // is this thread blocked on the queue?
                if (m_temp.obj == QXK_PTR_CAST_(QMState*, &m_eQueue)) {
                    static_cast<void>(teDisarm_());
                    QXK_attr_.readySet.insert(
                        static_cast<std::uint_fast8_t>(m_dynPrio));
                    if (!QXK_ISR_CONTEXT_()) {
                        static_cast<void>(QXK_sched_());
                    }
                }
            }
            // queue is not empty, insert event into the ring-buffer
            else {
                // insert event into the ring buffer (FIFO)
                QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_head) = e;

                // need to wrap the head couner?
                if (m_eQueue.m_head == 0U) {
                    m_eQueue.m_head = m_eQueue.m_end; // wrap around
                }
                --m_eQueue.m_head; // advance the head (counter clockwise)
            }

            QF_CRIT_X_();
        }
        else { // cannot post the event

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
                QS_TIME_PRE_();        // timestamp
                QS_OBJ_PRE_(sender);   // the sender object
                QS_SIG_PRE_(e->sig);   // the signal of the event
                QS_OBJ_PRE_(this);     // this active object (recipient)
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & ref Count
                QS_EQC_PRE_(nFree);    // number of free entries
                QS_EQC_PRE_(margin);   // margin
            QS_END_NOCRIT_PRE_()

            QF_CRIT_X_();

            QF::gc(e); // recycle the event to avoid a leak
        }
    }
    else { // the queue is not available
         QF::gc(e); // make sure the event is not leaked
         status = false;
         Q_ERROR_ID(320); // this extended thread cannot accept events
    }

    return status;
}

//****************************************************************************
/// @description
/// Last-In-First-Out (LIFO) policy is not supported for extened threads.
///
/// @param[in]  e  pointer to the event to post to the queue
///
/// @sa
/// QActive::postLIFO_()
///
void QXThread::postLIFO(QEvt const * const e) noexcept {
    static_cast<void>(e); // unused parameter
    Q_ERROR_ID(410);
}

//****************************************************************************
/// @description
/// The QXThread::queueGet() operation allows the calling extended thread to
/// receive QP events directly into its own built-in event queue from an ISR,
/// basic thread (AO), or another extended thread.
///
/// If QXThread::queueGet() is called when no events are present in the
/// thread's private event queue, the operation blocks the current extended
/// thread until either an event is received, or a user-specified timeout
/// expires.
///
/// @param[in]  nTicks    number of clock ticks (at the associated rate)
///                       to wait for the event to arrive. The value of
///                       QXTHREAD_NO_TIMEOUT indicates that no timeout will
///                       occur and the queue will block indefinitely.
/// @returns
/// A pointer to the event. If the pointer is not NULL, the event
/// was delivered. Otherwise the event pointer of NULL indicates that the
/// queue has timed out.
///
QEvt const *QXThread::queueGet(std::uint_fast16_t const nTicks) noexcept {
    QEQueueCtr nFree;
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    QXThread * const thr = QXTHREAD_CAST_(QXK_attr_.curr);

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - be called from an extended thread;
    /// - the thread must NOT be already blocked on any object.
    Q_REQUIRE_ID(500, (!QXK_ISR_CONTEXT_())
        && (thr != nullptr)
        && (thr->m_temp.obj == nullptr));
    /// @pre also: the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_ID(501, QXK_attr_.lockHolder != thr->m_prio);

    // is the queue empty? -- block and wait for event(s)
    if (thr->m_eQueue.m_frontEvt == nullptr) {

        // remember the blocking object (the thread's queue)
        thr->m_temp.obj = QXK_PTR_CAST_(QMState*, &thr->m_eQueue);

        thr->teArm_(static_cast<enum_t>(QXK_QUEUE_SIG), nTicks);
        QXK_attr_.readySet.rmove(
                           static_cast<std::uint_fast8_t>(thr->m_dynPrio));
        static_cast<void>(QXK_sched_());
        QF_CRIT_X_();
        QF_CRIT_EXIT_NOP(); // BLOCK here

        QF_CRIT_E_();
        // the blocking object must be this queue
        Q_ASSERT_ID(510, thr->m_temp.obj ==
                         QXK_PTR_CAST_(QMState *, &thr->m_eQueue));
        thr->m_temp.obj = nullptr; // clear
    }

    // is the queue not empty?
    if (thr->m_eQueue.m_frontEvt != nullptr) {
        e = thr->m_eQueue.m_frontEvt; // always remove from the front
        // volatile into tmp
        nFree= thr->m_eQueue.m_nFree + 1U;
        thr->m_eQueue.m_nFree = nFree; // update the number of free

        // any events in the ring buffer?
        if (nFree <= thr->m_eQueue.m_end) {

            // remove event from the tail
            thr->m_eQueue.m_frontEvt =
                QF_PTR_AT_(thr->m_eQueue.m_ring, thr->m_eQueue.m_tail);
            if (thr->m_eQueue.m_tail == 0U) {
                thr->m_eQueue.m_tail = thr->m_eQueue.m_end; // wrap
            }
            --thr->m_eQueue.m_tail;

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_GET, thr->m_prio)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(&thr);   // this active object
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & ref Count
                QS_EQC_PRE_(nFree);  // number of free entries
            QS_END_NOCRIT_PRE_()
        }
        else {
            thr->m_eQueue.m_frontEvt = nullptr; // the queue becomes empty

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_ID(520, nFree == (thr->m_eQueue.m_end + 1U));

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_GET_LAST, thr->m_prio)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(&thr);   // this active object
                QS_2U8_PRE_(e->poolId_, e->refCtr_); // poolID & ref Count
            QS_END_NOCRIT_PRE_()
        }
    }
    else { // the queue is still empty -- the timeout must have fired
         e = nullptr;
    }
    QF_CRIT_X_();

    return e;
}

//****************************************************************************
/// @description
/// Intenral implementation of blocking the given extended thread.
///
/// @note
/// Must be called from within a critical section
///
void QXThread::block_(void) const noexcept {
    /// @pre the thread holding the lock cannot block!
    Q_REQUIRE_ID(600, (QXK_attr_.lockHolder != m_prio));
    QXK_attr_.readySet.rmove(static_cast<std::uint_fast8_t>(m_dynPrio));
    static_cast<void>(QXK_sched_());
}

//****************************************************************************
/// @description
/// Intenral implementation of un-blocking the given extended thread.
///
/// @note
/// must be called from within a critical section
///
void QXThread::unblock_(void) const noexcept {
    QXK_attr_.readySet.insert(static_cast<std::uint_fast8_t>(m_dynPrio));

    if ((!QXK_ISR_CONTEXT_()) // not inside ISR?
        && (QF::active_[0] != nullptr)) // kernel started?
    {
        static_cast<void>(QXK_sched_());
    }
}

//****************************************************************************
/// @description
/// Intenral implementation of arming the private time event for
/// a given timeout at a given system tick rate.
///
/// @note
/// Must be called from within a critical section
///
void QXThread::teArm_(enum_t const sig,
                      std::uint_fast16_t const nTicks) noexcept
{
    /// @pre the time event must be unused
    Q_REQUIRE_ID(700, m_timeEvt.m_ctr == 0U);

    m_timeEvt.sig = static_cast<QSignal>(sig);

    if (nTicks != QXTHREAD_NO_TIMEOUT) {
        m_timeEvt.m_ctr = static_cast<QTimeEvtCtr>(nTicks);
        m_timeEvt.m_interval = 0U;

        // is the time event unlinked?
        // NOTE: For the duration of a single clock tick of the specified tick
        // rate a time event can be disarmed and yet still linked in the list,
        // because un-linking is performed exclusively in QF::tickX().
        if ((m_timeEvt.refCtr_ & TE_IS_LINKED) == 0U) {
            std::uint_fast8_t const tickRate =
                static_cast<std::uint_fast8_t>(m_timeEvt.refCtr_);
            m_timeEvt.refCtr_ |= TE_IS_LINKED; // mark as linked

            // The time event is initially inserted into the separate
            // "freshly armed" list based on QF::timeEvtHead_[tickRate].act.
            // Only later, inside QF::tickX() function, the "freshly armed"
            // list is appended to the main list of armed time events based on
            // QF_timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QF::tickX().
            m_timeEvt.m_next
                = QXK_PTR_CAST_(QTimeEvt*, QF::timeEvtHead_[tickRate].m_act);
            QF::timeEvtHead_[tickRate].m_act = &m_timeEvt;
        }
    }
}

//****************************************************************************
/// @description
/// Intenral implementation of disarming the private time event.
///
/// @note
/// Must be called from within a critical section
///
bool QXThread::teDisarm_(void) noexcept {
    bool wasArmed;
    // is the time evt running?
    if (m_timeEvt.m_ctr != 0U) {
        wasArmed = true;
        // schedule removal from list
        m_timeEvt.m_ctr = 0U;
    }
    // the time event was already automatically disarmed
    else {
        wasArmed = false;
    }
    return wasArmed;
}

//****************************************************************************
//! delay (timed blocking of) the current extended thread (static)
bool QXThread::delay(std::uint_fast16_t const nTicks) noexcept {
    QF_CRIT_STAT_

    QF_CRIT_E_();
    QXThread * const thr = QXK_PTR_CAST_(QXThread*, QXK_attr_.curr);

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - be called from an extended thread;
    /// - the thread must NOT be already blocked on any object.
    Q_REQUIRE_ID(800, (!QXK_ISR_CONTEXT_())
        && (thr != nullptr)
        && (thr->m_temp.obj == nullptr));
    /// @pre also: the thread must NOT be holding a scheduler lock
    Q_REQUIRE_ID(801, QXK_attr_.lockHolder != thr->m_prio);

    // remember the blocking object
    thr->m_temp.obj = QXK_PTR_CAST_(QMState*, &thr->m_timeEvt);
    thr->teArm_(static_cast<enum_t>(QXK_DELAY_SIG), nTicks);
    thr->block_();
    QF_CRIT_X_();
    QF_CRIT_EXIT_NOP(); // BLOCK here

    QF_CRIT_E_();
    // the blocking object must be the time event
    Q_ENSURE_ID(890, thr->m_temp.obj
                     == QXK_PTR_CAST_(QMState*, &thr->m_timeEvt));
    thr->m_temp.obj = nullptr; // clear
    QF_CRIT_X_();

    // signal of zero means that the time event was posted without
    // being canceled.
    return (thr->m_timeEvt.sig == 0U);
}

//****************************************************************************
//! cancel the delay
bool QXThread::delayCancel(void) noexcept {
    bool wasArmed;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    if (m_temp.obj == QXK_PTR_CAST_(QMState*, &m_timeEvt)) {
        wasArmed = teDisarm_();
        unblock_();
    }
    else {
        wasArmed = false;
    }
    QF_CRIT_X_();

    return wasArmed;
}

} // namespace QP


//****************************************************************************
extern "C" {

/// @description
/// Called when the extended-thread handler function returns.
///
/// @note
/// Most thread handler functions are structured as endless loops that never
/// return. But it is also possible to structure threads as one-shot functions
/// that perform their job and return. In that case this function peforms
/// cleanup after the thread.
///
void QXK_threadRet_(void) noexcept {
    QF_CRIT_STAT_

    QF_CRIT_E_();
    QP::QXThread * const thr = QXTHREAD_CAST_(QXK_attr_.curr);

    /// @pre this function must:
    /// - NOT be called from an ISR;
    /// - be called from an extended thread;
    Q_REQUIRE_ID(900, (!QXK_ISR_CONTEXT_())
        && (thr != nullptr));
    /// @pre also: the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_ID(901, QXK_attr_.lockHolder != thr->m_prio);

    std::uint_fast8_t const p =
        static_cast<std::uint_fast8_t>(thr->m_prio);

    // remove this thread from the QF
    QP::QF::active_[p] = nullptr;
    QXK_attr_.readySet.rmove(p);
    static_cast<void>(QXK_sched_());
    QF_CRIT_X_();
}

} // extern "C"

