//****************************************************************************
// Product: QF/C++ generic port to FreeRTOS.org
// Last updated for version 5.3.1
// Last updated on  2014-09-29
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, www.state-machine.com.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// global varibles used by this FreeRTOS port (outside the QP namespace)
FreeRTOSExtras FreeRTOS_extras;

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

//............................................................................
void QF::init(void) {
    FreeRTOS_extras.isrNest = static_cast<BaseType_t>(0);
}
//............................................................................
int_t QF::run(void) {

    onStartup();           // startup callback
    vTaskStartScheduler(); // start the FreeRTOS.org scheduler
    Q_ERROR();             // the FreeRTOS scheduler should never return
    return static_cast<int_t>(0); // return success (just in case)
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}
//............................................................................
extern "C" void task_function(void *pvParameters) { // FreeRTOS task signature
    QF::thread_(static_cast<QActive *>(pvParameters));
}
//............................................................................
void QActive::start(uint_fast8_t const prio,
                    QEvt const *qSto[], uint_fast16_t const qLen,
                    void * const stkSto, uint_fast16_t const stkSize,
                    QEvt const * const ie)
{
    Q_REQUIRE(
        (qSto != static_cast<QEvt const **>(0)) /* queue storage provided */
        && (qLen > static_cast<uint_fast16_t>(0)) /* queue size provided */
        && (stkSto == static_cast<void *>(0))     /* no stack storage */
        && (stkSize > static_cast<uint_fast16_t>(0))); // stack size provided

    m_eQueue.init(qSto, qLen); // initialize QEQueue of this AO

    m_prio = prio;  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO
    this->init(ie); // execute initial transition (virtual call)

    // create the FreeRTOS task for the AO...
    portBASE_TYPE err = xTaskCreate(
        &task_function,   // the task function
        static_cast<const char *>("AO"),  // the name of the task
        static_cast<uint16_t>(stkSize/sizeof(portSTACK_TYPE)), // stack size
        static_cast<void *>(this), // the 'pvParameters' parameter
        static_cast<UBaseType_t>(prio + tskIDLE_PRIORITY),// FreeRTOS priority
        &m_thread); // task handle
    Q_ENSURE(err == pdPASS); // FreeRTOS task must be created
    m_osObject = m_thread;   // OS-Object for FreeRTOS is the task handle
}

//............................................................................
void QF::thread_(QActive *act) {
    // event-loop of an active object thread
    while (act->m_thread != static_cast<TaskHandle_t>(0)) {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }

    QF::remove_(act);        // remove this object from the framework
    vTaskDelete(static_cast<TaskHandle_t>(0)); // delete this FreeRTOS task
}
//............................................................................
void QActive::stop(void) {
    m_thread = static_cast<TaskHandle_t>(0); // stop the AO thread loop
}

} // namespace QP


