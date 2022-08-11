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
//! @date Last updated on: 2022-06-30
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QF/C++ port to FreeRTOS 10.x

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// FreeRTOS event queue and thread types
#define QF_EQUEUE_TYPE        QueueHandle_t
#define QF_OS_OBJECT_TYPE     StaticQueue_t
#define QF_THREAD_TYPE        StaticTask_t

// The maximum number of active objects in the application, see NOTE1
#define QF_MAX_ACTIVE         32U

// QF interrupt disabling/enabling (task level)
#define QF_INT_DISABLE()      taskDISABLE_INTERRUPTS()
#define QF_INT_ENABLE()       taskENABLE_INTERRUPTS()

// QF critical section for FreeRTOS (task level), see NOTE2
// #define QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(stat_)  taskENTER_CRITICAL()
#define QF_CRIT_EXIT(stat_)   taskEXIT_CRITICAL()

// FreeRTOS requires the "FromISR" API in QP/C++
#define QF_ISR_API      1

// enable the QActive::stop()
#define QF_ACTIVE_STOP  1

#include "FreeRTOS.h"   // FreeRTOS master include file, see NOTE4/
#include "task.h"       // FreeRTOS task management
#include "queue.h"      // FreeRTOS queue management

#include "qep_port.hpp" // QEP port
#include "qequeue.hpp"  // QF event queue (for deferring events)
#include "qmpool.hpp"   // this QP port uses memory pool (for event pools)
#include "qf.hpp"       // QF platform-independent public interface

// the "FromISR" versions of the QF APIs, see NOTE3
#ifdef Q_SPY
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        publishFromISR_((e_), (pxHigherPrioTaskWoken_), (sender_))

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR_((e_), QF_NO_MARGIN, \
                     (pxHigherPrioTaskWoken_), (sender_))

    #define POST_X_FROM_ISR(e_, margin_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR_((e_), (margin_), (pxHigherPrioTaskWoken_), (sender_))

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, sender_) \
        tickFromISR_((tickRate_), (pxHigherPrioTaskWoken_), (sender_))
#else
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        publishFromISR_((e_), (pxHigherPrioTaskWoken_), nullptr)

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        postFromISR_((e_), QF_NO_MARGIN, (pxHigherPrioTaskWoken_), \
                     nullptr)

    #define POST_X_FROM_ISR(e_, margin_, pxHigherPrioTaskWoken_, dummy) \
        postFromISR_((e_), (margin_), (pxHigherPrioTaskWoken_), nullptr)

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, dummy) \
        tickFromISR_((tickRate_), (pxHigherPrioTaskWoken_), nullptr)
#endif

#define TICK_FROM_ISR(pxHigherPrioTaskWoken_, sender_) \
    TICK_X_FROM_ISR(0U, (pxHigherPrioTaskWoken_), (sender_))

#ifdef Q_EVT_CTOR
    #define Q_NEW_FROM_ISR(evtT_, sig_, ...) \
        (new(QP::QF::newXfromISR_(sizeof(evtT_), QF_NO_MARGIN, 0)) \
            evtT_((sig_),  ##__VA_ARGS__))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_, ...) do {        \
        (e_) = static_cast<evtT_ *>(                                    \
                  QP::QF::newXfromISR_(static_cast<std::uint_fast16_t>( \
                  sizeof(evtT_)), (margin_), 0));                       \
        if ((e_) != nullptr) {                                          \
            new((e_)) evtT_((sig_),  ##__VA_ARGS__); \
        } \
     } while (false)

#else // QEvt is a POD (Plain Old Datatype)
    #define Q_NEW_FROM_ISR(evtT_, sig_)                         \
        (static_cast<evtT_ *>(QP::QF::newXfromISR_(             \
                static_cast<std::uint_fast16_t>(sizeof(evtT_)), \
                QF_NO_MARGIN, (sig_))))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_)          \
        ((e_) = static_cast<evtT_ *>(                           \
        QP::QF::newXfromISR_(sizeof(evtT_), (margin_), (sig_))))
#endif

namespace QP {

enum FreeRTOS_TaskAttrs {
    TASK_NAME_ATTR
};

} // namespace QP

// FreeRTOS hooks prototypes (not provided by FreeRTOS)
extern "C" {
#if (configUSE_IDLE_HOOK > 0)
    void vApplicationIdleHook(void);
#endif
#if (configUSE_TICK_HOOK > 0)
    void vApplicationTickHook(void);
#endif
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
#endif
#if (configSUPPORT_STATIC_ALLOCATION > 0)
    void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                       StackType_t **ppxIdleTaskStackBuffer,
                                       uint32_t *pulIdleTaskStackSize);
#endif
} // extern "C"

//============================================================================
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL
    #define FREERTOS_TASK_PRIO(qp_prio_) \
        ((UBaseType_t)((qp_prio_) + tskIDLE_PRIORITY))

    /* FreeRTOS scheduler locking for QF_publish_() (task context only) */
    #define QF_SCHED_STAT_      \
        UBaseType_t curr_prio;  \
        TaskHandle_t curr_task;
    #define QF_SCHED_LOCK_(prio_) do {                              \
         curr_task = xTaskGetCurrentTaskHandle();                   \
         curr_prio = uxTaskPriorityGet(curr_task);                  \
         if (FREERTOS_TASK_PRIO(prio_) > curr_prio) {               \
             vTaskPrioritySet(curr_task, FREERTOS_TASK_PRIO(prio_));\
         }                                                          \
         else {                                                     \
             curr_prio = tskIDLE_PRIORITY;                          \
         }                                                          \
    } while (0)

    #define QF_SCHED_UNLOCK_()                                      \
         if (curr_prio != tskIDLE_PRIORITY) {                       \
             vTaskPrioritySet(curr_task, curr_prio);                \
         } else ((void)0)

    // native QF event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

#endif /* ifdef QP_IMPL */

//============================================================================
// NOTE1:
// The maximum number of active objects QF_MAX_ACTIVE can be increased to 64,
// inclusive, but it can be reduced to save some memory. Also, the number of
// active objects cannot exceed the number of FreeRTOS task priorities,
// because each QP active object requires a unique priority level.
//
// NOTE2:
// The critical section definition applies only to the FreeRTOS "task level"
// APIs. The "FromISR" APIs are defined separately.
//
// NOTE3:
// The design of FreeRTOS requires using different APIs inside the ISRs
// (the "FromISR" variant) than at the task level. Accordingly, this port
// provides the "FromISR" variants for QP functions and "FROM_ISR" variants
// for QP macros to be used inside ISRs. ONLY THESE "FROM_ISR" VARIANTS
// ARE ALLOWED INSIDE ISRs AND CALLING THE TASK-LEVEL APIs IS AN ERROR.
//

#endif // QF_PORT_HPP
