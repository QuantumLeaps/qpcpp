//============================================================================
// Product: DPP example on MSP-EXP430F5529LP board, preemptive QK kernel
// Last updated for version 6.9.1
// Last updated on  2020-09-21
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

#include <msp430f5529.h>  // MSP430 variant used
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

// namespace DPP *************************************************************
namespace DPP {

// Local-scope objects -------------------------------------------------------
// 1MHz clock setting, see BSP::init()
#define BSP_MCK     1000000U
#define BSP_SMCLK   1000000U

#define LED1        (1U << 0)
#define LED2        (1U << 7)

#define BTN_S1      (1U << 1)

// random seed
static uint32_t l_rnd;

#ifdef Q_SPY
    /* UART1 pins TX:P4.4, RX:P4.5 */
    #define TXD     (1U << 4)
    #define RXD     (1U << 5)

    QP::QSTimeCtr QS_tickTime_;

    static uint8_t const l_timer0_ISR = 0U;

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        COMMAND_STAT
    };

#endif

// ISRs used in this project =================================================
extern "C" {

//............................................................................
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    __interrupt void TIMER0_A0_ISR(void); // prototype
    #pragma vector=TIMER0_A0_VECTOR
    __interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
    __attribute__ ((interrupt(TIMER0_A0_VECTOR)))
    void TIMER0_A0_ISR (void)
#else
    #error MSP430 compiler not supported!
#endif
{
#ifdef Q_SPY
    QS_tickTime_ +=
       (((BSP_SMCLK / 8) + DPP::BSP::TICKS_PER_SEC/2)
       / DPP::BSP::TICKS_PER_SEC) + 1;
#endif
    QK_ISR_ENTRY();    // inform QK about entering the ISR

    QP::QTimeEvt::TICK_X(0U, &l_timer0_ISR); // process all time events at rate 0

    QK_ISR_EXIT();     // inform QK about exiting the ISR

#ifdef NDEBUG
    __low_power_mode_off_on_exit(); //  turn the low-power mode OFF, NOTE1
#endif
}

} // extern "C"

// BSP functions =============================================================
void BSP::init(void) {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    // leave the MCK and SMCLK at default DCO setting

    // configure pins for LEDs
    P1DIR |= LED1;  // set LED1 pin to output
    P4DIR |= LED2;  // set LED2 pin to output

    if (QS_INIT(nullptr) == 0) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_timer0_ISR);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(COMMAND_STAT);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
    //QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
    QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == 'h') { // is Philo hungry?
        P1OUT |=  LED1;  // turn LED1 on
    }
    else {
        P1OUT &= ~LED1;  // turn LED1 off
    }

    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);   // Philosopher number
        QS_STR(stat);  // Philosopher status
    QS_END()
}
void BSP::displayPaused(uint8_t paused) {
    // not enouhg LEDs to implement this feature
    if (paused != 0U) {
        //P1OUT |=  LED1;
    }
    else {
        //P1OUT &= ~LED1;
    }
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * ((uint32_t)3U*7U*11U*13U*23U);

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

//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    // implement the error-handling policy for your application!!!
    QF_INT_DISABLE(); // disable all interrupts
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));

    // write invalid password to WDT: cause a password-validation RESET
    WDTCTL = 0xDEAD;
}

// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    TA0CCTL0 = CCIE;  // CCR0 interrupt enabled
    TA0CCR0 = BSP_MCK / DPP::BSP::TICKS_PER_SEC;
    TA0CTL = TASSEL_2 | MC_1 | TACLR; // SMCLK, upmode, clear TAR
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {
    // toggle LED2 on and then off, see NOTE2
    QF_INT_DISABLE();
    P4OUT |=  LED2;  // turn LED2 on
    P4OUT &= ~LED2;  // turn LED2 off
    QF_INT_ENABLE();

#ifdef Q_SPY
    QS::rxParse();  // parse all the received bytes

    if ((UCA1STAT & UCBUSY) == 0U) { // TX NOT busy?

        uint16_t b;

        QF_INT_DISABLE();
        b = QS::getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {
            UCA1TXBUF = (uint8_t)b; // stick the byte to the TX BUF
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular MSP430 MCU.
    //
    __low_power_mode_1(); // enter LPM1; also ENABLES interrupts, see NOTE1
#endif
}

// QS callbacks ==============================================================
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
    // NOTE: no need to call QK_ISR_ENTRY/EXIT

    if (UCA1IV == 2) {
        uint16_t b = UCA1RXBUF;
        QP::QS::rxPut(b);
    }
}
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[256];  // buffer for QS; RAM is tight!
    static uint8_t qsRxBuf[80]; // buffer for QS receive channel
    //uint16_t tmp;

    initBuf(qsBuf, sizeof(qsBuf));
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
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {  // invoked with interrupts DISABLED
    if ((TA0CTL & TAIFG) == 0U) {  /* interrupt not pending? */
        return DPP::QS_tickTime_ + TA0R;
    }
    else { /* the rollover occured, but the timerA_ISR did not run yet */
        return DPP::QS_tickTime_
           + (((BSP_SMCLK/8U) + DPP::BSP::TICKS_PER_SEC/2U)
                 / DPP::BSP::TICKS_PER_SEC) + 1U
           + TA0R;
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    QF_INT_DISABLE();
    while ((b = getByte()) != QS_EOD) { // next QS byte available?
        QF_INT_ENABLE();
        while ((UCA1STAT & UCBUSY) != 0U) { /* TX busy? */
        }
        UCA1TXBUF = (uint8_t)b; /* stick the byte to the TX BUF */
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    /* write invalid password to WDT: cause a password-validation RESET */
    WDTCTL = 0xDEAD;
}
//............................................................................
//! callback function to execute a uesr command (to be implemented in BSP)
void QS::onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
    QS_BEGIN_ID(DPP::COMMAND_STAT, 0U)
        QS_U8(2, cmdId);
        QS_U32(8, param1);
        QS_U32(8, param2);
        QS_U32(8, param3);
    QS_END()
}

#endif // Q_SPY

} // namespace QP

//============================================================================
// NOTE1:
// With the preemptive QK kernel for MSP430, the idle callback QK::onIdle()
// will execute only ONCE, if the low-power mode is not explicitly turned OFF
// in the interrupt. This might or might not be what you want.
//
// NOTE2:
// One of the LEDs is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
