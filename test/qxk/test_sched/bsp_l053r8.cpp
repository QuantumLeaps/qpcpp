//============================================================================
// Product: BSP for system-testing QXK kernel, NUCLEO-L053R8 board
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

#include "stm32l0xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

// namespace BSP *************************************************************
namespace BSP {

// Local-scope objects -------------------------------------------------------
// LED pins available on the board (just one user LED LD2--Green on PA.5)
#define LED_LD2  (1U << 5)

// Button pins available on the board (just one user Button B1 on PC.13)
#define BTN_B1   (1U << 13)

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
    QXK_ISR_ENTRY();   // inform QXK about entering an ISR

    QP::QTimeEvt::TICK_X(0U, &BSP::l_SysTick_Handler);
    //the_Ticker0->POST(0, &BSP::l_SysTick_Handler);

    QXK_ISR_EXIT();  // inform QXK about exiting an ISR
}
//............................................................................
void EXTI0_1_IRQHandler(void);  // prototype
void EXTI0_1_IRQHandler(void) {
    QXK_ISR_ENTRY(); // inform QXK about entering an ISR

    // for testing...
    static QP::QEvt const t1 = { TEST1_SIG, 0U, 0U };
    QP::QF::PUBLISH(&t1, &BSP::l_test_ISR);

    QXK_ISR_EXIT();  // inform QXK about exiting an ISR
}

} // extern "C"

// BSP functions =============================================================
void BSP::init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // enable GPIOA clock port for the LED LD2
    RCC->IOPENR |= (1U << 0);

    /* configure LED (PA.5) pin as push-pull output, no pull-up, pull-down */
    GPIOA->MODER   &= ~((3U << 2*5));
    GPIOA->MODER   |=  ((1U << 2*5));
    GPIOA->OTYPER  &= ~((1U <<   5));
    GPIOA->OSPEEDR &= ~((3U << 2*5));
    GPIOA->OSPEEDR |=  ((1U << 2*5));
    GPIOA->PUPDR   &= ~((3U << 2*5));

    // enable GPIOC clock port for the Button B1
    RCC->IOPENR |=  (1U << 2);

    // configure Button (PC.13) pins as input, no pull-up, pull-down
    GPIOC->MODER   &= ~(3U << 2*13);
    GPIOC->OSPEEDR &= ~(3U << 2*13);
    GPIOC->OSPEEDR |=  (1U << 2*13);
    GPIOC->PUPDR   &= ~(3U << 2*13);

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
    GPIOA->BSRR |= (LED_LD2); // turn LED2 on
}
//............................................................................
void BSP::ledOff(void) {
    GPIOA->BSRR |= (LED_LD2 << 16); // turn LED2 off
}
//............................................................................
void BSP::trigISR(void) {
    NVIC_SetPendingIRQ(EXTI0_1_IRQn);
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
    NVIC_SetPriority(USART2_IRQn,  0U); // kernel UNAWARE interrupt
    NVIC_SetPriority(SysTick_IRQn, QF_AWARE_ISR_CMSIS_PRI + 0U);
    NVIC_SetPriority(EXTI0_1_IRQn, QF_AWARE_ISR_CMSIS_PRI + 1U);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(EXTI0_1_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QXK::onIdle(void) {
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
#ifdef QXK_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QXK_onContextSw(QP::QActive *prev, QP::QActive *next) {
    QS_BEGIN_NOCRIT(BSP::CONTEXT_SW, 0U) // no critical section!
        QS_OBJ(prev);
        QS_OBJ(next);
    QS_END_NOCRIT()
}
#endif // QXK_ON_CONTEXT_SW

} // extern "C"

