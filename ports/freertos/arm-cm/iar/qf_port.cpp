/// @file
/// @brief QF/C++ port to FreeRTOS.org kernel, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-05-11
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
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

// global varibles used by this FreeRTOS port
FreeRTOSExtras FreeRTOS_extras;

//............................................................................
void QF::init(void) {
    FreeRTOS_extras.isrNest = static_cast<BaseType_t>(0);
}
//............................................................................
int_t QF::run(void) {
    onStartup();  // the startup callback (configure/enable interrupts)
    vTaskStartScheduler(); // start the FreeRTOS.org scheduler
    Q_ERROR_ID(200);       // vTaskStartScheduler() should never return
    return static_cast<int_t>(0); // dummy return to make the compiler happy
}
//............................................................................
void QF::stop(void) {
    onCleanup();  // cleanup callback
}
// thread for active objects -------------------------------------------------
void QF::thread_(QMActive *act) {
    do {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    } while (act->m_thread != static_cast<TaskHandle_t>(0));
    act->unsubscribeAll();
}
//............................................................................
static void task_function(void *pvParameters) { // FreeRTOS signature
    QMActive *act = reinterpret_cast<QMActive *>(pvParameters);

    QF::thread_(act);
    QF::remove_(act); // remove this object from QF
    vTaskDelete(static_cast<TaskHandle_t>(0)); // delete this FreeRTOS task
}
//............................................................................
void QMActive::start(uint_fast8_t prio,
                     QEvt const *qSto[], uint_fast16_t qLen,
                     void *stkSto, uint_fast16_t stkSize,
                     QEvt const *ie)
{
    Q_REQUIRE_ID(600,
        (qSto != static_cast<QEvt const **>(0))   // queue storage provided
        && (qLen > static_cast<uint_fast16_t>(0)) // queue size must provided
        && (stkSto == static_cast<void *>(0))     // no stack storage
        && (stkSize > static_cast<uint_fast16_t>(0))); // stack size provided

    // create the event queue for the AO
    m_eQueue.init(qSto, qLen);

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object
    init(ie);       // thake the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // create the FreeRTOS.org task for the AO
    portBASE_TYPE err = xTaskCreate(
        &task_function,   // the task function
        (const char *)"AO",  // the name of the task
        (uint16_t)stkSize/sizeof(portSTACK_TYPE), // stack size
        this,                // the 'pvParameters' parameter
        (UBaseType_t)(prio + tskIDLE_PRIORITY),  // FreeRTOS priority
        &m_thread);          // task handle
    Q_ENSURE(err == pdPASS); // FreeRTOS task must be created
    m_osObject = m_thread;   // OS-Object for FreeRTOS is the task handle
}
//............................................................................
void QMActive::stop() {
    m_thread = static_cast<TaskHandle_t>(0); // stop the thread loop
}

} // namespace QP
