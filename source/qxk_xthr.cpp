/// @file
/// @brief QXK/C++ preemptive kernel extended (blocking) thread implementation
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 5.6.2
/// Last updated on  2016-03-31
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
//! constructor of a "naked" thread
QXThread::QXThread(QXThreadHandler const handler, uint_fast8_t const tickRate)
  : QMActive(Q_STATE_CAST(handler)),
    m_timeEvt(this, static_cast<enum_t>(QXK_DELAY_SIG),
                    static_cast<uint8_t>(tickRate))
{
    m_state.act = Q_ACTION_CAST(0); // mark as naked thread
}

//****************************************************************************
// QXThread virtual function implementations...
void QXThread::init(QEvt const * const /*e*/) {
    Q_ERROR_ID(100);
}

//****************************************************************************
void QXThread::dispatch(QEvt const * const /*e*/) {
    Q_ERROR_ID(200);
}

//****************************************************************************
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

    // "naked" threads provide their thread function in place of
    // the top-most initial transition 'me->super.temp.act'
    QXK_stackInit_(this, reinterpret_cast<QXThreadHandler>(m_temp.act),
                   stkSto, stkSize);

    m_prio = prio;
    QF::add_(this); // make QF aware of this naked thread

    QF_CRIT_ENTRY_();
    QXK_attr_.readySet.insert(m_prio);

    // is QXK running?
    if (QXK_attr_.curr != static_cast<QMActive *>(0)) {
        QXK_sched_();
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
#ifndef Q_SPY
bool QXThread::post_(QEvt const * const e, uint_fast16_t const margin)
#else
bool QXThread::post_(QEvt const * const e, uint_fast16_t const margin,
                     void const * const sender)
#endif
{
    bool stat;
    QF_CRIT_STAT_

    // is it the private time event?
    if (e == &m_timeEvt) {
        QF_CRIT_ENTRY_();
        stat = true;
        // the private time event is disarmed and not in any queue,
        // so it is safe to change its signal. The signal of 0 means
        // that the time event __has__ expired.
        m_timeEvt.sig = static_cast<QSignal>(0);

        unblock();
        QF_CRIT_EXIT_();
    }
    // is the event queue provided?
    else if (m_eQueue.m_end != static_cast<QEQueueCtr>(0)) {
        QF_CRIT_ENTRY_();
        (void)teDisarm_();
        QF_CRIT_EXIT_();

        stat = POST_X(e, margin, sender);
    }
    else { // the queue is not available
         QF::gc(e); // make sure the event is not leaked
         stat = false;
         Q_ERROR_ID(410);
    }

    return stat;
}

//****************************************************************************
void QXThread::postLIFO(QEvt const * const /*e*/) {
    Q_ERROR_ID(510);
}

//****************************************************************************
// must be called from within a critical section
void QXThread::block_(void) const {
    /// @pre the thread holding the lock cannot block!
    Q_REQUIRE_ID(100,  m_prio != QXK_attr_.lockPrio);
    QXK_attr_.readySet.remove(m_prio);
    QXK_sched_();
}

//****************************************************************************
// must be called from within a critical section
void QXThread::unblock_(void) const {
    QXK_attr_.readySet.insert(m_prio);

    if ((!QXK_ISR_CONTEXT_()) // not inside ISR?
        && (QXK_attr_.curr != static_cast<void *>(0))) // kernel started?
    {
        QXK_sched_();
    }
}

//****************************************************************************
// must be called from within a critical section
void QXThread::teArm_(enum_t const sig,
                      uint_fast16_t const nTicks, uint_fast8_t const tickRate)
{
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
// must be called from within a critical section
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
//! block (suspend) the current "naked" thread
void QXThread::block(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QXThread *thr = static_cast<QXThread *>(QXK_attr_.curr);
    Q_REQUIRE_ID(700, (!QXK_ISR_CONTEXT_()) /* can't block inside an ISR */
        /* this must be a "naked" thread (no state) */
        && (thr->m_state.act == static_cast<QActionHandler>(0)));
    thr->block_();
    QF_CRIT_EXIT_();
}

//****************************************************************************
//! unblock (resume) a given "naked" thread
void QXThread::unblock(void) const {
    QF_CRIT_STAT_

    // the unblocked thread must be a "naked" thread (no state)
    Q_REQUIRE_ID(800, m_state.act == (QActionHandler)0);

    QF_CRIT_ENTRY_();
    unblock_();
    QF_CRIT_EXIT_();
}

//****************************************************************************
//! delay (timed block) the current "naked" thread
bool QXThread::delay(uint_fast16_t const nTicks,
                     uint_fast8_t const tickRate)
{
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QXThread *thr = static_cast<QXThread *>(QXK_attr_.curr);
    // remember the blocking object
    thr->m_temp.obj = reinterpret_cast<QMState const *>(&thr->m_timeEvt);
    thr->teArm_(static_cast<enum_t>(QXK_DELAY_SIG), nTicks, tickRate);
    thr->block_();
    QF_CRIT_EXIT_();

    // signal of zero means that the time event was posted without
    // being canceled.
    return (thr->m_timeEvt.sig == static_cast<QSignal>(0));
}

//****************************************************************************
//! cancel the delay
bool QXThread::delayCancel(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    bool wasArmed = teDisarm_();
    unblock_();
    QF_CRIT_EXIT_();

    return wasArmed;
}

//****************************************************************************
//! obtain a message from the private message queue (block if no messages)
void const *QXThread::queueGet(uint_fast16_t const nTicks,
                               uint_fast8_t const tickRate)
{
    QEQueueCtr nFree;
    QEvt const *e;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    QXThread *thr = static_cast<QXThread *>(QXK_attr_.curr);

    Q_REQUIRE_ID(900, (!QXK_ISR_CONTEXT_()) /* can't block inside an ISR */
        /* this must be a "naked" thread (no state) */
        && (thr->m_state.act == (QActionHandler)0));

    // is the queue empty? -- block and wait for event(s)
    if (thr->m_eQueue.m_frontEvt == static_cast<QEvt *>(0)) {
        thr->m_temp.obj = reinterpret_cast<QMState const *>(&thr->m_eQueue);
        thr->teArm_(static_cast<enum_t>(QXK_QUEUE_SIG), nTicks, tickRate);
        QXK_attr_.readySet.remove(thr->m_prio);
        QXK_sched_();
        QF_CRIT_EXIT_();
        QF_CRIT_EXIT_NOP();
        QF_CRIT_ENTRY_();
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

            QS_BEGIN_NOCRIT_(QP::QS_QF_ACTIVE_GET, QP::QS::priv_.aoObjFilter,
                             thr)
                QS_TIME_();                   // timestamp
                QS_SIG_(e->sig);              // the signal of this event
                QS_OBJ_(&thr);                // this active object
                QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
                QS_EQC_(nFree);               // number of free entries
            QS_END_NOCRIT_()
        }
        else {
            // the queue becomes empty
            thr->m_eQueue.m_frontEvt = static_cast<QEvt const *>(0);

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_ID(910, nFree ==
                             (thr->m_eQueue.m_end +static_cast<QEQueueCtr>(1)));

            QS_BEGIN_NOCRIT_(QP::QS_QF_ACTIVE_GET_LAST, QP::QS::priv_.aoObjFilter,
                             thr)
                QS_TIME_();                   // timestamp
                QS_SIG_(e->sig);              // the signal of this event
                QS_OBJ_(&thr);                // this active object
                QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_END_NOCRIT_()
        }
    }
    else { // the queue is still empty -- the timeout must have fired
         e = static_cast<QEvt const *>(0);
    }
    QF_CRIT_EXIT_();

    return e;
}

} // namespace QP
