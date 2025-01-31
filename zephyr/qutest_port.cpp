//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef Q_SPY
    #error Q_SPY must be defined to compile qutest_cpp.cpp
#endif // Q_SPY

#define QP_IMPL        // this is QP implementation
#include "qp_port.hpp" // QP port
#include "qs_port.hpp" // QS port
#include "qs_pkg.hpp"  // QS package-scope interface
#include "qsafe.h"     // QP Functional Safety (FuSa) Subsystem

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/uart.h>
// add other drivers if necessary...

namespace {

//Q_DEFINE_THIS_MODULE("qutest_port")

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

} // unnamed namespace

// QS callbacks ==============================================================
namespace QP {

bool QS::onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    Q_REQUIRE(uart_dev != nullptr);

    static std::uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    initBuf(qsTxBuf, sizeof(qsTxBuf));

    static std::uint8_t qsRxBuf[128];  // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // configure interrupt and callback to receive data
    uart_irq_callback_user_data_set(uart_dev, &uart_cb, nullptr);
    uart_irq_rx_enable(uart_dev);

    return true; // return success
}
//............................................................................
void QS::onCleanup() {
}
//............................................................................
QSTimeCtr QS::onGetTime() {  // NOTE: invoked inside a critical section
    return k_cycle_get_32();
}
//............................................................................
// NOTE:
// No critical section in QS::onFlush() to avoid nesting of critical sections
// in case QS::onFlush() is called from Q_onError().
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
void QS::doOutput() {
    std::uint16_t len = 0xFFFFU; // big number to get all available bytes

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    std::uint8_t const *buf = getBlock(&len);
    QF_CRIT_EXIT();

    // transmit the bytes via the UART...
    for (; len != 0U; --len, ++buf) {
        uart_poll_out(uart_dev, *buf);
    }
}
//............................................................................
// callback function to reset the target (to be implemented in the BSP)
void QS::onReset(void) {
    sys_reboot(SYS_REBOOT_COLD);
}
//............................................................................
void QS::onTestLoop() {
    rxPriv_.inTestLoop = true;
    while (rxPriv_.inTestLoop) {
        rxParse();  // parse all the received bytes
        doOutput(); // perform the QS-TX
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    rxPriv_.inTestLoop = true;
}

} // namespace QP
