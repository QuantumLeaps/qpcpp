/**
* @file
* @brief "Experimental" QF/CPP port to Espressif ESP-IDF (version 4.x)
* @ingroup ports
* @cond
******************************************************************************
* Last updated for version 6.9.4
* Last updated on  2022-03-20
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2022 Victor Chavez
* Copyright (C) 2005-2021 Quantum Leaps, LLC. All rights reserved.
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses>.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
******************************************************************************
* @endcond
*/
#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// Select the CPU at which the QP Framework will be attached
#define CONFIG_QP_PINNED_TO_CORE_0
//#define CONFIG_QP_PINNED_TO_CORE_1

// Activate the QF ISR API required for FreeRTOS
#define QF_ISR_API            1

/* FreeRTOS-ESP32 event queue and thread types, see NOTE0 */
#define QF_EQUEUE_TYPE        QEQueue
#define QF_THREAD_TYPE        TaskHandle_t

/* The maximum number of active objects in the application, see NOTE1 */
#define QF_MAX_ACTIVE         8U

// QF interrupt disabling/enabling (task level)
#define QF_INT_DISABLE()      taskDISABLE_INTERRUPTS()
#define QF_INT_ENABLE()       taskENABLE_INTERRUPTS()

/* QF critical section for FreeRTOS-ESP32 (task level), see NOTE2 */
/* #define QF_CRIT_STAT_TYPE not defined */
#define QF_CRIT_ENTRY(dummy)  portENTER_CRITICAL(&QF_esp32mux)
#define QF_CRIT_EXIT(dummy)   portEXIT_CRITICAL(&QF_esp32mux)

#include "freertos/FreeRTOS.h"  /* FreeRTOS master include file, see NOTE4 */
#include "freertos/task.h"      /* FreeRTOS task  management */

#include "qep_port.hpp"  /* QEP port */
#include "qequeue.hpp"   /* this QP port uses the native QF event queue */
#include "qmpool.hpp"    /* this QP port uses the native QF memory pool */
#include "qf.hpp"        /* QF platform-independent public interface */

/* global spinlock "mutex" for all critical sections in QF (see NOTE3) */
extern PRIVILEGED_DATA portMUX_TYPE QF_esp32mux;

#if defined( CONFIG_QP_PINNED_TO_CORE_0 )
    #define QP_CPU_NUM         PRO_CPU_NUM
#elif defined( CONFIG_QP_PINNED_TO_CORE_0 )
    #define QP_CPU_NUM         APP_CPU_NUM
#else
    /* Defaults to APP_CPU */
    #define QP_CPU_NUM         APP_CPU_NUM
#endif

/* the "FromISR" versions of the QF APIs, see NOTE4 */
#ifdef Q_SPY
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        publishFromISR_((e_), (pxHigherPrioTaskWoken_),(sender_))

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR_((e_), QP::QF_NO_MARGIN, \
                      (pxHigherPrioTaskWoken_), (sender_))

    #define POST_X_FROM_ISR(e_, margin_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR_((e_), (margin_), (pxHigherPrioTaskWoken_), (sender_))

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, sender_) \
        tickXfromISR_((tickRate_), (pxHigherPrioTaskWoken_), (sender_))
#else
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        publishFromISR_((e_), (pxHigherPrioTaskWoken_))

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        postFromISR_((e_), QP::QF_NO_MARGIN, (pxHigherPrioTaskWoken_))

    #define POST_X_FROM_ISR(me_, e_, margin_,               \
                                    pxHigherPrioTaskWoken_,  dummy) \
        postFromISR_((e_), (margin_), (pxHigherPrioTaskWoken_))

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, dummy) \
        tickXfromISR_((tickRate_), (pxHigherPrioTaskWoken_))
#endif

#define TICK_FROM_ISR(pxHigherPrioTaskWoken_, sender_) \
    TICK_X_FROM_ISR(0U, pxHigherPrioTaskWoken_, sender_)

#ifdef Q_EVT_CTOR /* Shall the ctor for the ::QEvt class be provided? */

    #define Q_NEW_FROM_ISR(evtT_, sig_, ...)                  \
        (new(QP::QF::newXfromISR_(sizeof(evtT_), QP::QF_NO_MARGIN, 0)) \
            evtT_((sig_),  ##__VA_ARGS__))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_, ...) do { \
        (e_) = (evtT_ *)QF_newXFromISR_(sizeof(evtT_),           \
                                 (margin_), 0);                  \
        if ((e_) != (evtT_ *)0) {                                \
            evtT_##_ctor((e_), (sig_), ##__VA_ARGS__);           \
        }                                                        \
     } while (false)

#else

    #define Q_NEW_FROM_ISR(evtT_, sig_)                         \
        (static_cast<evtT_ *>(QP::QF::newXfromISR_(             \
                static_cast<std::uint_fast16_t>(sizeof(evtT_)), \
                QP::QF_NO_MARGIN, (sig_))))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_) ((e_) = \
        (evtT_ *)QF_newXFromISR_((uint_fast16_t)sizeof(evtT_), \
                                 (margin_), (sig_)))

#endif /* Q_EVT_CTOR */


namespace QP {
	
enum FreeRTOS_TaskAttrs {
    TASK_NAME_ATTR
};
} // namespace QP

/*****************************************************************************
* interface used only inside QF, but not in applications
*/
#ifdef QP_IMPL
    /* FreeRTOS blocking for event queue implementation (task level) */
    #define QACTIVE_EQUEUE_WAIT_(me_)                 \
        while ((me_)->m_eQueue.m_frontEvt == nullptr) { \
            QF_CRIT_X_();                             \
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  \
            QF_CRIT_E_();                             \
        }

    /* FreeRTOS signaling (unblocking) for event queue (task level) */
    #define QACTIVE_EQUEUE_SIGNAL_(me_) do {           \
        QF_CRIT_X_();                                  \
        xTaskNotifyGive((TaskHandle_t)(me_)->m_thread); \
        QF_CRIT_E_();                                  \
    } while (false)

    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) vTaskSuspendAll()
    #define QF_SCHED_UNLOCK_()    xTaskResumeAll()

    /* native QF event pool operations */
    #define QF_EPOOL_TYPE_            QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) \
        ((p_).put((e_), (qs_id_)))

#endif /* ifdef QP_IMPL */

/*****************************************************************************
* NOTE0:
* This is the "experimental" port to the [Espressif ESP-IDF][1]
* IoT Framework, which is loosely based on the [FreeRTOS kernel][2].
*
* "Experimental" means that the port has not been thouroughly tested at
* Quantum Leaps and no working examples are provided.
*
* The [Espressif ESP-IDF][1] is based on a significantly changed version
* of the FreeRTOS kernel developed by Espressif to support the ESP32 multi-core
* CPUs (see [ESP-IDF][1]).
*
* The Espressif version of FreeRTOS is __NOT__ compatible with the baseline
* FreeRTOS and it needs to be treated as a separate RTOS kernel.
* According to the comments in the Espressif source code, FreeRTOS-ESP-IDF
* is based on FreeRTOS V8.2.0, but apparently FreeRTOS-ESP-IDF has been
* updated with the newer features introduced to the original FreeRTOS in the
* later versions. For example, FreeRTOS-ESP32 supports the "static allocation",
* first introduced in baseline FreeRTOS V9.x. 
*
* [1]: https://www.espressif.com/en/products/sdks/esp-idf
* [2]: https://freertos.org
*
* NOTE1:
* The maximum number of active objects QF_MAX_ACTIVE can be increased to 64,
* inclusive, but it can be reduced to save some memory. Also, the number of
* active objects cannot exceed the number of FreeRTOS task priorities,
* because each QP active object requires a unique priority level.
*
* NOTE2:
* The critical section definition applies only to the FreeRTOS "task level"
* APIs. The "FromISR" APIs are defined separately.
*
* NOTE3:
* This QF port to FreeRTOS-ESP32 uses the FreeRTOS-ESP32 spin lock "mutex",
* similar to the internal implementation of FreeRTOS-ESP32 (see tasks.c).
* However, the QF port uses its own "mutex" object QF_esp32mux.
* 
* NOTE4:
* This QP/C++ port used as reference the esp-idf QP/C port counterpart. The main 
* difference implementation-wise is that instead of using xTaskCreateStaticPinnedToCore,
* it uses xTaskCreatePinnedToCore. The reason is that this port was designed to work
* with the Arduino SDK, which does not include xTaskCreateStaticPinnedToCore.
*
* NOTE5:
* The design of FreeRTOS requires using different APIs inside the ISRs
* (the "FromISR" variant) than at the task level. Accordingly, this port
* provides the "FromISR" variants for QP functions and "FROM_ISR" variants
* for QP macros to be used inside ISRs. ONLY THESE "FROM_ISR" VARIANTS
* ARE ALLOWED INSIDE ISRs AND CALLING THE TASK-LEVEL APIs IS AN ERROR.
*/

#endif /* QF_PORT_HPP */
