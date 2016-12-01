//****************************************************************************
// Product: DPP example, STM32 NUCLEO-L152RE board, uC/OS-II kernel
// Last updated for version 5.8.0
// Last updated on  2016-11-30
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
// http://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

#include "stm32l1xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

// namespace DPP *************************************************************
namespace DPP {

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
// LED pins available on the board (just one LED)
#define LED_LD2    (1U << 5)

// Button pins available on the board (just one Button)
#define BTN_B1     (1U << 13)

static uint32_t l_rnd; // random seed
OS_EVENT *l_rndMutex;  // to protect the random number generator

#ifdef Q_SPY

    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    // source IDs for QS for non-QP event producers
    static uint8_t const l_tickHook = 0U;
    static uint8_t const l_EXTI0_IRQHandler = 0U;

    #define UART_BAUD_RATE      115200U
    #define UART_FR_TXFE        0x80U
    #define UART_TXFIFO_DEPTH   16U

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER
    };

#endif

// ISRs used in this project =================================================
extern "C" {

// example ISR handler for uCOS-II
void EXTI0_IRQHandler(void);
void EXTI0_IRQHandler(void) {
#if OS_CRITICAL_METHOD == 3u  // Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL();
    OSIntEnter();  // Tell uC/OS-II that we are starting an ISR
    OS_EXIT_CRITICAL();

    // perform the application work...
    AO_Table->POST(Q_NEW(QP::QEvt, MAX_SIG), // for testing...
                   &l_EXTI0_IRQHandler);

    OSIntExit();   // Tell uC/OS-II that we are leaving the ISR
}

// uCOS-II application hooks --===============================================
void App_TaskCreateHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskDelHook    (OS_TCB *ptcb) { (void)ptcb; }
//............................................................................
void App_TaskIdleHook(void) {
#if OS_CRITICAL_METHOD == 3u  // Allocate storage for CPU status register
    OS_CPU_SR cpu_sr = 0u;
#endif

    // toggle LED2 on and then off, see NOTE01, not enough LEDs to implement
    OS_ENTER_CRITICAL();
    //GPIOA->BSRRL |= LED_LD2;  // turn LED on
    //GPIOA->BSRRH |= LED_LD2;  // turn LED off
    OS_EXIT_CRITICAL();

#ifdef Q_SPY
    if ((USART2->SR & 0x0080U) != 0) {  // is TXE empty?
        OS_ENTER_CRITICAL();
        uint16_t b = QP::QS::getByte();
        OS_EXIT_CRITICAL();

        if (b != QP::QS_EOD) {  // not End-Of-Data?
            USART2->DR  = (b & 0xFFU);  // put into the DR register
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    // !!!CAUTION!!!
    // The WFI instruction stops the CPU clock, which unfortunately disables
    // the JTAG port, so the ST-Link debugger can no longer connect to the
    // board. For that reason, the call to __WFI() has to be used with caution.
    //
    // NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    // reset the board, then connect with ST-Link Utilities and erase the part.
    // The trick with BOOT(0) is it gets the part to run the System Loader instead
    // of your broken code. When done disconnect BOOT0, and start over.
    //
    //__WFI(); // Wait-For-Interrupt
#endif
}
//............................................................................
void App_TaskReturnHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskStatHook   (void)         {}
void App_TaskSwHook     (void)         {}
void App_TCBInitHook    (OS_TCB *ptcb) { (void)ptcb; }
//............................................................................
void App_TimeTickHook(void) {
    uint32_t tmp;

#ifdef Q_SPY
    tmp = SysTick->CTRL; // clear SysTick_CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif

    QP::QF::TICK_X(0U, &l_tickHook); // process time events for rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    //
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { ~0U, ~0U }; // state of the button debouncing

    uint32_t current = ~GPIOC->IDR;  // read button B1
    tmp = buttons.depressed;     // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current);  // set depressed
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

// BSP functions =============================================================
void BSP::init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // enable GPIOA clock for the LED
    RCC->AHBENR |= (1U << 0);

    // configure LED (PA.5) pin as push-pull outputs, No pull-up, pull-down
    GPIOA->MODER   &= ~((3U << 2*5));
    GPIOA->MODER   |=  ((1U << 2*5));
    GPIOA->OTYPER  &= ~((1U <<   5));
    GPIOA->OSPEEDR &= ~((3U << 2*5));
    GPIOA->OSPEEDR |=  ((1U << 2*5));
    GPIOA->PUPDR   &= ~((3U << 2*5));

    // enable GPIOC clock for the Button
    RCC->AHBENR |=  (1ul << 2);

    // configure BTN (PC.13) pin as push-pull outputs, No pull-up, pull-down
    GPIOC->MODER   &= ~(3ul << 2*13);
    GPIOC->OSPEEDR &= ~(3ul << 2*13);
    GPIOC->OSPEEDR |=  (1ul << 2*13);
    GPIOC->PUPDR   &= ~(3ul << 2*13);

    BSP::randomSeed(1234U);

    if (!QS_INIT((void *)0)) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_tickHook);
    QS_OBJ_DICTIONARY(&l_EXTI0_IRQHandler);
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == 'e') {
        GPIOA->BSRRL |= LED_LD2;  // turn LED on
    }
    else {
        GPIOA->BSRRH |= LED_LD2;  // turn LED off
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    // not enough LEDs to show the "Paused" status
    if (paused != (uint8_t)0) {
        //GPIOA->BSRRL |= LED_LD2;  // turn LED on
    }
    else {
        //GPIOA->BSRRH |= LED_LD2;  // turn LED off
    }
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    INT8U err;

    OSMutexPend(l_rndMutex, 0, &err); // lock the random-seed mutex
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time
    OSMutexPost(l_rndMutex);           // unlock the random-seed mutex

    return (rnd >> 8);
}
//............................................................................
void BSP::randomSeed(uint32_t seed) {
      INT8U err;

    l_rnd = seed;
      l_rndMutex = OSMutexCreate(N_PHILO, &err);
}
//............................................................................
void BSP::terminate(int16_t result) {
    (void)result;
}

} // namespace DPP


// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // initialize the system clock tick...
    OS_CPU_SysTickInit(SystemCoreClock / OS_TICKS_PER_SEC);

    // set priorities of the ISRs used in the system
    NVIC_SetPriority(EXTI0_IRQn,   0xFFU);
    // ...

    // enable IRQs in the NVIC...
    NVIC_EnableIRQ(EXTI0_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}

//............................................................................
extern "C" void Q_onAssert(char const *module, int loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
    NVIC_SystemReset();
}

// QS callbacks ==============================================================
#ifdef Q_SPY

#define __DIV(__PCLK, __BAUD)       (((__PCLK / 4) *25)/(__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   \
    (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) \
        * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) \
    ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[2*1024]; // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    // enable peripheral clock for USART2
    RCC->AHBENR   |=  (1U <<  0);      // Enable GPIOA clock
    RCC->APB1ENR  |=  (1U << 17);      // Enable USART#2 clock

    // Configure PA3 to USART2_RX, PA2 to USART2_TX
    GPIOA->AFR[0] &= ~((15U << 4*3) | (15U << 4*2));
    GPIOA->AFR[0] |=  (( 7U << 4*3) | ( 7U << 4*2));
    GPIOA->MODER  &= ~(( 3U << 2*3) | ( 3U << 2*2));
    GPIOA->MODER  |=  (( 2U << 2*3) | ( 2U << 2*2));

    USART2->BRR  = __USART_BRR(SystemCoreClock, 115200U);  // baud rate
    USART2->CR3  = 0x0000U;        // no flow control
    USART2->CR2  = 0x0000U;        // 1 stop bit
    USART2->CR1  = ((1U <<  2) |   // enable RX
                    (1U <<  3) |   // enable TX
                    (0U << 12) |   // 1 start bit, 8 data bits
                    (1U << 13));   // enable USART

    DPP::QS_tickPeriod_ = SystemCoreClock / DPP::BSP::TICKS_PER_SEC;
    DPP::QS_tickTime_ = DPP::QS_tickPeriod_; // to start the timestamp at zero

    // setup the QS filters...
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

    QS_FILTER_ON(DPP::PHILO_STAT);

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
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL();
    while ((b = getByte()) != QS_EOD) {    // while not End-Of-Data...
        OS_EXIT_CRITICAL();
        while ((USART2->SR & 0x0080U) == 0U) { // while TXE not empty
        }
        USART2->DR  = (b & 0xFFU);  // put into the DR register
        OS_ENTER_CRITICAL();
    }
    OS_EXIT_CRITICAL();
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    //TBD
}
//............................................................................
//! callback function to execute a uesr command (to be implemented in BSP)
void QS::onCommand(uint8_t cmdId, uint32_t param) {
    (void)cmdId;
    (void)param;
    //TBD
}
#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//****************************************************************************
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
