//============================================================================
// Product: DPP example, NXP mbed-LPC1768  board, QK kernel
// Last updated for version 7.3.2
// Last updated on  2023-12-13
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

#include "LPC17xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
constexpr std::uint32_t LED_1    {1U << 18U};  // P1.18
constexpr std::uint32_t LED_2    {1U << 20U};  // P1.20
constexpr std::uint32_t LED_3    {1U << 21U};  // P1.21
constexpr std::uint32_t LED_4    {1U << 23U};  // P1.23

// Push-Button wired externally to DIP8 (P0.6)
constexpr std::uint32_t BTN_EXT  {1U << 6U};    // P0.6

static std::uint32_t l_rndSeed;

#ifdef Q_SPY

    // QSpy source IDs
    static QP::QSpyId const l_SysTick_Handler = { 0U };
    static QP::QSpyId const l_EINT0_IRQHandler = { 0U };

    static QP::QSTimeCtr QS_tickTime_;
    static QP::QSTimeCtr QS_tickPeriod_;

    constexpr std::uint32_t UART_BAUD_RATE      {115200U};
    constexpr std::uint32_t UART_FR_TXFE        {0x80U};
    constexpr std::uint32_t UART_TXFIFO_DEPTH   {16U};


    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        PAUSED_STAT,
        CONTEXT_SW,
    };

#endif

} // unnamed namespace

//============================================================================
// Error handler and ISRs...
extern "C" {

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U);

#ifndef NDEBUG
    // light up all LEDs
    LPC_GPIO1->FIOSET = LED_1;  // turn LED on
    LPC_GPIO1->FIOSET = LED_2;  // turn LED on
    LPC_GPIO1->FIOSET = LED_3;  // turn LED on
    LPC_GPIO1->FIOSET = LED_4;  // turn LED on
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#else
    NVIC_SystemReset();
    for (;;) { // explicitly "no-return"
    }
#endif
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// ISRs used in the application ==============================================

void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    QK_ISR_ENTRY();   // inform QK about entering an ISR

    QP::QTimeEvt::TICK_X(0U, &l_SysTick_Handler); // time events at rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    static struct {
        std::uint32_t depressed;
        std::uint32_t previous;
    } buttons = { 0U, 0U };

    std::uint32_t current = ~LPC_GPIO0->FIOPIN; // read P0 with the buttons
    std::uint32_t tmp = buttons.depressed; // save the depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    current = buttons.depressed;

    if ((tmp & BTN_EXT) != 0U) {  // debounced BTN_EXT state changed?
        if ((current & BTN_EXT) != 0U) { // is BTN_EXT depressed?
            static QP::QEvt const pauseEvt(APP::PAUSE_SIG);
            QP::QActive::PUBLISH(&pauseEvt, &l_SysTick_Handler);
        }
        else { // the button is released
            static QP::QEvt const serveEvt(APP::SERVE_SIG);
            QP::QActive::PUBLISH(&serveEvt, &l_SysTick_Handler);
        }
    }

#ifdef Q_SPY
    tmp = SysTick->CTRL; // clear CTRL_COUNTFLAG
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif // Q_SPY

    QK_ISR_EXIT();  // inform QK about exiting an ISR
}
//............................................................................
// interrupt handler for testing preemptions
void EINT0_IRQHandler(void); // prototype
void EINT0_IRQHandler(void) {
    QK_ISR_ENTRY(); // inform QK about entering an ISR

    static QP::QEvt const testEvt(APP::TEST_SIG);
    APP::AO_Table->POST(&testEvt, &l_EINT0_IRQHandler);

    QK_ISR_EXIT();  // inform QK about exiting an ISR
}

//............................................................................
#ifdef Q_SPY
// ISR for receiving bytes from the QSPY Back-End
// NOTE: This ISR is "QF-unaware" meaning that it does not interact with
// the QF/QK and is not disabled. Such ISRs don't need to call
// QK_ISR_ENTRY/QK_ISR_EXIT and they cannot post or publish events.

// TBD: UART ISR for QS-RX channel...

#endif // Q_SPY

//............................................................................
#ifdef QF_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QF_onContextSw(QP::QActive *prev, QP::QActive *next) {
    Q_UNUSED_PAR(prev);
    Q_UNUSED_PAR(next);

    QS_BEGIN_INCRIT(CONTEXT_SW, 0U) // in critical section!
        QS_OBJ(prev);
        QS_OBJ(next);
    QS_END_INCRIT()
}
#endif // QF_ON_CONTEXT_SW

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

    // enable the MemManage_Handler for MPU exception
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

    // NOTE: SystemInit() has been already called from the startup code
    // but SystemCoreClock needs to be updated
    SystemCoreClockUpdate();

    // turn the GPIO clock on
    LPC_SC->PCONP |= (1U << 15);

    // setup the GPIO pin functions for the LEDs...
    LPC_PINCON->PINSEL3 &= ~(3U <<  4); // LED_1: function P1.18 to GPIO
    LPC_PINCON->PINSEL3 &= ~(3U <<  8); // LED_2: function P1.20 to GPIO
    LPC_PINCON->PINSEL3 &= ~(3U << 10); // LED_3: function P1.21 to GPIO
    LPC_PINCON->PINSEL3 &= ~(3U << 14); // LED_4: function P1.23 to GPIO

    // Set GPIO-P1 LED pins to output
    LPC_GPIO1->FIODIR |= (LED_1 | LED_2 | LED_3 | LED_4);


    // setup the GPIO pin function for the Button...
    LPC_PINCON->PINSEL0 &= ~(3U << 12); // function P0.6 to GPIO, pull-up

    // Set GPIO-P0 Button pin as input
    LPC_GPIO0->FIODIR &= ~BTN_EXT;

    BSP::randomSeed(1234U); // seed the random number generator

    // initialize the QS software tracing...
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }

    // dictionaries...
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_EINT0_IRQHandler);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);
    QS_USR_DICTIONARY(CONTEXT_SW);

    QS_ONLY(APP::produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);   // all records
    QS_GLB_FILTER(-QP::QS_QF_TICK);      // exclude the clock tick
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
    static QP::QEvt const *philoQueueSto[APP::N_PHILO][APP::N_PHILO];
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
        APP::AO_Philo[n]->start(

            // NOTE: set the preemption-threshold of all Philos to
            // the same level, so that they cannot preempt each other.
            Q_PRIO(n + 3U, APP::N_PHILO + 2U), // QF-prio/pre-thre.

            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            nullptr, 0U);            // no stack storage
    }

    static QP::QEvt const *tableQueueSto[APP::N_PHILO];
    APP::AO_Table->start(
        APP::N_PHILO + 7U,           // QP prio. of the AO
        tableQueueSto,               // event queue storage
        Q_DIM(tableQueueSto),        // queue length [events]
        nullptr, 0U);                // no stack storage
}
//............................................................................
void displayPhilStat(std::uint8_t n, char const *stat) {
    Q_UNUSED_PAR(n);

    if (stat[0] == 'h') {
        LPC_GPIO1->FIOSET = LED_1;  // turn LED on
    }
    else {
        LPC_GPIO1->FIOCLR = LED_1;  // turn LED off
    }
    if (stat[0] == 'e') {
        LPC_GPIO1->FIOSET = LED_2;  // turn LED on
    }
    else {
        LPC_GPIO1->FIOCLR = LED_2;  // turn LED off
    }

    // app-specific trace record...
    QS_BEGIN_ID(PHILO_STAT, APP::AO_Table->getPrio())
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void displayPaused(std::uint8_t const paused) {
    if (paused != 0U) {
        LPC_GPIO1->FIOSET = LED_3;  // turn LED on
    }
    else {
        LPC_GPIO1->FIOCLR = LED_3;  // turn LED off
    }

    // application-specific trace record
    QS_BEGIN_ID(PAUSED_STAT, APP::AO_Table->getPrio())
        QS_U8(1, paused);  // Paused status
    QS_END()
}
//............................................................................
void randomSeed(uint32_t const seed) {
    l_rndSeed = seed;
}
//............................................................................
std::uint32_t random() { // a very cheap pseudo-random-number generator
    // Some floating point code is to exercise the VFP...
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;

    QP::QSchedStatus lockStat = QP::QK::schedLock(APP::N_PHILO);
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    std::uint32_t rnd = l_rndSeed * (3U*7U*11U*13U*23U);
    l_rndSeed = rnd; // set for the next time
    QP::QK::schedUnlock(lockStat);

    return (rnd >> 8U);
}
//............................................................................
void ledOn() {
    LPC_GPIO1->FIOSET = LED_3;  // turn LED on
}
//............................................................................
void ledOff() {
    LPC_GPIO1->FIOCLR = LED_3;  // turn LED off
}
//............................................................................
void terminate(int16_t result) {
    LPC_GPIO1->FIOCLR = LED_3;  // turn LED off
}

} // namespace BSP

//============================================================================

//============================================================================
namespace QP {

// QF callbacks --------------------------------------------------------------

void QF::onStartup() {
    // set up the SysTick timer to fire at BSP::TICKS_PER_SEC rate
    SysTick_Config(SystemCoreClock / BSP::TICKS_PER_SEC);

    // assign all priority bits for preemption-prio. and none to sub-prio.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE1
    NVIC_SetPriority(EINT0_IRQn,     QF_AWARE_ISR_CMSIS_PRI + 0U);
    NVIC_SetPriority(SysTick_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 1U);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(EINT0_IRQn);
}
//............................................................................
void QF::onCleanup() {
}
//............................................................................
void QK::onIdle() {
    // toggle the User LED on and then off, see NOTE3
    QF_INT_DISABLE();
    LPC_GPIO1->FIOSET = LED_4;  // turn LED on
    __NOP();   // a couple of NOPs to actually see the LED glow
    __NOP();
    __NOP();
    __NOP();
    LPC_GPIO1->FIOCLR = LED_4;  // turn LED off
    QF_INT_ENABLE();

#ifdef Q_SPY
    QF_INT_DISABLE();
    QS::rxParse();  // parse all the received bytes
    QF_INT_ENABLE();

    if ((LPC_UART0->LSR & 0x20U) != 0U) {  // TX Holding Register empty?
        std::uint16_t fifo = UART_TXFIFO_DEPTH; // max bytes we can accept

        QF_INT_DISABLE();
        // try to get next contiguous block to transmit
        uint8_t const *block = QS::getBlock(&fifo);
        QF_INT_ENABLE();

        while (fifo-- != 0U) { // any bytes in the block?
            LPC_UART0->THR = *block++; // put into the FIFO
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    //
    __WFI(); // Wait-For-Interrupt
#endif
}

//============================================================================
// QS callbacks...
#ifdef Q_SPY

namespace QS {

static void UART0_setBaudrate(std::uint32_t baud);  // helper function

//............................................................................
bool onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static std::uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    initBuf(qsTxBuf, sizeof(qsTxBuf));

    static std::uint8_t qsRxBuf[100];    // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // setup the P0_2 UART0 TX pin
    LPC_PINCON->PINSEL0  &= ~(3U << 4); // clear P0_2 function
    LPC_PINCON->PINSEL0  |=  (1U << 4); // P0_2 to UART function (TX)
    LPC_PINCON->PINMODE0 &= ~(3U << 4); // P0_2 pull-up register

    // setup the P0_3 UART0 RX pin
    LPC_PINCON->PINSEL0  &= ~(3U << 6); // clear P0_3 function
    LPC_PINCON->PINSEL0  |=  (1U << 6); // P0_3 to UART function (RX)
    LPC_PINCON->PINMODE0 &= ~(3U << 6); // P0_3 pull-up register

    // enable power to UART0
    LPC_SC->PCONP |= (1U << 3);

    // enable FIFOs and default RX trigger level
    LPC_UART0->FCR =
        (1U << 0)    // FIFO Enable - 0 = Disables, 1 = Enabled
        | (0U << 1)  // Rx Fifo Reset
        | (0U << 2)  // Tx Fifo Reset
        | (0U << 6); // Rx irq trig: 0=1char, 1=4chars, 2=8chars, 3=14chars

    // disable IRQs
    LPC_UART0->IER =
        (0U << 0)    // Rx Data available IRQ disable
        | (0U << 1)  // Tx Fifo empty IRQ disable
        | (0U << 2); // Rx Line Status IRQ disable


    // set default baud rate
    UART0_setBaudrate(115200U);

    // format 8-data-bits, 1-stop-bit, parity-none
    LPC_UART0->LCR =
        (3U << 0)    // 8-data-bits
        | (0U << 2)  // 1 stop-bit
        | (0U << 3)  // parity disable
        | (0U << 4); // parity none


    QS_tickPeriod_ = SystemCoreClock / BSP::TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_; // to start the timestamp at zero

    return true; // return success
}
//............................................................................
void onCleanup() {
}
//............................................................................
QSTimeCtr onGetTime() {  // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) { // not set?
        return QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else { // the rollover occured, but the SysTick_ISR did not run yet
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr)SysTick->VAL;
    }
}
//............................................................................
// NOTE:
// No critical section in QS::onFlush() to avoid nesting of critical sections
// in case QS::onFlush() is called from Q_onError().
void onFlush() {
    for (;;) {
        std::uint16_t b = getByte();
        if (b != QS_EOD) {
            while ((LPC_UART0->LSR & 0x20U) == 0U) { // while THR empty...
            }
            LPC_UART0->THR = b; // put into the THR register
        }
        else {
            break;
        }
    }
}
//............................................................................
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

//............................................................................
//
// Set the LPC UART0 barud-rate generator according to
// Section 14.4.12 in LPC176x Manual (document UM10360)
//
static void UART0_setBaudrate(std::uint32_t baud) {

    // First we check to see if the basic divide with no DivAddVal/MulVal
    // ratio gives us an integer result. If it does, we set DivAddVal = 0,
    // MulVal = 1. Otherwise, we search the valid ratio value range to find
    // the closest match. This could be more elegant, using search methods
    // and/or lookup tables, but the brute force method is not that much
    // slower, and is more maintainable.
    //
    std::uint32_t PCLK     = SystemCoreClock; // divider /1 set below
    std::uint16_t DL       = PCLK / (16U * baud);
    std::uint8_t DivAddVal = 0U;
    std::uint8_t MulVal    = 1U;

    // set PCLK divider to 1
    LPC_SC->PCLKSEL0 &= ~(0x3U << 6); // clear divider bits
    LPC_SC->PCLKSEL0 |=  (0x1U << 6); // set divider to 1

    if ((PCLK % (16U * baud)) != 0U) { // non zero remainder?
        uint32_t err_best = baud;
        bool found = false;
        std::uint32_t b;
        std::uint8_t mv;
        for (mv = 1U; mv < 16U && !found; mv++) {
            std::uint16_t dlv;
            std::uint8_t dav;
            for (dav = 0U; dav < mv; ++dav) {
                //
                // baud = PCLK / (16 * dlv * (1 + (DivAdd / Mul))
                // solving for dlv, we get
                // dlv = mul * PCLK / (16 * baud * (divadd + mul))
                // mul has 4 bits, PCLK has 27 so we have 1 bit headroom,
                // which can be used for rounding for many values of mul
                // and PCLK we have 2 or more bits of headroom which can
                // be used to improve precision
                // note: X / 32 doesn't round correctly.
                // Instead, we use ((X / 16) + 1) / 2 for correct rounding
                //
                if ((mv*PCLK*2U) & 0x80000000U) { // 1 bit headroom
                    dlv = ((((2U*mv*PCLK) / (baud*(dav + mv)))/16U) + 1U)/2U;
                }
                else { // 2 bits headroom, use more precision
                    dlv = ((((4U*mv*PCLK) / (baud*(dav+mv)))/32U) + 1U)/2U;
                }

                // datasheet says if DLL==DLM==0, then 1 is used instead
                if (dlv == 0U) {
                    dlv = 1U;
                }
                // datasheet says if dav > 0 then DL must be >= 2
                if ((dav > 0U) && (dlv < 2U)) {
                    dlv = 2U;
                }
                // integer rearrangement of baud equation (with rounding)
                b = ((PCLK*mv / (dlv*(dav + mv)*8U)) + 1U)/2U;
                b = (b >= baud) ? (b - baud) : (baud - b);

                // check to see how we did
                if (b < err_best) {
                    err_best  = b;
                    DL        = dlv;
                    MulVal    = mv;
                    DivAddVal = dav;

                    if (b == baud) {
                        found = true;
                        break;   // break out of the inner for-loop
                    }
                }
            }
        }
    }

    // set LCR[DLAB] to enable writing to divider registers
    LPC_UART0->LCR |= (1U << 7);

    // set divider values
    LPC_UART0->DLM = (DL >> 8) & 0xFFU;
    LPC_UART0->DLL = (DL >> 0) & 0xFFU;
    LPC_UART0->FDR = ((uint32_t)DivAddVal << 0)
                   | ((uint32_t)MulVal    << 4);

    // clear LCR[DLAB]
    LPC_UART0->LCR &= ~(1U << 7);
}

} // namespace QS
#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//============================================================================
// NOTE1:
// The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
// ISR priority that is disabled by the QF framework. The value is suitable
// for the NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
// with the numerical values of priorities equal or higher than
// QF_AWARE_ISR_CMSIS_PRI) are allowed to call the QK_ISR_ENTRY/
// QK_ISR_ENTRY macros or any other QF/QK services. These ISRs are
// "QF-aware".
//
// Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
// level (i.e., with the numerical values of priorities less than
// QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
// Such "QF-unaware" ISRs cannot call ANY QF/QK services. In particular they
// can NOT call the macros QK_ISR_ENTRY/QK_ISR_ENTRY. The only mechanism
// by which a "QF-unaware" ISR can communicate with the QF framework is by
// triggering a "QF-aware" ISR, which can post/publish events.
//
// NOTE2:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.

