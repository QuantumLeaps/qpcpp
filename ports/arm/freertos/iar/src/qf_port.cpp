//////////////////////////////////////////////////////////////////////////////
// Product:  QF/C++, generic port to FreeRTOS.org, IAR compiler
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 19, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qf_pkg.h"
#include "qassert.h"

Q_DEFINE_THIS_MODULE(qf_port)

// Global objects ------------------------------------------------------------

//............................................................................
char const Q_ROM * Q_ROM_VAR QF::getPortVersion(void) {
    static const char Q_ROM Q_ROM_VAR version[] =  "4.2.02";
    return version;
}
//............................................................................
void QF::init(void) {
    // no special initialization required by FreeRTOS.org
}
//............................................................................
in16_t QF::run(void) {
    vTaskStartScheduler();                 // start the FreeRTOS.org scheduler
    Q_ERROR();               // the FreeRTOS.org scheduler should never return
    return static_cast<int16_t>(0);                          // return success
}
//............................................................................
void QF::stop(void) {
}
//............................................................................
static __arm void task_function(void *pvParameters) {    // FreeRTOS signature
    ((QActive *)pvParameters)->m_running = (uint8_t)1;// allow the thread-loop
    while (((QActive *)pvParameters)->m_running) {
        QEvt const *e = ((QActive *)pvParameters)->get_();   // wait for event
        ((QActive *)pvParameters)->dispatch(e);              // dispatch event
        QF::gc(e);      // check if the event is garbage, and collect it if so
    }

    QF::remove_((QActive *)pvParameters); // remove this AO from the framework
    vTaskDelete(((QActive *)pvParameters)->m_thread);  // delete FreeRTOS task
}
//............................................................................
void QActive::start(uint8_t prio,
                   QEvt const *qSto[], uint32_t qLen,
                   void *stkSto, uint32_t stkSize,
                   QEvt const *ie)
{
    portBASE_TYPE err;
    unsigned portBASE_TYPE freeRTOS_prio;

    Q_REQUIRE((qSto == (QEvt const **)0) && (qLen > 0)
              && (stkSto == (void *)0) && (stkSize > 0));

    m_eQueue = xQueueCreate(qLen, sizeof(QEvt*));        // create event queue
    Q_ASSERT(m_eQueue != (xQueueHandle)0);           // FreeRTOS queue created
    m_prio = prio;                                     // save the QF priority
    QF::add_(this);                     // make QF aware of this active object
    init(ie);                                    // execute initial transition

    QS_FLUSH();                          // flush the trace buffer to the host

    freeRTOS_prio = tskIDLE_PRIORITY + m_prio;     // FreeRTOS task priority
                                    // create the FreeRTOS.org task for the AO
    err = xTaskCreate(&task_function,                     // the task function
              (signed portCHAR *)"AO",                 // the name of the task
              (unsigned portSHORT)stkSize/sizeof(portSTACK_TYPE),     // stack
              this,                            // the 'pvParameters' parameter
              freeRTOS_prio,                         // FreeRTOS task priority
              &m_thread);                                       // task handle
    Q_ASSERT(err == pdPASS);                      // FreeRTOS.org task created
}
//............................................................................
void QActive::stop(void) {
    m_running = (uint8_t)0;                            // stop the thread loop
    vQueueDelete(m_eQueue);                               // cleanup the queue
}
//............................................................................
void QActive::postFIFO(QEvt const *e) {
    portBASE_TYPE err;
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();
    if (e->dynamic_ != (uint8_t)0) {
        ++((QEvt *)e)->dynamic_;
    }

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_POST_FIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(e->dynamic_);             // the dynamic attribute of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
    if (ulInterruptNesting == (unsigned portLONG)0) {           // task level?
        err = xQueueSendToBack(m_eQueue, &e, (portTickType)0);
    }
    else {                                                // must be ISR level
        portBASE_TYPE xHigherPriorityTaskWoken;
        err = xQueueSendToBackFromISR(m_eQueue, &e,
                                      &xHigherPriorityTaskWoken);
    }
    Q_ASSERT(err == pdPASS);
}
//............................................................................
void QActive::postLIFO(QEvt const *e) {
    portBASE_TYPE err;
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();
    if (e->dynamic_ != (uint8_t)0) {
        ++((QEvt *)e)->dynamic_;
    }

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_POST_LIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(e->dynamic_);             // the dynamic attribute of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
    if (ulInterruptNesting == (unsigned portLONG)0) {           // task level?
        err = xQueueSendToFront(m_eQueue, &e, (portTickType)0);
    }
    else {                                                // must be ISR level
        portBASE_TYPE xHigherPriorityTaskWoken;
        err = xQueueSendToFrontFromISR(m_eQueue, &e,
                                       &xHigherPriorityTaskWoken);
    }
    Q_ASSERT(err == pdPASS);
}
//............................................................................
QEvt const *QActive::get_(void) {
    QEvt const *e;
                 // wait indefinitely, INCLUDE_vTaskSuspend must be set to '1'
    Q_ALLEGE(xQueueReceive(m_eQueue, &e, portMAX_DELAY) == pdPASS);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(e->dynamic_);             // the dynamic attribute of the event
        QS_EQC_(0);                        // number of free entries (unknown)
    QS_END_()

    return e;
}
