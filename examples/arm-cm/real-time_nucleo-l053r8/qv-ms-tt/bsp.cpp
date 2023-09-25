//============================================================================
// BSP for "real-time" Example
// Last updated for version 7.3.1
// Last updated on  2023-10-03
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
#include "qpcpp.hpp"           // QP/C++ real-time embedded framework
#include "bsp.hpp"             // Board Support Package
#include "app.hpp"             // Application interface

#include "stm32l0xx.h"  // CMSIS-compliant header file for the MCU used
// add other drivers if necessary...

Q_DEFINE_THIS_MODULE("bsp") // for functional-safety assertions

// Local-scope defines -----------------------------------------------------

// test pins on GPIO PA (output)
#define TST1_PIN  7U
#define TST2_PIN  6U
#define TST3_PIN  4U
#define TST4_PIN  1U
#define TST5_PIN  0U
#define TST6_PIN  9U
#define TST7_PIN  5U // LED LD2-Green

// button on GPIO PC (input)
#define B1_PIN    13U

#ifdef Q_SPY
    // QSpy source IDs
    static QSpyId const l_SysTick_Handler = { 100U };

#endif

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
    // light up the user LED
    GPIOA->BSRR = (1U << TST6_PIN);
    for (;;) { // for debugging, hang on in an endless loop...
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

// ISRs used in the application ==========================================

void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {
    BSP::d1on();

    APP::TTSched_tick(); // run the TT scheduler

    // Perform the debouncing of sporadics. The algorithm for debouncing
    // adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    // and Michael Barr, page 71.
    static struct {
        uint32_t depressed;
        uint32_t previous;
    } sporadics = { 0U, 0U };
    uint32_t current = ~GPIOC->IDR; // read Port C with state of Sporadic B1
    uint32_t tmp = sporadics.depressed; // save the depressed sporadics
    sporadics.depressed |= (sporadics.previous & current); // set depressed
    sporadics.depressed &= (sporadics.previous | current); // clear released
    sporadics.previous   = current; // update the history
    tmp ^= sporadics.depressed;     // changed debounced depressed
    current = sporadics.depressed;

    if ((tmp & (1U << B1_PIN)) != 0U) { // debounced B1 state changed?
        if ((current & (1U << B1_PIN)) != 0U) { // is B1 depressed?
            // immutable sporadic-press event
            static APP::SporadicSpecEvt const
                sporadicA(APP::SPORADIC_A_SIG, 189U, 23U);
            // immutable forward-press event
            static APP::SporadicSpecEvt const
                sporadicB(APP::SPORADIC_B_SIG, 89U, 23U);
            APP::TTSched_mode(1U);
            APP::AO_Sporadic2->POST(&sporadicA, &l_SysTick_Handler);
            APP::AO_Sporadic2->POST(&sporadicB, &l_SysTick_Handler);
        }
        else { // B1 is released
            APP::TTSched_mode(0U);
            APP::AO_Periodic4->POST(BSP::getEvtPeriodic4(0U), &l_SysTick_Handler);
            APP::AO_Periodic1->POST(BSP::getEvtPeriodic1(0U), &l_SysTick_Handler);
        }
    }

    QV_ARM_ERRATUM_838869();

    BSP::d1off();
}

} // extern "C"

// BSP functions =============================================================
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

    // enable GPIO port PA clock
    RCC->IOPENR |= (1U << 0U);

    // set all used GPIOA pins as push-pull output, no pull-up, pull-down
    GPIOA->MODER &=
        ~((3U << 2U*TST1_PIN) | (3U << 2U*TST2_PIN) | (3U << 2U*TST3_PIN) |
          (3U << 2U*TST4_PIN) | (3U << 2U*TST5_PIN) | (3U << 2U*TST6_PIN) |
          (3U << 2U*TST7_PIN));
    GPIOA->MODER |=
         ((1U << 2U*TST1_PIN) | (1U << 2U*TST2_PIN) | (1U << 2U*TST3_PIN) |
          (1U << 2U*TST4_PIN) | (1U << 2U*TST5_PIN) | (1U << 2U*TST6_PIN) |
          (1U << 2U*TST7_PIN));
    GPIOA->OTYPER &=
        ~((1U <<    TST1_PIN) | (1U <<    TST2_PIN) | (1U <<    TST3_PIN) |
          (1U <<    TST4_PIN) | (1U <<    TST5_PIN) | (1U <<    TST6_PIN) |
          (1U <<    TST7_PIN));
    GPIOA->PUPDR &=
        ~((3U << 2U*TST1_PIN) | (3U << 2U*TST2_PIN) | (3U << 2U*TST3_PIN) |
          (3U << 2U*TST4_PIN) | (3U << 2U*TST5_PIN) | (3U << 2U*TST6_PIN) |
          (3U << 2U*TST7_PIN));

    // enable GPIOC clock port for the Sporadic B1
    RCC->IOPENR |=  (1U << 2U);

    // configure Sporadic B1 pin on GPIOC as input, no pull-up, pull-down
    GPIOC->MODER &= ~(3U << 2U*B1_PIN);
    GPIOC->PUPDR &= ~(3U << 2U*B1_PIN);
}
//............................................................................
void start() {
    // instantiate and start QP/C active objects...
    static QP::QEvt const *periodic1QSto[10]; // Event queue storage
    APP::AO_Periodic1->start(
        1U,                    // QF-prio
        periodic1QSto,         // storage for the AO's queue
        Q_DIM(periodic1QSto),  // queue length
        nullptr, 0U,           // stack storage, size (not used)
        getEvtPeriodic1(0U));  // initialization param

    static QP::QEvt const *sporadic2QSto[8]; // Event queue storage
    APP::AO_Sporadic2->start(
        2U,                    // QF-prio
        sporadic2QSto,         // storage for the AO's queue
        Q_DIM(sporadic2QSto),  // queue length
        nullptr, 0U,           // stack storage, size (not used)
        (void const *)0);      // initialization param -- not used

    static QP::QEvt const *sporadic3QSto[8]; // Event queue storage
    APP::AO_Sporadic3->start(
        3U,                    // QF-prio
        sporadic3QSto,         // storage for the AO's queue
        Q_DIM(sporadic3QSto),  // queue length
        nullptr, 0U,           // stack storage, size (not used)
        (void const *)0);      // initialization param -- not used

    static QP::QEvt const *periodic4QSto[8]; // Event queue storage
    APP::AO_Periodic4->start(
        4U,                    // QF-prio
        periodic4QSto,         // storage for the AO's queue
        Q_DIM(periodic4QSto),  // queue length
        nullptr, 0U,           // stack storage, size (not used)
        getEvtPeriodic4(0U));  // initialization event
}
//............................................................................
void d1on()  { GPIOA->BSRR = (1U << TST1_PIN);         }
void d1off() { GPIOA->BSRR = (1U << (TST1_PIN + 16U)); }
//............................................................................
void d2on()  { GPIOA->BSRR = (1U << TST2_PIN);         }
void d2off() { GPIOA->BSRR = (1U << (TST2_PIN + 16U)); }
//............................................................................
void d3on()  { GPIOA->BSRR = (1U << TST3_PIN);         }
void d3off() { GPIOA->BSRR = (1U << (TST3_PIN + 16U)); }
//............................................................................
void d4on()  { GPIOA->BSRR = (1U << TST4_PIN);         }
void d4off() { GPIOA->BSRR = (1U << (TST4_PIN + 16U)); }
//............................................................................
void d5on()  { GPIOA->BSRR = (1U << TST5_PIN);         }
void d5off() { GPIOA->BSRR = (1U << (TST5_PIN + 16U)); }
//............................................................................
void d6on()  { GPIOA->BSRR = (1U << TST6_PIN);         } // LED2
void d6off() { GPIOA->BSRR = (1U << (TST6_PIN + 16U)); }
//............................................................................
void d7on()  { GPIOA->BSRR = (1U << TST7_PIN);         }
void d7off() { GPIOA->BSRR = (1U << (TST7_PIN + 16U)); }

//............................................................................
QP::QEvt const *getEvtPeriodic1(uint8_t num) {
    // immutable PERIODIC_SPEC events for Periodic1
    static APP::PeriodicSpecEvt const periodicSpec1[] {
        APP::PeriodicSpecEvt(APP::PERIODIC_SPEC_SIG, 40U, 5U),
        APP::PeriodicSpecEvt(APP::PERIODIC_SPEC_SIG, 30U, 7U)
    };
    Q_REQUIRE_ID(500, num < Q_DIM(periodicSpec1)); // must be in range
    return &periodicSpec1[num];
}
//............................................................................
QP::QEvt const *getEvtPeriodic4(uint8_t num) {
    // immutable PERIODIC_SPEC events for Periodic4
    static APP::PeriodicSpecEvt const periodicSpec4[] {
        APP::PeriodicSpecEvt(APP::PERIODIC_SPEC_SIG, 20U, 2U),
        APP::PeriodicSpecEvt(APP::PERIODIC_SPEC_SIG, 10U, 1U)
    };
    Q_REQUIRE_ID(600, num < Q_DIM(periodicSpec4)); // must be in range
    return &periodicSpec4[num];
}

} // namespace BSP

// QF callbacks ==============================================================
namespace QP {

void QF::onStartup() {
    SystemCoreClockUpdate();

    // set up the SysTick timer to fire at TICKS_PER_SEC rate
    SysTick_Config((SystemCoreClock / BSP::TICKS_PER_SEC) + 1U);

    // set priorities of ISRs used in the system
    NVIC_SetPriority(SysTick_IRQn, 0U);
    // ...
}
//............................................................................
void QF::onCleanup() {
}
//............................................................................
void QV::onIdle() { // CAUTION: called with interrupts DISABLED
    BSP::d7on(); // LED LD2
#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    //
    BSP::d7off();
    QV_CPU_SLEEP(); // atomically go to sleep and enable interrupts
    BSP::d7on();
#else
    QF_INT_ENABLE(); // just enable interrupts
#endif
    BSP::d7off();
}

} // namespace QP
