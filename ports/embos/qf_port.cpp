//============================================================================
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
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C++ software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C++ in closed-source proprietary applications.
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

//............................................................................
// define __TARGET_FPU_VFP symbol depending on the compiler...
#if defined (__CC_ARM)          // ARM Compiler
    // in ARM Compiler __TARGET_FPU_VFP is a pre-defined symbol
#elif defined (__ICCARM__)      // IAR Compiler
    #if defined __ARMVFP__
        #define __TARGET_FPU_VFP 1
    #endif
#elif defined (__GNUC__)        // GNU Compiler
    #if defined (__VFP_FP__) && !defined(__SOFTFP__)
        #define __TARGET_FPU_VFP 1
    #endif
#endif

//============================================================================
namespace { // anonymous namespace with local definitions

Q_DEFINE_THIS_MODULE("qf_port")

//............................................................................
static void thread_function(void *pVoid); // prototype
static void thread_function(void *pVoid) { // embOS signature
    QP::QActive * const act = reinterpret_cast<QP::QActive *>(pVoid);

#ifdef __TARGET_FPU_VFP
    // does the task use the FPU? see NOTE1
    if ((act->getOsObject() & QP::TASK_USES_FPU) != 0U) {
        OS_ExtendTaskContext_VFP();
    }
#endif  // __TARGET_FPU_VFP

    QP::QActive::evtLoop_(act);
}

} // anonymous namespace

// namespace QP ==============================================================
namespace QP {

//............................................................................
void QF::init() {
    OS_InitKern();  // initialize embOS
    OS_InitHW();    // initialize the hardware used by embOS
}
//............................................................................
int QF::run() {
    onStartup();  // QF callback to configure and start interrupts

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();

    OS_Start(); // start embOS multitasking

    return 0; // this unreachable return keeps the compiler happy
}
//............................................................................
void QF::stop() {
    onCleanup(); // cleanup callback
}

// thread for active objects -------------------------------------------------
void QActive::evtLoop_(QActive *act) {
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
    //act->unregister_(); // remove this object from QF
    //OS_TerminateTask(&act->m_thread);
}

//............................................................................
void QActive::start(QPrioSpec const prioSpec,
                    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    // create the embOS message box for the AO
    OS_MAILBOX_Create(&m_eQueue,
                static_cast<OS_U16>(sizeof(QEvtPtr)),
                static_cast<OS_UINT>(qLen),
                static_cast<void *>(&qSto[0]));

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used)
    register_(); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // The embOS priority of the AO thread can be specified in two ways:
    //
    // 1. Implictily based on the AO's priority (embOS uses the same
    //    priority numbering scheme as QP). This option is chosen when
    //    the higher-byte of the prioSpec parameter is set to zero.
    //
    // 2. Explicitly as the higher-byte of the prioSpec parameter.
    //    This option is chosen when the prioSpec parameter is not-zero.
    //    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
    //    as 10 and embOS priority as 5.
    //
    //    NOTE: The explicit embOS priority is NOT sanity-checked,
    //    so it is the responsibility of the application to ensure that
    //    it is consistent with the AO's priority. An example of
    //    inconsistent setting would be assigning embOS priorities that
    //    would result in a different relative priritization of AO's threads
    //    than indicated by the AO priorities assigned.
    //
    OS_PRIO embos_prio = (prioSpec >> 8U);
    if (embos_prio == 0U) {
        embos_prio = m_prio;
    }

    // create an embOS task for the AO
    OS_TASK_CreateEx(&m_thread,
#if (OS_TRACKNAME != 0)
        m_thread.Name,   // the configured task name
#elif
        "AO",            // a generic AO task name
#endif
        embos_prio,      // embOS priority
        &thread_function,
        static_cast<void OS_STACKPTR *>(stkSto),
        static_cast<OS_UINT>(stkSize),
        0U, // no AOs at the same prio
        this);
}
//............................................................................
void QActive::setAttr(std::uint32_t const attr1, void const *attr2) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    switch (attr1) {
        case TASK_NAME_ATTR: {
#if (OS_TRACKNAME != 0)
            Q_ASSERT_INCRIT(300, m_thread.Name == nullptr);
            m_thread.Name = static_cast<char const *>(attr2);
#endif
            break;
        }
        case TASK_USES_FPU:
            m_osObject = attr1;
            break;
        // ...
        default:
            break;
    }
    QF_CRIT_EXIT();
}
//............................................................................
bool QActive::postx_(QEvt const * const e, std::uint_fast16_t const margin,
                     void const * const sender) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != nullptr);

    std::uint_fast16_t nFree =
        static_cast<std::uint_fast16_t>(m_eQueue.maxMsg - m_eQueue.nofMsg);

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
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);  // # free entries
            QS_EQC_PRE(0U);     // min # free entries (unknown)
        QS_END_PRE()

        if (e->poolNum_ != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        QF_CRIT_EXIT();

        char err = OS_MAILBOX_Put(&m_eQueue,
                                  static_cast<OS_CONST_PTR void *>(&e));
        QF_CRIT_ENTRY();
        // posting to the embOS mailbox must succeed, see NOTE3
        Q_ASSERT_INCRIT(220, err == '\0');
    }
    else {

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // poolNum & refCtr
            QS_EQC_PRE(nFree);  // # free entries
            QS_EQC_PRE(margin); // margin requested
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
        QS_TIME_PRE();          // timestamp
        QS_SIG_PRE(e->sig);     // the signal of this event
        QS_OBJ_PRE(this);       // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(m_eQueue.maxMsg - m_eQueue.nofMsg); // # free entries
        QS_EQC_PRE(0U);         // min # free entries (unknown)
    QS_END_PRE()

    if (e->poolNum_ != 0U) { // is it a pool event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
    QF_CRIT_EXIT();

    char err = OS_MAILBOX_PutFront(&m_eQueue,
                                   static_cast<OS_CONST_PTR void *>(&e));
    QF_CRIT_ENTRY();
    // posting to the embOS mailbox must succeed, see NOTE3
    Q_ASSERT_INCRIT(320, err == '\0');
    QF_CRIT_EXIT();
}
//............................................................................
QEvt const *QActive::get_() noexcept {
    QEvt const *e;
    OS_MAILBOX_GetBlocked(&m_eQueue, static_cast<void *>(&e));

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();          // timestamp
        QS_SIG_PRE(e->sig);     // the signal of this event
        QS_OBJ_PRE(this);       // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // poolNum & refCtr
        QS_EQC_PRE(m_eQueue.maxMsg - m_eQueue.nofMsg); // # free
    QS_END_PRE()
    QS_CRIT_EXIT();

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

//============================================================================
// NOTE1:
// In case of hardware-supported floating point unit (FPU), a task must
// preserve the FPU registers across the context switch. However, this
// additional overhead is necessary only for tasks that actually use the
// FPU. In this QP-embOS port, an active object task that uses the FPU is
// designated by the QF_TASK_USES_FPU attribute, which can be set with the
// QF_setEmbOsTaskAttr() function. The task attributes must be set *before*
// calling QActive::start(). The task attributes are saved in the
// QActive.osObject member.
//
// NOTE3:
// The event posting to embOS mailbox occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
