//============================================================================
// Product: DPP example, NUCLEO-L053R8 board, uC/OS-II RTOS
// Last updated for version 6.9.3
// Last updated on  2021-03-03
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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
#include "dpp.hpp"
#include "bsp.hpp"

#include "stm32l0xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

// namespace DPP *************************************************************
namespace DPP {

Q_DEFINE_THIS_FILE

// Local-scope defines -------------------------------------------------------
// LED pins available on the board (just one user LED LD2--Green on PA.5)
#define LED_LD2  (1U << 5)

// Button pins available on the board (just one user Button B1 on PC.13)
#define BTN_B1   (1U << 13)


static uint32_t l_rnd;  // random seed

#ifdef Q_SPY
    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    // QSpy source IDs
    static QP::QSpyId const l_tickHook = { 0U };

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        ON_CONTEXT_SW
    };

#endif

// ISRs used in the application ==============================================
// uCOS-II application hooks =================================================
extern "C" {

void App_TaskCreateHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskDelHook    (OS_TCB *ptcb) { (void)ptcb; }
//............................................................................
void App_TaskIdleHook(void) {
#if OS_CRITICAL_METHOD == 3u  // Allocate storage for CPU status register
    OS_CPU_SR cpu_sr;
#endif

    // toggle LED2 on and then off, see NOTE01
    OS_ENTER_CRITICAL();
    //GPIOA->BSRR |= (LED_LD2);        // turn LED[n] on
    //GPIOA->BSRR |= (LED_LD2 << 16);  // turn LED[n] off
    OS_EXIT_CRITICAL();

#ifdef Q_SPY
    if ((USART2->ISR & 0x0080U) != 0) {  // is TXE empty?
        uint16_t b;

        OS_ENTER_CRITICAL();
        b = QP::QS::getByte();
        OS_EXIT_CRITICAL();

        if (b != QP::QS_EOD) {  // not End-Of-Data?
            USART2->TDR  = (b & 0xFFU);  // put into the DR register
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    __WFI(); // Wait-For-Interrupt
#endif
}
//............................................................................
void App_TaskReturnHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskStatHook   (void)         {}
void App_TaskSwHook     (void)         {}
void App_TCBInitHook    (OS_TCB *ptcb) { (void)ptcb; }
//............................................................................
void App_TimeTickHook(void) {
    // state of the button debouncing, see below
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current;
    uint32_t tmp;

#ifdef Q_SPY
    {
        tmp = SysTick->CTRL; // clear CTRL_COUNTFLAG
        QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
    }
#endif

    QP::QTimeEvt::TICK_X(0U, &l_tickHook); // process time events for rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.

    current = ~GPIOC->IDR; // read Port C with the state of Button B1
    tmp = buttons.depressed; // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    if ((tmp & BTN_B1) != 0U) {  // debounced B1 state changed?
        if ((buttons.depressed & BTN_B1) != 0U) { // is B1 depressed?
            static QP::QEvt const pauseEvt = { PAUSE_SIG, 0U, 0U};
            QP::QF::PUBLISH(&pauseEvt, &l_tickHook);
        }
        else {            // the button is released
            static QP::QEvt const serveEvt = { SERVE_SIG, 0U, 0U};
            QP::QF::PUBLISH(&serveEvt, &l_tickHook);
        }
    }
}

} // extern "C"

// BSP functions ===========================================================*/
void BSP::init(void) {
    // NOTE: SystemInit() has been already called from the startup code
    //  but SystemCoreClock needs to be updated

    SystemCoreClockUpdate();

    // enable GPIOA clock port for the LED LD2
    RCC->IOPENR |= (1U << 0);

    // configure LED (PA.5) pin as push-pull output, no pull-up, pull-down
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

    randomSeed(1234U); // seed the random number generator

    // initialize the QS software tracing...
    if (QS_INIT((void *)0) == 0U) {
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_tickHook);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(ON_CONTEXT_SW);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS);
    QS_GLB_FILTER(QP::QS_UA_RECORDS);
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == 'h') {
        GPIOA->BSRR |= LED_LD2;  // turn LED on
    }
    else {
        GPIOA->BSRR |= (LED_LD2 << 16);  // turn LED off
    }

    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);                  // Philosopher number
        QS_STR(stat);                 // Philosopher status
    QS_END()
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    // not enough LEDs to implement this feature
    if (paused != 0U) {
        //GPIOA->BSRR |= (LED_LD2);  // turn LED[n] on
    }
    else {
        //GPIOA->BSRR |= (LED_LD2 << 16);  // turn LED[n] off
    }
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)

    l_rnd = l_rnd * (3U*7U*11U*13U*23U);
    return l_rnd >> 8;
}
//............................................................................
void BSP::randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
void BSP::terminate(int16_t result) {
    (void)result;
}

} // namespace DPP


// namespace QP **************************************************************
namespace QP {

// QF callbacks ============================================================*/
void QF::onStartup(void) {
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    // NOTE: do NOT call OS_CPU_SysTickInit() from uC/OS-II
    SysTick_Config(SystemCoreClock / DPP::BSP::TICKS_PER_SEC);

    // set priorities of ALL ISRs used in the system
    NVIC_SetPriority(SysTick_IRQn,  1U);
    // ...

    // enable IRQs in the NVIC...
    // ...
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
extern "C"
Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, 10000U); // report assertion to QS

#ifndef NDEBUG
    // wait until button B1 is pressed...
    while ((GPIOC->IDR  & BTN_B1) != 0U) {
        GPIOA->BSRR |= (LED_LD2);        // turn LED2 on
        GPIOA->BSRR |= (LED_LD2 << 16);  // turn LED2 off
    }
#endif

    NVIC_SystemReset();
}

// QS callbacks ============================================================*/
#ifdef Q_SPY
//............................................................................
#define __DIV(__PCLK, __BAUD)       (((__PCLK / 4) *25)/(__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   \
    (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) \
        * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) \
    ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[1024]; // buffer for Quantum Spy

    (void)arg; // avoid the "unused parameter" compiler warning
    initBuf(qsBuf, sizeof(qsBuf));

    // enable peripheral clock for USART2
    RCC->IOPENR  |= ( 1ul <<  0);   // Enable GPIOA clock
    RCC->APB1ENR |= ( 1ul << 17);   // Enable USART#2 clock

    // Configure PA3 to USART2_RX, PA2 to USART2_TX
    GPIOA->AFR[0] &= ~((15ul << 4* 3) | (15ul << 4* 2) );
    GPIOA->AFR[0] |=  (( 4ul << 4* 3) | ( 4ul << 4* 2) );
    GPIOA->MODER  &= ~(( 3ul << 2* 3) | ( 3ul << 2* 2) );
    GPIOA->MODER  |=  (( 2ul << 2* 3) | ( 2ul << 2* 2) );

    USART2->BRR  = __USART_BRR(SystemCoreClock, 115200ul);  // baud rate
    USART2->CR3  = 0x0000;         // no flow control
    USART2->CR2  = 0x0000;         // 1 stop bit
    USART2->CR1  = ((1ul <<  2) |  // enable RX
                    (1ul <<  3) |  // enable TX
                    (0ul << 12) |  // 8 data bits
                    (0ul << 28) |  // 8 data bits
                    (1ul <<  0) ); // enable USART

    DPP::QS_tickPeriod_ = SystemCoreClock / DPP::BSP::TICKS_PER_SEC;
    DPP::QS_tickTime_ = DPP::QS_tickPeriod_; // to start the timestamp at zero

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) { // not set?
        return DPP::QS_tickTime_ - static_cast<QSTimeCtr>(SysTick->VAL);
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return DPP::QS_tickTime_ + DPP::QS_tickPeriod_
               - static_cast<QSTimeCtr>(SysTick->VAL);
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
#if OS_CRITICAL_METHOD == 3u  // Allocate storage for CPU status register
    OS_CPU_SR cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    while ((b = getByte()) != QS_EOD) {    // while not End-Of-Data...
        OS_EXIT_CRITICAL();
        while ((USART2->ISR & 0x0080U) == 0U) { // while TXE not empty
        }
        USART2->TDR  = (b & 0xFFU);  // put into the DR register
        OS_ENTER_CRITICAL();
    }
    OS_EXIT_CRITICAL();
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
//! callback function to execute a user command (to be implemented in BSP)
void QS::onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

/*****************************************************************************
* NOTE01:
* Usually, one of the LEDs is used to visualize the idle loop activity.
* However, the board has not enough LEDs (only one, actually), so this
* feature is disabled.
*/
