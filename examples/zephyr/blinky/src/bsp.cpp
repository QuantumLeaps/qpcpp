//============================================================================
// Product: Blinky example, Zephyr RTOS kernel
// Last updated for: @ref qpcpp_7_3_0
// Last updated on  2023-08-24
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
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
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"             // QP/C++ real-time embedded framework
#include "blinky.hpp"            // Blinky Application interface
#include "bsp.hpp"               // Board Support Package

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
// add other drivers if necessary...

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS(led0)

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

// Local-scope objects -----------------------------------------------------
namespace { // unnamed local namespace

Q_DEFINE_THIS_FILE // define the name of this file for assertions

static struct gpio_dt_spec const l_led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static struct k_timer zephyr_tick_timer;

} // unnamed local namespace

//============================================================================
// Error handler

extern "C" {

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the assertion handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U);
    Q_PRINTK("\nERROR in %s:%d\n", module, id);

#ifndef NDEBUG
    k_panic(); // debug build: halt the system for error search...
#else
    sys_reboot(SYS_REBOOT_COLD); // release build: reboot the system
#endif
    for (;;) { // explicitly no-return
    }
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

//............................................................................
static void zephyr_tick_function(struct k_timer *tid); // prototype
static void zephyr_tick_function(struct k_timer *tid) {
    Q_UNUSED_PAR(tid);

    QP::QTimeEvt::TICK_X(0U, &timerID);
}

} // extern "C"

//============================================================================
namespace BSP {

void init() {
    int ret = gpio_pin_configure_dt(&l_led0, GPIO_OUTPUT_ACTIVE);
    Q_ASSERT(ret >= 0);

    k_timer_init(&zephyr_tick_timer, &zephyr_tick_function, nullptr);
}
//............................................................................
void start() {
    // initialize event pools
    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[10];
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // start AOs/threads...

    static QP::QEvt const *blinkyQueueSto[10];
    static K_THREAD_STACK_DEFINE(blinkyStack, 1024);
    APP::AO_Blinky->start(
        1U,                         // QP prio. of the AO
        blinkyQueueSto,             // event queue storage
        Q_DIM(blinkyQueueSto),      // queue length [events]
        blinkyStack,                // private stack for embOS
        K_THREAD_STACK_SIZEOF(blinkyStack), // stack size [Zephyr]
        nullptr);                   // no initialization param
}
//............................................................................
void ledOn() {
    gpio_pin_set_dt(&l_led0, true);
}
//............................................................................
void ledOff() {
    gpio_pin_set_dt(&l_led0, false);
}

} // namespace BSP

//============================================================================
namespace QP {

//............................................................................
void QF::onStartup() {
    k_timer_start(&zephyr_tick_timer, K_MSEC(1), K_MSEC(1));
    Q_PRINTK("QF::onStartup\n");
}
//............................................................................
void QF::onCleanup() {
    Q_PRINTK("QF::onCleanup\n");
}

} // namespace QP

