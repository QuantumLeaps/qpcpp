/// @file
/// @brief QF/C++ port to CMIS-RTOS RTX kernel, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-05-08
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

//............................................................................
static osThreadId l_tickerThreadId;
static uint32_t   l_tickerPeriod; // period of the ticker thread

// define __TARGET_FPU_VFP symbol depending on the compiler...
#if defined (__CC_ARM)          // ARM Compiler
    // in ARM Compiler __TARGET_FPU_VFP is a pre-defined symbol
#elif defined (__ICCARM__)      // IAR Compiler
    #if defined __ARMVFP__
        #define __TARGET_FPU_VFP 1
    #endif
#elif defined (__GNUC__)        // GNU Compiler
    #if defined (__VFP_FP__) && !defined(__SOFTFP__)
        #define __TARGET_FPU_VFP 1
    #endif
#else
    #error Unsupported ARM compiler
#endif

//............................................................................
void QF::init(void) {
    // CMSIS-RTX must be initialized and started from the starupt code.
    Q_REQUIRE_ID(100, osKernelRunning());

    // Special consideration for Cortex-M systems with the hardware FPU...
#ifdef __TARGET_FPU_VFP
    // Enable access to Floating-point coprocessor in NVIC_CPACR
    *reinterpret_cast<uint32_t volatile *>(0xE000ED88) |=
        ((3U << 10*2) | (3U << 11*2));

    // Explictily Disable the automatic FPU state preservation and
    // the FPU lazy stacking in SCB_FPCCR
    *reinterpret_cast<uint32_t volatile *>(0xE000EF34U) &=
        ~((1U << 31) | (1U << 30));

#endif // __TARGET_FPU_VFP

    // The main thread will be used as the ticker thread; save its ID
    l_tickerThreadId = osThreadGetId();

    // set the default period of the ticker thread...
    // NOTE: can be changed later in QF_setRtxTicker()
    //
    l_tickerPeriod = 100000U;  // [microseconds]
}
//............................................................................
int_t QF::run(void) {
    // CMSIS-RTX must be initialized and started from the starupt code
    Q_REQUIRE_ID(200, osKernelRunning());

    // call-back to the application to configure and start interrupts.
    // In the RTX port, QF_onStartup() typically also calls QF_setRtxTicker().
    //
    onStartup();

    // CMSIS-RTOS starts thread execution with the function main().
    // This main thread will continue executing after osKernelStart().
    //
    for (;;) {  // loop of the ticker thread
        QF_onRtxTicker(); // call-back to the app to handle the tick
        osDelay(l_tickerPeriod); // delay for the configurable period
    }
#if defined (__GNUC__)
    return static_cast<int_t>(0); // dummy return to make the compiler happy
#endif
}
//............................................................................
void QF::stop(void) {
    onCleanup(); // cleanup callback
}
//............................................................................
void QF_setRtxPrio(QMActive *act, osPriority prio) {
    // RTX thread not started yet?
    if (act->getThread() == static_cast<osThreadId>(0)) {
        // store the RTX prio, see NOTE1
        act->getOsObject() = static_cast<uint32_t>(prio);
    }
    else {
        osThreadSetPriority(act->getThread(), prio);
    }
}
//............................................................................
void QF_setRtxTicker(uint32_t period, osPriority prio) {
    l_tickerPeriod = period;
    osThreadSetPriority(l_tickerThreadId,
                        (prio != 0 ? prio : osPriorityNormal));
}

// thread for active objects -------------------------------------------------
void QF::thread_(QMActive *act) {

    // enable thread-loop, see NOTE2
    act->m_osObject = static_cast<uint32_t>(1);  // set event-loop control
    do {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    } while (act->m_osObject != static_cast<uint32_t>(0));
}
//............................................................................
static void ao_thread(void const *argument);  // prototype
static void ao_thread(void const *argument) { // RTX signature
    QMActive *act = (QMActive *)(argument);
    QF::thread_(act);
    act->unsubscribeAll();
    QF::remove_(act); // remove this object from QF
    osThreadTerminate(act->getThread()); // CMSIS-RTX: remove the thread
}
//............................................................................
void QMActive::start(uint_fast8_t prio,
                     QEvt const *qSto[], uint_fast16_t qLen,
                     void *stkSto, uint_fast16_t stkSize,
                     QEvt const *ie)
{

    // no stack storage because RTX pre-allocates stacks internally,
    // but the queue storage and size must be provided
    Q_REQUIRE_ID(510, (stkSto == (void *)0)
                      && (qSto != (QEvt const **)0)
                      && (qLen > (uint_fast16_t)0));

    // create the QP event queue for the AO
    m_eQueue.init(qSto, qLen);

    m_prio = prio;  // save the QF priority
    QF::add_(this);  // make QF aware of this active object
    init(ie);       // thake the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // create the RTX thread for the AO...
    // The following os_thread_def_AO object specifies the attributes of the
    // RTX thread for Active Objects. This object is allocated on the stack,
    // because it only serves to create the specific task for this AO.
    //
    osThreadDef_t os_thread_def_AO;
    os_thread_def_AO.pthread   = &ao_thread;  // RTX thread routine for AOs
    os_thread_def_AO.instances = (uint32_t)1; // # instances of this thread
    os_thread_def_AO.stacksize = (uint32_t)stkSize; // stack size [BYTES!]
    os_thread_def_AO.tpriority =
        (m_osObject != (uint32_t)0)    // RTX priority provided?
            ? (osPriority)(m_osObject) // use the provided value
            : osPriorityNormal;        // use the default
    m_thread = osThreadCreate(&os_thread_def_AO, (void *)this);

    // ensure that the thread was created correctly
    Q_ENSURE_ID(520, m_thread != (osThreadId)0);
}
//............................................................................
void QMActive::stop() {
    m_osObject = static_cast<uint32_t>(0); // stop the thread loop, see NOTE2
}

} // namespace QP

//****************************************************************************
// NOTE1:
// Before RTX thread for an AO is created, the member QActive.osObject is
// used to store the RTX priority to be applied in osThreadCreate().
//
// NOTE2:
// The member QActive.osObject is reused as the loop control variable,
// because the thread priority is alredy applied.
//
