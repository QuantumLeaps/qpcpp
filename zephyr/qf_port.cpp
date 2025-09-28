//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
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
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
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
    m_eQueue.used_msgs = attr1; // will be used for thread options
#ifdef CONFIG_THREAD_NAME
    m_thread.init_data = const_cast<void *>(attr2);
#else
    Q_UNUSED_PAR(attr2);
#endif
}
//............................................................................
void QActive::start(QPrioSpec const prioSpec,
    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
    void * const stkSto, std::uint_fast16_t const stkSize,
    void const * const par)
{
    // extract data temporarily saved in QActive::setAttr()
    std::uint32_t opt = m_eQueue.used_msgs;
#ifdef CONFIG_THREAD_NAME
    char const *name = static_cast<char const *>(m_thread.init_data);
#endif

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U;   // preemption-threshold (not used for AO registration)
    register_(); // make QF aware of this active object

    // initialize the Zephyr message queue
    k_msgq_init(&m_eQueue, reinterpret_cast<char *>(qSto),
                sizeof(QEvtPtr), static_cast<uint32_t>(qLen));

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // The Zephyr priority of the AO thread can be specified in two ways:
    //
    // 1. Implicitly based on the AO's priority (Zephyr uses the reverse
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
    //    it is consistent with the AO's priority. An example of
    //    inconsistent setting would be assigning Zephyr priorities that
    //    would result in a different relative prioritization of AO's
    //    threads than indicated by the AO priorities assigned.
    //
    int zephyr_prio = (static_cast<int>(prioSpec) >> 8);
    if (zephyr_prio == 0) {
        zephyr_prio = static_cast<int>(QF_MAX_ACTIVE) - static_cast<int>(m_prio);
    }

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
bool QActive::postx_(QEvt const * const e, std::uint_fast16_t const margin,
                     void const * const sender) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != nullptr);

    // NOTE: k_msgq_num_free_get() can be safely called from crit-section
    std::uint_fast16_t nFree =
         static_cast<std::uint_fast16_t>(k_msgq_num_free_get(&m_eQueue));

    bool status;
    if (margin == QF::NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_INCRIT(210); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);// pool-Id & ref-Count
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()

        if (e->poolNum_ != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }

        QF_CRIT_EXIT(); // exit crit.sect. before calling Zephyr API
        int err = k_msgq_put(&m_eQueue, static_cast<void const *>(&e), K_NO_WAIT);
        QF_CRIT_ENTRY(); // re-enter crit.sect. after calling Zephyr API

        // posting to the Zephyr message queue must succeed, see NOTE1
        Q_ASSERT_INCRIT(220, err == 0);
#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
    }
    else {

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(0U);      // min # free entries (unknown)
        QS_END_PRE()
    }
    QF_CRIT_EXIT();

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(300, e != nullptr);

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(k_msgq_num_free_get(&m_eQueue)); // # free entries
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    if (e->poolNum_ != 0U) { // is it a pool event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    // NOTE: Zephyr message queue does not currently support LIFO posting
    // so normal FIFO posting is used instead.
    QF_CRIT_EXIT(); // exit crit.sect. before calling Zephyr API
    int err = k_msgq_put(&m_eQueue, static_cast<void const *>(&e), K_NO_WAIT);
    QF_CRIT_ENTRY(); // re-enter crit.sect. after calling Zephyr API

    Q_ASSERT_INCRIT(310, err == 0);
#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif

    QF_CRIT_EXIT();
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    // wait for an event (forever)
    QEvtPtr e;
    int err = k_msgq_get(&m_eQueue, static_cast<void *>(&e), K_FOREVER);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_ASSERT_INCRIT(410, err == 0); // queue-get must succeed
#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Id & ref-Count
        QS_EQC_PRE(k_msgq_num_free_get(&m_eQueue)); // # free entries
    QS_END_PRE()

    QF_CRIT_EXIT();

    return e;
}
//............................................................................
std::uint16_t QActive::getQueueUse(
    std::uint_fast8_t const prio) noexcept
{
    return 0U; // queue use not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueFree(std::uint_fast8_t const prio) noexcept {
    return 0U; // queue free elements not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueMin(std::uint_fast8_t const prio) noexcept {
    return 0U; // queue minimum not supported in this RTOS
}

} // namespace QP

