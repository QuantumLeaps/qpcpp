///***************************************************************************
// Product: DPP example, EK-TM4C123GXL board, TI-RTOS kernel (SYS/BIOS)
// Last updated for version 5.7.3
// Last updated on  2016-10-06
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

#include "Board.h"                // the board specific header (TI)
#include "ti/sysbios/knl/Clock.h" // the Clock driver (TI)
#include "ti/drivers/GPIO.h"      // GPIO driver (TI)
// add other drivers if necessary...

// TI-RTOS callback functions ================================================
extern "C" {

// Clock function to service the QP clock tick ...............................
static void clk0Fxn(UArg arg0) {
    // state of the button debouncing, see below
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { ~0U, ~0U };
    uint32_t current;
    uint32_t tmp;

    QP::QF::TICK_X(0U, &l_SysTick_Handler); // process time events for rate 0

    // Perform the debouncing of buttons. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    //
    current = GPIO_read(EK_TM4C123GXL_SW1);  // read SW1
    tmp = buttons.depressed; // save the debounced depressed buttons
    buttons.depressed |= (buttons.previous & current); // set depressed
    buttons.depressed &= (buttons.previous | current); // clear released
    buttons.previous   = current; // update the history
    tmp ^= buttons.depressed;     // changed debounced depressed
    if (tmp != 0U) {  // debounced SW1 state changed?
        if (buttons.depressed == 0U) { // is SW1 depressed?
            static QP::QEvt const pauseEvt = { DPP::PAUSE_SIG, 0U, 0U};
            QP::QF::PUBLISH(&pauseEvt, &l_SysTick_Handler);
        }
        else {            // the button is released
            static QP::QEvt const serveEvt = { DPP::SERVE_SIG, 0U, 0U};
            QP::QF::PUBLISH(&serveEvt, &l_SysTick_Handler);
        }
    }
}

// Idle callback (see dpp.cfg) ...............................................
void myIdleFunc() {
    QF_CRIT_STAT_TYPE key;

    QF_CRIT_ENTRY(key);
    GPIO_write(EK_TM4C123GXL_LED_RED, 1); // turn the LED on
    GPIO_write(EK_TM4C123GXL_LED_RED, 0); // turn the LED off
    QF_CRIT_EXIT(key);

#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    __asm (" WFI"); // Wait-For-Interrupt
#endif
}

} // extern "C"

// namespace DPP *************************************************************
namespace DPP {

Q_DEFINE_THIS_FILE

// Local-scope objects -------------------------------------------------------
static uint32_t l_rnd; // random seed

// BSP functions =============================================================
void BSP::init(void) {
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();

    BSP::randomSeed(1234U);
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    // exercise the FPU with some floating point computations */
    //
    float volatile x;
    x = 3.1415926F;
    x = x + 2.7182818F;

    GPIO_write(EK_TM4C123GXL_LED_BLUE,
               ((stat[0] == 'e')   // Is Philo[n] eating?
               ? 1    // turn the LED1 on
               : 0)); // turn the LED1 off
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    GPIO_write(EK_TM4C123GXL_LED_GREEN,
               ((paused != 0U)   // is Eating paused?
               ? 1    // turn the LED1 on
               : 0)); // turn the LED1 off
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time

    return (rnd >> 8);
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

// QF callbacks ==============================================================
void QF::onStartup(void) {
    static Clock_Struct clk0Struct;
    Clock_Params clkParams;

    Clock_Params_init(&clkParams);
    clkParams.startFlag = TRUE;
    clkParams.period = 1000U/DPP::BSP::TICKS_PER_SEC;

    // Construct a periodic Clock Instance
    Clock_construct(&clk0Struct, &clk0Fxn, clkParams.period, &clkParams);
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
    //NVIC_SystemReset();
    for (;;) { // for-ever loop (NOT a good idea for production code!)
    }
}

} // namespace QP

//****************************************************************************
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
