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
//! @version Last updated for: @ref qpcpp_7_3_1
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
    // empty for FreeRTOS
}
//............................................................................
int_t QF::run() {
    onStartup(); // the startup callback (configure/enable interrupts)

    // produce the QS_QF_RUN trace record
#ifdef Q_SPY
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()
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
    QEvt const * * const qSto,
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
    Q_REQUIRE_INCRIT(200,
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
    Q_ASSERT_INCRIT(210, m_eQueue != static_cast<QueueHandle_t>(0));
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

    // The FreeRTOS priority of the AO thread can be specificed in two ways:
    //
    // 1. Implictily based on the AO's priority (by the forumla specified
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
    //    it is consistent witht the AO's priority. An example of
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
    Q_ASSERT_INCRIT(220, task != static_cast<TaskHandle_t>(0));
    QF_CRIT_EXIT();
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
    Q_REQUIRE_INCRIT(300, m_thread.pxDummy1 == nullptr);
    switch (attr1) {
        case TASK_NAME_ATTR:
            // temporarily store the name, cast 'const' away
            m_thread.pxDummy1 = const_cast<void *>(attr2);
            break;
        // ...
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

    // find the number of free slots available in the queue
    std::uint_fast16_t nFree =
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

        QS_BEGIN_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->getPoolId_(), e->refCtr_); // pool Id&ref Count
            QS_EQC_PRE_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_PRE_(0U);     // min # free entries (unknown)
        QS_END_PRE_()

        if (e->getPoolId_() != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        QF_CRIT_EXIT();

        BaseType_t err = xQueueSendToBack(
                             m_eQueue, static_cast<void const *>(&e), 0U);

        // posting to the FreeRTOS message queue must succeed, see NOTE3
        QF_CRIT_ENTRY();
        Q_ASSERT_INCRIT(520, err == pdPASS);
    }
    else {

        QS_BEGIN_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->getPoolId_(), e->refCtr_); // pool Id&ref Count
            QS_EQC_PRE_(static_cast<QEQueueCtr>(nFree)); // # free entries
            QS_EQC_PRE_(margin); // margin requested
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
        QS_2U8_PRE_(e->evtTag_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_PRE_(static_cast<QEQueueCtr>(FREERTOS_QUEUE_GET_FREE()));
        QS_EQC_PRE_(0U);      // min # free entries (unknown)
    QS_END_PRE_()

    if (e->getPoolId_() != 0U) { // is it a pool event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }
    QF_CRIT_EXIT();

    BaseType_t err = xQueueSendToFront(
                         m_eQueue, static_cast<void const *>(&e), 0U);

    // LIFO posting to the FreeRTOS queue must succeed, see NOTE3
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(610, err == pdPASS);
    QF_CRIT_EXIT();
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    QEvt const *e;
    xQueueReceive(m_eQueue, (void *)&e, portMAX_DELAY);

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->getPoolId_(), e->refCtr_); // pool Id&ref Count
        QS_EQC_PRE_(static_cast<QEQueueCtr>(FREERTOS_QUEUE_GET_FREE()));
    QS_END_PRE_()
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
            Q_ERROR_INCRIT(810); // must be able to post the event
        }
    }
    else if (nFree > margin) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->getPoolId_(), e->refCtr_); // pool Id&ref Count
            QS_EQC_PRE_(nFree);  // # free entries available
            QS_EQC_PRE_(0U);     // min # free entries (unknown)
        QS_END_PRE_()

        if (e->getPoolId_() != 0U) { // is it a pool event?
            QEvt_refCtr_inc_(e); // increment the reference counter
        }
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        BaseType_t err = xQueueSendToBackFromISR(m_eQueue,
            static_cast<void const *>(&e),
            static_cast<BaseType_t*>(par));

        // posting to the FreeRTOS message queue must succeed
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        Q_ASSERT_INCRIT(820, err == pdPASS);
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    }
    else {

        QS_BEGIN_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->getPoolId_(), e->refCtr_); // pool Id&ref Count
            QS_EQC_PRE_(nFree);  // # free entries available
            QS_EQC_PRE_(margin); // margin requested
        QS_END_PRE_()
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
    QSignal const sig = e->sig;

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    //! @pre the published signal must be within the configured range
    Q_REQUIRE_INCRIT(500, sig < QActive::maxPubSignal_);
    Q_REQUIRE_INCRIT(502,
        subscrList_[sig].m_set.verify_(&subscrList_[sig].m_set_dis));

    QS_BEGIN_PRE_(QS_QF_PUBLISH, 0U)
        QS_TIME_PRE_();          // the timestamp
        QS_OBJ_PRE_(sender);     // the sender object
        QS_SIG_PRE_(sig);        // the signal of the event
        QS_2U8_PRE_(e->getPoolId_(), e->refCtr_);// pool-Id & ref-Count
    QS_END_PRE_()

    // is it a dynamic event?
    if (e->getPoolId_() != 0U) {
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
            Q_ASSERT_INCRIT(510, registry_[p] != nullptr);
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
                           void *par,
                           void const * const sender) noexcept
{
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    QTimeEvt *prev = &timeEvtHead_[tickRate];

    QS_BEGIN_PRE_(QS_QF_TICK, 0U)
        ++prev->m_ctr;
        QS_TEC_PRE_(prev->m_ctr); // tick ctr
        QS_U8_PRE_(tickRate);     // tick rate
    QS_END_PRE_()

    // scan the linked-list of time events at this rate...
    for (;;) {
        QTimeEvt *t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == nullptr) {

            // any new time events armed since the last run?
            if (timeEvtHead_[tickRate].m_act != nullptr) {

                // sanity check
                Q_ASSERT_INCRIT(610, prev != nullptr);
                prev->m_next = QTimeEvt::timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = nullptr;
                t = prev->m_next;  // switch to the new list
            }
            else {
                break; // all currently armed time evts. processed
            }
        }

        // time event scheduled for removal?
        if (t->m_ctr == 0U) {
            prev->m_next = t->m_next;
            // mark time event 't' as NOT linked
            t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);
            // do NOT advance the prev pointer
            // exit crit. section to reduce latency
            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        else {
            --t->m_ctr;

            // is time evt about to expire?
            if (t->m_ctr == 0U) {
                QActive *act = t->toActive(); // temp. for volatile

                // periodic time evt?
                if (t->m_interval != 0U) {
                    t->m_ctr = t->m_interval; // rearm the time event
                    prev = t; // advance to this time event
                }
                // one-shot time event: automatically disarm
                else {
                    prev->m_next = t->m_next;
                    // mark time event 't' as NOT linked
                    t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);
                    // do NOT advance the prev pointer

                    QS_BEGIN_PRE_(QS_QF_TIMEEVT_AUTO_DISARM, act->m_prio)
                        QS_OBJ_PRE_(t);        // this time event object
                        QS_OBJ_PRE_(act);      // the target AO
                        QS_U8_PRE_(tickRate);  // tick rate
                    QS_END_PRE_()
                }

                QS_BEGIN_PRE_(QS_QF_TIMEEVT_POST, act->m_prio)
                    QS_TIME_PRE_();            // timestamp
                    QS_OBJ_PRE_(t);            // the time event object
                    QS_SIG_PRE_(t->sig);       // signal of time event
                    QS_OBJ_PRE_(act);          // the target AO
                    QS_U8_PRE_(tickRate);      // tick rate
                QS_END_PRE_()

                // exit critical section before posting
                portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

                // asserts if the queue overflows
                act->POST_FROM_ISR(t, par, sender);
            }
            else {
                prev = t; // advance to this time event
                // exit crit. section to reduce latency
                portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
            }
        }
        // re-enter crit. section to continue
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}
//............................................................................
QEvt *QF::newXfromISR_(std::uint_fast16_t const evtSize,
                       std::uint_fast16_t const margin,
                       enum_t const sig) noexcept
{
    // find the poolId that fits the requested event size ...
    std::uint_fast8_t idx;
    for (idx = 0U; idx < priv_.maxPool_; ++idx) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(priv_.ePool_[idx])) {
            break;
        }
    }
    // cannot run out of registered pools
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    Q_REQUIRE_INCRIT(700, idx < priv_.maxPool_);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    // get e -- platform-dependent
#ifdef Q_SPY
    QEvt *e = static_cast<QEvt *>(
              priv_.ePool_[idx].getFromISR(((margin != QF::NO_MARGIN)
                  ? margin : 0U),
                  static_cast<std::uint_fast8_t>(QS_EP_ID) + idx + 1U));
#else
    QEvt *e = static_cast<QEvt *>(
              priv_.ePool_[idx].getFromISR(((margin != QF::NO_MARGIN)
                  ? margin : 0U), 0U));
#endif

    // was e allocated correctly?
    if (e != nullptr) {
        e->sig     = static_cast<QSignal>(sig); // set the signal
        e->refCtr_ = 0U;
        e->evtTag_ = static_cast<std::uint8_t>(QEvt::MARKER | (idx + 1U));

#ifdef Q_SPY
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        QS_BEGIN_PRE_(QS_QF_NEW,
                      static_cast<uint_fast8_t>(QS_EP_ID) + idx + 1U)
            QS_TIME_PRE_();         // timestamp
            QS_EVS_PRE_(evtSize);   // the size of the event
            QS_SIG_PRE_(sig);       // the signal of the event
        QS_END_PRE_()
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
#endif // Q_SPY
    }
    else { // event cannot be allocated
        // must tolerate bad alloc.
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        Q_ASSERT_INCRIT(720, margin != QF::NO_MARGIN);
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#ifdef Q_SPY
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        QS_BEGIN_PRE_(QS_QF_NEW_ATTEMPT,
                             static_cast<uint_fast8_t>(QS_EP_ID) + idx + 1U)
            QS_TIME_PRE_();         // timestamp
            QS_EVS_PRE_(evtSize);   // the size of the event
            QS_SIG_PRE_(sig);       // the signal of the event
        QS_END_PRE_()
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
#endif // Q_SPY
    }
    return e; // can't be NULL if we can't tolerate bad allocation
}
//............................................................................
void QF::gcFromISR(QEvt const * const e) noexcept {
    // is it a dynamic event?
    if (e->getPoolId_() != 0U) {
        UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        // isn't this the last ref?
        if (e->refCtr_ > 1U) {
            QEvt_refCtr_dec_(e); // decrement the ref counter

            QS_BEGIN_PRE_(QS_QF_GC_ATTEMPT,
                          static_cast<uint_fast8_t>(e->getPoolId_()))
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of the event
                QS_2U8_PRE_(e->getPoolId_(), e->refCtr_);//pool-Id&ref-Count
            QS_END_PRE_()

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        // this is the last reference to this event, recycle it
        else {
            std::uint_fast8_t idx =
                static_cast<std::uint_fast8_t>(e->getPoolId_()) - 1U;

            QS_BEGIN_PRE_(QS_QF_GC,
                          static_cast<uint_fast8_t>(e->getPoolId_()))
                QS_TIME_PRE_();         // timestamp
                QS_SIG_PRE_(e->sig);    // the signal of the event
                QS_2U8_PRE_(e->getPoolId_(), e->refCtr_);//pool-Id&ref-Count
            QS_END_PRE_()

            // pool ID must be in range
            Q_ASSERT_INCRIT(810, idx < priv_.maxPool_);

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

#ifdef Q_SPY
            // cast 'const' away, which is OK because it's a pool event
            priv_.ePool_[idx].putFromISR(QF_CONST_CAST_(QEvt*, e),
                static_cast<uint_fast8_t>(QS_EP_ID) + e->getPoolId_());
#else
            priv_.ePool_[idx].putFromISR(QF_CONST_CAST_(QEvt*, e), 0U);
#endif
        }
    }
}
//............................................................................
void QMPool::putFromISR(void *block,
                        std::uint_fast8_t const qs_id) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qs_id);
#endif

    QFreeBlock * const fb = static_cast<QFreeBlock *>(block);

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // precondition:
    // - # free blocks cannot exceed the total # blocks and
    // - the block pointer must be from this pool.
    Q_REQUIRE_INCRIT(900, (m_nFree < m_nTot)
                          && QF_PTR_RANGE_(fb, m_start, m_end));

    fb->m_next = static_cast<QFreeBlock *>(m_free_head); // link into list
#ifndef Q_UNSAFE
    fb->m_next_dis = static_cast<uintptr_t>(~Q_UINTPTR_CAST_(fb->m_next));
#endif

    m_free_head = fb; // set as new head of the free list
    ++m_nFree;        // one more free block in this pool

    QS_BEGIN_PRE_(QS_QF_MPOOL_PUT, qs_id)
        QS_TIME_PRE_();         // timestamp
        QS_OBJ_PRE_(this);      // this memory pool
        QS_MPC_PRE_(m_nFree);   // the number of free blocks in the pool
    QS_END_PRE_()

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}
//............................................................................
void *QMPool::getFromISR(std::uint_fast16_t const margin,
                         std::uint_fast8_t const qs_id) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qs_id);
#endif

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // have more free blocks than the requested margin?
    QFreeBlock *fb;
    if (m_nFree > static_cast<QMPoolCtr>(margin)) {
        fb = static_cast<QFreeBlock *>(m_free_head); // get a free block

        // the pool has some free blocks, so a free block must be available
        Q_ASSERT_INCRIT(900, fb != nullptr);

        QFreeBlock * const fb_next = fb->m_next; // fast temporary to avoid UB

        // the free block must have integrity (duplicate inverse storage)
        Q_ASSERT_INCRIT(902, Q_UINTPTR_CAST_(fb_next)
                              == static_cast<uintptr_t>(~fb->m_next_dis));

        // is the pool becoming empty?
        --m_nFree; // one less free block
        if (m_nFree == 0U) {
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_INCRIT(920, fb_next == nullptr);

            m_nMin = 0U; // remember that the pool got empty
        }
        else {
            // invariant
            // The pool is not empty, so the next free-block pointer,
            // so the next free block must be in range.
            //
            // NOTE: The next free block pointer can fall out of range
            // when the client code writes past the memory block, thus
            // corrupting the next block.
            Q_ASSERT_INCRIT(930, QF_PTR_RANGE_(fb_next, m_start, m_end));

            // is the number of free blocks the new minimum so far?
            if (m_nMin > m_nFree) {
                m_nMin = m_nFree; // remember the new minimum
            }
        }

        m_free_head = fb_next; // set the head to the next free block

        QS_BEGIN_PRE_(QS_QF_MPOOL_GET, qs_id)
            QS_TIME_PRE_();         // timestamp
            QS_OBJ_PRE_(this);      // this memory pool
            QS_MPC_PRE_(m_nFree);   // # free blocks in the pool
            QS_MPC_PRE_(m_nMin);    // min # free blocks ever in the pool
        QS_END_PRE_()
    }
    else { // don't have enough free blocks at this point
        fb = nullptr;

        QS_BEGIN_PRE_(QS_QF_MPOOL_GET_ATTEMPT, qs_id)
            QS_TIME_PRE_();         // timestamp
            QS_OBJ_PRE_(this);      // this memory pool
            QS_MPC_PRE_(m_nFree);   // # free blocks in the pool
            QS_MPC_PRE_(margin);    // the requested margin
        QS_END_PRE_()
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    return fb; // return the block or NULL pointer to the caller
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

