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
//! @date Last updated on: 2022-08-25
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief DPP example (BSP)

#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
// add other drivers if necessary...

// The devicetree node identifier for the "led0" alias.
#define LED0_NODE DT_ALIAS(led0)

namespace { // unnamed local namespace

Q_DEFINE_THIS_FILE // define the name of this file for assertions

static struct gpio_dt_spec const l_led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static struct k_timer QF_tick_timer;
static std::uint32_t l_rnd; // random seed

#ifdef Q_SPY

    // QS source IDs
    static QP::QSpyId const timerID = { 0U };

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        PAUSED_STAT,
        COMMAND_STAT,
        TEST_MSG
    };

#endif

static void QF_tick_function(struct k_timer *tid) {
    static_cast<void>(tid); // unused parameter
    QP::QTimeEvt::TICK_X(0U, &timerID);
}

} // unnamed local namespace

// namespace DPP ============================================================
namespace DPP {

// BSP functions .............................................................
void BSP::init(void) {
   int ret = gpio_pin_configure_dt(&l_led0, GPIO_OUTPUT_ACTIVE);
    Q_ASSERT(ret >= 0);

    k_timer_init(&QF_tick_timer, &QF_tick_function, NULL);

    //...
    BSP::randomSeed(1234U);

    // initialize the QS software tracing...
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }

    // object dictionaries...
    QS_OBJ_DICTIONARY(AO_Table);
    for (int n = 0; n < N_PHILO; ++n) {
        QS_OBJ_ARR_DICTIONARY(AO_Philo[n], n);
    }

    QS_OBJ_DICTIONARY(&timerID);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);
    QS_USR_DICTIONARY(COMMAND_STAT);
    QS_USR_DICTIONARY(TEST_MSG);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
    QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
    QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records
    QS_GLB_FILTER(TEST_MSG);
}
//............................................................................
void BSP::ledOn(void) {
    gpio_pin_set_dt(&l_led0, true);
}
//............................................................................
void BSP::ledOff(void) {
    gpio_pin_set_dt(&l_led0, false);
}
//............................................................................
void BSP::displayPhilStat(std::uint8_t n, char const *stat) {
    if (stat[0] == 'e') {
        ledOn();
    }
    else {
        ledOff();
    }
    Q_PRINTK("Philo[%d]->%s\n", n, stat);

    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()          // application-specific record end
}
//............................................................................
void BSP::displayPaused(std::uint8_t paused) {
    Q_UNUSED_PAR(paused);
}
//............................................................................
std::uint32_t BSP::random(void) { // a cheap pseudo-random-number generator
    // flating point code is to exercise the FPU
    double volatile x = 3.1415926F;
    x = x + 2.7182818F;

    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    std::uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time

    return (rnd >> 8);
}
//............................................................................
void BSP::randomSeed(uint32_t seed) {
    l_rnd = seed;
}

//............................................................................
void BSP::terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}

} // namespace DPP


// namespace QP **************************************************************
namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    k_timer_start(&QF_tick_timer, K_MSEC(1), K_MSEC(1));
    Q_PRINTK("QF::onStartup()");
}
//............................................................................
void QF::onCleanup(void) {
    Q_PRINTK("QF::onCleanup()");
}

// QS callbacks ==============================================================
#ifdef Q_SPY

#include <zephyr/drivers/uart.h>

// select the Zephyr shell UART
// NOTE: you can change this to other UART peripheral if desired
static struct device const *uart_dev =
     DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

//............................................................................
static void uart_cb(const struct device *dev, void *user_data) {
    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (uart_irq_rx_ready(uart_dev)) {
        std::uint8_t buf[32];
        int n = uart_fifo_read(uart_dev, buf, sizeof(buf));
        for (std::uint8_t const *p = buf; n > 0; --n, ++p) {
            QS::rxPut(*p);
        }
    }
}
//............................................................................
bool QS::onStartup(void const *arg) {
    static std::uint8_t qsTxBuf[2*1024]; // buffer for QS transmit channel
    static std::uint8_t qsRxBuf[256];    // buffer for QS receive channel

    Q_REQUIRE(uart_dev != NULL);

    initBuf  (qsTxBuf, sizeof(qsTxBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // configure interrupt and callback to receive data
    uart_irq_callback_user_data_set(uart_dev, &uart_cb, nullptr);
    uart_irq_rx_enable(uart_dev);

    return true; // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {  // NOTE: invoked with interrupts DISABLED
    return k_cycle_get_32();
}
//............................................................................
void QS::onFlush(void) {
    std::uint16_t len = 0xFFFFU; // to get as many bytes as available
    std::uint8_t const *buf;
    while ((buf = getBlock(&len)) != nullptr) { // QS-TX data available?
        for (; len != 0U; --len, ++buf) {
            uart_poll_out(uart_dev, *buf);
        }
        len = 0xFFFFU; // to get as many bytes as available
    }
}
//............................................................................
void QS::doOutput(void) {
    std::uint16_t len = 0xFFFFU; // big number to get all available bytes

    QS_CRIT_STAT_
    QS_CRIT_E_();
    std::uint8_t const *buf = getBlock(&len);
    QS_CRIT_X_();

    // transmit the bytes via the UART...
    for (; len != 0U; --len, ++buf) {
        uart_poll_out(uart_dev, *buf);
    }
}
//............................................................................
//! callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    sys_reboot(SYS_REBOOT_COLD);
}
//............................................................................
//! callback function to execute a user command (to be implemented in BSP)
extern "C" void assert_failed(char const *module, int loc);
void QS::onCommand(std::uint8_t cmdId, std::uint32_t param1,
                   std::uint32_t param2, std::uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

//============================================================================
extern "C" {

Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    //
    // NOTE: add here your application-specific error handling
    //
    QS_ASSERTION(module, loc, static_cast<std::uint32_t>(10000U));
    Q_PRINTK("\nASSERTION in %s:%d\n", module, loc);

#ifndef NDEBUG
    k_panic(); // debug build: halt the system for error search...
#else
    sys_reboot(SYS_REBOOT_COLD); // release build: reboot the system
#endif
}

} // extern "C"
