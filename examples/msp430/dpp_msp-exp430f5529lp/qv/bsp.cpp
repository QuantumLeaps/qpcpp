//============================================================================
// DPP example on MSP-EXP430F5529LP board, preemptive QK kernel
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

#include <msp430f5529.h>  // MSP430 variant used
// add other drivers if necessary...

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
constexpr std::uint32_t BSP_MCK    {1000000U};
constexpr std::uint32_t BSP_SMCLK  {1000000U};

constexpr std::uint32_t LED1       {1U << 0U};
constexpr std::uint32_t LED2       {1U << 7U};

constexpr std::uint32_t BTN_S1     {1U << 1U};

static std::uint32_t l_rndSeed;

#ifdef Q_SPY
    // UART1 pins TX:P4.4, RX:P4.5
    constexpr std::uint32_t TXD    {1U << 4U};
    constexpr std::uint32_t RXD    {1U << 5U};

    // QSpy source IDs
    static QP::QSpyId const l_timer0_ISR = { 0U };

    static QP::QSTimeCtr QS_tickTime_;

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

Q_NORETURN Q_onError(char const * const module, int const id) {
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U); // report assertion to QS

#ifndef NDEBUG
    P4OUT |=  LED2;  // turn LED2 on
    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#else
    WDTCTL = 0xDEAD;
    for (;;) { // explicitly "no-return"
    }
#endif
}
//............................................................................
void assert_failed(char const * const module, int const id); // prototype
void assert_failed(char const * const module, int const id) {
    Q_onError(module, id);
}

// ISRs used in the application ==============================================

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    __interrupt void TIMER0_A0_ISR(void); // prototype
    #pragma vector=TIMER0_A0_VECTOR
    __interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
    __attribute__ ((interrupt(TIMER0_A0_VECTOR)))
    void TIMER0_A0_ISR(void)
#else
    #error MSP430 compiler not supported!
#endif
{
    QP::QTimeEvt::TICK_X(0U, &l_timer0_ISR); // time events at rate 0

#ifdef Q_SPY
    QS_tickTime_ +=
       (((BSP_SMCLK / 8) + BSP::TICKS_PER_SEC/2) / BSP::TICKS_PER_SEC) + 1;
#endif

#ifdef NDEBUG
    __low_power_mode_off_on_exit(); // turn the low-power mode OFF, NOTE1
#endif
}
//............................................................................
#ifdef Q_SPY

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    __interrupt void USCI_A1_ISR(void); /* prototype */
    #pragma vector=USCI_A1_VECTOR
    __interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
    __attribute__ ((interrupt(USCI_A1_VECTOR)))
    void USCI_A1_ISR(void)
#else
    #error MSP430 compiler not supported!
#endif
{
    if (UCA1IV == 2U) {
        std::uint16_t b = UCA1RXBUF;
        QP::QS::rxPut(b);
    }
}

#endif // Q_SPY

//............................................................................
#ifdef QF_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QF_onContextSw(QP::QActive *prev, QP::QActive *next) {
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
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    // leave the MCK and SMCLK at default DCO setting

    // configure pins for LEDs
    P1DIR |= LED1;  // set LED1 pin to output
    P4DIR |= LED2;  // set LED2 pin to output

    BSP::randomSeed(1234U);

    // initialize the QS software tracing...
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }

    // dictionaries...
    QS_OBJ_DICTIONARY(&l_timer0_ISR);
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
    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10];
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
        APP::AO_Philo[n]->start(

            // NOTE: set the preemption-threshold of all Philos to
            // the same level, so that they cannot preempt each other.
            Q_PRIO(n + 1U, APP::N_PHILO), // QF-prio/pre-thre.

            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            nullptr, 0U);            // no stack storage
    }

    static QP::QEvt const *tableQueueSto[APP::N_PHILO];
    APP::AO_Table->start(
        APP::N_PHILO + 1U,           // QP prio. of the AO
        tableQueueSto,               // event queue storage
        Q_DIM(tableQueueSto),        // queue length [events]
        nullptr, 0U);                // no stack storage
}
//............................................................................
void displayPhilStat(std::uint8_t n, char const *stat) {
    Q_UNUSED_PAR(n);

    if (stat[0] == 'e') { // is Philo eating?
        P1OUT |=  LED1;  // turn LED1 on
    }
    else {
        P1OUT &= ~LED1;  // turn LED1 off
    }

    // app-specific trace record...
    QS_BEGIN_ID(PHILO_STAT, APP::AO_Table->getPrio())
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void displayPaused(std::uint8_t const paused) {
    // not enouhg LEDs to implement this feature
    if (paused != 0U) {
        //P1OUT |=  LED1;
    }
    else {
        //P1OUT &= ~LED1;
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

    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    std::uint32_t rnd = l_rndSeed * (3U*7U*11U*13U*23U);
    l_rndSeed = rnd; // set for the next time

    return (rnd >> 8U);
}
//............................................................................
void ledOn() {
    //P1OUT |=  LED1;
}
//............................................................................
void ledOff() {
    //P1OUT &= ~LED1;
}
//............................................................................
void terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}

} // namespace BSP

//============================================================================
// namespace QP
namespace QP {

// QF callbacks...
void QF::onStartup() {
    TA0CCTL0 = CCIE;  // CCR0 interrupt enabled
    TA0CCR0 = BSP_MCK / BSP::TICKS_PER_SEC;
    TA0CTL = TASSEL_2 | MC_1 | TACLR; // SMCLK, upmode, clear TAR
}
//............................................................................
void QF::onCleanup() {
}
//............................................................................
void QV::onIdle() { // CAUTION: called with interrutps DISABLED, see NOTE1
    // toggle LED2 on and then off, see NOTE2
    QF_INT_DISABLE();
    P4OUT |=  LED2;  // turn LED2 on
    P4OUT &= ~LED2;  // turn LED2 off
    QF_INT_ENABLE();

#ifdef Q_SPY
    QF_INT_DISABLE();
    QS::rxParse();  // parse all the received bytes
    QF_INT_ENABLE();

    if ((UCA1STAT & UCBUSY) == 0U) { // TX NOT busy?
        QF_INT_DISABLE();
        std::uint16_t b = QS::getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {
            UCA1TXBUF = b; // stick the byte to the TX BUF
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular MSP430 MCU.
    //
    __low_power_mode_1(); // enter LPM1; also ENABLES interrupts
#else
    QF_INT_ENABLE(); // just enable interrupts
#endif
}

//============================================================================
// QS callbacks...
#ifdef Q_SPY
namespace QS {

//............................................................................
bool onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static std::uint8_t qsTxBuf[256]; // buffer for QS-TX channel
    initBuf(qsTxBuf, sizeof(qsTxBuf));

    static std::uint8_t qsRxBuf[64];    // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // USCI setup code...
    P4SEL |= (RXD | TXD);  // select the UART function for the pins
    UCA1CTL1 |= UCSWRST;   // reset USCI state machine
    UCA1CTL1 |= UCSSEL_2;  // choose the SMCLK clock
#if 1 // 9600 baud rate
    UCA1BR0 = 6; // 1MHz 9600 (see User's Guide)
    UCA1BR1 = 0; // 1MHz 9600 */
    // modulationUCBRSx=0, UCBRFx=0, oversampling
    UCA1MCTL = UCBRS_0 | UCBRF_13 | UCOS16;
#else // 115200 baud rate
    UCA1BR0 = 9;           // 1MHz 115200 (see User's Guide)
    UCA1BR1 = 0;           // 1MHz 115200
    UCA1MCTL |= UCBRS_1 | UCBRF_0; // modulation UCBRSx=1, UCBRFx=0
#endif
    UCA1CTL1 &= ~UCSWRST;  // initialize USCI state machine
    UCA1IE |= UCRXIE;      // Enable USCI_A1 RX interrupt

    return true; // return success
}
//............................................................................
void onCleanup() {
}
//............................................................................
QSTimeCtr onGetTime() { // NOTE: invoked with interrupts DISABLED
    if ((TA0CTL & TAIFG) == 0U) {  /* interrupt not pending? */
        return QS_tickTime_ + TA0R;
    }
    else { /* the rollover occured, but the timerA_ISR did not run yet */
        return QS_tickTime_
           + (((BSP_SMCLK/8U) + BSP::TICKS_PER_SEC/2U)
                 / BSP::TICKS_PER_SEC) + 1U
           + TA0R;
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
            while ((UCA1STAT & UCBUSY) != 0U) { // TX busy?
            }
            UCA1TXBUF = b; // stick the byte to the TX BUF
        }
        else {
            break;
        }
    }
}
//............................................................................
void onReset() {
    WDTCTL = 0xDEAD;
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
// With the cooperative QV kernel for MSP430, it is necessary to explicitly
// turn the low-power mode OFF in the interrupt, because the return
// from the interrupt will restore the CPU status register, which will
// re-enter the low-power mode. This, in turn, will prevent the QV event-loop
// from running.
//
// NOTE2:
// One of the LEDs is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
