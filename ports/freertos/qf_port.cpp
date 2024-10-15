//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// The QP/C software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2024-10-29
//! @version Last updated for: @ref qpcpp_8_0_0
//!
//! @file
//! @brief QF/C++ port to FreeRTOS 10.x, generic C++11 compiler

#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-level interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#if ( configSUPPORT_STATIC_ALLOCATION == 0 )
#error "This QP/C++ port to FreeRTOS requires configSUPPORT_STATIC_ALLOCATION"
#endif

#if ( configMAX_PRIORITIES < QF_MAX_ACTIVE )
#error "FreeRTOS configMAX_PRIORITIES must not be less than QF_MAX_ACTIVE"
#endif

namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects -------------------------------------------------------------
static void task_function(void *pvParameters) { // FreeRTOS task signature
    QP::QActive::evtLoop_(reinterpret_cast<QP::QActive *>(pvParameters));
}

} // unnamed local namespace

// The following macro provides the number of free slots in the FreeRTOS
// queue.
//
// NOTE1:
// The official FreeRTOS API uxQueueSpacesAvailable() is not used
// here, because that API uses task-level critical section internally.
// Instead, the free slots calculation happens here in already
// established critical section. Unfortunately, the bizarre "information
// obfuscating" policy of FreeRTOS (incorrectly called "information
// hiding") forces the use of the StaticQueue_t with "dummy" members.
// This could potentially break in the future releases of FreeRTOS.
//
// Currently, the correspondence between xQUEUE and StaticQueue_t
// is as follows (see queue.c and FreeRTOS.h, respectively):
//
// xQUEUE.uxMessagesWaiting == StaticQueue_t.uxDummy4[0];
// xQUEUE.uxLength          == StaticQueue_t.uxDummy4[1];
//
#define FREERTOS_QUEUE_GET_FREE() \
    (m_osObject.uxDummy4[1] - m_osObject.uxDummy4[0])

// namespace QP ==============================================================
namespace QP {

//............................................................................
void QF::init() {
    bzero_(&QF::priv_,             sizeof(QF::priv_));
    bzero_(&QActive::registry_[0], sizeof(QActive::registry_));
    // nothing to do for FreeRTOS
}
//............................................................................
int_t QF::run() {
    onStartup(); // the startup callback (configure/enable interrupts)

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QF_CRIT_EXIT();
#endif

    vTaskStartScheduler(); // start the FreeRTOS scheduler

    QF_CRIT_ENTRY();
    Q_ERROR_INCRIT(110); // the FreeRTOS scheduler should never return
    QF_CRIT_EXIT();

    return 0; // dummy return to make the compiler happy
}
//............................................................................
void QF::stop() {
    onCleanup(); // cleanup callback
}

// thread for active objects -------------------------------------------------
void QActive::evtLoop_(QActive *act) {
#ifdef QACTIVE_CAN_STOP
    while (act->m_eQueue != static_cast<QueueHandle_t>(0))
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = act->get_();   // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the SM
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    act->unregister_(); // remove this object from the framewrok
    vTaskDelete(static_cast<TaskHandle_t>(0)); // delete this FreeRTOS task
#endif
}

//............................................................................
void QActive::start(
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // precondition:
    // - queue storage must be provided
    // - queue size must be provided
    // - stack storage must be provided
    // - stack size must be provided
    Q_REQUIRE_INCRIT(100,
        (qSto != nullptr) && (qLen > 0U)
        && (stkSto != nullptr) && (stkSize > 0U));
    QF_CRIT_EXIT();

    // create FreeRTOS message queue
    m_eQueue = xQueueCreateStatic(
            static_cast<UBaseType_t>(qLen),           // length of the queue
            static_cast<UBaseType_t>(sizeof(QEvt *)), // element size
            reinterpret_cast<std::uint8_t *>(qSto),   // queue buffer
            &m_osObject);                             // static queue buffer
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(110, m_eQueue != static_cast<QueueHandle_t>(0));
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used)
    register_(); // register this AO

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // task name provided by the user in QActive::setAttr() or default name
    char const *taskName = (m_thread.pxDummy1 != nullptr)
                             ? static_cast<char const *>(m_thread.pxDummy1)
                             : static_cast<char const *>("AO");

    // The FreeRTOS priority of the AO thread can be specified in two ways:
    //
    // 1. Implictily based on the AO's priority (by the formula specified
    //    in the macro FREERTOS_TASK_PRIO(), see qp_port.h). This option
    //    is chosen, when the higher-byte of the prioSpec parameter is set
    //    to zero.
    //
    // 2. Explicitly as the higher-byte of the prioSpec parameter.
    //    This option is chosen when the prioSpec parameter is not-zero.
    //    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
    //    as 10 and FreeRTOS priority as 5.
    //
    //    NOTE: The explicit FreeRTOS priority is NOT sanity-checked,
    //    so it is the responsibility of the application to ensure that
    //    it is consistent with the AO's priority. An example of
    //    inconsistent setting would be assigning FreeRTOS priorities that
    //    would result in a different relative priritization of AO's threads
    //    than indicated by the AO priorities assigned.
    //
    UBaseType_t freertos_prio = (prioSpec >> 8U);
    if (freertos_prio == 0U) {
        freertos_prio = FREERTOS_TASK_PRIO(m_prio);
    }

    // statically create the FreeRTOS task for the AO
    TaskHandle_t task = xTaskCreateStatic(
        &task_function, // the task function
        taskName ,      // the name of the task
        stkSize/sizeof(portSTACK_TYPE), // stack length
        this,           // the 'pvParameters' parameter
        freertos_prio,  // FreeRTOS priority
        static_cast<StackType_t *>(stkSto), // stack storage
        &m_thread);     // task buffer

    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(120, task != static_cast<TaskHandle_t>(0));
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(task);
#endif
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    unsubscribeAll(); // unsubscribe from all events
    m_eQueue = static_cast<QueueHandle_t>(0); // stop thread (see QF::thread_)
}
#endif
//............................................................................
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // this function must be called before QACTIVE_START(),
    // which implies that m_thread.pxDummy1 must not be used yet;
    Q_REQUIRE_INCRIT(150, m_thread.pxDummy1 == nullptr);
    switch (attr1) {
        case TASK_NAME_ATTR:
            // temporarily store the name, cast 'const' away
            m_thread.pxDummy1 = const_cast<void *>(attr2);
            break;
        // ...
        default:
            break;
    }
    QF_CRIT_EXIT();
}

//============================================================================
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
{
    Q_UNUSED_PAR(sender); // unused when Q_SPY is undefined

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(200, e != nullptr);
#ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(201, e->verify_());
#endif // ndef Q_UNSAFE

    // the number of free slots available in the queue
    std::uint_fast16_t nFree =
         static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

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
    else if (nFree > margin) {
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
            QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
            QS_EQC_PRE(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_PRE(0U);     // min # free entries (unknown)
        QS_END_PRE()

        if (e->getPoolNum_() != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        QF_CRIT_EXIT();

        BaseType_t err = xQueueSendToBack(
                             m_eQueue, static_cast<void const *>(&e), 0U);

        QF_CRIT_ENTRY();
        // posting to the FreeRTOS message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(220, err == pdPASS);

#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
    }
    else { // cannot post the event

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
            QS_EQC_PRE(static_cast<QEQueueCtr>(nFree)); // # free entries
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
#ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(301, e->verify_());
#endif // ndef Q_UNSAFE

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->evtTag_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_PRE(static_cast<QEQueueCtr>(FREERTOS_QUEUE_GET_FREE()));
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    if (e->getPoolNum_() != 0U) { // is it a pool event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
    QF_CRIT_EXIT();

    BaseType_t err = xQueueSendToFront(
                         m_eQueue, static_cast<void const *>(&e), 0U);

    QF_CRIT_ENTRY();
    // LIFO posting to the FreeRTOS queue must succeed, see NOTE3
    Q_ASSERT_INCRIT(320, err == pdPASS);
    QF_CRIT_EXIT();

#ifdef Q_UNSAFE
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    QEvt const *e;
    xQueueReceive(m_eQueue, (void *)&e, portMAX_DELAY);

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->getPoolNum_(), e->refCtr_); // pool Id&ref Count
        QS_EQC_PRE(static_cast<QEQueueCtr>(FREERTOS_QUEUE_GET_FREE()));
    QS_END_PRE()
    QS_CRIT_EXIT();

    return e;
}

//============================================================================
// The "FromISR" QP APIs for the FreeRTOS port...
bool QActive::postFromISR(QEvt const * const e,
                          std::uint_fast16_t const margin,
                          void *par,
                          void const * const sender) noexcept
{
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    Q_REQUIRE_INCRIT(500, e != nullptr);
#ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(501, e->verify_());
#endif // ndef Q_UNSAFE

    // find the number of free slots available in the queue
    std::uint_fast16_t const nFree =
        static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

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
    else if (nFree > margin) {
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
            QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(0U);     // min # free entries (unknown)
        QS_END_PRE()

        if (e->getPoolNum_() != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        BaseType_t err = xQueueSendToBackFromISR(m_eQueue,
            static_cast<void const *>(&e),
            static_cast<BaseType_t*>(par));

        // posting to the FreeRTOS message queue must succeed
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        Q_ASSERT_INCRIT(520, err == pdPASS);
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
    }
    else {

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(margin); // margin requested
        QS_END_PRE()
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        QF::gcFromISR(e); // recycle the event to avoid a leak
    }

    return status;
}
//............................................................................
void QActive::publishFromISR(QEvt const *e,
                             void *par,
                             void const * const sender) noexcept
{
    Q_REQUIRE_INCRIT(600, e != nullptr);
#ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(601, e->verify_());
#endif // ndef Q_UNSAFE

    QSignal const sig = e->sig;

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // the published signal must be within the configured range
    Q_REQUIRE_INCRIT(610, sig < QActive::maxPubSignal_);
    Q_REQUIRE_INCRIT(611,
        subscrList_[sig].m_set.verify_(&subscrList_[sig].m_set_dis));

    QS_BEGIN_PRE(QS_QF_PUBLISH, 0U)
        QS_TIME_PRE();          // the timestamp
        QS_OBJ_PRE(sender);     // the sender object
        QS_SIG_PRE(sig);        // the signal of the event
        QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);
    QS_END_PRE()

    // is it a dynamic event?
    if (e->getPoolNum_() != 0U) {
        // NOTE: The reference counter of a dynamic event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF::gcFromISR()) decrements the reference counter
        // and recycles the event if the counter drops to zero. This covers
        // the case when the event was published without any subscribers.
        QEvt_refCtr_inc_(e);
    }

    // make a local, modifiable copy of the subscriber list
    QPSet subscrSet = QActive::subscrList_[sig].m_set;
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    if (subscrSet.notEmpty()) { // any subscribers?
        // the highest-prio subscriber
        std::uint_fast8_t p = subscrSet.findMax();

        // no need to lock the scheduler in the ISR context
        do { // loop over all subscribers
            // the prio of the AO must be registered with the framework
            uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
            Q_ASSERT_INCRIT(620, registry_[p] != nullptr);
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // POST_FROM_ISR() asserts if the queue overflows
            registry_[p]->POST_FROM_ISR(e, par, sender);

            subscrSet.remove(p); // remove the handled subscriber
            if (subscrSet.notEmpty()) { // still more subscribers?
                p = subscrSet.findMax(); // the highest-prio subscriber
            }
            else {
                p = 0U; // no more subscribers
            }
        } while (p != 0U);
        // no need to unlock the scheduler in the ISR context
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
    QF::gcFromISR(e);
}
//............................................................................
void QTimeEvt::tickFromISR(std::uint_fast8_t const tickRate,
                           void *pxHigherPriorityTaskWoken,
                           void const * const sender) noexcept
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(sender);
    #endif

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    Q_REQUIRE_INCRIT(700, tickRate < Q_DIM(timeEvtHead_));

    QTimeEvt *prev = &timeEvtHead_[tickRate];

    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        prev->m_ctr = (prev->m_ctr + 1U);
        QS_TEC_PRE(prev->m_ctr); // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()

    // scan the linked-list of time events at this rate...
    std::uint_fast8_t lbound = 2U*QF_MAX_ACTIVE; // fixed upper loop bound
    for (; lbound > 0U; --lbound) {
        Q_ASSERT_INCRIT(710, prev != nullptr); // sanity check

        QTimeEvt *te = prev->m_next; // advance down the time evt. list
#ifndef Q_UNSAFE
        Q_INVARIANT_INCRIT(711,
            Q_PTR2UINT_CAST_(te) ==
                static_cast<std::uintptr_t>(~prev->m_next_dis));
#endif // ndef Q_UNSAFE

        if (te == nullptr) { // end of the list?

            // any new time events armed since the last QTimeEvt_tick_()?
            if (timeEvtHead_[tickRate].m_act != nullptr) {
#ifndef Q_UNSAFE
                Q_INVARIANT_INCRIT(712,
                    Q_PTR2UINT_CAST_(timeEvtHead_[tickRate].m_act) ==
                    static_cast<std::uintptr_t>(
                        ~timeEvtHead_dis_[tickRate].m_ptr_dis));
#endif // ndef Q_UNSAFE
                prev->m_next = timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = nullptr;
#ifndef Q_UNSAFE
                prev->m_next_dis =
                    static_cast<std::uintptr_t>(
                        ~Q_PTR2UINT_CAST_(prev->m_next));
                timeEvtHead_dis_[tickRate].m_ptr_dis =
                    static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(nullptr));
#endif // ndef Q_UNSAFE

                te = prev->m_next; // switch to the new list
            }
            else { // all currently armed time events are processed
                break; // terminate the for-loop
            }
        }

        // the time event 'te' must be valid
        Q_ASSERT_INCRIT(720, te != nullptr);
        Q_INVARIANT_INCRIT(721, te->verify_());

        QTimeEvtCtr ctr = te->m_ctr;
#ifndef Q_UNSAFE
        Q_INVARIANT_INCRIT(722, ctr ==
            static_cast<QTimeEvtCtr>(~te->m_ctr_dis));
#endif // ndef Q_UNSAFE

        if (ctr == 0U) { // time event scheduled for removal?
            prev->m_next = te->m_next;
#ifndef Q_UNSAFE
            prev->m_next_dis =
                static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(te->m_next));
#endif // ndef Q_UNSAFE

            // mark time event 'te' as NOT linked
            te->m_flags &= static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);

            // do NOT advance the prev pointer

            // exit crit. section to reduce latency
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        else if (ctr == 1U) { // is time event about to expire?
            QActive * const act = te->toActive();
            if (te->m_interval != 0U) { // periodic time evt?
                te->m_ctr = te->m_interval; // rearm the time event
#ifndef Q_UNSAFE
                te->m_ctr_dis = static_cast<QTimeEvtCtr>(~te->m_interval);
#endif // ndef Q_UNSAFE
                prev = te; // advance to this time event
            }
            else { // one-shot time event: automatically disarm
                te->m_ctr = 0U;
                prev->m_next = te->m_next;
#ifndef Q_UNSAFE
                te->m_ctr_dis =
                    static_cast<QTimeEvtCtr>(~static_cast<QTimeEvtCtr>(0U));
                prev->m_next_dis =
                    static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(te->m_next));
#endif // ndef Q_UNSAFE

                // mark time event 'te' as NOT linked
                te->m_flags &=
                    static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);
                // do NOT advance the prev pointer

                QS_BEGIN_PRE(QS_QF_TIMEEVT_AUTO_DISARM, act->m_prio)
                    QS_OBJ_PRE(te);       // this time event object
                    QS_OBJ_PRE(act);      // the target AO
                    QS_U8_PRE(tickRate);  // tick rate
                QS_END_PRE()
            }

            QS_BEGIN_PRE(QS_QF_TIMEEVT_POST, act->m_prio)
                QS_TIME_PRE();            // timestamp
                QS_OBJ_PRE(te);           // the time event object
                QS_SIG_PRE(te->sig);      // signal of this time event
                QS_OBJ_PRE(act);          // the target AO
                QS_U8_PRE(tickRate);      // tick rate
            QS_END_PRE()

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // POST_FROM_ISR() asserts if the queue overflows
            act->POST_FROM_ISR(te,
                pxHigherPriorityTaskWoken,
                sender);
        }
        else { // time event keeps timing out
            --ctr; // decrement the tick counter
            te->m_ctr = ctr; // update the original
#ifndef Q_UNSAFE
            te->m_ctr_dis = static_cast<QTimeEvtCtr>(~ctr);
#endif // ndef Q_UNSAFE

            prev = te; // advance to this time event

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        // re-enter crit. section to continue the loop
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    }

    Q_ENSURE_INCRIT(890, lbound > 0U);

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}
//............................................................................
QEvt *QF::newXfromISR_(std::uint_fast16_t const evtSize,
                       std::uint_fast16_t const margin,
                       enum_t const sig) noexcept
{
    // find the pool index that fits the requested event size...
    std::uint_fast8_t poolNum = 0U; // zero-based poolNum initially
    for (; poolNum < priv_.maxPool_; ++poolNum) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(priv_.ePool_[poolNum])) {
            break;
        }
    }

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // precondition:
    // - cannot run out of registered pools
    Q_REQUIRE_INCRIT(800, poolNum < priv_.maxPool_);

    ++poolNum; // convert to 1-based poolNum

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    // get event e (port-dependent)...
#ifdef Q_SPY
    QEvt *e = static_cast<QEvt *>(
        priv_.ePool_[poolNum - 1U].getFromISR(((margin != QF::NO_MARGIN)
            ? margin : 0U),
        static_cast<std::uint_fast8_t>(QS_EP_ID) + poolNum));
#else
    QEvt *e = static_cast<QEvt *>(
        priv_.ePool_[poolNum - 1U].getFromISR(((margin != QF::NO_MARGIN)
            ? margin : 0U), 0U));
#endif

    if (e != nullptr) { // was e allocated correctly?
        e->sig     = static_cast<QSignal>(sig); // set the signal
        e->refCtr_ = 0U;
        e->evtTag_ = static_cast<std::uint8_t>((poolNum << 4U) | 0x0FU);

#ifdef Q_SPY
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        QS_BEGIN_PRE(QS_QF_NEW,
                static_cast<uint_fast8_t>(QS_EP_ID) + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
#endif // def Q_SPY
    }
    else { // event was not allocated
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        // This assertion means that the event allocation failed,
        // and this failure cannot be tolerated. The most frequent
        // reason is an event leak in the application.
        Q_ASSERT_INCRIT(820, margin != QF::NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_NEW_ATTEMPT,
                static_cast<uint_fast8_t>(QS_EP_ID) + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    }

    // the returned event e is guaranteed to be valid (not NULL)
    // if we can't tolerate failed allocation
    return e;
}
//............................................................................
void QF::gcFromISR(QEvt const * const e) noexcept {
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    Q_REQUIRE_INCRIT(700, e != nullptr);
#ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(701, e->verify_());
#endif

    std::uint_fast8_t const poolNum = e->getPoolNum_();

    if (poolNum != 0U) { // is it a pool event (mutable)?

        if (e->refCtr_ > 1U) { // isn't this the last ref?

            QS_BEGIN_PRE(QS_QF_GC_ATTEMPT,
                    static_cast<uint_fast8_t>(QS_EP_ID) + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            QEvt_refCtr_dec_(e); // decrement the ref counter

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        else { // this is the last reference to this event, recycle it

            QS_BEGIN_PRE(QS_QF_GC,
                    static_cast<uint_fast8_t>(QS_EP_ID) + poolNum)
                QS_TIME_PRE();         // timestamp
                QS_SIG_PRE(e->sig);    // the signal of the event
                QS_2U8_PRE(e->getPoolNum_(), e->refCtr_);//poolNum & refCtr
            QS_END_PRE()

            // pool number must be in range
            Q_ASSERT_INCRIT(710, (poolNum <= priv_.maxPool_)
                                     && (poolNum <= QF_MAX_EPOOL));

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#ifdef Q_SPY
            // cast 'const' away in (QEvt *)e is OK because it's a pool event
            priv_.ePool_[poolNum - 1U].putFromISR(QF_CONST_CAST_(QEvt*, e),
                static_cast<uint_fast8_t>(QS_EP_ID) + e->getPoolNum_());
#else
            priv_.ePool_[poolNum - 1U].putFromISR(QF_CONST_CAST_(QEvt*, e), 0U);
#endif
        }
    }
    else {
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    }
}
//............................................................................
void *QMPool::getFromISR(std::uint_fast16_t const margin,
                         std::uint_fast8_t const qsId) noexcept
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // get volatile into temporaries
    QFreeBlock *fb = m_free_head;
    QMPoolCtr nFree = m_nFree;

    #ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(801, Q_PTR2UINT_CAST_(fb)
        == static_cast<std::uintptr_t>(~m_free_head_dis));
    Q_INVARIANT_INCRIT(802, nFree == static_cast<QMPoolCtr>(~m_nFree_dis));
    #endif // ndef Q_UNSAFE

    // have more free blocks than the requested margin?
    if (nFree > static_cast<QMPoolCtr>(margin)) {
        Q_ASSERT_INCRIT(810, fb != nullptr);

        QFreeBlock * const fb_next = fb->m_next;

    #ifndef Q_UNSAFE
        // the free block must have integrity (duplicate inverse storage)
        Q_INVARIANT_INCRIT(811, Q_PTR2UINT_CAST_(fb_next)
            == static_cast<std::uintptr_t>(~fb->m_next_dis));
    #endif // ndef Q_UNSAFE

        --nFree; // one less free block
        if (nFree == 0U) { // is the pool becoming empty?
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_INCRIT(820, fb_next == nullptr);

            m_nFree = 0U;
    #ifndef Q_UNSAFE
            m_nFree_dis = static_cast<QMPoolCtr>(~m_nFree);
            m_nMin = 0U; // remember that the pool got empty
    #endif // ndef Q_UNSAFE
        }
        else {
            m_nFree = nFree; // update the original
    #ifndef Q_UNSAFE
            m_nFree_dis = static_cast<QMPoolCtr>(~nFree);

            // The pool is not empty, so the next free-block pointer
            // must be in range.
            Q_INVARIANT_INCRIT(830,
                QF_PTR_RANGE_(fb_next, m_start, m_end));

            // is the # free blocks the new minimum so far?
            if (m_nMin > nFree) {
                m_nMin = nFree; // remember the minimum so far
            }
    #endif // ndef Q_UNSAFE
        }

        m_free_head = fb_next; // set the head to the next free block
    #ifndef Q_UNSAFE
        m_free_head_dis =
            static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(fb_next));
    #endif // ndef Q_UNSAFE

        QS_BEGIN_PRE(QS_QF_MPOOL_GET, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(this);      // this memory pool
            QS_MPC_PRE(nFree);     // # of free blocks in the pool
    #ifndef Q_UNSAFE
            QS_MPC_PRE(m_nMin);    // min # free blocks ever in the pool
    #else
            QS_MPC_PRE(0U);        // min # free blocks (not available)
    #endif // ndef Q_UNSAFE
        QS_END_PRE()
    }
    else { // don't have enough free blocks at this point
        fb = nullptr;

        QS_BEGIN_PRE(QS_QF_MPOOL_GET_ATTEMPT, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(this);      // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(margin);    // the requested margin
        QS_END_PRE()
    }

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    return fb; // return the block or nullptr to the caller
}
//............................................................................
void QMPool::putFromISR(void *block,
                        std::uint_fast8_t const qsId) noexcept
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QFreeBlock * const fb = static_cast<QFreeBlock *>(block);

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // get volatile into temporaries
    QFreeBlock * const free_head = m_free_head;
    QMPoolCtr nFree = m_nFree;

    #ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(901, Q_PTR2UINT_CAST_(free_head)
        == static_cast<std::uintptr_t>(~m_free_head_dis));
    Q_INVARIANT_INCRIT(902, nFree == static_cast<QMPoolCtr>(~m_nFree_dis));
    #endif // ndef Q_UNSAFE

    Q_REQUIRE_INCRIT(910, nFree < m_nTot);
    Q_REQUIRE_INCRIT(911, QF_PTR_RANGE_(fb, m_start, m_end));

    ++nFree; // one more free block in this pool

    m_free_head = fb; // set as new head of the free list
    m_nFree     = nFree;
    fb->m_next  = free_head; // link into the list
    #ifndef Q_UNSAFE
    m_free_head_dis = static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(fb));
    m_nFree_dis     = static_cast<QMPoolCtr>(~nFree);
    fb->m_next_dis  = static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(free_head));
    #endif

    QS_BEGIN_PRE(QS_QF_MPOOL_PUT, qsId)
        QS_TIME_PRE();         // timestamp
        QS_OBJ_PRE(this);      // this memory pool
        QS_MPC_PRE(nFree);     // the # free blocks in the pool
    QS_END_PRE()

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}

} // namespace QP

//============================================================================
// NOTE3:
// The event posting to FreeRTOS message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//

