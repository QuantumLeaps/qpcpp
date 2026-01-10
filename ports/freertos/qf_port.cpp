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

#if (configSUPPORT_STATIC_ALLOCATION == 0)
    #error This QP/C++ port to FreeRTOS requires configSUPPORT_STATIC_ALLOCATION
#endif

#if (configMAX_PRIORITIES < QF_MAX_ACTIVE)
    #error FreeRTOS configMAX_PRIORITIES must not be less than QF_MAX_ACTIVE
#endif

//============================================================================
namespace { // anonymous namespace with local definitions

Q_DEFINE_THIS_MODULE("qf_port")

//............................................................................
static void task_main(void *pvParameters);  // prototype
static void task_main(void *pvParameters) { // FreeRTOS task signature
    QP::QActive::evtLoop_(reinterpret_cast<QP::QActive *>(pvParameters));
}

} // anonymous namespace

// The following macro provides the number of free slots in the FreeRTOS
// queue, see NOTE1 and NOTE2
#define FREERTOS_QUEUE_GET_FREE() \
    (m_osObject.m_queue.uxDummy4[1] - m_osObject.m_queue.uxDummy4[0])

//============================================================================
// Active Object customization...

namespace QP {

//............................................................................
bool QActive::postx_(
    QEvt const * const e,
    std::uint_fast16_t const margin,
    void const * const sender) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the event to post must not be NULL
    Q_REQUIRE_INCRIT(100, e != nullptr);

    // the number of free slots available in the FreeRTOS queue
    std::uint_fast16_t nFree =
         static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

    // should we try to post the event?
    bool status = ((margin == QF::NO_MARGIN)
        || (nFree > static_cast<std::uint_fast16_t>(margin)));

    if (status) { // should try to post the event?
#if (QF_MAX_EPOOL > 0U)
        if (e->poolNum_ != 0U) { // is it a mutable event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
#endif // (QF_MAX_EPOOL > 0U)

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(0U);     // min # free entries (unknown)
        QS_END_PRE()

        QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

        // post the event to the FreeRTOS event queue, see NOTE3
        status = (xQueueSendToBack(
            m_eQueue, static_cast<void const *>(&e), 0U) == pdPASS);
    }

    if (!status) { // event NOT posted?
        QF_CRIT_ENTRY();

        // posting is allowed to fail only when margin != QF_NO_MARGIN
        Q_ASSERT_INCRIT(130, margin != QF::NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(margin); // margin requested
        QS_END_PRE()

        QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
        QF::gc(e); // recycle the event to avoid a leak
#endif
    }

    return status;
}
//............................................................................
void QActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the posted event must be be valid (which includes not NULL)
    Q_REQUIRE_INCRIT(200, e != nullptr);

#if (QF_MAX_EPOOL > 0U)
    if (e->poolNum_ != 0U) { // is it a mutable event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
#endif // (QF_MAX_EPOOL > 0U)

    QS_BEGIN_PRE(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
        QS_EQC_PRE(FREERTOS_QUEUE_GET_FREE()); // # free entries available
        QS_EQC_PRE(0U);      // min # free entries (unknown)
    QS_END_PRE()

    QF_CRIT_EXIT(); // exit crit.sect. before calling RTOS API

    BaseType_t const err =
        xQueueSendToFront(m_eQueue, static_cast<void const *>(&e), 0U);

#ifndef Q_UNSAFE
    QF_CRIT_ENTRY();
    // LIFO posting to FreeRTOS message queue must succeed, see NOTE3
    Q_ASSERT_INCRIT(230, err == pdPASS);
    QF_CRIT_EXIT();
#else
    Q_UNUSED_PAR(err);
#endif
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    // wait for an event (forever)
    QEvt const *e;
    BaseType_t const err = xQueueReceive(m_eQueue, (void *)&e, portMAX_DELAY);

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    Q_ASSERT_INCRIT(310, err == pdPASS); // queue-get must succeed
#else
    Q_UNUSED_PAR(err);
#endif

    QS_BEGIN_PRE(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of this event
        QS_OBJ_PRE(this);    // this active object
        QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
        QS_EQC_PRE(FREERTOS_QUEUE_GET_FREE()); // # free entries available
    QS_END_PRE()

    QF_CRIT_EXIT();

    return e;
}
//............................................................................
std::uint16_t QActive::getQueueUse(std::uint_fast8_t const prio) noexcept {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueFree(std::uint_fast8_t const prio) noexcept {
    Q_UNUSED_PAR(prio);
    return 0U; // current use level in a queue not supported in this RTOS
}
//............................................................................
std::uint16_t QActive::getQueueMin(std::uint_fast8_t const prio) noexcept {
    Q_UNUSED_PAR(prio);
    return 0U; // minimum free entries in a queue not supported in this RTOS
}

//............................................................................
void QActive::start(
    QPrioSpec const prioSpec,
    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
    void * const stkSto, std::uint_fast16_t const stkSize,
    void const * const par)
{
    // create FreeRTOS message queue
    m_eQueue = xQueueCreateStatic(
        static_cast<UBaseType_t>(qLen),           // length of the queue
        static_cast<UBaseType_t>(sizeof(QEvt *)), // element size
        reinterpret_cast<std::uint8_t *>(qSto),   // queue buffer
        &m_osObject.m_queue);                     // static queue buffer

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // the FreeRTOS queue must be created correctly
    Q_ASSERT_INCRIT(410, m_eQueue != nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QP-priority
    m_pthre = 0U; // preemption-threshold (not used for AO registration)
    register_(); // make QF aware of this AO

    // top-most initial tran. (virtual call)
    init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    // task name provided by the user in QActive::setAttr() or default name
    char const *taskName = (m_osObject.m_task.pxDummy1 != nullptr)
        ? static_cast<char const *>(m_osObject.m_task.pxDummy1)
        : static_cast<char const *>("AO");

    // FreeRTOS priority, see NOTE1
    UBaseType_t freertos_prio = (prioSpec >> 8U);
    if (freertos_prio == 0U) {
        freertos_prio = FREERTOS_TASK_PRIO(m_prio);
    }

    // create the FreeRTOS task for the AO
    m_thread = xTaskCreateStatic(
        &task_main,     // the task function
        taskName,       // the name of the task
        stkSize/sizeof(portSTACK_TYPE), // stack length
        this,           // the 'pvParameters' parameter
        freertos_prio,  // FreeRTOS priority
        static_cast<StackType_t *>(stkSto), // stack storage
        &m_osObject.m_task); // task buffer

    QF_CRIT_ENTRY();
    // FreeRTOS task must be created correctly
    Q_ASSERT_INCRIT(490, m_thread != nullptr);
    QF_CRIT_EXIT();
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    if (QActive_subscrList_ != nullptr) {
        unsubscribeAll(); // unsubscribe from all events
    }
    m_eQueue = static_cast<QueueHandle_t>(0); // stop thread (see QF::thread_)
}
#endif
//............................................................................
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    switch (attr1) {
        case TASK_NAME_ATTR:
            // m_osObject must not be used yet
            Q_ASSERT_INCRIT(510, m_osObject.m_task.pxDummy1 == nullptr);
            // temporarily store the name, cast 'const' away
            m_osObject.m_task.pxDummy1 = const_cast<void *>(attr2);
            break;
        // ...
        default:
            break;
    }
    QF_CRIT_EXIT();
}
//............................................................................
void QActive::evtLoop_(QActive *act) {
    // the event-loop...
#ifdef QACTIVE_CAN_STOP
    while (act->m_eQueue != static_cast<QueueHandle_t>(0)) {
#else
    for (;;) { // for-ever
#endif
        QEvt const *e = act->get_();   // BLOCK for event
        act->dispatch(e, act->m_prio); // dispatch event (virtual call)
#if (QF_MAX_EPOOL > 0U)
        QF::gc(e); // check if the event is garbage, and collect it if so
#endif
    }
#ifdef QACTIVE_CAN_STOP
    act->unregister_(); // remove this object from the framewrok
    vTaskDelete(static_cast<TaskHandle_t>(0)); // delete this FreeRTOS task
#endif
}

//============================================================================
// The "FromISR" QP APIs for the FreeRTOS port...

//............................................................................
bool QActive::postFromISR(
    QEvt const * const e,
    std::uint_fast16_t const margin,
    void *par,
    void const * const sender) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // the event to post must not be NULL
    Q_REQUIRE_INCRIT(600, e != nullptr);

    // the number of free slots available in the FreeRTOS queue
    std::uint_fast16_t const nFree =
        static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

    // should we try to post the event?
    bool status = ((margin == QF::NO_MARGIN)
        || (nFree > static_cast<std::uint_fast16_t>(margin)));

    if (status) { // should try to post the event?
#if (QF_MAX_EPOOL > 0U)
        if (e->poolNum_ != 0U) { // is it a mutable event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
#endif // (QF_MAX_EPOOL > 0U)

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(0U);     // min # free entries (unknown)
        QS_END_PRE()

        // exit crit.sect. before calling RTOS API
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        // post the event to the FreeRTOS event queue, see NOTE3
        status = (xQueueSendToBackFromISR(m_eQueue,
            static_cast<void const *>(&e),
            static_cast<BaseType_t *>(par)) == pdPASS);
    }

    if (!status) { // event NOT posted?
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        // posting is allowed to fail only when margin != QF::NO_MARGIN
        Q_ASSERT_INCRIT(630, margin != QF::NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE();      // timestamp
            QS_OBJ_PRE(sender); // the sender object
            QS_SIG_PRE(e->sig); // the signal of the event
            QS_OBJ_PRE(this);   // this active object (recipient)
            QS_2U8_PRE(e->poolNum_, e->refCtr_); // pool-Num & ref-Count
            QS_EQC_PRE(nFree);  // # free entries available
            QS_EQC_PRE(margin); // margin requested
        QS_END_PRE()

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#if (QF_MAX_EPOOL > 0U)
        QF::gcFromISR(e); // recycle the event to avoid a leak
#endif
    }

    return status;
}
//............................................................................
void QActive::publishFromISR(
    QEvt const * const e,
    void *par,
    void const * const sender) noexcept
{
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // the event must be valid
    Q_REQUIRE_INCRIT(700, e != nullptr);
    QSignal const sig = e->sig;
    // the published signal must be within the configured range
    Q_REQUIRE_INCRIT(710, sig < QActive_maxPubSignal_);

    QS_BEGIN_PRE(QS_QF_PUBLISH, 0U)
        QS_TIME_PRE();          // the timestamp
        QS_OBJ_PRE(sender);     // the sender object
        QS_SIG_PRE(sig);        // the signal of the event
        QS_2U8_PRE(e->poolNum_, e->refCtr_);
    QS_END_PRE()

#if (QF_MAX_EPOOL > 0U)
    // is it a mutable event?
    if (e->poolNum_ != 0U) {
        QEvt_refCtr_inc_(e);
    }
#endif // (QF_MAX_EPOOL > 0U)

    // make a local, modifiable copy of the subscriber set
    QPSet subscrSet = QActive_subscrList_[sig].m_set;

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    if (subscrSet.notEmpty()) { // any subscribers?
        // the highest-prio subscriber
        std::uint_fast8_t p = subscrSet.findMax();

        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        // no need to lock the scheduler in the ISR context
        QActive *a = QActive_registry_[p];
        // the AO must be registered with the framework
        Q_ASSERT_INCRIT(720, a != nullptr);

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        //QF_SCHED_LOCK_(p); // no scheduler locking in FreeRTOS
        do { // loop over all subscribers
            // QACTIVE_POST() asserts internally if the queue overflows
            a->POST_FROM_ISR(e, par, sender);

            subscrSet.remove(p); // remove the handled subscriber
            if (subscrSet.notEmpty()) {  // still more subscribers?
                p = subscrSet.findMax(); // the highest-prio subscriber

                uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

                a = QActive_registry_[p];
                // the AO must be registered with the framework
                Q_ASSERT_INCRIT(730, a != nullptr);

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

//----------------------------------------------------------------------------
// QTimeEvt "fromISR"

//............................................................................
void QTimeEvt::tickFromISR(
    std::uint_fast8_t const tickRate,
    void *pxHigherPriorityTaskWoken,
    void const * const sender) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(sender);
#endif

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    Q_REQUIRE_INCRIT(800, tickRate < Q_DIM(QTimeEvt_head_));

    QTimeEvt *prev = &QTimeEvt_head_[tickRate];

#ifdef Q_SPY
    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        prev->m_ctr = (prev->m_ctr + 1U);
        QS_TEC_PRE(prev->m_ctr); // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()
#endif // def Q_SPY

    // scan the linked-list of time events at this rate...
    while (true) {
        Q_ASSERT_INCRIT(810, prev != nullptr); // sanity check

        QTimeEvt *te = prev->m_next; // advance down the time evt. list

        if (te == nullptr) { // end of the list?

            // NO any new time events armed since the last QTimeEvt_tick_()?
            if (QTimeEvt_head_[tickRate].m_act == nullptr) {
                break; // terminate the for-loop
            }

            prev->m_next = QTimeEvt_head_[tickRate].toTimeEvt();
            QTimeEvt_head_[tickRate].m_act = nullptr;

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

//----------------------------------------------------------------------------
#if (QF_MAX_EPOOL > 0U)

QEvt *QF::newXfromISR_(
    std::uint_fast16_t const evtSize,
    std::uint_fast16_t const margin,
    QSignal const sig) noexcept
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
        static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum));
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
                static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
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
                static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
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

    Q_REQUIRE_INCRIT(900, e != nullptr);

    std::uint_fast8_t const poolNum = e->poolNum_;

    if (poolNum != 0U) { // is it a pool event (mutable)?

        if (e->refCtr_ > 1U) { // isn't this the last ref?

            QS_BEGIN_PRE(QS_QF_GC_ATTEMPT,
                    static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
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
                    static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
                QS_TIME_PRE();         // timestamp
                QS_SIG_PRE(e->sig);    // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            // pool number must be in range
            Q_ASSERT_INCRIT(910, (poolNum <= priv_.maxPool_)
                                  && (poolNum <= QF_MAX_EPOOL));
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // NOTE: casting 'const' away is legit because 'e' is a pool event
#ifdef Q_SPY
            // cast 'const' away in (QEvt *)e is OK because it's a pool event
            priv_.ePool_[poolNum - 1U].putFromISR(const_cast<QEvt *>(e),
                static_cast<std::uint_fast8_t>(QS_ID_EP) + e->poolNum_);
#else
            priv_.ePool_[poolNum - 1U].putFromISR(const_cast<QEvt *>(e), 0U);
#endif
        }
    }
    else {
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    }
}
//............................................................................
void *QMPool::getFromISR(
    std::uint_fast16_t const margin,
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
        Q_ASSERT_INCRIT(1010, pfb != nullptr);

        void ** const pfb_next = static_cast<void**>(pfb[0]); // fast temporary

        --nFree; // one less free block
        if (nFree == 0U) { // is the pool becoming empty?
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_INCRIT(920, pfb_next == nullptr);

            m_nFree = 0U; // no more free blocks
            m_nMin  = 0U; // remember that the pool got empty
        }
        else { // the pool is NOT empty

            // the next free-block pointer must be in range
            Q_ASSERT_INCRIT(930,
                (m_start <= pfb_next) && (pfb_next <= m_end));

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
void QMPool::putFromISR(
    void *block,
    std::uint_fast8_t const qsId) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    void * * const pfb = static_cast<void**>(block);

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // get volatile into temporaries
    void * * const freeHead = static_cast<void**>(m_freeHead);
    QMPoolCtr nFree = m_nFree;

    Q_REQUIRE_INCRIT(1000, nFree < m_nTot);
    Q_REQUIRE_INCRIT(1010, (m_start <= pfb) && (pfb < m_end));

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
#endif // (QF_MAX_EPOOL > 0U)

//============================================================================
namespace QF {

void init() {
    // nothing to do for FreeRTOS
}
//............................................................................
int_t run() {
    onStartup(); // the startup callback

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()
    QS_CRIT_EXIT();
#endif // Q_SPY

    vTaskStartScheduler(); // start the FreeRTOS scheduler--should never return

    return 0; // this unreachable return keeps the compiler happy
}
//............................................................................
void stop() {
    onCleanup(); // cleanup callback
}

} // namespace QF
} // namespace QP

//============================================================================
// NOTE1:
// The FreeRTOS priority of the AO thread can be specified in two ways:
//
// A. Implicitly based on the AO's priority (by the formula specified
//    in the macro FREERTOS_TASK_PRIO(), see qp_port.h). This option
//    is chosen, when the higher-byte of the prioSpec parameter is set
//    to zero.
//
// B. Explicitly as the higher-byte of the prioSpec parameter.
//    This option is chosen when the prioSpec parameter is not-zero.
//    For example, Q_PRIO(10U, 5U) will explicitly specify AO priority
//    as 10 and FreeRTOS priority as 5.
//
// CAUTION: The explicit FreeRTOS priority is NOT sanity-checked, so it is the
// responsibility of the application to ensure that it is consistent with the
// QP priority. An example of inconsistent setting would be assigning FreeRTOS
// priorities that would result in a different relative prioritization of AOs
// than indicated by the QP priorities assigned to the AOs.
//
// NOTE2:
// The official FreeRTOS API uxQueueSpacesAvailable() is not used because
// that API uses task-level critical section internally. Instead, the free
// slots calculation happens here in already established critical section.
// Unfortunately, the bizarre "information obfuscating" policy of FreeRTOS
// (incorrectly called "information hiding") forces the use of the StaticQueue_t
// with "dummy" members. This could potentially break in the future releases
// of FreeRTOS.
//
// Currently, the correspondence between xQUEUE and StaticQueue_t is as follows
// (see queue.c and FreeRTOS.h, respectively):
//
// xQUEUE.uxMessagesWaiting == StaticQueue_t.uxDummy4[0];
// xQUEUE.uxLength          == StaticQueue_t.uxDummy4[1];
//
// NOTE3:
// The event posting to FreeRTOS message queue occurs OUTSIDE critical section,
// which means that the remaining margin of available slots in the queue
// cannot be guaranteed. The problem is that interrupts and other tasks can
// preempt the event posting after checking the margin, but before actually
// posting the event to the queue.
//
