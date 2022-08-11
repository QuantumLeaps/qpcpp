//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-08-06
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief Blinky example (BSP for Zephyr)

#include "qpcpp.hpp"
//#include "blinky.hpp"
#include "bsp.hpp"

#include <drivers/gpio.h>
#include <sys/reboot.h>
// add other drivers if necessary...

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

namespace {
    Q_DEFINE_THIS_FILE
}

using namespace QP;

// The devicetree node identifier for the "led0" alias
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0    DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN     DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS   DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
// A build error here means your board isn't set up to blink an LED.
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0    ""
#define PIN     0
#define FLAGS   0
#endif

static struct device const *dev_LED0;
static struct k_timer QF_tick_timer;
static void QF_tick_function(struct k_timer *tid) {
    (void)tid; /* unused parameter */
    QTimeEvt::TICK_X(0U, nullptr);
    //printk("tick\n");
}

// BSP functions =============================================================
void BSP_init(void) {
    dev_LED0 = device_get_binding(LED0);
    Q_ASSERT(dev_LED0 != NULL);

    int ret = gpio_pin_configure(dev_LED0, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
    Q_ASSERT(ret >= 0);

    k_timer_init(&QF_tick_timer, &QF_tick_function, nullptr);
}
//............................................................................
void BSP_ledOff(void) {
    printk("BSP_ledOn\n");
    gpio_pin_set(dev_LED0, PIN, true);
}
//............................................................................
void BSP_ledOn(void) {
    printk("BSP_ledOff\n");
    gpio_pin_set(dev_LED0, PIN, false);
}

// QF callbacks ==============================================================
void QF::onStartup(void) {
    k_timer_start(&QF_tick_timer, K_MSEC(1), K_MSEC(1));
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    printk("\nASSERTION in %s:%d\n", module, loc);
    QS_ASSERTION(module, loc, 10000U); /* report assertion to QS */
#ifndef NDEBUG
    k_panic(); /* debug build: halt the system for error search... */
#else
    sys_reboot(SYS_REBOOT_COLD); /* release build: reboot the system */
#endif
}

