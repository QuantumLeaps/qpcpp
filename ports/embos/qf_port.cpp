/// @file
/// @brief QF/C++ port to embOS RTOS kernel, all supported compilers
/// @cond
////**************************************************************************
/// Last updated for version 6.9.2a
/// Last updated on  2021-01-26
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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
////**************************************************************************
/// @endcond

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

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

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

//............................................................................
void QF::init(void) {
    OS_InitKern();  // initialize embOS
    OS_InitHW();    // initialize the hardware used by embOS
}
//............................................................................
int_t QF::run(void) {
    onStartup();     // QF callback to configure and start interrupts

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    OS_Start();      // start embOS multitasking
    Q_ERROR_ID(100); // OS_Start() should never return
    return 0;       // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}

// thread for active objects -------------------------------------------------
void QF::thread_(QActive *act) {
    // event-loop
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
}

//............................................................................
static void thread_function(void *pVoid) { // embOS signature
    QActive *act = reinterpret_cast<QActive *>(pVoid);

#ifdef __TARGET_FPU_VFP
    // does the task use the FPU? see NOTE1
    if ((act->getOsObject() & QF_TASK_USES_FPU) != 0U) {
        OS_ExtendTaskContext_VFP();
    }
#endif  // __TARGET_FPU_VFP

    QF::thread_(act);
    QF::remove_(act); // remove this object from QF
    OS_TerminateTask(&act->getThread());
}
//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    // create the embOS message box for the AO
    OS_CreateMB(&m_eQueue,
                static_cast<OS_U16>(sizeof(QEvt *)),
                static_cast<OS_UINT>(qLen),
                static_cast<void *>(&qSto[0]));

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object
    init(par, m_prio); // take the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // prepare the unique task name of the form "Axx",
    // where xx is a 2-digit QP priority of the task
    char task_name[4];
    task_name[0] = 'A';
    task_name[1] = '0' + (prio / 10U);
    task_name[2] = '0' + (prio % 10U);
    task_name[3] = '\0'; // zero-terminate

    // create an embOS task for the AO
    OS_CreateTaskEx(&m_thread,
        task_name, // unique name of the task
        static_cast<OS_PRIO>(prio), // embOS uses same numbering as QP
        &thread_function,
        static_cast<void OS_STACKPTR *>(stkSto),
        static_cast<OS_UINT>(stkSize),
        0U, // no AOs at the same prio
        this);
}
//............................................................................
void QActive::setAttr(std::uint32_t attr1, void const * /*attr2*/) {
    m_osObject = attr1;
}
//............................................................................
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e,
                    std::uint_fast16_t const margin) noexcept
#else
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                     void const * const sender) noexcept
#endif
{
    std::uint_fast16_t nFree;
    bool status;
    QF_CRIT_STAT_

    QF_CRIT_E_();
    nFree = static_cast<std::uint_fast16_t>(m_eQueue.maxMsg - m_eQueue.nofMsg);

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
            QS_TIME_PRE_();        // timestamp
            QS_OBJ_PRE_(sender);   // the sender object
            QS_SIG_PRE_(e->sig);   // the signal of the event
            QS_OBJ_PRE_(this);     // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);    // # free entries
            QS_EQC_PRE_(0U);       // min # free (unknown)
        QS_END_NOCRIT_PRE_()

        if (e->poolId_ != 0U) {     // is it a pool event?
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        QF_CRIT_X_();

        // posting to the embOS mailbox must succeed, see NOTE3
        Q_ALLEGE_ID(520,
            OS_PutMailCond(&m_eQueue, static_cast<OS_CONST_PTR void *>(&e))
            == 0);
    }
    else {

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();             // timestamp
            QS_OBJ_PRE_(sender);        // the sender object
            QS_SIG_PRE_(e->sig);        // the signal of the event
            QS_OBJ_PRE_(this);          // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);         // # free entries
            QS_EQC_PRE_(0U);            // min # free (unknown)
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
        QS_TIME_PRE_();             // timestamp
        QS_SIG_PRE_(e->sig);        // the signal of this event
        QS_OBJ_PRE_(this);          // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
        QS_EQC_PRE_(m_eQueue.maxMsg - m_eQueue.nofMsg); // # free entries
        QS_EQC_PRE_(0U); // min # free entries (unknown)
    QS_END_NOCRIT_PRE_()

    if (e->poolId_ != 0U) {     // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QF_CRIT_X_();

    // posting to the embOS mailbox must succeed, see NOTE3
    Q_ALLEGE_ID(810,
        OS_PutMailFrontCond(&m_eQueue, static_cast<OS_CONST_PTR void *>(&e))
        == static_cast<char>(0));
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    QEvt const *e;
    QS_CRIT_STAT_

    OS_GetMail(&m_eQueue, &e);

    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();             // timestamp
        QS_SIG_PRE_(e->sig);        // the signal of this event
        QS_OBJ_PRE_(this);          // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
        QS_EQC_PRE_(m_eQueue.maxMsg - m_eQueue.nofMsg); // # free entries
    QS_END_PRE_()

    return e;
}

} // namespace QP

//****************************************************************************
// NOTE1:
// In case of hardware-supported floating point unit (FPU), a task must
// preserve the FPU registers accross the context switch. However, this
// additional overhead is necessary only for tasks that actually use the
// FPU. In this QP-embOS port, an active object task that uses the FPU is
// designated by the QF_TASK_USES_FPU attribute, which can be set wiht the
// QF_setEmbOsTaskAttr() function. The task attributes must be set *before*
// calling QACTIVE_START(). The task attributes are saved in
// QActive.m_osObject member.
//
// NOTE3:
// The event posting to embOS mailbox occurs inside a critical section,
// but this is OK, because the QF/embOS critical sections are designed
// to nest.
//
