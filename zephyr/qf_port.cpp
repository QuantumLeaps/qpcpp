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
//! @date Last updated on: 2023-12-04
//! @version Last updated for @ref qpcpp_7_3_1
//!
//! @file
//! @brief QF/C++ port to Zephyr RTOS kernel, all supported compilers

#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

namespace { // unnamed namespace

Q_DEFINE_THIS_MODULE("qf_port")

//............................................................................
static void thread_entry(void *p1, void *p2, void *p3) { // Zephyr signature
    Q_UNUSED_PAR(p2);
    Q_UNUSED_PAR(p3);

    // run the thread routine (typically endless loop)
    QP::QActive::evtLoop_(reinterpret_cast<QP::QActive *>(p1));
}

} // unnamed namespace

// namespace QP ==============================================================
namespace QP {
namespace QF {

// Zephyr spinlock for QF critical section
struct k_spinlock spinlock;

//............................................................................
void init() {
    spinlock = (struct k_spinlock){};
}
//............................................................................
int_t run() {
    onStartup();
#ifdef Q_SPY

#if (CONFIG_NUM_PREEMPT_PRIORITIES > 0)
    // lower the priority of the main thread to the level of idle thread
    k_thread_priority_set(k_current_get(),
                          CONFIG_NUM_PREEMPT_PRIORITIES - 1);
#endif

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()
    QS_CRIT_EXIT();

    // perform QS work...
    while (true) {
        QS::rxParse();   // parse any QS-RX bytes
        QS::doOutput();  // perform the QS-TX output
    }
#else
    return 0; // return from the main Zephyr thread
#endif
}
//............................................................................
void stop(void) {
    onCleanup();  // cleanup callback
}

} // namespace QF

// thread for active objects -------------------------------------------------
void QActive::evtLoop_(QActive *act) {
    // event-loop
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        // dispatch event (virtual call)
        act->dispatch(e, act->m_prio);
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
}

//............................................................................
//
// In the Zephyr port the generic function QActive::setAttr() is used to
// set the options for the Zephyr thread (attr1) and thread name (attr2).
// QActive::setAttr() needs to be called *before* QActive::start() for the
// given active object.
//
// In this Zephyr port the attributes will be used as follows
// (see also QActive::start()):
// - attr1 - will be used for thread options in k_thread_create()
// - attr2 - will be used for thread name in k_thread_name_set()
//
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    m_thread.base.order_key = attr1;
    m_thread.init_data = const_cast<void *>(attr2);
}
//............................................................................
void QActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U); // preemption-thre.
    register_(); // make QF aware of this active object

    // initialize the Zephyr message queue
    k_msgq_init(&m_eQueue, reinterpret_cast<char *>(qSto),
                sizeof(QEvt *), static_cast<uint32_t>(qLen));

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // The Zephyr priority of the AO thread can be specificed in two ways:
    //
    // 1. Implictily based on the AO's priority (Zephyr uses the reverse
    //    priority numbering scheme than QP). This option is chosen, when
    //    the higher-byte of the prioSpec parameter is set to zero.
    //
    // 2. Explicitly as the higher-byte of the prioSpec parameter.
    //    This option is chosen when the prioSpec parameter is not-zero.
    //    For example, Q_PRIO(10U, -1U) will explicitly specify AO priority
    //    as 10 and Zephyr priority as -1.
    //
    //    NOTE: The explicit Zephyr priority is NOT sanity-checked,
    //    so it is the responsibility of the application to ensure that
    //    it is consistent witht the AO's priority. An example of
    //    inconsistent setting would be assigning Zephyr priorities that
    //    would result in a different relative priritization of AO's threads
    //    than indicated by the AO priorities assigned.
    //
    int zephyr_prio = (int)((int16_t)qp_prio >> 8);
    if (zephyr_prio == 0) {
        zephyr_prio = (int)QF_MAX_ACTIVE - (int)m_prio;
    }

    // extract data temporarily saved in m_thread by QActive::setAttr()
    std::uint32_t opt = m_thread.base.order_key;
#ifdef CONFIG_THREAD_NAME
    char const *name = static_cast<char const *>(m_thread.init_data);
#endif

    // clear the Zephyr thread structure before creating the thread
    m_thread = (struct k_thread){};

    // create a Zephyr thread for the AO...
    k_thread_create(&m_thread,
                    static_cast<k_thread_stack_t *>(stkSto),
                    static_cast<size_t>(stkSize),
                    &thread_entry,
                    static_cast<void *>(this), // p1
                    nullptr,    // p2
                    nullptr,    // p3
                    zephyr_prio,// Zephyr priority
                    opt,        // thread options
                    K_NO_WAIT); // start immediately

#ifdef CONFIG_THREAD_NAME
    // set the Zephyr thread name, if initialized, or the default name "AO"
    k_thread_name_set(&m_thread, (name != nullptr) ? name : "AO");
#endif
}
//............................................................................
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    std::uint_fast16_t nFree =
         static_cast<std::uint_fast16_t>(k_msgq_num_free_get(&m_eQueue));

    bool status;
    if (margin == QF::NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_INCRIT(510); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();       // timestamp
            QS_OBJ_PRE_(sender);  // the sender object
            QS_SIG_PRE_(e->sig);  // the signal of the event
            QS_OBJ_PRE_(this);    // this active object (recipient)
            QS_2U8_PRE_(e->getPoolId_(), e->refCtr_);// pool-Id & ref-Count
            QS_EQC_PRE_(nFree);   // # free entries available
            QS_EQC_PRE_(0U);      // min # free entries (unknown)
        QS_END_PRE_()

        if (e->getPoolId_() != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        QF_CRIT_EXIT();

        int err = k_msgq_put(&m_eQueue, static_cast<void const *>(&e), K_NO_WAIT);

        // posting to the Zephyr message queue must succeed, see NOTE1
        QF_CRIT_ENTRY();
        Q_ASSERT_INCRIT(520, err == 0);
    }
    else {

        QS_BEGIN_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();       // timestamp
            QS_OBJ_PRE_(sender);  // the sender object
            QS_SIG_PRE_(e->sig);  // the signal of the event
            QS_OBJ_PRE_(this);    // this active object (recipient)
            QS_2U8_PRE_(e->getPoolId_(), e->refCtr_);// pool-Id & ref-Count
            QS_EQC_PRE_(nFree);   // # free entries available
            QS_EQC_PRE_(0U);      // min # free entries (unknown)
        QS_END_PRE_()
    }
    QF_CRIT_EXIT();

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QS_BEGIN_PRE_(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->getPoolId_(), e->refCtr_); // pool-Id & ref-Count
        QS_EQC_PRE_(k_msgq_num_free_get(&m_eQueue)); // # free entries
        QS_EQC_PRE_(0U);      // min # free entries (unknown)
    QS_END_PRE_()

    if (e->getPoolId_() != 0U) { // is it a pool event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
    QF_CRIT_EXIT();

    // NOTE: Zephyr message queue does not currently support LIFO posting
    // so normal FIFO posting is used instead.
    int err = k_msgq_put(&m_eQueue, static_cast<void const *>(&e), K_NO_WAIT);

    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(710, err == 0);
    QF_CRIT_EXIT();
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {

    // wait for an event (forever)
    QEvt const *e;
    int err = k_msgq_get(&m_eQueue, static_cast<void *>(&e), K_FOREVER);

    // queue-get must succeed
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(810, err == 0);


    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->getPoolId_(), e->refCtr_); // pool-Id & ref-Count
        QS_EQC_PRE_(k_msgq_num_free_get(&m_eQueue)); // # free entries
    QS_END_PRE_()

    QF_CRIT_EXIT();

    return e;
}

} // namespace QP

