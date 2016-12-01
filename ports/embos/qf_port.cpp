/// @file
/// @brief QF/C++ port to embOS (v4.00) kernel, all supported compilers
/// @cond
////**************************************************************************
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
////**************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
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
    OS_Start();      // start embOS multitasking
    Q_ERROR_ID(100); // OS_Start() should never return
    return static_cast<int_t>(0); // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}
//............................................................................
void QF_setEmbOsTaskAttr(QActive *act, uint32_t attr) {
    act->getOsObject() = attr;
}

// thread for active objects -------------------------------------------------
void QF::thread_(QActive *act) {
    // enable thread-loop, see NOTE2
    act->m_osObject = static_cast<uint32_t>(1);  // set event-loop control
    do {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    } while (act->m_osObject != static_cast<uint32_t>(0));
    act->unsubscribeAll();
    OS_DeleteMB(&act->m_eQueue);
}

//............................................................................
static void thread_function(void *pVoid) { // embOS signature
    QActive *act = reinterpret_cast<QActive *>(pVoid);

#ifdef __TARGET_FPU_VFP
    // does the task use the FPU? see NOTE1
    if ((act->getOsObject() & QF_TASK_USES_FPU) != static_cast<uint32_t>(0)) {
        OS_ExtendTaskContext_VFP();
    }
#endif  // __TARGET_FPU_VFP

    QF::thread_(act);
    QF::remove_(act); // remove this object from QF
    OS_TerminateTask(&act->getThread());
}
//............................................................................
void QActive::start(uint_fast8_t prio,
                     QEvt const *qSto[], uint_fast16_t qLen,
                     void *stkSto, uint_fast16_t stkSize,
                     QEvt const *ie)
{
    // create the embOS message box for the AO
    OS_CreateMB(&m_eQueue,
                static_cast<OS_U16>(sizeof(QEvt *)),
                static_cast<OS_UINT>(qLen),
                static_cast<void *>(&qSto[0]));

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object
    init(ie);       // thake the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // create an embOS task for the AO
    OS_CreateTaskEx(&m_thread,
        "AO",
        static_cast<OS_PRIO>(prio), // embOS uses same numbering as QP
        &thread_function,
        static_cast<void OS_STACKPTR *>(stkSto),
        static_cast<OS_UINT>(stkSize),
        static_cast<OS_UINT>(0),    // no AOs at the same prio
        this);
}
//............................................................................
void QActive::stop() {
    m_osObject = static_cast<uint32_t>(0); // stop the thread loop, see NOTE2
}
//............................................................................
#ifndef Q_SPY
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin)
#else
bool QActive::post_(QEvt const * const e, uint_fast16_t const margin,
                     void const * const sender)
#endif
{
    uint_fast16_t nFree;
    bool status;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    nFree = static_cast<uint_fast16_t>(m_eQueue.maxMsg - m_eQueue.nofMsg);

    if (nFree > margin) {
        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::priv_.aoObjFilter, this)
            QS_TIME_();             // timestamp
            QS_OBJ_(sender);        // the sender object
            QS_SIG_(e->sig);        // the signal of the event
            QS_OBJ_(this);          // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        if (e->poolId_ != static_cast<uint8_t>(0)) { // is it a pool event?
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        QF_CRIT_EXIT_();

        // posting to the embOS mailbox must succeed, see NOTE3
        Q_ALLEGE_ID(710,
            OS_PutMailCond(&m_eQueue, static_cast<OS_CONST_PTR void *>(&e))
            == static_cast<char>(0));

        status = true; // report success
    }
    else {
        // can tolerate dropping evts?
        Q_ASSERT_ID(720, margin != static_cast<uint_fast16_t>(0));

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT,
                         QS::priv_.aoObjFilter, this)
            QS_TIME_();             // timestamp
            QS_OBJ_(sender);        // the sender object
            QS_SIG_(e->sig);        // the signal of the event
            QS_OBJ_(this);          // this active object (recipient)
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free (unknown)
        QS_END_NOCRIT_()

        QF_CRIT_EXIT_();

        status = false; // report failure
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS::priv_.aoObjFilter, this)
        QS_TIME_();             // timestamp
        QS_SIG_(e->sig);        // the signal of this event
        QS_OBJ_(this);          // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
        // # free entries
        QS_EQC_(static_cast<QEQueueCtr>(m_eQueue.maxMsg - m_eQueue.nofMsg));
        QS_EQC_(static_cast<QEQueueCtr>(0)); // min # free entries (unknown)
    QS_END_NOCRIT_()

    if (e->poolId_ != static_cast<uint8_t>(0)) { // is it a pool event?
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QF_CRIT_EXIT_();

    // posting to the embOS mailbox must succeed, see NOTE3
    Q_ALLEGE_ID(810,
        OS_PutMailFrontCond(&m_eQueue, static_cast<OS_CONST_PTR void *>(&e))
        == static_cast<char>(0));
}
//............................................................................
QEvt const *QActive::get_(void) {
    QEvt const *e;
    QS_CRIT_STAT_

    OS_GetMail(&m_eQueue, &e);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::priv_.aoObjFilter, this)
        QS_TIME_();             // timestamp
        QS_SIG_(e->sig);        // the signal of this event
        QS_OBJ_(this);          // this active object
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
        // # free entries
        QS_EQC_(static_cast<QEQueueCtr>(m_eQueue.maxMsg - m_eQueue.nofMsg));
    QS_END_()

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
// NOTE2:
// The member QActive.osObject is reused as the loop control variable,
// because the task attributes are alredy applied.
//
// NOTE3:
// The event posting to embOS mailbox occurs inside a critical section,
// but this is OK, because the QF/embOS critical sections are designed
// to nest.
//