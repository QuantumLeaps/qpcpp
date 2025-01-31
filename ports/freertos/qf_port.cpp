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
#include "qp_pkg.hpp"       // QP package-level interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#if ( configSUPPORT_STATIC_ALLOCATION == 0 )
    #error This QP/C++ port to FreeRTOS requires configSUPPORT_STATIC_ALLOCATION
#endif

#if ( configMAX_PRIORITIES < QF_MAX_ACTIVE )
    #error FreeRTOS configMAX_PRIORITIES must not be less than QF_MAX_ACTIVE
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
    // 1. Implicitly based on the AO's priority (by the formula specified
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
    //    would result in a different relative prioritization of AO's threads
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

    // the number of free slots available in the queue
    std::uint_fast16_t nFree =
         static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

    // required margin available?
    bool status = false; // assume that event cannot be posted
    if (margin == QF::NO_MARGIN) {
        if (nFree > 0U) { // free entries available in the queue?
            status = true; // can post
        }
        else { // no free entries available
            // The queue overflows, but QF_NO_MARGIN indicates that
            // the "event delivery guarantee" is required.
            Q_ERROR_INCRIT(210); // must be able to post the event
        }
    }
    else if (nFree > margin) { // enough free entries?
        status = true; // can post
    }
    else {
        // empty
    }

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(205, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    if (status) { // can post the event?
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE();        // timestamp
            QS_OBJ_PRE(sender);   // the sender object
            QS_SIG_PRE(e->sig);   // the signal of the event
            QS_OBJ_PRE(this);     // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_PRE(0U);       // min # free entries (unknown)
        QS_END_PRE()

        QF_CRIT_EXIT();

        BaseType_t err = xQueueSendToBack(
                             m_eQueue, static_cast<void const *>(&e), 0U);

        QF_CRIT_ENTRY();
        // posting to the FreeRTOS message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(220, err == pdPASS);
        QF_CRIT_EXIT();

#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
    }
    else { // event cannot be posted
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_PRE(margin);  // margin requested
        QS_END_PRE()

        QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
        QF::gc(e); // recycle the event to avoid a leak
#endif // (QF_MAX_EPOOL > 0U)
    }

    return status;
}

//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    Q_REQUIRE_INCRIT(300, e != nullptr);

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(205, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
        QS_EQC_PRE(static_cast<QEQueueCtr>(FREERTOS_QUEUE_GET_FREE()));
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

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
        QS_TIME_PRE();          // timestamp
        QS_SIG_PRE(e->sig);     // the signal of this event
        QS_OBJ_PRE(this);       // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
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

    Q_REQUIRE_INCRIT(400, e != nullptr);

    // the number of free slots available in the FreeRTOS queue
    std::uint_fast16_t const nFree =
        static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

    // required margin available?
    bool status = false; // assume that event cannot be posted
    if (margin == QF::NO_MARGIN) {
        if (nFree > 0U) { // free entries available in the queue?
            status = true; // can post
        }
        else { // no free entries available
            // The queue overflows, but QF_NO_MARGIN indicates that
            // the "event delivery guarantee" is required.
            Q_ERROR_INCRIT(410); // must be able to post the event
        }
    }
    else if (nFree > margin) { // enough free entries?
        status = true; // can post
    }
    else {
        // empty
    }

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        Q_ASSERT_INCRIT(405, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    if (status) { // can post the event?
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE();        // timestamp
            QS_OBJ_PRE(sender);   // the sender object
            QS_SIG_PRE(e->sig);   // the signal of the event
            QS_OBJ_PRE(this);     // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);    // # free entries available
            QS_EQC_PRE(0U);       // min # free entries (unknown)
        QS_END_PRE()

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        BaseType_t err = xQueueSendToBackFromISR(m_eQueue,
            static_cast<void const *>(&e),
            static_cast<BaseType_t*>(par));

        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        // posting to the FreeRTOS message queue must succeed, see NOTE3
        Q_ASSERT_INCRIT(420, err == pdPASS);
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#ifdef Q_UNSAFE
        Q_UNUSED_PAR(err);
#endif
    }
    else { // event cannot be posted
        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();       // timestamp
            QS_OBJ_PRE(sender);  // the sender object
            QS_SIG_PRE(e->sig);  // the signal of the event
            QS_OBJ_PRE(this);    // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_);
            QS_EQC_PRE(nFree);   // # free entries available
            QS_EQC_PRE(margin);  // margin requested
        QS_END_PRE()

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#if (QF_MAX_EPOOL > 0U)
        QF::gcFromISR(e); // recycle the event to avoid a leak
#endif // (QF_MAX_EPOOL > 0U)
    }

    return status;
}
//............................................................................
void QActive::publishFromISR(QEvt const *e,
                             void *par,
                             void const * const sender) noexcept
{
    Q_REQUIRE_INCRIT(500, e != nullptr);

    QSignal const sig = e->sig;

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // the published signal must be within the configured range
    Q_REQUIRE_INCRIT(510, sig < QActive::maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_PUBLISH, 0U)
        QS_TIME_PRE();          // the timestamp
        QS_OBJ_PRE(sender);     // the sender object
        QS_SIG_PRE(sig);        // the signal of the event
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
    QS_END_PRE()

    // is it a mutable event?
    if (e->poolNum_ != 0U) {
        // NOTE: The reference counter of a mutable event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF::gcFromISR()) decrements the reference counter
        // and recycles the event if the counter drops to zero. This covers
        // the case when the event was published without any subscribers.
        Q_ASSERT_INCRIT(505, e->refCtr_ < (2U * QF_MAX_ACTIVE));
        QEvt_refCtr_inc_(e);
    }

    // make a local, modifiable copy of the subscriber set
    QPSet subscrSet = QActive::subscrList_[sig].m_set;

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    if (subscrSet.notEmpty()) { // any subscribers?
        // the highest-prio subscriber
        std::uint_fast8_t p = subscrSet.findMax();

        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        // no need to lock the scheduler in the ISR context
        QActive *a = registry_[p];
        Q_ASSERT_INCRIT(520, a != nullptr);

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        //QF_SCHED_LOCK_(p); // no scheduler locking in FreeRTOS
        do { // loop over all subscribers
            // QACTIVE_POST() asserts internally if the queue overflows
            a->POST_FROM_ISR(e, par, sender);

            subscrSet.remove(p); // remove the handled subscriber
            if (subscrSet.notEmpty()) {  // still more subscribers?
                p = subscrSet.findMax(); // the highest-prio subscriber

                uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

                a = registry_[p];
                // the AO must be registered with the framework
                Q_ASSERT_INCRIT(530, a != nullptr);

                portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
            }
            else {
                p = 0U; // no more subscribers
            }
        } while (p != 0U);

        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        Q_ASSERT_INCRIT(590, p == 0U); // all subscribers processed
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        //QF_SCHED_UNLOCK_(); // no scheduler locking in FreeRTOS
    }

#if (QF_MAX_EPOOL > 0U)
    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
    QF::gcFromISR(e);
#endif // (QF_MAX_EPOOL > 0U)
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

    Q_REQUIRE_INCRIT(600, tickRate < Q_DIM(timeEvtHead_));

    QTimeEvt *prev = &timeEvtHead_[tickRate];

#ifdef Q_SPY
    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        prev->m_ctr = (prev->m_ctr + 1U);
        QS_TEC_PRE(prev->m_ctr); // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()
#endif // def Q_SPY

    // scan the linked-list of time events at this rate...
    while (true) {
        Q_ASSERT_INCRIT(610, prev != nullptr); // sanity check

        QTimeEvt *te = prev->m_next; // advance down the time evt. list

        if (te == nullptr) { // end of the list?

            // NO any new time events armed since the last QTimeEvt_tick_()?
            if (timeEvtHead_[tickRate].m_act == nullptr) {
                break; // terminate the for-loop
            }

            prev->m_next = timeEvtHead_[tickRate].toTimeEvt();
            timeEvtHead_[tickRate].m_act = nullptr;

            te = prev->m_next; // switch to the new list
        }

        // the time event 'te' must be valid
        Q_ASSERT_INCRIT(640, te != nullptr);

        QTimeEvtCtr ctr = te->m_ctr;

        if (ctr == 0U) { // time event scheduled for removal?
            prev->m_next = te->m_next;

            // mark time event 'te' as NOT linked
            te->m_flags &= static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);
            // do NOT advance the prev pointer

            // exit crit. section to reduce latency
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        else if (ctr == 1U) { // is time event about to expire?
            QActive * const act = te->toActive();
            prev = te->expire_(prev, act, tickRate);

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // POST_FROM_ISR() asserts if the queue overflows
            act->POST_FROM_ISR(te,
                pxHigherPriorityTaskWoken,
                sender);
        }
        else { // time event keeps timing out
            --ctr; // decrement the tick counter
            te->m_ctr = ctr; // update the original

            prev = te; // advance to this time event

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        // re-enter crit. section to continue the loop
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    }

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}

//............................................................................
QEvt *QF::newXfromISR_(std::uint_fast16_t const evtSize,
                       std::uint_fast16_t const margin,
                       enum_t const sig) noexcept
{
    // find the pool number that fits the requested event size...
    std::uint_fast8_t poolNum = 0U; // zero-based poolNum initially
    for (; poolNum < priv_.maxPool_; ++poolNum) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(priv_.ePool_[poolNum])) {
            break;
        }
    }

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // precondition:
    // - cannot run out of registered pools
    Q_REQUIRE_INCRIT(700, poolNum < priv_.maxPool_);

    ++poolNum; // convert to 1-based poolNum

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    // get event e (port-dependent)...
    QEvt *e;
#ifdef Q_SPY
    e = static_cast<QEvt *>(
        priv_.ePool_[poolNum - 1U].getFromISR(((margin != QF::NO_MARGIN)
            ? margin : 0U),
        static_cast<std::uint_fast8_t>(QS_EP_ID) + poolNum));
#else
    e = static_cast<QEvt *>(
        priv_.ePool_[poolNum - 1U].getFromISR(((margin != QF::NO_MARGIN)
            ? margin : 0U), 0U));
#endif

    if (e != nullptr) { // was e allocated correctly?
        e->sig      = static_cast<QSignal>(sig); // set the signal
        e->poolNum_ = poolNum;
        e->refCtr_  = 0U;

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
        Q_ASSERT_INCRIT(720, margin != QF::NO_MARGIN);

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

    Q_REQUIRE_INCRIT(800, e != nullptr);

    std::uint_fast8_t const poolNum = e->poolNum_;

    if (poolNum != 0U) { // is it a pool event (mutable)?

        if (e->refCtr_ > 1U) { // isn't this the last ref?

            QS_BEGIN_PRE(QS_QF_GC_ATTEMPT,
                    static_cast<uint_fast8_t>(QS_EP_ID) + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            Q_ASSERT_INCRIT(805, e->refCtr_ > 0U);
            QEvt_refCtr_dec_(e); // decrement the ref counter

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        else { // this is the last reference to this event, recycle it

            QS_BEGIN_PRE(QS_QF_GC,
                    static_cast<uint_fast8_t>(QS_EP_ID) + poolNum)
                QS_TIME_PRE();         // timestamp
                QS_SIG_PRE(e->sig);    // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            // pool number must be in range
            Q_ASSERT_INCRIT(810, (poolNum <= priv_.maxPool_)
                                     && (poolNum <= QF_MAX_EPOOL));
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // NOTE: casting 'const' away is legit because 'e' is a pool event
#ifdef Q_SPY
            // cast 'const' away in (QEvt *)e is OK because it's a pool event
            priv_.ePool_[poolNum - 1U].putFromISR(QF_CONST_CAST_(QEvt*, e),
                static_cast<uint_fast8_t>(QS_EP_ID) + e->poolNum_);
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
    void * *pfb = static_cast<void**>(m_freeHead); // pointer to free block
    QMPoolCtr nFree = m_nFree;

    // have more free blocks than the requested margin?
    if (nFree > static_cast<QMPoolCtr>(margin)) {
        Q_ASSERT_INCRIT(910, pfb != nullptr);

        void ** const pfb_next = static_cast<void**>(pfb[0]); // fast temporary

        --nFree; // one less free block
        if (nFree == 0U) { // is the pool becoming empty?
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_INCRIT(920, pfb_next == nullptr);

            m_nFree = 0U; // no more free blocks
            m_nMin  = 0U;  // remember that the pool got empty
        }
        else { // the pool is NOT empty

            // the next free-block pointer must be in range
            Q_INVARIANT_INCRIT(930,
                QF_PTR_RANGE_(pfb_next, m_start, m_end));

            m_nFree = nFree; // update the original
            if (m_nMin > nFree) { // is this the new minimum?
                m_nMin = nFree; // remember the minimum so far
            }
        }

        m_freeHead = pfb_next; // set the head to the next free block

        // change the allocated block contents so that it is different
        // than a free block inside the pool.
        pfb[0] = &m_end[1]; // invalid location beyond the end

        QS_BEGIN_PRE(QS_QF_MPOOL_GET, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(this);      // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(m_nMin);    // min # free blocks ever in the pool
        QS_END_PRE()
    }
    else { // don't have enough free blocks at this point
        pfb = nullptr;

        QS_BEGIN_PRE(QS_QF_MPOOL_GET_ATTEMPT, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(this);      // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(margin);    // the requested margin
        QS_END_PRE()
    }

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    return pfb; // return the block or nullptr to the caller
}
//............................................................................
void QMPool::putFromISR(void *block,
                        std::uint_fast8_t const qsId) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    void * * const pfb = static_cast<void**>(block);

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // get volatile into temporaries
    void ** const freeHead = static_cast<void**>(m_freeHead);
    QMPoolCtr nFree = m_nFree;

    Q_REQUIRE_INCRIT(1000, nFree < m_nTot);
    Q_REQUIRE_INCRIT(1010, QF_PTR_RANGE_(pfb, m_start, m_end));

    ++nFree; // one more free block in this pool

    m_freeHead = pfb; // set as new head of the free list
    m_nFree     = nFree;
    pfb[0]  = freeHead; // link into the list

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

