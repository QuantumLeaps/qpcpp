//$file${src::qxk::qxk_xthr.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qxk::qxk_xthr.cpp}
//
// This code has been generated by QM 6.2.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2025-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${src::qxk::qxk_xthr.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QXK_HPP_
    #error "Source file included in a project NOT based on the QXK kernel"
#endif // QXK_HPP_

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qxk_xthr")
} // unnamed namespace

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U)%0x2710U))
#error qpcpp version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QXK::QXThread} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QXK::QXThread} ...........................................................

//${QXK::QXThread::QXThread} .................................................
QXThread::QXThread(
    QXThreadHandler const handler,
    std::uint_fast8_t const tickRate) noexcept
  : QActive(Q_STATE_CAST(handler)),
    m_timeEvt(this, static_cast<QSignal>(QXK::DELAY_SIG), tickRate)
{
    m_state.act = nullptr; // mark as extended thread
}

//${QXK::QXThread::init} .....................................................
void QXThread::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qsId);
    Q_ERROR_INCRIT(110);
}

//${QXK::QXThread::dispatch} .................................................
void QXThread::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(qsId);
    Q_ERROR_INCRIT(120);
}

//${QXK::QXThread::delay} ....................................................
bool QXThread::delay(QTimeEvtCtr const nTicks) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    QXThread * const thr = QXK_PTR_CAST_(QXThread*, QXK_priv_.curr);

    // precondition, this function:
    // - must NOT be called from an ISR;
    // - number of ticks cannot be zero
    // - be called from an extended thread;
    // - the thread must NOT be already blocked on any object.
    Q_REQUIRE_INCRIT(800, (!QXK_ISR_CONTEXT_())
        && (nTicks != 0U)
        && (thr != nullptr)
        && (thr->m_temp.obj == nullptr));
    // - the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_INCRIT(801, QXK_priv_.lockHolder != thr->m_prio);

    // remember the blocking object
    thr->m_temp.obj = QXK_PTR_CAST_(QMState const*, &thr->m_timeEvt);
    thr->teArm_(static_cast<enum_t>(QXK::DELAY_SIG), nTicks);
    thr->block_();

    QF_MEM_APP();
    QF_CRIT_EXIT();
    QF_CRIT_EXIT_NOP(); // BLOCK here

    // after unblocking...
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    // the blocking object must be the time event
    Q_ASSERT_INCRIT(890, thr->m_temp.obj
                     == QXK_PTR_CAST_(QMState*, &thr->m_timeEvt));
    thr->m_temp.obj = nullptr; // clear

    QF_MEM_APP();
    QF_CRIT_EXIT();

    // signal of zero means that the time event was posted without
    // being canceled.
    return (thr->m_timeEvt.sig == 0U);
}

//${QXK::QXThread::delayCancel} ..............................................
bool QXThread::delayCancel() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    bool wasArmed;
    if (m_temp.obj == QXK_PTR_CAST_(QMState*, &m_timeEvt)) {
        wasArmed = teDisarm_();
        unblock_();
    }
    else {
        wasArmed = false;
    }
    QF_MEM_APP();
    QF_CRIT_EXIT();

    return wasArmed;
}

//${QXK::QXThread::queueGet} .................................................
QEvt const * QXThread::queueGet(QTimeEvtCtr const nTicks) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    QXThread * const thr = QXTHREAD_CAST_(QXK_priv_.curr);

    // precondition, this function:
    // - must NOT be called from an ISR;
    // - be called from an extended thread;
    // - the thread must NOT be already blocked on any object.
    Q_REQUIRE_INCRIT(500, (!QXK_ISR_CONTEXT_())
        && (thr != nullptr)
        && (thr->m_temp.obj == nullptr));
    // - the thread must NOT be holding a scheduler lock.
    Q_REQUIRE_INCRIT(501, QXK_priv_.lockHolder != thr->m_prio);

    // is the queue empty?
    if (thr->m_eQueue.m_frontEvt == nullptr) {

        // remember the blocking object (the thread's queue)
        thr->m_temp.obj = QXK_PTR_CAST_(QMState*, &thr->m_eQueue);

        thr->teArm_(static_cast<enum_t>(QXK::TIMEOUT_SIG), nTicks);
        QXK_priv_.readySet.remove(
                           static_cast<std::uint_fast8_t>(thr->m_prio));
    #ifndef Q_UNSAFE
        QXK_priv_.readySet.update_(&QXK_priv_.readySet_dis);
    #endif

        static_cast<void>(QXK_sched_()); // synchronous scheduling

        QF_MEM_APP();
        QF_CRIT_EXIT();
        QF_CRIT_EXIT_NOP(); // BLOCK here

        // after unblocking...
        QF_CRIT_ENTRY();
        QF_MEM_SYS();

        // the blocking object must be this queue
        Q_ASSERT_INCRIT(510, thr->m_temp.obj ==
                         QXK_PTR_CAST_(QMState *, &thr->m_eQueue));
        thr->m_temp.obj = nullptr; // clear
    }

    // is the queue not empty?
    QEvt const *e;
    if (thr->m_eQueue.m_frontEvt != nullptr) {
        e = thr->m_eQueue.m_frontEvt; // remove from the front
        QEQueueCtr const nFree = thr->m_eQueue.m_nFree + 1U;
        thr->m_eQueue.m_nFree = nFree; // update the # free

        // any events in the ring buffer?
        if (nFree <= thr->m_eQueue.m_end) {

            // remove event from the tail
            thr->m_eQueue.m_frontEvt =
                thr->m_eQueue.m_ring[thr->m_eQueue.m_tail];
            if (thr->m_eQueue.m_tail == 0U) {
                thr->m_eQueue.m_tail = thr->m_eQueue.m_end; // wrap
            }
            // advance the tail (counter clockwise)
            thr->m_eQueue.m_tail = (thr->m_eQueue.m_tail - 1U);

            QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, thr->m_prio)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(&thr);   // this active object
                QS_2U8_PRE_(e->getPoolNum_(), e->refCtr_); // poolNum & refCtr
                QS_EQC_PRE_(nFree);  // # free entries
            QS_END_PRE_()
        }
        else {
            thr->m_eQueue.m_frontEvt = nullptr; // empty queue

            // all entries in the queue must be free (+1 for fronEvt)
            Q_ASSERT_INCRIT(520, nFree == (thr->m_eQueue.m_end + 1U));

            QS_BEGIN_PRE_(QS_QF_ACTIVE_GET_LAST, thr->m_prio)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(&thr);   // this active object
                QS_2U8_PRE_(e->getPoolNum_(), e->refCtr_); // poolNum & refCtr
            QS_END_PRE_()
        }
    }
    else { // the queue is still empty -- the timeout must have fired
         e = nullptr;
    }
    QF_MEM_APP();
    QF_CRIT_EXIT();

    return e;
}

//${QXK::QXThread::block_} ...................................................
void QXThread::block_() const noexcept {
    // NOTE: must be called IN a critical section

    Q_REQUIRE_INCRIT(600, (QXK_priv_.lockHolder != m_prio));

    QXK_priv_.readySet.remove(static_cast<std::uint_fast8_t>(m_prio));
    #ifndef Q_UNSAFE
    QXK_priv_.readySet.update_(&QXK_priv_.readySet_dis);
    #endif

    static_cast<void>(QXK_sched_()); // schedule other threads
}

//${QXK::QXThread::unblock_} .................................................
void QXThread::unblock_() const noexcept {
    // NOTE: must be called IN a critical section

    QXK_priv_.readySet.insert(static_cast<std::uint_fast8_t>(m_prio));
    #ifndef Q_UNSAFE
    QXK_priv_.readySet.update_(&QXK_priv_.readySet_dis);
    #endif

    if ((!QXK_ISR_CONTEXT_()) // not inside ISR?
        && (QActive::registry_[0] != nullptr)) // kernel started?
    {
        static_cast<void>(QXK_sched_()); // schedule other threads
    }
}

//${QXK::QXThread::timeout_} .................................................
void QXThread::timeout_(QActive * const act) {
    // NOTE: must be called IN a critical section

    // the private time event is now disarmed and not in any queue,
    // so it is safe to change its signal. The signal of 0 means
    // that the time event has expired.
    QXTHREAD_CAST_(act)->m_timeEvt.sig = 0U;

    QXTHREAD_CAST_(act)->unblock_();
}

//${QXK::QXThread::teArm_} ...................................................
void QXThread::teArm_(
    enum_t const sig,
    QTimeEvtCtr const nTicks) noexcept
{
    // NOTE: must be called IN a critical section

    // precondition:
    // - the time event must be unused
    Q_REQUIRE_INCRIT(700, m_timeEvt.m_ctr == 0U);

    m_timeEvt.sig = static_cast<QSignal>(sig);

    if (nTicks != QXTHREAD_NO_TIMEOUT) {
        m_timeEvt.m_ctr = static_cast<QTimeEvtCtr>(nTicks);
        m_timeEvt.m_interval = 0U;

        // is the time event unlinked?
        // NOTE: For the duration of a single clock tick of the specified tick
        // rate a time event can be disarmed and yet still linked in the list,
        // because un-linking is performed exclusively in QTimeEvt::tickX().
        if (static_cast<std::uint8_t>(m_timeEvt.refCtr_ & TE_IS_LINKED) == 0U)
        {
            std::uint_fast8_t const tickRate =
                static_cast<std::uint_fast8_t>(m_timeEvt.refCtr_);
            Q_ASSERT_INCRIT(710, tickRate < QF_MAX_TICK_RATE);

            // mark as linked
            m_timeEvt.refCtr_ = static_cast<std::uint8_t>(
                m_timeEvt.refCtr_ | TE_IS_LINKED);

            // The time event is initially inserted into the separate
            // "freshly armed" list based on timeEvtHead_[tickRate].act.
            // Only later, inside QTimeEvt::tick(), the "freshly armed"
            // list is appended to the main list of armed time events based on
            // timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QTimeEvt::tick().
            m_timeEvt.m_next
                = QXK_PTR_CAST_(QTimeEvt*,
                                QTimeEvt::timeEvtHead_[tickRate].m_act);
            QTimeEvt::timeEvtHead_[tickRate].m_act = &m_timeEvt;
        }
    }
}

//${QXK::QXThread::teDisarm_} ................................................
bool QXThread::teDisarm_() noexcept {
    // NOTE: must be called IN a critical section

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

} // namespace QP
//$enddef${QXK::QXThread} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
