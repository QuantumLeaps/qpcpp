//****************************************************************************
// Product: "Blinky" example, EK-TM4C123GXL board, FreeRTOS.org kernel
// Last updated for version 5.4.0
// Last updated on  2015-05-11
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
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "blinky.h"
#include "bsp.h"

#include "TM4C123GH6PM.h"        // the device specific header (TI)
#include "rom.h"                 // the built-in ROM functions (TI)
#include "sysctl.h"              // system control driver (TI)
#include "gpio.h"                // GPIO driver (TI)
// add other drivers if necessary...

Q_DEFINE_THIS_FILE

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
// DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
//
enum KernelUnawareISRs { // see NOTE00
    // ...
    MAX_KERNEL_UNAWARE_CMSIS_PRI  // keep always last
};
// "kernel-unaware" interrupts can't overlap "kernel-aware" interrupts
Q_ASSERT_COMPILE(MAX_KERNEL_UNAWARE_CMSIS_PRI
                 <= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

enum KernelAwareISRs {
    GPIOA_PRIO = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, // see NOTE1
    // ...
    MAX_KERNEL_AWARE_CMSIS_PRI // keep always last
};
// "kernel-aware" interrupts should not overlap the PendSV priority
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));

// Local-scope objects -------------------------------------------------------
#define LED_RED     (1U << 1)
#define LED_GREEN   (1U << 3)
#define LED_BLUE    (1U << 2)

#define BTN_SW1     (1U << 4)
#define BTN_SW2     (1U << 0)

// ISRs used in this project =================================================
extern "C" {

void GPIOPortA_IRQHandler(void); // prototype
void GPIOPortA_IRQHandler(void) {
    QF_CRIT_STAT_TYPE intStat;
    BaseType_t lHigherPriorityTaskWoken = pdFALSE;

    QF_ISR_ENTRY(intStat);              // <=== inform QF about ISR entry

    AO_Blinky->POST(Q_NEW(QP::QEvt, MAX_SIG), 0); // for testing...

    QF_ISR_EXIT(intStat, lHigherPriorityTaskWoken); // <=== ISR exit

    // the usual end of FreeRTOS ISR...
    portEND_SWITCHING_ISR(lHigherPriorityTaskWoken);
}

// Application hooks used in this project ====================================
void vApplicationTickHook(void) {
    QF_CRIT_STAT_TYPE intStat;
    BaseType_t lHigherPriorityTaskWoken = pdFALSE;

    QF_ISR_ENTRY(intStat);     // <=== inform QF about ISR entry

    QP::QF::TICK_X(0U, 0); // process time events for rate 0

    QF_ISR_EXIT(intStat, lHigherPriorityTaskWoken); // <=== ISR exit

    // yield only when needed...
    if (lHigherPriorityTaskWoken != pdFALSE) {
        vTaskMissedYield();
    }
}
//............................................................................
void vApplicationIdleHook(void) {
    // visualize the idle loop activity, see NOTE2
    QF_INT_DISABLE();
    GPIOF->DATA_Bits[LED_BLUE] = 0xFFU;
    GPIOF->DATA_Bits[LED_BLUE] = 0U;
    QF_INT_ENABLE();

#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // You might need to customize the clock management for your project,
    // see the datasheet for your particular Cortex-M3 MCU.
    //
    __WFI(); // Wait-For-Interrupt
#endif
}
//............................................................................
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    Q_ERROR();
}

} // extern "C"

// BSP functions =============================================================
void BSP_init(void) {
    // NOTE: SystemInit() already called from the startup code
    //  but SystemCoreClock needs to be updated
    //
    SystemCoreClockUpdate();

    // configure the FPU
    // Do NOT to use the automatic FPU state preservation and
    // do NOT to use the FPU lazy stacking.
    //
    FPU->FPCCR &= ~((1U << FPU_FPCCR_ASPEN_Pos) | (1U << FPU_FPCCR_LSPEN_Pos));

    // enable clock to the peripherals used by the application
    SYSCTL->RCGC2 |= (1U << 5); // enable clock to GPIOF
    __NOP();                    // wait after enabling clocks
    __NOP();
    __NOP();

    // configure the LEDs and push buttons
    GPIOF->DIR |= (LED_RED | LED_GREEN | LED_BLUE); // set direction: output
    GPIOF->DEN |= (LED_RED | LED_GREEN | LED_BLUE); // digital enable
    GPIOF->DATA_Bits[LED_RED]   = 0U;  // turn the LED off
    GPIOF->DATA_Bits[LED_GREEN] = 0U;  // turn the LED off
    GPIOF->DATA_Bits[LED_BLUE]  = 0U;  // turn the LED off

    // configure the Buttons
    GPIOF->DIR &= ~(BTN_SW1 | BTN_SW2); //  set direction: input
    ROM_GPIOPadConfigSet(GPIOF_BASE, (BTN_SW1 | BTN_SW2),
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}
//............................................................................
void BSP_ledOff(void) {
    GPIOF->DATA_Bits[LED_GREEN] = 0U;
}
/*..........................................................................*/
void BSP_ledOn(void) {
    GPIOF->DATA_Bits[LED_GREEN] = 0xFFU;
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
}

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    //SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    // set priorities of ALL ISRs used in the system, see NOTE00
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    // DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    //
    //NVIC_SetPriority(SysTick_IRQn,   SYSTICK_PRIO);
    NVIC_SetPriority(GPIOA_IRQn, GPIOA_PRIO);
    // ...

    // enable IRQs...
    NVIC_EnableIRQ(GPIOA_IRQn);
    // ...
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
// NOTE Q_onAssert() defined in assembly in startup_TM4C123GH6PM.s

///***************************************************************************
// NOTE1:
// The configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY constant from the
// FreeRTOS configuration file specifies the highest ISR priority that
// is disabled by the QF framework. The value is suitable for the
// NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY level (i.e.,
// with the numerical values of priorities equal or higher than
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any
// QP/FreeRTOS services. These ISRs are "kernel-aware".
//
// Conversely, any ISRs prioritized above the
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY priority level (i.e., with
// the numerical values of priorities less than
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) are never disabled and are
// not aware of the kernel. Such "kernel-unaware" ISRs cannot call any
// QP/FreeRTOS services. The only mechanism by which a "kernel-unaware" ISR
// can communicate with the QF framework is by triggering a "kernel-aware"
// ISR, which can post/publish events.
//
// For more information, see article "Running the RTOS on a ARM Cortex-M Core"
// http://www.freertos.org/RTOS-Cortex-M3-M4.html
//
// NOTE2:
// The blue LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the blue LED.
//
