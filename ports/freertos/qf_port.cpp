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
//! @date Last updated on: 2022-08-28
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QF/C++ port to FreeRTOS (v10.x) kernel, all supported compilers

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
    QP::QActive::thread_(reinterpret_cast<QP::QActive *>(pvParameters));
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
void QF::init(void) {
    // empty for FreeRTOS
}
//............................................................................
int_t QF::run(void) {
    onStartup();  // the startup callback (configure/enable interrupts)

    // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    vTaskStartScheduler(); // start the FreeRTOS scheduler
    Q_ERROR_ID(110);       // the FreeRTOS scheduler should never return
    return 0; // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}

//............................................................................
void QActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    Q_REQUIRE_ID(200,
        (qSto != nullptr)          /* queue storage */
        && (qLen > 0U)             /* queue size */
        && (stkSto != nullptr)     /* stack storage */
        && (stkSize > 0U));        // stack size

    m_prio = static_cast<std::uint8_t>(prioSpec & 0xFF); // QF-priority
    register_(); // make QF aware of this AO

    // create FreeRTOS message queue
    m_eQueue = xQueueCreateStatic(
            static_cast<UBaseType_t>(qLen), // length of the queue
            static_cast<UBaseType_t>(sizeof(QEvt *)), // element size
            reinterpret_cast<std::uint8_t *>(qSto), // queue buffer
            &m_osObject);                   // static queue buffer
    Q_ASSERT_ID(210, m_eQueue != static_cast<QueueHandle_t>(0));

    init(par, m_prio); // take the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // task name provided by the user in QF_setTaskName() or default name
    char const *taskName = (m_thread.pxDummy1 != nullptr)
                             ? static_cast<char const *>(m_thread.pxDummy1)
                             : static_cast<char const *>("AO");

    // statically create the FreeRTOS task for the AO
    Q_ALLEGE_ID(220,
        static_cast<TaskHandle_t>(0) != xTaskCreateStatic(
              &task_function, // the task function
              taskName ,      // the name of the task
              stkSize/sizeof(portSTACK_TYPE), // stack length
              this,           // the 'pvParameters' parameter
              FREERTOS_TASK_PRIO(m_prio), // also FreeRTOS priority
              static_cast<StackType_t *>(stkSto), // stack storage
              &m_thread));    // task buffer
}
//............................................................................
#ifdef QF_ACTIVE_STOP
void QActive::stop(void) {
    unsubscribeAll(); // unsubscribe from all events
    m_eQueue = nullptr; // stop the thread loop (see QF::thread_)
}
#endif
//............................................................................
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    // this function must be called before QACTIVE_START(),
    // which implies that me->thread.pxDummy1 must not be used yet;
    Q_REQUIRE_ID(300, m_thread.pxDummy1 == nullptr);
    switch (attr1) {
        case TASK_NAME_ATTR:
            // temporarily store the name
            m_thread.pxDummy1 = const_cast<void *>(attr2);
            break;
        //...
    }
}
// thread for active objects -------------------------------------------------
void QActive::thread_(QActive *act) {
#ifdef QF_ACTIVE_STOP
    while (act->m_eQueue != nullptr)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = act->get_();   // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QF_ACTIVE_STOP
    act->unregister_(); // remove this object from QF
    vTaskDelete(static_cast<TaskHandle_t>(0)); // delete this FreeRTOS task
#endif
}

//============================================================================
bool QActive::post_(QEvt const * const e, std::uint_fast16_t const margin,
                    void const * const sender) noexcept
{
    static_cast<void>(sender); // unused parameter when Q_SPY is undefined

    QF_CRIT_STAT_
    QF_CRIT_E_();

    // find the number of free slots available in the queue
    std::uint_fast16_t nFree =
        static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

    bool status;
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false;  // cannot post
            Q_ERROR_ID(510); // must be able to post the event
        }
    }
    else if (nFree > margin) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();       // timestamp
            QS_OBJ_PRE_(sender);  // the sender object
            QS_SIG_PRE_(e->sig);  // the signal of the event
            QS_OBJ_PRE_(this);    // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of evt
            QS_EQC_PRE_(nFree);   // # free entries
            QS_EQC_PRE_(0U);      // min # free (unknown)
        QS_END_NOCRIT_PRE_()

        // is it a pool event?
        if (e->poolId_ != 0U) {
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        QF_CRIT_X_();

        // posting to the FreeRTOS queue must succeed
        Q_ALLEGE_ID(520,
            xQueueSend(m_eQueue, static_cast<void const *>(&e), portMAX_DELAY)
            == pdPASS);
    }
    else {

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();       // timestamp
            QS_OBJ_PRE_(sender);  // the sender object
            QS_SIG_PRE_(e->sig);  // the signal of the event
            QS_OBJ_PRE_(this);    // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of evt
            QS_EQC_PRE_(nFree);   // # free entries
            QS_EQC_PRE_(0U);      // min # free (unknown)
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
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_PRE_(FREERTOS_QUEUE_GET_FREE()); // # free slots
        QS_EQC_PRE_(0U); // min # free entries (unknown)
    QS_END_NOCRIT_PRE_()

    // is it a pool event?
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e);  // increment the reference counter
    }

    QF_CRIT_X_();

    // LIFO posting to the FreeRTOS queue must succeed
    Q_ALLEGE_ID(610,
        xQueueSendToBack(m_eQueue, static_cast<void const *>(&e),
                         portMAX_DELAY) == pdPASS);
}
//............................................................................
QEvt const *QActive::get_(void) noexcept {
    QEvt const *e;
    xQueueReceive(m_eQueue, (void *)&e, portMAX_DELAY);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_ACTIVE_GET, m_prio)
        QS_TIME_PRE_();       // timestamp
        QS_SIG_PRE_(e->sig);  // the signal of this event
        QS_OBJ_PRE_(this);    // this active object
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
        QS_EQC_PRE_(FREERTOS_QUEUE_GET_FREE()); // # free entries
    QS_END_PRE_()

    return e;
}

//============================================================================
// The "FromISR" QP APIs for the FreeRTOS port...
bool QActive::postFromISR_(QEvt const * const e,
                           std::uint_fast16_t const margin, void *par,
                           void const * const sender) noexcept
{
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // find the number of free slots available in the queue
    std::uint_fast16_t nFree =
        static_cast<std::uint_fast16_t>(FREERTOS_QUEUE_GET_FREE());

    bool status;
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
            status = true; // can post
        }
        else {
            status = false; // cannot post
            Q_ERROR_ID(810); // must be able to post the event
        }
    }
    else if (nFree > margin) {
        status = true; // can post
    }
    else {
        status = false; // cannot post
    }

    if (status) { // can post the event?

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);  // # free entries available
            QS_EQC_PRE_(0U);     // min # free entries (unknown)
        QS_END_NOCRIT_PRE_()

        if (e->poolId_ != 0U) { // is it a pool event?
            QF_EVT_REF_CTR_INC_(e); // increment the reference counter
        }

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        // posting to the FreeRTOS message queue must succeed
        Q_ALLEGE_ID(820,
            xQueueSendFromISR(m_eQueue, static_cast<void const *>(&e),
                              static_cast<BaseType_t*>(par))
            == pdTRUE);
    }
    else {

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();      // timestamp
            QS_OBJ_PRE_(sender); // the sender object
            QS_SIG_PRE_(e->sig); // the signal of the event
            QS_OBJ_PRE_(this);   // this active object (recipient)
            QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & ref Count
            QS_EQC_PRE_(nFree);  // # free entries available
            QS_EQC_PRE_(margin); // margin requested
        QS_END_NOCRIT_PRE_()

        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

        QF::gcFromISR(e); // recycle the event to avoid a leak
    }

    return status;
}
//............................................................................
void QActive::publishFromISR_(QEvt const *e, void *par,
                              void const * const sender) noexcept
{
    //! @pre the published signal must be within the configured range
    Q_REQUIRE_ID(500, static_cast<enum_t>(e->sig) < QActive::maxPubSignal_);

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_PUBLISH, 0U)
        QS_TIME_PRE_();           // the timestamp
        QS_OBJ_PRE_(sender);      // the sender object
        QS_SIG_PRE_(e->sig);      // the signal of the event
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
    QS_END_NOCRIT_PRE_()

    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        // NOTE: The reference counter of a dynamic event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF::gcFromISR()) decrements the reference counter
        // and recycles the event if the counter drops to zero. This covers
        // the case when the event was published without any subscribers.
        //
        QF_EVT_REF_CTR_INC_(e);
    }

    // make a local, modifiable copy of the subscriber list
    QPSet subscrList = QActive::subscrList_[e->sig];
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    if (subscrList.notEmpty()) {
        // the highest-prio subscriber
        std::uint_fast8_t p = subscrList.findMax();

        // no need to lock the scheduler in the ISR context
        do { // loop over all subscribers
            // the prio of the AO must be registered with the framework
            Q_ASSERT_ID(510, registry_[p] != nullptr);

            // POST_FROM_ISR() asserts internally if the queue overflows
            (void)registry_[p]->POST_FROM_ISR(e, par, sender);

            subscrList.remove(p); // remove the handled subscriber
            if (subscrList.notEmpty()) {  // still more subscribers?
                p = subscrList.findMax(); // the highest-prio subscriber
            }
            else {
                p = 0U; // no more subscribers
            }
        } while (p != 0U);
        // no need to unlock the scheduler in the IRS context
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
    QF::gcFromISR(e);
}
//............................................................................
void QTimeEvt::tickFromISR_(std::uint_fast8_t const tickRate, void *par,
                            void const * const sender) noexcept
{
    QTimeEvt *prev = &timeEvtHead_[tickRate];
    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_TICK, 0U)
       ++prev->m_ctr;
        QS_TEC_PRE_(prev->m_ctr);  // tick ctr
        QS_U8_PRE_(tickRate);      // tick rate
    QS_END_NOCRIT_PRE_()

    // scan the linked-list of time events at this rate...
    for (;;) {
        QTimeEvt *t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == nullptr) {

            // any new time events armed since the last run?
            if (timeEvtHead_[tickRate].m_act != nullptr) {

                // sanity check
                Q_ASSERT_ID(610, prev != nullptr);
                prev->m_next = QTimeEvt::timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = nullptr;
                t = prev->m_next; // switch to the new list
            }
            else {
                break; // all currently armed time evts. processed
            }
        }

        // time event scheduled for removal?
        if (t->m_ctr == 0U) {
            prev->m_next = t->m_next;
            // mark as unlinked
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

                    // mark as unlinked
                    t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);
                    // do NOT advance the prev pointer

                    QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_AUTO_DISARM,
                                         act->m_prio)
                        QS_OBJ_PRE_(t);   // this time event object
                        QS_OBJ_PRE_(act); // the target AO
                        QS_U8_PRE_(tickRate);
                    QS_END_NOCRIT_PRE_()
                }

                QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_POST, act->m_prio)
                    QS_TIME_PRE_();       // timestamp
                    QS_OBJ_PRE_(t);       // the time event object
                    QS_SIG_PRE_(t->sig);  // signal of this time event
                    QS_OBJ_PRE_(act);     // the target AO
                    QS_U8_PRE_(tickRate);
                QS_END_NOCRIT_PRE_()

                // exit critical section before posting
                portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

                // asserts if queue overflows
                (void)act->POST_FROM_ISR(t, par, sender);
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
    // find the pool id that fits the requested event size ...
    std::uint_fast8_t idx;
    for (idx = 0U; idx < QF::maxPool_; ++idx) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(QF::ePool_[idx])) {
            break;
        }
    }
    // cannot run out of registered pools
    Q_ASSERT_ID(710, idx < QF::maxPool_);

    // get e -- platform-dependent
#ifdef Q_SPY
    QEvt *e = static_cast<QEvt *>(
              QF::ePool_[idx].getFromISR(((margin != QF_NO_MARGIN)
                  ? margin : 0U),
                  static_cast<std::uint_fast8_t>(QS_EP_ID) + idx + 1U));
    UBaseType_t uxSavedInterruptStatus;
#else
    QEvt *e = static_cast<QEvt *>(
              QF::ePool_[idx].getFromISR(((margin != QF_NO_MARGIN)
                  ? margin : 0U), 0U));
#endif

    // was e allocated correctly?
    if (e != nullptr) {
        e->sig     = static_cast<QSignal>(sig); // set the signal
        // store pool ID
        e->poolId_ = static_cast<std::uint8_t>(idx + 1U);
        // initialize the reference counter to 0
        e->refCtr_ = 0U;

    #ifdef Q_SPY
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        QS_BEGIN_NOCRIT_PRE_(QS_QF_NEW,
                             static_cast<uint_fast8_t>(QS_EP_ID) + idx + 1U)
            QS_TIME_PRE_();       // timestamp
            QS_EVS_PRE_(evtSize); // the size of the event
            QS_SIG_PRE_(sig);     // the signal of the event
        QS_END_NOCRIT_PRE_()
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    #endif // Q_SPY
    }
    else {
        // event was not allocated, assert that the caller provided non-zero
        // margin, which means that they can tollerate bad allocation
        Q_ASSERT_ID(720, margin != QF_NO_MARGIN);

    #ifdef Q_SPY
        uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
        QS_BEGIN_NOCRIT_PRE_(QS_QF_NEW_ATTEMPT,
                             static_cast<uint_fast8_t>(QS_EP_ID) + idx + 1U)
            QS_TIME_PRE_();       // timestamp
            QS_EVS_PRE_(evtSize); // the size of the event
            QS_SIG_PRE_(sig);     // the signal of the event
        QS_END_NOCRIT_PRE_()
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    #endif // Q_SPY
    }
    return e;
}
//............................................................................
void QF::gcFromISR(QEvt const * const e) noexcept {
    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        // isn't this the last reference?
        if (e->refCtr_ > 1U) {
            QF_EVT_REF_CTR_DEC_(e); // decrement the ref counter

            QS_BEGIN_NOCRIT_PRE_(QS_QF_GC_ATTEMPT,
                                 static_cast<uint_fast8_t>(e->poolId_))
                QS_TIME_PRE_();        // timestamp
                QS_SIG_PRE_(e->sig);   // the signal of the event
                QS_2U8_PRE_(e->poolId_, e->refCtr_);// pool Id & refCtr
            QS_END_NOCRIT_PRE_()

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        }
        // this is the last reference to this event, recycle it
        else {
            std::uint_fast8_t idx =
                static_cast<std::uint_fast8_t>(e->poolId_) - 1U;

            QS_BEGIN_NOCRIT_PRE_(QS_QF_GC,
                                 static_cast<uint_fast8_t>(e->poolId_))
                QS_TIME_PRE_();        // timestamp
                QS_SIG_PRE_(e->sig);   // the signal of the event
                QS_2U8_PRE_(e->poolId_, e->refCtr_);// pool Id & refCtr
            QS_END_NOCRIT_PRE_()

            portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

            // pool ID must be in range
            Q_ASSERT_ID(810, idx < QF::maxPool_);

#ifdef Q_EVT_XTOR
            // explicitly exectute the destructor'
            // NOTE: casting 'const' away is legitimate,
            // because it's a pool event
            QF_CONST_CAST_(QEvt*, e)->~QEvt(); // xtor,
#endif

#ifdef Q_SPY
            // cast 'const' away, which is OK, because it's a pool event
            QF::ePool_[idx].putFromISR(QF_CONST_CAST_(QEvt*, e),
                static_cast<uint_fast8_t>(QS_EP_ID) + e->poolId_);
#else
            QF::ePool_[idx].putFromISR(QF_CONST_CAST_(QEvt*, e), 0U);
#endif
        }
    }
}
//............................................................................
void QMPool::putFromISR(void *b, std::uint_fast8_t const qs_id) noexcept {
    //! @pre # free blocks cannot exceed the total # blocks and
    //! the block pointer must be in range to come from this pool.
    //!
    Q_REQUIRE_ID(900, (m_nFree < m_nTot)
                      && QF_PTR_RANGE_(b, m_start, m_end));

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    static_cast<QFreeBlock*>(b)->m_next =
        static_cast<QFreeBlock *>(m_free_head); // link into the free list
    m_free_head = b; // set as new head of the free list
    ++m_nFree;       // one more free block in this pool

    QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_PUT, qs_id)
        QS_TIME_PRE_();       // timestamp
        QS_OBJ_PRE_(m_start); // the memory managed by this pool
        QS_MPC_PRE_(m_nFree); // the number of free blocks in the pool
    QS_END_NOCRIT_PRE_()

    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}
//............................................................................
void *QMPool::getFromISR(std::uint_fast16_t const margin,
                         std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(qs_id); // unused parameter (outside Q_SPY conf.)

    UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    // have the than margin?
    QFreeBlock *fb;
    if (m_nFree > static_cast<QMPoolCtr>(margin)) {
        fb = static_cast<QFreeBlock *>(m_free_head);  // get a free block

        // the pool has some free blocks, so a free block must be available
        Q_ASSERT_ID(910, fb != nullptr);

        void *fb_next = fb->m_next; // put volatile to a temporary to avoid UB

        // is the pool becoming empty?
        --m_nFree;  // one free block less
        if (m_nFree == 0U) {
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_ID(920, fb_next == nullptr);

            m_nMin = 0U;// remember that pool got empty
        }
        else {
            // pool is not empty, so the next free block must be in range
            //
            // NOTE: the next free block pointer can fall out of range
            // when the client code writes past the memory block, thus
            // corrupting the next block.
            Q_ASSERT_ID(930, QF_PTR_RANGE_(fb_next, m_start, m_end));

            // is the number of free blocks the new minimum so far?
            if (m_nMin > m_nFree) {
                m_nMin = m_nFree; // remember the minimum so far
            }
        }

        m_free_head = fb_next; // adjust list head to the next free block

        QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_GET, qs_id)
            QS_TIME_PRE_();        // timestamp
            QS_OBJ_PRE_(m_start);  // the memory managed by this pool
            QS_MPC_PRE_(m_nFree);  // the number of free blocks in the pool
            QS_MPC_PRE_(m_nMin);   // the min # free blocks in the pool
        QS_END_NOCRIT_PRE_()
    }
    else {
        fb = nullptr;

        QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_GET_ATTEMPT, qs_id)
            QS_TIME_PRE_();        // timestamp
            QS_OBJ_PRE_(m_start);  // the memory managed by this pool
            QS_MPC_PRE_(m_nFree);  // the # free blocks in the pool
            QS_MPC_PRE_(margin);   // the requested margin
        QS_END_NOCRIT_PRE_()
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    return fb; // return the block or NULL pointer to the caller
}

} // namespace QP

