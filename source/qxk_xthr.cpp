/// @file
/// @brief QXK/C++ preemptive kernel extended (blocking) thread implementation
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 5.8.0
/// Last updated on  2016-11-19
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
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

Q_DEFINE_THIS_MODULE("qxk_xthr")

//****************************************************************************
/// @description
/// Performs the first step of QXThread initialization by assigning the
/// thread-handler function and the tick rate at which it will handle
/// the timeouts.
///
/// @param[in]     handler  the thread-handler function
/// @param[in]     tickRate the system clock tick rate to use for timeouts
///
/// @note  Must be called only ONCE before QXThread::start().
///
/// @usage
/// The following example illustrates how to invoke the QXThread ctor in the
/// main() function
/// @include qxk_xthread_ctor.cpp
///
QXThread::QXThread(QXThreadHandler const handler, uint_fast8_t const tickRate)
  : QActive(Q_STATE_CAST(handler)),
    m_timeEvt(this, static_cast<enum_t>(QXK_DELAY_SIG),
                    static_cast<uint8_t>(tickRate))
{
    m_state.act = Q_ACTION_CAST(0); // mark as extended thread
}

//****************************************************************************
// QXThread virtual function implementations...
void QXThread::init(QEvt const * const /*e*/) {
    Q_ERROR_ID(110);
}

//****************************************************************************
void QXThread::dispatch(QEvt const * const /*e*/) {
    Q_ERROR_ID(120);
}

//****************************************************************************
///
/// @description
/// Starts an extended thread and registers it with the framework.
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
/// @param[in]     ie      pointer to the initial event (not used).
///
/// @note This function should be called via the macro QXTHREAD_START().
///
/// @usage
/// The following example shows starting an extended thread:
/// @include qxk_xthread_start.cpp
///
void QXThread::start(uint_fast8_t const prio,
                       QEvt const *qSto[], uint_fast16_t const qLen,
                       void * const stkSto, uint_fast16_t const stkSize,
                       QEvt const * const /*ie*/)
{
    QF_CRIT_STAT_

    Q_REQUIRE_ID(300, (!QXK_ISR_CONTEXT_()) /* don't start AO's in an ISR! */
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
        && (stkSto != static_cast<void *>(0))
        && (stkSize != static_cast<uint_fast16_t>(0))
        && (m_state.act == static_cast<QActionHandler>(0)));

    // is storage for the queue buffer provided?
    if (qSto != static_cast<QEvt const **>(0)) {
        m_eQueue.init(qSto, qLen);
    }

    // extended threads provide their thread function in place of
    // the top-most initial transition 'm_temp.act'
    QXK_stackInit_(this, reinterpret_cast<QXThreadHandler>(m_temp.act),
                   stkSto, stkSize);

    m_prio = prio;

    QF_CRIT_ENTRY_();
    QF::add_(this); // make QF aware of this extended thread

    // the new thread is not blocked on any object
    m_temp.obj = static_cast<QMState const *>(0);

    // extended-thread becomes ready immediately
    QXK_attr_.readySet.insert(m_prio);

    // see if this thread needs to be scheduled in case QXK is running
    (void)QXK_sched_();
    QF_CRIT_EXIT_();
}

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
/// @param[in]     e      pointer to the event to be posted
/// @param[in]     margin number of required free slots in the queue
///                       after posting the event.
///
/// @note this function should be called only via the macro QXTHREAD_POST_X().
///
/// @note The zero value of the @p margin parameter is special and denotes
/// situation when the post() operation is assumed to succeed (event delivery
/// guarantee). An assertion fires, when the event cannot be delivered in
/// this case.
///
#ifndef Q_SPY
bool QXThread::post_(QEvt const * const e, uint_fast16_t const margin)
#else
bool QXThread::post_(QEvt const * const e, uint_fast16_t const margin,
                     void const * const sender)
#endif
{
    bool status;
    QF_CRIT_STAT_

    // is it the private time event?
    if (e == &m_timeEvt) {
        QF_CRIT_ENTRY_();
        status = true;
        // the private time event is disarmed and not in any queue,
        // so it is safe to change its signal. The signal of 0 means
        // that the time event has expired.
        m_timeEvt.sig = static_cast<QSignal>(0);

        unblock_();
        QF_CRIT_EXIT_();
    }
    // is the event queue provided?
    else if (m_eQueue.m_end != static_cast<QEQueueCtr>(0)) {

        /// @pre event pointer must be valid
        Q_REQUIRE_ID(300, e != static_cast<QEvt const *>(0));

        QF_CRIT_ENTRY_();

        QEQueueCtr nFree = m_eQueue.m_nFree; // get volatile into temporary

        // margin available?
        if (nFree > static_cast<QEQueueCtr>(margin)) {

            QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO,
                             QS::priv_.aoObjFilter, this)
                QS_TIME_();               // timestamp
                QS_OBJ_(sender);          // the sender object
                QS_SIG_(e->sig);          // the signal of the event
                QS_OBJ_(this);            // this active object
                QS_2U8_(e->poolId_, e->refCtr_); // poolID & refCtr of the evt
                QS_EQC_(nFree);           // number of free entries
                QS_EQC_(m_eQueue.m_nMin); // min number of free entries
            QS_END_NOCRIT_()

            // is it a pool event?
            if (e->poolId_ != static_cast<uint8_t>(0)) {
                QF_EVT_REF_CTR_INC_(e); // increment the reference counter
            }

            --nFree;  // one free entry just used up
            m_eQueue.m_nFree = nFree;     // update the volatile
            if (m_eQueue.m_nMin > nFree) {
                m_eQueue.m_nMin = nFree;  // update minimum so far
            }

            // is the queue empty?
            if (m_eQueue.m_frontEvt == static_cast<QEvt const *>(0)) {
                m_eQueue.m_frontEvt = e;  // deliver event directly

                // is this thread blocked on the queue?
                if (m_temp.obj
                    == reinterpret_cast<QMState const *>(&m_eQueue))
                {
                    (void)teDisarm_();
                    QXK_attr_.readySet.insert(m_prio);
                    if (!QXK_ISR_CONTEXT_()) {
                        (void)QXK_sched_();
                    }
                }
            }
            // queue is not empty, insert event into the ring-buffer
            else {
                // insert event into the ring buffer (FIFO)
                QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_head) = e;

                // need to wrap head?
                if (m_eQueue.m_head == static_cast<QEQueueCtr>(0)) {
                    m_eQueue.m_head = m_eQueue.m_end; // wrap around
                }
                --m_eQueue.m_head;
            }
            QF_CRIT_EXIT_();

            status = true; // event posted successfully
        }
        else {
            /// @note assert if event cannot be posted and dropping events is
            /// not acceptable
            Q_ASSERT_ID(310, margin != static_cast<uint_fast16_t>(0));

            QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT, QS::priv_.aoObjFilter,
                             this)
                QS_TIME_();        // timestamp
                QS_OBJ_(sender);   // the sender object
                QS_SIG_(e->sig);   // the signal of the event
                QS_OBJ_(this);     // this active object
                QS_2U8_(e->poolId_, e->refCtr_); // poolID & refCtr of the evt
                QS_EQC_(nFree);    // number of free entries
                QS_EQC_(static_cast<QEQueueCtr>(margin)); // margin requested
            QS_END_NOCRIT_()

            QF_CRIT_EXIT_();

            QF::gc(e); // recycle the evnet to avoid a leak
            status = false; // event not posted
        }
    }
    else { // the queue is not available
         QF::gc(e); // make sure the event is not leaked
         status = false;
         Q_ERROR_ID(320);
    }

    return status;
}

//****************************************************************************
/// @description
/// Last-In-First-Out (LIFO) policy is not supported for extened threads.
///
/// @param[in  e  pointer to the event to post to the queue
///
/// @sa QActive_postLIFO_()
///
void QXThread::postLIFO(QEvt const * const /*e*/) {
    Q_ERROR_ID(410);
}

//****************************************************************************
/// @description
/// The QXThread_queueGet() operation allows the calling extended thread to
/// receive QP events directly into its own built-in event queue from an ISR,
/// basic thread (AO), or another extended thread.
///
/// If QXThread_queueGet() is called when no events are present in the thread’s
/// event queue, the operation blocks the current extended thread until either
/// an event is received, or a user-specified timeout expires.
///
/// @param[in]  nTicks    number of clock ticks (at the associated rate)
///                       to wait for the event to arrive. The value of
///                       QXTHREAD_NO_TIMEOUT indicates that no timeout will
///                       occur and the queue will block indefinitely.
/// @param[in]  tickRate  system clock tick rate serviced in this call.
///
/// @returns
/// Returns pointer to the event. If the pointer is not NULL, the event
/// was delivered. Otherwise the event pointer of NULL indicates that the
/// queue has timed out.
///
QEvt const *QXThread::queueGet(uint_fast16_t const nTicks,
                               uint_fast8_t const tickRate)
{
    QEQueueCtr nFree;
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QXThread *thr = static_cast<QXThread *>(QXK_attr_.curr);

    Q_REQUIRE_ID(500, (!QXK_ISR_CONTEXT_()) /* can't block inside an ISR */
        && (thr != static_cast<QXThread *>(0)) /* current must be extended */
        && (thr->m_temp.obj == static_cast<QMState const *>(0))); // !blocked

    // is the queue empty? -- block and wait for event(s)
    if (thr->m_eQueue.m_frontEvt == static_cast<QEvt *>(0)) {

        // remember the blocking object (the thread's queue)
        thr->m_temp.obj = reinterpret_cast<QMState const *>(&thr->m_eQueue);

        thr->teArm_(static_cast<enum_t>(QXK_QUEUE_SIG), nTicks, tickRate);
        QXK_attr_.readySet.remove(thr->m_prio);
        (void)QXK_sched_();
        QF_CRIT_EXIT_();
        QF_CRIT_EXIT_NOP(); // BLOCK here

        QF_CRIT_ENTRY_();
        // the blocking object must be this queue
        Q_ASSERT_ID(510, thr->m_temp.obj ==
                         reinterpret_cast<QMState const *>(&thr->m_eQueue));
        thr->m_temp.obj = static_cast<QMState const *>(0); // clear
    }

    // is the queue not empty?
    if (thr->m_eQueue.m_frontEvt != static_cast<QEvt *>(0)) {
        e = thr->m_eQueue.m_frontEvt; // always remove from the front
        // volatile into tmp
        nFree= thr->m_eQueue.m_nFree + static_cast<QEQueueCtr>(1);
        thr->m_eQueue.m_nFree = nFree; // update the number of free

        // any events in the ring buffer?
        if (nFree <= thr->m_eQueue.m_end) {

            // remove event from the tail
            thr->m_eQueue.m_frontEvt =
                QF_PTR_AT_(thr->m_eQueue.m_ring, thr->m_eQueue.m_tail);
            if (thr->m_eQueue.m_tail == static_cast<QEQueueCtr>(0)) {
                thr->m_eQueue.m_tail = thr->m_eQueue.m_end;  // wrap
            }
            --thr->m_eQueue.m_tail;

            QS_BEGIN_NOCRIT_(QP::QS_QF_ACTIVE_GET,
                             QP::QS::priv_.aoObjFilter, thr)
                QS_TIME_();      // timestamp
                QS_SIG_(e->sig); // the signal of this event
                QS_OBJ_(&thr);   // this active object
                QS_2U8_(e->poolId_, e->refCtr_); // poolID & ref Count
                QS_EQC_(nFree);  // number of free entries
            QS_END_NOCRIT_()
        }
        else {
            // the queue becomes empty
            thr->m_eQueue.m_frontEvt = static_cast<QEvt const *>(0);

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_ID(520, nFree == (thr->m_eQueue.m_end
                                       + static_cast<QEQueueCtr>(1)));

            QS_BEGIN_NOCRIT_(QP::QS_QF_ACTIVE_GET_LAST,
                             QP::QS::priv_.aoObjFilter, thr)
                QS_TIME_();      // timestamp
                QS_SIG_(e->sig); // the signal of this event
                QS_OBJ_(&thr);   // this active object
                QS_2U8_(e->poolId_, e->refCtr_); // poolID & ref Count
            QS_END_NOCRIT_()
        }
    }
    else { // the queue is still empty -- the timeout must have fired
         e = static_cast<QEvt const *>(0);
    }
    QF_CRIT_EXIT_();

    return e;
}

//****************************************************************************
/// @description
/// Intenral implementation of blocking the given extended thread.
///
/// @note
/// must be called from within a critical section
///
void QXThread::block_(void) const {
    /// @pre the thread holding the lock cannot block!
    Q_REQUIRE_ID(600,  m_prio != QXK_attr_.lockPrio);
    QXK_attr_.readySet.remove(m_prio);
    (void)QXK_sched_();
}

//****************************************************************************
/// @description
/// Intenral implementation of un-blocking the given extended thread.
///
/// @note
/// must be called from within a critical section
///
void QXThread::unblock_(void) const {
    QXK_attr_.readySet.insert(m_prio);

    if ((!QXK_ISR_CONTEXT_()) // not inside ISR?
        && (QF::active_[0] != static_cast<QActive *>(0))) // kernel started?
    {
        (void)QXK_sched_();
    }
}

//****************************************************************************
/// @description
/// Intenral implementation of arming the private time event for
/// a given timeout at a given system tick rate.
///
/// @note
/// must be called from within a critical section
///
void QXThread::teArm_(enum_t const sig,
                      uint_fast16_t const nTicks,
                      uint_fast8_t const tickRate)
{
    // the time event must be unused
    Q_REQUIRE_ID(700, m_timeEvt.m_ctr == static_cast<QTimeEvtCtr>(0));

    m_timeEvt.sig = static_cast<QSignal>(sig);

    if (nTicks != QXTHREAD_NO_TIMEOUT) {
        m_timeEvt.m_ctr = static_cast<QTimeEvtCtr>(nTicks);
        m_timeEvt.m_interval = static_cast<QTimeEvtCtr>(0);

        // is the time event unlinked?
        // NOTE: For the duration of a single clock tick of the specified tick
        // rate a time event can be disarmed and yet still linked in the list,
        // because un-linking is performed exclusively in QF_tickX().
        //
        if ((m_timeEvt.refCtr_ & static_cast<uint8_t>(0x80))
            == static_cast<uint8_t>(0))
        {
            m_timeEvt.refCtr_ |= static_cast<uint8_t>(0x80); // mark as linked

            // The time event is initially inserted into the separate
            // "freshly armed" list based on QF::timeEvtHead_[tickRate].act.
            // Only later, inside QF::tickX() function, the "freshly armed"
            // list is appended to the main list of armed time events based on
            // QF_timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QF::tickX().
            m_timeEvt.m_next =
                static_cast<QTimeEvt *>(QF::timeEvtHead_[tickRate].m_act);
            QF::timeEvtHead_[tickRate].m_act = &m_timeEvt;
        }
    }
}

//****************************************************************************
/// @description
/// Intenral implementation of disarming the private time event.
///
/// @note
/// must be called from within a critical section
///
bool QXThread::teDisarm_(void) {
    bool wasArmed;
    // is the time evt running?
    if (m_timeEvt.m_ctr != static_cast<QTimeEvtCtr>(0)) {
        wasArmed = true;
        // schedule removal from list
        m_timeEvt.m_ctr = static_cast<QTimeEvtCtr>(0);
    }
    // the time event was already automatically disarmed
    else {
        wasArmed = false;
    }
    return wasArmed;
}

//****************************************************************************
//! delay (timed block) the current extended thread
bool QXThread::delay(uint_fast16_t const nTicks,
                     uint_fast8_t const tickRate)
{
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QXThread *thr = static_cast<QXThread *>(QXK_attr_.curr);

    // the delaying thread must not be blocked on any object
    Q_REQUIRE_ID(900, thr->m_temp.obj == static_cast<QMState const *>(0));

    // remember the blocking object
    thr->m_temp.obj = reinterpret_cast<QMState const *>(&thr->m_timeEvt);
    thr->teArm_(static_cast<enum_t>(QXK_DELAY_SIG), nTicks, tickRate);
    thr->block_();
    QF_CRIT_EXIT_();
    QF_CRIT_EXIT_NOP(); // BLOCK here

    QF_CRIT_ENTRY_();
    // the blocking object must be the time event
    Q_ENSURE_ID(990, thr->m_temp.obj ==
                     reinterpret_cast<QMState const *>(&thr->m_timeEvt));
    thr->m_temp.obj = static_cast<QMState const *>(0); // clear
    QF_CRIT_EXIT_();

    // signal of zero means that the time event was posted without
    // being canceled.
    return (thr->m_timeEvt.sig == static_cast<QSignal>(0));
}

//****************************************************************************
//! cancel the delay
bool QXThread::delayCancel(void) {
    bool wasArmed;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    if (m_temp.obj == reinterpret_cast<QMState const *>(&m_timeEvt)) {
        wasArmed = teDisarm_();
        unblock_();
    }
    else {
        wasArmed = false;
    }
    QF_CRIT_EXIT_();

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
void QXK_threadRet_(void) {
    uint_fast8_t p;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    p = static_cast<QP::QActive *>(QXK_attr_.curr)->m_prio;
    // remove this thread from the QF
    QP::QF::active_[p] = static_cast<QP::QActive *>(0);
    QXK_attr_.readySet.remove(p);
    (void)QXK_sched_();
    QF_CRIT_EXIT_();
}

} // extern "C"
