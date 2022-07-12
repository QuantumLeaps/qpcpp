//============================================================================
// Product: DPP example, EK-TM4C123GXL board, uC/OS-II kernel
// Last updated for version 6.9.3
// Last updated on  2021-03-03
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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

#include "TM4C123GH6PM.h"        // the device specific header (TI)
#include "rom.h"                 // the built-in ROM functions (TI)
#include "sysctl.h"              // system control driver (TI)
#include "gpio.h"                // GPIO driver (TI)
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

// namespace DPP *************************************************************
namespace DPP {

// Local-scope objects -------------------------------------------------------
#define LED_RED     (1U << 1)
#define LED_GREEN   (1U << 3)
#define LED_BLUE    (1U << 2)

#define BTN_SW1     (1U << 4)
#define BTN_SW2     (1U << 0)

static uint32_t l_rnd; // random seed
OS_EVENT *l_rndMutex;  // to protect the random number generator

#ifdef Q_SPY

    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    // QSpy source IDs
    static QP::QSpyId const l_tickHook = { 0U };
    static QP::QSpyId const l_GPIOPortA_IRQHandler = { 0U };

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
void GPIOPortA_IRQHandler(void);
void GPIOPortA_IRQHandler(void) {
#if OS_CRITICAL_METHOD == 3u  // Allocate storage for CPU status register
    OS_CPU_SR cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    OSIntEnter();  // Tell uC/OS-II that we are starting an ISR
    OS_EXIT_CRITICAL();

    // perform the application work...
    AO_Table->POST(Q_NEW(QP::QEvt, MAX_SIG), // for testing...
                   &l_GPIOPortA_IRQHandler);

    OSIntExit();   // Tell uC/OS-II that we are leaving the ISR
}


// uCOS-II application hooks --===============================================
void App_TaskCreateHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskDelHook    (OS_TCB *ptcb) { (void)ptcb; }
//............................................................................
void App_TaskIdleHook(void) {
#if OS_CRITICAL_METHOD == 3u  // Allocate storage for CPU status register
    OS_CPU_SR cpu_sr;
#endif

    // toggle LED2 on and then off, see NOTE01
    OS_ENTER_CRITICAL();
    GPIOF->DATA_Bits[LED_BLUE] = 0xFFU; // turn the LED on
    GPIOF->DATA_Bits[LED_BLUE] = 0x00U; // turn the LED off
    OS_EXIT_CRITICAL();

#ifdef Q_SPY
    if ((UART0->FR & UART_FR_TXFE) != 0) { // TX done?
        uint16_t fifo = UART_TXFIFO_DEPTH; // max bytes we can accept
        uint8_t const *block;

        OS_EXIT_CRITICAL();
        block = QP::QS::getBlock(&fifo); // try to get next block to transmit
        OS_EXIT_CRITICAL();

        while (fifo-- != 0) {     // any bytes in the block?
            UART0->DR = *block++; // put into the FIFO
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
    uint32_t tmp;

#ifdef Q_SPY
    tmp = SysTick->CTRL; // clear SysTick_CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif

    QP::QTimeEvt::TICK_X(0U, &l_tickHook); // process time events for rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    //
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U }; // state of the button debouncing

    uint32_t current = ~GPIOF->DATA_Bits[BTN_SW1 | BTN_SW2]; // read SW1 & SW2
    tmp = buttons.depressed;     // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current);  // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    if ((tmp & BTN_SW1) != 0U) {  // debounced SW1 state changed?
        if ((buttons.depressed & BTN_SW1) != 0U) { // is SW1 depressed?
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

    // configure the FPU usage by choosing one of the options...
    // Do NOT to use the automatic FPU state preservation and
    // do NOT to use the FPU lazy stacking.
    //
    // NOTE:
    // Use the following setting when FPU is used in ONE task only and not
    // in any ISR. This option should be used with CAUTION.
    //
    FPU->FPCCR &= ~((1U << FPU_FPCCR_ASPEN_Pos) | (1U << FPU_FPCCR_LSPEN_Pos));

    // enable clock for to the peripherals used by this application...
    SYSCTL->RCGCGPIO |= (1U << 5); // enable Run mode for GPIOF

    // configure the LEDs and push buttons
    GPIOF->DIR |= (LED_RED | LED_GREEN | LED_BLUE); // set direction: output
    GPIOF->DEN |= (LED_RED | LED_GREEN | LED_BLUE); // digital enable
    GPIOF->DATA_Bits[LED_RED | LED_GREEN | LED_BLUE] = 0U;  // turn the LEDs off

    // configure the Buttons
    GPIOF->DIR &= ~(BTN_SW1 | BTN_SW2); //  set direction: input
    ROM_GPIOPadConfigSet(GPIOF_BASE, (BTN_SW1 | BTN_SW2),
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    BSP::randomSeed(1234U);

    if (!QS_INIT(nullptr)) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_tickHook);
    QS_OBJ_DICTIONARY(&l_GPIOPortA_IRQHandler);
    QS_USR_DICTIONARY(PHILO_STAT);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS);
    QS_GLB_FILTER(QP::QS_UA_RECORDS);
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    // exercise the FPU with some floating point computations
    // NOTE: this code can be only called from a task that created with
    // the option OS_TASK_OPT_SAVE_FP.
    //
    float volatile x;
    x = 3.1415926F;
    x = x + 2.7182818F;

    GPIOF->DATA_Bits[LED_GREEN] =
         ((stat[0] == 'e')   // Is Philo[n] eating?
         ? 0xFFU             // turn the LED1 on
         : 0U);              // turn the LED1 off

    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    GPIOF->DATA_Bits[LED_RED] = ((paused != 0U) ? 0xFFU : 0U);
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
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    // NOTE: do NOT call OS_CPU_SysTickInit() from uC/OS-II
    //
    SysTick_Config(SystemCoreClock / DPP::BSP::TICKS_PER_SEC);

    // set priorities of ALL ISRs used in the system, see NOTE1
    NVIC_SetPriority(SysTick_IRQn,  CPU_CFG_KA_IPL_BOUNDARY + 1U);
    NVIC_SetPriority(GPIOA_IRQn,    CPU_CFG_KA_IPL_BOUNDARY);
    // ...

    // enable IRQs in the NVIC...
    NVIC_EnableIRQ(GPIOA_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}

//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, 10000U); // report assertion to QS

#ifndef NDEBUG
    // light all both LEDs
    GPIOF->DATA_Bits[LED_RED | LED_GREEN | LED_BLUE] = 0xFFU;
    // for debugging, hang on in an endless loop until SW1 is pressed...
    while (GPIOF->DATA_Bits[BTN_SW1] != 0) {
    }
#endif

    NVIC_SystemReset();
}

// QS callbacks ==============================================================
#ifdef Q_SPY
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[2*1024]; // buffer for Quantum Spy
    uint32_t tmp;
    initBuf(qsBuf, sizeof(qsBuf));

    // enable clock for UART0 and GPIOA (used by UART0 pins)
    SYSCTL->RCGCUART |= (1U << 0); // enable Run mode for UART0
    SYSCTL->RCGCGPIO |= (1U << 0); // enable Run mode for GPIOA

    // configure UART0 pins for UART operation
    tmp = (1U << 0) | (1U << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->AFSEL |= tmp;
    GPIOA->DR2R  |= tmp;   // set 2mA drive, DR4R and DR8R are cleared
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->DEN   |= tmp;

    // configure the UART for the desired baud rate, 8-N-1 operation
    tmp = (((SystemCoreClock * 8U) / UART_BAUD_RATE) + 1U) / 2U;
    UART0->IBRD   = tmp / 64U;
    UART0->FBRD   = tmp % 64U;
    UART0->LCRH   = 0x60U; // configure 8-N-1 operation
    UART0->LCRH  |= 0x10U;
    UART0->CTL   |= (1U << 0) | (1U << 8) | (1U << 9);

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
    uint16_t fifo = UART_TXFIFO_DEPTH; // Tx FIFO depth
    uint8_t const *block;
#if OS_CRITICAL_METHOD == 3u  // Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL();
    while ((block = getBlock(&fifo)) != nullptr) {
        OS_EXIT_CRITICAL();
        // busy-wait until TX FIFO empty
        while ((UART0->FR & UART_FR_TXFE) == 0U) {
        }

        while (fifo-- != 0U) {    // any bytes in the block?
            UART0->DR = *block++; // put into the TX FIFO
        }
        fifo = UART_TXFIFO_DEPTH; // re-load the Tx FIFO depth
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
void QS::onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
    //TBD
}
#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//============================================================================
// NOTE1:
// All ISRs that make system calls MUST be prioritized as "kernel-aware".
// On Cortex-M3/4/7 this means ISR priorities with numerical values higher
// or equal CPU_CFG_KA_IPL_BOUNDARY.
//
