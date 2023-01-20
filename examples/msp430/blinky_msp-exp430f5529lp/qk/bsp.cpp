//============================================================================
// Product: "Blinky" example on MSP-EXP430F5529LP board, preemptive QK kernel
// Last updated for version 7.3.0
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
#include "blinky.hpp"            // Application interface
#include "bsp.hpp"               // Board Support Package

#include <msp430f5529.h>  // MSP430 variant used
// add other drivers if necessary...

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
constexpr std::uint32_t BSP_MCK    {1000000U};
constexpr std::uint32_t BSP_SMCLK  {1000000U};

constexpr std::uint32_t LED1       {1U << 0U};
constexpr std::uint32_t LED2       {1U << 7U};

constexpr std::uint32_t BTN_S1     {1U << 1U};

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

Q_NORETURN Q_onError(char const * const module, int_t const id) {
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
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

// ISRs --------------------------------------------------------------------

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
    QK_ISR_ENTRY();   // inform QK about entering the ISR

    QTimeEvt::TICK_X(0U, nullptr);  // time events at rate 0

    QK_ISR_EXIT();  // inform QK about exiting the ISR

#ifdef NDEBUG
    __low_power_mode_off_on_exit(); // turn the low-power mode OFF, NOTE1
#endif
}

} // extern "C"

//============================================================================
namespace BSP {

void init() {
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    // leave the MCK and SMCLK at default DCO setting

    // configure pins for LEDs
    P1DIR |= LED1;  // set LED1 pin to output
    P4DIR |= LED2;  // set LED2 pin to output
}
//............................................................................
void start() {
    // publish-subscribe not used, no call to QActive::psInit()
    // dynamic event allocation not used, no call to QF::poolInit()

    // instantiate and start the active objects...
    static QEvt const *blinkyQSto[10]; // Event queue storage for Blinky
    AO_Blinky->start(1U,                            // priority
                     blinkyQSto, Q_DIM(blinkyQSto), // event queue
                     nullptr, 0U);                // stack (unused)
}
//............................................................................
void ledOn() {
    P1OUT |=  LED1;
}
//............................................................................
void ledOff() {
    P1OUT &= ~LED1;
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
void QK::onIdle() {
    // toggle LED2 on and then off, see NOTE2
    QF_INT_DISABLE();
    P4OUT |=  LED2;  // turn LED2 on
    P4OUT &= ~LED2;  // turn LED2 off
    QF_INT_ENABLE();

#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular MSP430 MCU.
    //
    __low_power_mode_1(); // enter LPM1; also ENABLES interrupts, see NOTE1
#endif
}

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
