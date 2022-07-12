//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-12
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QF/C++ port to ThreadX, all supported compilers

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------
static void thread_function(ULONG thread_input); // prototype

//............................................................................
void QF::init(void) {
}
//............................................................................
int_t QF::run(void) {
    onStartup();  // the startup callback

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    return 0; // return success
}
//............................................................................
void QF::stop(void) {
    onCleanup(); // the cleanup callback
}
//............................................................................
void QActive::thread_(QActive *act) {
    // event loop of the active object thread
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
}
//............................................................................
static void thread_function(ULONG thread_input) { // ThreadX signature
    // run the active-object thread
    QActive::thread_(reinterpret_cast<QActive *>(thread_input));
}
//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    // allege that the ThreadX queue is created successfully
    Q_ALLEGE_ID(210,
        tx_queue_create(&m_eQueue,
            m_thread.tx_thread_name, // the same name as thread
            TX_1_ULONG,
            static_cast<VOID *>(qSto),
            static_cast<ULONG>(qLen * sizeof(ULONG)))
        == TX_SUCCESS);

    m_prio = prio;  // save the QF priority
    register_(); // make QF aware of this active object

    init(par, m_prio); // execute initial transition
    QS_FLUSH();     // flush the trace buffer to the host

    // convert QF priority to the ThreadX priority
    UINT tx_prio = QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - prio;

    Q_ALLEGE_ID(220,
        tx_thread_create(
            &m_thread, // ThreadX thread control block
            m_thread.tx_thread_name,   // unique thread name
            &thread_function, // thread function
            reinterpret_cast<ULONG>(this), // thread parameter
            stkSto,    // stack start
            stkSize,   // stack size in bytes
            tx_prio,   // ThreadX priority
            tx_prio,   // preemption threshold disabled (same as priority)
            TX_NO_TIME_SLICE,
            TX_AUTO_START)
        == TX_SUCCESS);
}
//............................................................................
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    // this function must be called before QACTIVE_START(),
    // which implies that me->thread.tx_thread_name must not be used yet;
    Q_REQUIRE_ID(300, m_thread.tx_thread_name == nullptr);
    switch (attr1) {
        case THREAD_NAME_ATTR:
            // temporarily store the name, cast 'const' away
            m_thread.tx_thread_name = static_cast<char *>(
                                          const_cast<void *>(attr2));
            break;
        // ...
    }
}
//............................................................................
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
{
    bool status;
    QF_CRIT_STAT_
    QF_CRIT_E_();

    std::uint_fast16_t nFree =
        static_cast<std::uint_fast16_t>(m_eQueue.tx_queue_available_storage);

    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_ID(510); // must be able to post the event
        }
    }
    else if (nFree > margin) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();       // timestamp
            QS_OBJ_PRE_(sender);  // the sender object
            QS_SIG_PRE_(e->sig);  // the signal of the event
            QS_OBJ_PRE_(this);    // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of evt
            QS_EQC_PRE_(nFree);   // # free entries
            QS_EQC_PRE_(0U);      // min # free (unknown)
        QS_END_NOCRIT_PRE_()

        // is it a pool event?
        if (e->poolId_ != 0U) {
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        QF_CRIT_X_();

        // posting to the ThreadX queue must succeed
        Q_ALLEGE_ID(520,
            tx_queue_send(&m_eQueue, const_cast<QEvt **>(&e), TX_NO_WAIT)
            == TX_SUCCESS);
    }
    else {

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();       // timestamp
            QS_OBJ_PRE_(sender);  // the sender object
            QS_SIG_PRE_(e->sig);  // the signal of the event
            QS_OBJ_PRE_(this);    // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of evt
            QS_EQC_PRE_(nFree);   // # free entries
            QS_EQC_PRE_(0U);      // min # free (unknown)
        QS_END_NOCRIT_PRE_()

        QF_CRIT_X_();
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        // # free entries available
        QS_EQC_PRE_(m_eQueue.tx_queue_available_storage);
        QS_EQC_PRE_(0U); // min # free entries (unknown)
    QS_END_NOCRIT_PRE_()

    // is it a pool event?
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e);  // increment the reference counter
    }

    QF_CRIT_X_();

    // LIFO posting the ThreadX queue must succeed
    Q_ALLEGE_ID(610,
        tx_queue_front_send(&m_eQueue, const_cast<QEvt **>(&e), TX_NO_WAIT)
        == TX_SUCCESS);
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    QEvt const *e;
    QS_CRIT_STAT_

    Q_ALLEGE_ID(710,
        tx_queue_receive(&m_eQueue, (VOID *)&e, TX_WAIT_FOREVER)
        == TX_SUCCESS);

    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        // min # free entries
        QS_EQC_PRE_(m_eQueue.tx_queue_available_storage);
    QS_END_PRE_()

    return e;
}

//............................................................................
void QFSchedLock::lock(std::uint_fast8_t prio) {
    m_lockHolder = tx_thread_identify();

    //! @pre must be thread level, so current TX thread must be available
    Q_REQUIRE_ID(800, m_lockHolder != nullptr);

    UINT tx_err = tx_thread_preemption_change(m_lockHolder,
                     (QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - prio),
                     &m_prevThre);

    if (tx_err == TX_SUCCESS) {
        QS_CRIT_STAT_
        m_lockPrio = prio;

        QS_BEGIN_PRE_(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE_(); // timestamp
            QS_2U8_PRE_(QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - m_prevThre,
                        prio); // new lock prio
        QS_END_PRE_()
    }
    else if (tx_err == TX_THRESH_ERROR) {
        // threshold was greater than (lower prio) than the current prio
        m_lockPrio = 0U; // threshold not changed
    }
    else {
        /* no other errors are tolerated */
        Q_ERROR_ID(810);
    }
}

//............................................................................
void QFSchedLock::unlock(void) const {
    QS_CRIT_STAT_

    //! @pre the lock holder TX thread must be available
    Q_REQUIRE_ID(900, (m_lockHolder != nullptr)
                      && (m_lockPrio != 0U));

    QS_BEGIN_PRE_(QS_SCHED_UNLOCK, 0U)
        QS_TIME_PRE_(); // timestamp
        QS_2U8_PRE_(m_lockPrio,/* prev lock prio */
                    QF_TX_PRIO_OFFSET + QF_MAX_ACTIVE - m_prevThre);
    QS_END_PRE_()

    // restore the preemption threshold of the lock holder
    UINT old_thre;
    Q_ALLEGE_ID(910, tx_thread_preemption_change(m_lockHolder,
                     m_prevThre,
                     &old_thre) == TX_SUCCESS);
}

} // namespace QP

