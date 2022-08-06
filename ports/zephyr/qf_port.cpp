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
//! @date Last updated on: 2022-08-06
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QF/C++ port to Zephyr RTOS kernel, all supported compilers

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"
#include "qassert.h"
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
    QP::QActive::thread_(reinterpret_cast<QP::QActive *>(p1));
}

} // unnamed namespace

// namespace QP ==============================================================
namespace QP {

// Zephyr spinlock for QF critical section
struct k_spinlock QF::spinlock;

//............................................................................
void QF::init(void) {
    spinlock = (struct k_spinlock){};
}
//............................................................................
int_t QF::run(void) {
    onStartup();
    return 0; // return from the main Zephyr thread
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}

// thread for active objects -------------------------------------------------
void QActive::thread_(QActive *act) {
    // event-loop
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
    act->unregister_(); // remove this active object from QF
}

//............................................................................
//
// In the Zephyr port the generic function QActive::setAttr() is used to
// set the options for the Zephyr thread (attr1) and thread name (attr2).
// QActive::setAttr() needs to be called *before* QActive::start() for the
// given active object.
//
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    m_thread.base.order_key = attr1;
    m_thread.init_data      = const_cast<void *>(attr2);
}
//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    // initialize the Zephyr message queue
    k_msgq_init(&m_eQueue, reinterpret_cast<char *>(qSto),
                sizeof(QEvt *), static_cast<uint32_t>(qLen));

    m_prio = prio;  // save the QF priority
    register_(); // make QF aware of this active object

    init(par, m_prio); // take the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // Zephyr uses the reverse priority numbering than QP
    int zprio = (int)QF_MAX_ACTIVE - static_cast<int>(prio);

    // create an Zephyr thread for the AO...
    std::uint32_t opt = m_thread.base.order_key;
#ifdef CONFIG_THREAD_NAME
    char const *name = static_cast<char const *>(m_thread.init_data);
#endif
    m_thread = (struct k_thread){}; // clear the thread control block
#ifdef CONFIG_THREAD_NAME
    k_thread_name_set(&m_thread, name);
#endif
    k_thread_create(&m_thread,
                    static_cast<k_thread_stack_t *>(stkSto),
                    static_cast<size_t>(stkSize),
                    &thread_entry,
                    static_cast<void *>(this), // p1
                    nullptr,    // p2
                    nullptr,    // p3
                    zprio,      // Zephyr priority */
                    opt,        // thread options */
                    K_NO_WAIT); // start immediately */
}
//............................................................................
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
{
    QF_CRIT_STAT_
    QF_CRIT_E_();

    std::uint_fast16_t nFree =
         static_cast<std::uint_fast16_t>(k_msgq_num_free_get(&m_eQueue));

    bool status;
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_ID(510); // must be able to post the event
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();         // timestamp
            QS_OBJ_PRE_(sender);    // the sender object
            QS_SIG_PRE_(e->sig);    // the signal of the event
            QS_OBJ_PRE_(this);      // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);     // # free entries
            QS_EQC_PRE_(0U);        // min # free (unknown)
        QS_END_NOCRIT_PRE_()

        if (e->poolId_ != 0U) { // is it a pool event?
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        QF_CRIT_X_();

        // posting to the Zephyr mailbox must succeed, see NOTE3
        Q_ALLEGE_ID(520,
            k_msgq_put(&m_eQueue, static_cast<void const *>(&e), K_NO_WAIT)
             == 0);
    }
    else {

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();         // timestamp
            QS_OBJ_PRE_(sender);    // the sender object
            QS_SIG_PRE_(e->sig);    // the signal of the event
            QS_OBJ_PRE_(this);      // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);     // # free entries
            QS_EQC_PRE_(0U);        // min # free (unknown)
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
        QS_TIME_PRE_();              // timestamp
        QS_SIG_PRE_(e->sig);         // the signal of this event
        QS_OBJ_PRE_(this);           // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
        QS_EQC_PRE_(k_msgq_num_free_get(&m_eQueue)); // # free entries
        QS_EQC_PRE_(0U);             // min # free entries (unknown)
    QS_END_NOCRIT_PRE_()

    if (e->poolId_ != 0U) {  // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QF_CRIT_X_();

    // NOTE: Zephyr message queue currently does NOT support LIFO posting
    // so normal FIFO posting is used instead.
    //
    Q_ALLEGE_ID(810,
        k_msgq_put(&m_eQueue, static_cast<void const *>(&e), K_NO_WAIT)
        == 0);
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    QEvt const *e;
    QS_CRIT_STAT_

    // wait for an event (forever), which must succeed
    Q_ALLEGE_ID(710,
        k_msgq_get(&m_eQueue, static_cast<void *>(&e), K_FOREVER) == 0);

    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();               // timestamp
        QS_SIG_PRE_(e->sig);          // the signal of this event
        QS_OBJ_PRE_(this);            // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
        QS_EQC_PRE_(k_msgq_num_free_get(&m_eQueue)); // # free entries
    QS_END_PRE_()

    return e;
}

} // namespace QP

