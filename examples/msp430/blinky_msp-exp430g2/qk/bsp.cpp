//****************************************************************************
// Product: "Blinky" example on MSP-EXP430G2 board, preemptive QK kernel
// Last updated for version 5.6.0
// Last updated on  2015-12-26
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
#include "blinky.h"
#include "bsp.h"

#include <msp430g2553.h>  // MSP430 variant used
// add other drivers if necessary...

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
// 1MHz clock setting, see BSP_init()
#define BSP_MCK     1000000U
#define BSP_SMCLK   1000000U

#define LED1        (1U << 0)
#define LED2        (1U << 6)

// ISRs used in this project =================================================
extern "C" {

//............................................................................
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    __interrupt void TIMER0_A0_ISR(void); /* prototype */
    #pragma vector=TIMER0_A0_VECTOR
    __interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
    void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_ISR (void)
#else
    #error Compiler not supported!
#endif
{
    QK_ISR_ENTRY();    // inform QK about entering the ISR

    TACTL &= ~TAIFG;   // clear the interrupt pending flag
    QF::TICK_X(0U, (void *)0); // process time events for rate 0

    QK_ISR_EXIT();     // inform QK about exiting the ISR
}

} // extern "C"

// BSP functions =============================================================
void BSP_init(void) {
    WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */

    /* configure the Basic Clock Module */
    DCOCTL = 0;             // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;  // Set DCO
    DCOCTL = CALDCO_1MHZ;

    P1DIR |= (LED1 | LED2);  /* set LED1 and LED2 pins to output  */
}
//............................................................................
void BSP_ledOff(void) {
    P1OUT &= ~LED1; // turn LED1 off
}
//............................................................................
void BSP_ledOn(void) {
    P1OUT |= LED1;  // turn LED1 on
}

//............................................................................
extern "C" void Q_onAssert(char const *module, int loc) {
    // implement the error-handling policy for your application!!!
    QF_INT_DISABLE(); // disable all interrupts
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));

    // cause the reset of the CPU...
    WDTCTL = WDTPW | WDTHOLD;
    __asm("    push &0xFFFE");
    // return from function does the reset
}

// QF callbacks ==============================================================
void QF::onStartup(void) {
    TACTL  = (ID_3 | TASSEL_2 | MC_1);  // SMCLK, /8 divider, upmode
    TACCR0 = (((BSP_SMCLK / 8U) + BSP_TICKS_PER_SEC/2U) / BSP_TICKS_PER_SEC);
    CCTL0 = CCIE;  // CCR0 interrupt enabled
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {
    // toggle LED2 on and then off, see NOTE1
    QF_INT_DISABLE();
    P1OUT |=  LED2;   // turn LED2 on
    P1OUT &= ~LED2;   // turn LED2 off
    QF_INT_ENABLE();

#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular MSP430 MCU.
    //
    __low_power_mode_1(); // Enter LPM1; also ENABLES interrupts
#endif
}




//****************************************************************************
// NOTE1:
// One of the LEDs is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
