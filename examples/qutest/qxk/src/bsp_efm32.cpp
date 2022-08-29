//============================================================================
// Product: BSP for system-testing QXK, EFM32-SLSTK3401A board
// Last updated for version 7.1.0
// Last updated on  2022-08-21
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
    static QP::QSpyId const l_GPIO_EVEN_IRQHandler = { 0U };

    static USART_TypeDef * const l_USART0 = ((USART_TypeDef *)(0x40010000UL));

    enum AppRecords { // application-specific trace records
        CONTEXT_SW = QP::QS_USER,
        COMMAND_STAT
    };

#endif

} // namespace BSP

// ISRs used in this project =================================================
extern "C" {

//............................................................................
void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    QXK_ISR_ENTRY();   // inform QXK about entering an ISR

    QP::QTimeEvt::TICK_X(0U, &BSP::l_SysTick_Handler);
    //the_Ticker0->POST(0, &BSP::l_SysTick_Handler);

    QXK_ISR_EXIT();  // inform QXK about exiting an ISR
}
//............................................................................
void GPIO_EVEN_IRQHandler(void);  // prototype

// NOTE: to trigger this ISR from the debugger, write 0x200 to NVIC_ISPR0.
//
void GPIO_EVEN_IRQHandler(void) {
    QXK_ISR_ENTRY(); // inform QXK about entering an ISR

    // for testing...
    //DPP::AO_Table->POST(Q_NEW(QP::QEvt, DPP::MAX_PUB_SIG),
    //                    &BSP::l_GPIO_EVEN_IRQHandler);
    QP::QF::PUBLISH(Q_NEW(QP::QEvt, TEST_SIG), // for testing...
                         &BSP::l_GPIO_EVEN_IRQHandler);

    QXK_ISR_EXIT();  // inform QXK about exiting an ISR
}

} // extern "C"

// BSP functions =============================================================
void BSP::init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    /* NOTE: The VFP (hardware Floating Point) unit is configured by QXK */
    //FPU->FPCCR = FPU->FPCCR
    //              | (1U << FPU_FPCCR_ASPEN_Pos) | (1U << FPU_FPCCR_LSPEN_Pos);

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
    // global signals
    QS_SIG_DICTIONARY(TEST_SIG,      (void *)0);
    QS_SIG_DICTIONARY(TIMEOUT_SIG,   (void *)0);

    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_GPIO_EVEN_IRQHandler);

    QS_USR_DICTIONARY(CONTEXT_SW);
    QS_USR_DICTIONARY(COMMAND_STAT);
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
void BSP::terminate(int16_t result) {
    (void)result;
}

// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // assign all priority bits for preemption-prio. and none to sub-prio.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE00
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    // DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    //
    NVIC_SetPriority(USART0_RX_IRQn, 0U); // kernel unaware interrupt
    NVIC_SetPriority(GPIO_EVEN_IRQn, QF_AWARE_ISR_CMSIS_PRI);
    NVIC_SetPriority(SysTick_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 1U);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
#ifdef QXK_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QXK::onContextSw(QActive *prev, QActive *next) {
    QS_BEGIN_NOCRIT(CONTEXT_SW, 0U) // no critical section!
        QS_OBJ(prev);
        QS_OBJ(next);
    QS_END_NOCRIT()
}
#endif // QXK_ON_CONTEXT_SW
//............................................................................
void QXK::onIdle(void) {
    GPIO->P[LED_PORT].DOUT |=  (1U << LED1_PIN);
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
    GPIO->P[LED_PORT].DOUT &= ~(1U << LED1_PIN);
}

// QS callbacks ==============================================================
#ifdef Q_SPY
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
/*..........................................................................*/
//void QS::processTestEvts_(void) {
//}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

