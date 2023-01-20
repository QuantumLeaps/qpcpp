//============================================================================
// Product: DPP example, NUCLEO-L053R8 board, uC-OS2 RTOS kernel
// Last updated for @ref qpc_7_3_0
// Last updated on  2023-08-31
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"             // QP/C++ real-time embedded framework
#include "dpp.hpp"               // DPP Application interface
#include "bsp.hpp"               // Board Support Package

#include "stm32l0xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

// LEDs on the board
constexpr std::uint32_t LD2_PIN     {5U};

// Buttons on the board
constexpr std::uint32_t B1_PIN      {13U};

static std::uint32_t l_rndSeed;

#ifdef Q_SPY

    // QSpy source IDs
    static QP::QSpyId const l_tickHook = { 0U };

    static QP::QSTimeCtr QS_tickTime_;
    static QP::QSTimeCtr QS_tickPeriod_;

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        PAUSED_STAT,
    };

#endif

} // unnamed namespace

//============================================================================
// Error handler
extern "C" {

Q_NORETURN Q_onError(char const * const module, int const id) {
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U); // report assertion to QS

#ifndef NDEBUG
    // light up the user LED
    GPIOA->BSRR = (1U << LD2_PIN);
    for (;;) { // for debugging, hang on in an endless loop...
    }
#else
    NVIC_SystemReset();
    for (;;) { // explicitly "no-return"
    }
#endif
}
//............................................................................
void assert_failed(char const * const module, int const id); // prototype
void assert_failed(char const * const module, int const id) {
    Q_onError(module, id);
}

// uCOS-II application hooks ===============================================

void App_TimeTickHook(void) {

    QP::QTimeEvt::TICK_X(0U, &l_tickHook); // time events at rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    static struct {
        std::uint32_t depressed;
        std::uint32_t previous;
    } buttons = { 0U, 0U };

    std::uint32_t current = ~GPIOC->IDR; // read Port C with Button B1
    std::uint32_t tmp = buttons.depressed; // save debounced depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    current = buttons.depressed;

    if ((tmp & (1U << B1_PIN)) != 0U) { // debounced B1 state changed?
        if ((current & (1U << B1_PIN)) != 0U) { // is B1 depressed?
            static QP::QEvt const pauseEvt(APP::PAUSE_SIG);
            QP::QActive::PUBLISH(&pauseEvt, &l_tickHook);
        }
        else { // the button is released
            static QP::QEvt const serveEvt(APP::SERVE_SIG);
            QP::QActive::PUBLISH(&serveEvt, &l_tickHook);
        }
    }

#ifdef Q_SPY
    tmp = SysTick->CTRL; // clear CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif

}
//............................................................................
void App_TaskCreateHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskDelHook    (OS_TCB *ptcb) { (void)ptcb; }
//............................................................................
void App_TaskIdleHook(void) {
    // toggle the User LED on and then off, see NOTE3
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // not enough LEDs for this feature
    QF_CRIT_EXIT();

#ifdef Q_SPY
    QF_CRIT_ENTRY();
    QP::QS::rxParse();  // parse all the received bytes
    QF_CRIT_EXIT();

    if ((USART2->ISR & 0x0080U) != 0U) { // is TXE empty?

        QF_CRIT_ENTRY();
        std::uint16_t b = QP::QS::getByte();
        QF_CRIT_EXIT();

        if (b != QS_EOD) { // not End-Of-Data?
            USART2->TDR = b;
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    __WFI(); // Wait-For-Interrupt
#endif
}
//............................................................................
void App_TaskReturnHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskStatHook   (void)         {}
void App_TaskSwHook     (void)         {}
void App_TCBInitHook    (OS_TCB *ptcb) { (void)ptcb; }

// ISRs used in the application ============================================

#ifdef Q_SPY
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "kernel-unaware" meaning that it does not interact with
// the QF/RTOS and is not disabled. Such ISRs cannot post or publish events.

void USART2_IRQHandler(void); // prototype
void USART2_IRQHandler(void) { // used in QS-RX (kernel UNAWARE interrutp)
    // is RX register NOT empty?
    if ((USART2->ISR & (1U << 5U)) != 0U) {
        uint32_t b = USART2->RDR;
        QP::QS::rxPut(b);
    }
}
#endif // Q_SPY

} // extern "C"

//============================================================================

namespace BSP {

void init() {
    // Configure the MPU to prevent NULL-pointer dereferencing ...
    MPU->RBAR = 0x0U                          // base address (NULL)
                | MPU_RBAR_VALID_Msk          // valid region
                | (MPU_RBAR_REGION_Msk & 7U); // region #7
    MPU->RASR = (7U << MPU_RASR_SIZE_Pos)     // 2^(7+1) region
                | (0x0U << MPU_RASR_AP_Pos)   // no-access region
                | MPU_RASR_ENABLE_Msk;        // region enable
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk       // enable background region
                | MPU_CTRL_ENABLE_Msk;        // enable the MPU
    __ISB();
    __DSB();

    // NOTE: SystemInit() has been already called from the startup code
    // but SystemCoreClock needs to be updated
    SystemCoreClockUpdate();

    // enable GPIOA clock port for the LED LD2
    RCC->IOPENR |= (1U << 0U);

    // configure LED (PA.5) pin as push-pull output, no pull-up, pull-down
    GPIOA->MODER   &= ~((3U << 2U*LD2_PIN));
    GPIOA->MODER   |=  ((1U << 2U*LD2_PIN));
    GPIOA->OTYPER  &= ~((1U <<    LD2_PIN));
    GPIOA->OSPEEDR &= ~((3U << 2U*LD2_PIN));
    GPIOA->OSPEEDR |=  ((1U << 2U*LD2_PIN));
    GPIOA->PUPDR   &= ~((3U << 2U*LD2_PIN));

    // enable GPIOC clock port for the Button B1
    RCC->IOPENR |=  (1U << 2U);

    // configure Button (PC.13) pins as input, no pull-up, pull-down
    GPIOC->MODER   &= ~(3U << 2U*B1_PIN);
    GPIOC->OSPEEDR &= ~(3U << 2U*B1_PIN);
    GPIOC->OSPEEDR |=  (1U << 2U*B1_PIN);
    GPIOC->PUPDR   &= ~(3U << 2U*B1_PIN);

    // seed the random number generator
    BSP::randomSeed(1234U);

    // initialize the QS software tracing...
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }

    // dictionaries...
    QS_OBJ_DICTIONARY(&l_tickHook);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);

    QS_ONLY(APP::produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);  // all records
    QS_GLB_FILTER(-QP::QS_QF_TICK);     // exclude the clock tick
}
//............................................................................
void start() {
    // initialize event pools
    static QF_MPOOL_EL(APP::TableEvt) smlPoolSto[2*APP::N_PHILO];
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // start AOs/threads...
    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10];
    static OS_STK philoStack[APP::N_PHILO][72]; // stacks for the Philos
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
        APP::AO_Philo[n]->start(
            n + 1U,                  // QP prio. of the AO
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            philoStack[n],           // stack storage
            sizeof(philoStack[n]));  // stack size [bytes]
    }

    static QP::QEvt const *tableQueueSto[APP::N_PHILO];
    static OS_STK tableStack[80]; // stack for the Table
    APP::AO_Table->start(
        APP::N_PHILO + 1U,       // QP prio. of the AO
        tableQueueSto,           // event queue storage
        Q_DIM(tableQueueSto),    // queue length [events]
        tableStack,              // stack storage
        sizeof(tableStack));     // stack size [bytes]
}
//............................................................................
void displayPhilStat(std::uint8_t n, char const *stat) {
    Q_UNUSED_PAR(n);

    if (stat[0] == 'e') {
        GPIOA->BSRR = (1U << LD2_PIN);  // turn LED on
    }
    else {
        GPIOA->BSRR = (1U << (LD2_PIN + 16U));  // turn LED off
    }

    // app-specific trace record...
    QS_BEGIN_ID(PHILO_STAT, APP::AO_Table->getPrio())
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void displayPaused(std::uint8_t const paused) {
    // not enough LEDs to implement this feature
    if (paused != 0U) {
        //GPIOA->BSRR = (1U << LD2_PIN); // turn LED[n] on
    }
    else {
        //GPIOA->BSRR = (1U << (LD2_PIN + 16U)); // turn LED[n] off
    }

    // application-specific trace record
    QS_BEGIN_ID(PAUSED_STAT, APP::AO_Table->getPrio())
        QS_U8(1, paused);  // Paused status
    QS_END()
}
//............................................................................
void randomSeed(std::uint32_t const seed) {
    l_rndSeed = seed;
}
//............................................................................
std::uint32_t random() { // a very cheap pseudo-random-number generator

    OSSchedLock(); // lock uC-OS2 scheduler
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    std::uint32_t rnd = l_rndSeed * (3U*7U*11U*13U*23U);
    l_rndSeed = rnd; // set for the next time
    OSSchedUnlock(); // unlock uC-OS2 scheduler

    return (rnd >> 8U);
}
//............................................................................
void ledOn() {
    GPIOA->BSRR = (1U << LD2_PIN); // turn LED2 on
}
//............................................................................
void ledOff() {
    GPIOA->BSRR = (1U << (LD2_PIN + 16U)); // turn LED2 off
}
//............................................................................
void terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}

} // namespace BSP

//============================================================================

namespace QP {

// QF callbacks --------------------------------------------------------------

void QF::onStartup() {
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    // NOTE: do NOT call OS_CPU_SysTickInit() from uC/OS-II
    SysTick_Config(SystemCoreClock / BSP::TICKS_PER_SEC);

    // set priorities of ALL ISRs used in the system
    NVIC_SetPriority(USART2_IRQn,    0U); // kernel UNAWARE interrupt
    NVIC_SetPriority(SysTick_IRQn,   1U);
    // ...

    // enable IRQs in the NVIC...
    // ...

#ifdef Q_SPY
    NVIC_EnableIRQ(USART2_IRQn); // UART2 interrupt used for QS-RX
#endif
}
//............................................................................
void QF::onCleanup() {
}

// QS callbacks --------------------------------------------------------------
#ifdef Q_SPY

#define __DIV(__PCLK, __BAUD)       (((__PCLK / 4) *25)/(__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   \
    (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) \
        * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) \
    ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

// USART2 pins PA.2 and PA.3
#define USART2_TX_PIN 2U
#define USART2_RX_PIN 3U

namespace QS {

//............................................................................
bool onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static std::uint8_t qsTxBuf[800]; // buffer for QS-TX channel
    initBuf(qsTxBuf, sizeof(qsTxBuf));

    static std::uint8_t qsRxBuf[100];    // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // enable peripheral clock for USART2
    RCC->IOPENR  |= ( 1U <<  0U);  // Enable GPIOA clock for USART pins
    RCC->APB1ENR |= ( 1U << 17U);  // Enable USART#2 clock

    // Configure PA3 to USART2_RX, PA2 to USART2_TX
    GPIOA->AFR[0] &= ~((15U << 4U*USART2_RX_PIN) | (15U << 4U*USART2_TX_PIN));
    GPIOA->AFR[0] |=  (( 4U << 4U*USART2_RX_PIN) | ( 4U << 4U*USART2_TX_PIN));
    GPIOA->MODER  &= ~(( 3U << 2U*USART2_RX_PIN) | ( 3U << 2U*USART2_TX_PIN));
    GPIOA->MODER  |=  (( 2U << 2U*USART2_RX_PIN) | ( 2U << 2U*USART2_TX_PIN));

    USART2->BRR  = __USART_BRR(SystemCoreClock, 115200U);  // baud rate
    USART2->CR3  = 0x0000U |      // no flow control
                   (1U << 12U);   // disable overrun detection (OVRDIS)
    USART2->CR2  = 0x0000U;       // 1 stop bit
    USART2->CR1  = ((1U <<  2U) | // enable RX
                    (1U <<  3U) | // enable TX
                    (1U <<  5U) | // enable RX interrupt
                    (0U << 12U) | // 8 data bits
                    (0U << 28U) | // 8 data bits
                    (1U <<  0U)); // enable USART

    QS_tickPeriod_ = SystemCoreClock / BSP::TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_; // to start the timestamp at zero

    return true; // return success
}
//............................................................................
void onCleanup() {
}
//............................................................................
QSTimeCtr onGetTime() { // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) { // not set?
        return QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr)SysTick->VAL;
    }
}
//............................................................................
void onFlush() {
    for (;;) {
        QF_CRIT_STAT
        QF_CRIT_ENTRY();
        std::uint16_t b = getByte();
        QF_CRIT_EXIT();

        if (b != QS_EOD) { // NOT end-of-data
            // busy-wait as long as TX has data to transmit
            while ((USART2->ISR & (1U << 7U)) == 0U) {
            }
            USART2->TDR = b; // put into the TDR register
        }
        else {
            break; // break out of the loop
        }
    }
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void onReset() {
    NVIC_SystemReset();
}
//............................................................................
void onCommand(std::uint8_t cmdId, std::uint32_t param1,
               std::uint32_t param2, std::uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

} // namespace QS
#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//============================================================================
// NOTE1:
// All ISRs that make system calls MUST be prioritized as "kernel-aware".
// On Cortex-M3/4/7 this means ISR priorities with numerical values higher
// or equal CPU_CFG_KA_IPL_BOUNDARY.

