//============================================================================
// Product: BSP for system-testing QK, EFM32-SLSTK3401A board
// Last updated for version 7.1.2
// Last updated on  2022-10-06
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "bsp.hpp"

#include "em_device.h"  // the device specific header (SiLabs)
#include "em_cmu.h"     // Clock Management Unit (SiLabs)
#include "em_gpio.h"    // GPIO (SiLabs)
#include "em_usart.h"   // USART (SiLabs)
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

// namespace BSP *************************************************************
namespace BSP {

// Local-scope objects -------------------------------------------------------
#define LED_PORT    gpioPortF
#define LED0_PIN    4
#define LED1_PIN    5

#define PB_PORT     gpioPortF
#define PB0_PIN     6
#define PB1_PIN     7

#ifdef Q_SPY

    // QSpy source IDs
    static QP::QSpyId const l_SysTick_Handler = { 0U };
    static QP::QSpyId const l_test_ISR = { 0U };

    enum AppRecords { // application-specific trace records
        CONTEXT_SW = QP::QS_USER,
        TRACE_MSG
    };

#endif

} // namespace BSP

// ISRs used in this project =================================================
extern "C" {

//............................................................................
void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    QK_ISR_ENTRY();   // inform QK about entering an ISR

    QP::QTimeEvt::TICK_X(0U, &BSP::l_SysTick_Handler);
    //the_Ticker0->POST(0, &BSP::l_SysTick_Handler);

    QK_ISR_EXIT();  // inform QK about exiting an ISR
}
//............................................................................
void GPIO_EVEN_IRQHandler(void);  // prototype
void GPIO_EVEN_IRQHandler(void) {
    QK_ISR_ENTRY(); // inform QK about entering an ISR

    // for testing...
    static QP::QEvt const t1 = { TEST1_SIG, 0U, 0U };
    QP::QF::PUBLISH(&t1, &BSP::l_test_ISR);

    QK_ISR_EXIT();  // inform QK about exiting an ISR
}

} // extern "C"

// BSP functions =============================================================
void BSP::init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // NOTE: The VFP (hardware Floating Point) unit is configured by QK

    // enable clock for to the peripherals used by this application...
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO,  true);
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO,  true);

    // configure the LEDs
    GPIO_PinModeSet(LED_PORT, LED0_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED1_PIN, gpioModePushPull, 0);
    GPIO_PinOutClear(LED_PORT, LED0_PIN);
    GPIO_PinOutClear(LED_PORT, LED1_PIN);

    // configure the Buttons
    GPIO_PinModeSet(PB_PORT, PB0_PIN, gpioModeInputPull, 1);
    GPIO_PinModeSet(PB_PORT, PB1_PIN, gpioModeInputPull, 1);

    if (!QS_INIT(nullptr)) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_test_ISR);

    QS_USR_DICTIONARY(CONTEXT_SW);
    QS_USR_DICTIONARY(TRACE_MSG);
}
//............................................................................
void BSP::terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}
//............................................................................
void BSP::ledOn(void) {
    GPIO->P[LED_PORT].DOUT = GPIO->P[LED_PORT].DOUT | (1U << LED1_PIN);
}
//............................................................................
void BSP::ledOff(void) {
    GPIO->P[LED_PORT].DOUT = GPIO->P[LED_PORT].DOUT & ~(1U << LED1_PIN);
}
//............................................................................
void BSP::trigISR(void) {
    NVIC_SetPendingIRQ(GPIO_EVEN_IRQn);
}
//............................................................................
void BSP::trace(QP::QActive const *thr, char const *msg) {
    QS_BEGIN_ID(TRACE_MSG, 0U)
        QS_OBJ(thr);
        QS_STR(msg);
    QS_END()
}

// namespace QP ==============================================================
namespace QP {

// QF callbacks --------------------------------------------------------------
void QF::onStartup(void) {
    // assign all priority bits for preemption-prio. and none to sub-prio.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE00
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    // DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    //
    NVIC_SetPriority(USART0_RX_IRQn, 0U); // kernel UNAWARE interrupt
    NVIC_SetPriority(GPIO_EVEN_IRQn, QF_AWARE_ISR_CMSIS_PRI + 0U);
    NVIC_SetPriority(SysTick_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 1U);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {
#ifdef Q_SPY
    QS::rxParse();  // parse all the received bytes
    QS::doOutput();
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    __WFI(); // Wait-For-Interrupt
#endif
}

// QS callbacks ==============================================================
//............................................................................
void QTimeEvt::tick1_(
    uint_fast8_t const tickRate,
    void const * const sender)
{
    QF_INT_DISABLE();
    /* TODO pend the SysTick */
    *Q_UINT2PTR_CAST(uint32_t, 0xE000ED04U) = (1U << 26U);
    QF_INT_ENABLE();
}

//----------------------------------------------------------------------------

} // namespace QP

extern "C" {
//............................................................................
#ifdef QK_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QK_onContextSw(QP::QActive *prev, QP::QActive *next) {
    QS_BEGIN_NOCRIT(BSP::CONTEXT_SW, 0U) // no critical section!
        QS_OBJ(prev);
        QS_OBJ(next);
    QS_END_NOCRIT()
}
#endif // QK_ON_CONTEXT_SW

} // extern "C"

