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
//! @date Last updated on: 2023-08-21
//! @version Last updated for: @ref qpcpp_7_3_0
//!
//! @file
//! @brief QUTEST port for POSIX, GNU

// expose features from the 2008 POSIX standard (IEEE Standard 1003.1-2008)
#define _POSIX_C_SOURCE 200809L

#ifndef Q_SPY
    #error "Q_SPY must be defined for QUTest application"
#endif // Q_SPY

#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#include "qs_port.hpp"      // QS port

#include "safe_std.h"       // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>

#define QS_TX_SIZE     (8*1024)
#define QS_RX_SIZE     (2*1024)
#define QS_TX_CHUNK    QS_TX_SIZE
#define QS_TIMEOUT_MS  10L

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

namespace { // unnamed local namespace

//Q_DEFINE_THIS_MODULE("qutest_port")

// local variables ...........................................................
static int l_sock = INVALID_SOCKET;
static struct timespec const c_timeout = { 0, QS_TIMEOUT_MS*1000000L };

static void sigIntHandler(int); // prototype
static void sigIntHandler(int) {
    QP::QS::onCleanup();
    //PRINTF_S("\n%s\n","<TARGET> disconnecting from QSPY");
    exit(-1);
}

} // unnamed local namespace

//============================================================================
namespace QP {

//............................................................................
bool QS::onStartup(void const *arg) {

    static uint8_t qsBuf[QS_TX_SIZE];   // buffer for QS-TX channel
    initBuf(qsBuf, sizeof(qsBuf));

    static uint8_t qsRxBuf[QS_RX_SIZE]; // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    char hostName[128];
    char const *serviceName = "6601";   // default QSPY server port
    char const *src;
    char *dst;
    int status;

    struct addrinfo *result = NULL;
    struct addrinfo *rp = NULL;
    struct addrinfo hints;
    int sockopt_bool;

    // extract hostName from 'arg' (hostName:port_remote)...
    src = (arg != nullptr)
          ? static_cast<char const *>(arg)
          : "localhost"; // default QSPY host
    dst = hostName;
    while ((*src != '\0')
           && (*src != ':')
           && (dst < &hostName[sizeof(hostName) - 1]))
    {
        *dst++ = *src++;
    }
    *dst = '\0'; // zero-terminate hostName

    // extract serviceName from 'arg' (hostName:serviceName)...
    if (*src == ':') {
        serviceName = src + 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    status = getaddrinfo(hostName, serviceName, &hints, &result);
    if (status != 0) {
        FPRINTF_S(stderr,
            "<TARGET> ERROR   cannot resolve host Name=%s:%s,Err=%d\n",
                    hostName, serviceName, status);
        goto error;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        l_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (l_sock != INVALID_SOCKET) {
            if (connect(l_sock, rp->ai_addr, rp->ai_addrlen)
                == SOCKET_ERROR)
            {
                close(l_sock);
                l_sock = INVALID_SOCKET;
            }
            break;
        }
    }

    freeaddrinfo(result);

    // socket could not be opened & connected?
    if (l_sock == INVALID_SOCKET) {
        FPRINTF_S(stderr, "<TARGET> ERROR   cannot connect to QSPY at "
            "host=%s:%s\n",
            hostName, serviceName);
        goto error;
    }

    // set the socket to non-blocking mode
    status = fcntl(l_sock, F_GETFL, 0);
    if (status == -1) {
        FPRINTF_S(stderr,
            "<TARGET> ERROR   Socket configuration failed errno=%d\n",
            errno);
        QS_EXIT();
        goto error;
    }
    if (fcntl(l_sock, F_SETFL, status | O_NONBLOCK) != 0) {
        FPRINTF_S(stderr, "<TARGET> ERROR   Failed to set non-blocking socket "
            "errno=%d\n", errno);
        QF::stop(); // <== stop and exit the application
        goto error;
    }

    // configure the socket to reuse the address and not to linger
    sockopt_bool = 1;
    setsockopt(l_sock, SOL_SOCKET, SO_REUSEADDR,
               &sockopt_bool, sizeof(sockopt_bool));
    sockopt_bool = 0; // negative option
    setsockopt(l_sock, SOL_SOCKET, SO_LINGER,
               &sockopt_bool, sizeof(sockopt_bool));
    onFlush();

    // install the SIGINT (Ctrl-C) signal handler
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = &sigIntHandler;
    sigaction(SIGINT, &sig_act, NULL);

    return true; // success

error:
    return false; // failure
}
//............................................................................
void QS::onCleanup() {
    nanosleep(&c_timeout, NULL); // allow the last QS output to come out
    if (l_sock != INVALID_SOCKET) {
        close(l_sock);
        l_sock = INVALID_SOCKET;
    }
    //PRINTF_S("%s\n", "<TARGET> Disconnected from QSPY");
}
//............................................................................
void QS::onReset() {
    onCleanup();
    //PRINTF_S("\n%s\n", "QS_onReset");
    exit(0);
}
//............................................................................
void QS::onFlush() {
    if (l_sock == INVALID_SOCKET) { // socket NOT initialized?
        FPRINTF_S(stderr, "<TARGET> ERROR   %s\n",
                  "invalid TCP socket");
        QF::stop(); // <== stop and exit the application
        return;
    }

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    std::uint16_t nBytes = QS_TX_CHUNK;
    std::uint8_t const *data;
    while ((data = getBlock(&nBytes)) != nullptr) {
        QS_CRIT_EXIT();
        for (;;) { // for-ever until break or return
            int nSent = send(l_sock, (char const *)data, (int)nBytes, 0);
            if (nSent == SOCKET_ERROR) { // sending failed?
                if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                    // sleep for the timeout and then loop back
                    // to send() the SAME data again
                    nanosleep(&c_timeout, NULL);
                }
                else { // some other socket error...
                    FPRINTF_S(stderr, "<TARGET> ERROR   sending data over TCP,"
                           "errno=%d\n", errno);
                    QF::stop(); // <== stop and exit the application
                    return;
                }
            }
            else if (nSent < (int)nBytes) { // sent fewer than requested?
                nanosleep(&c_timeout, NULL); // sleep for the timeout
                // adjust the data and loop back to send() the rest
                data   += nSent;
                nBytes -= (uint16_t)nSent;
            }
            else {
                break; // break out of the for-ever loop
            }
        }
        // set nBytes for the next call to QS::getBlock()
        nBytes = QS_TX_CHUNK;
        QS_CRIT_ENTRY();
    }
    QS_CRIT_EXIT();
}
//............................................................................
void QS::onTestLoop() {
    fd_set readSet;
    FD_ZERO(&readSet);

    rxPriv_.inTestLoop = true;
    while (rxPriv_.inTestLoop) {
        FD_SET(l_sock, &readSet);

        struct timeval timeout = {
            (long)0, (long)(QS_TIMEOUT_MS * 1000)
        };

        // selective, timed blocking on the TCP/IP socket...
        timeout.tv_usec = (long)(QS_TIMEOUT_MS * 1000);
        int status = select(l_sock + 1, &readSet,
                      (fd_set *)0, (fd_set *)0, &timeout);
        if (status < 0) {
            FPRINTF_S(stderr, "<TARGET> ERROR socket select,errno=%d\n",
                errno);
            QF::stop(); // <== stop and exit the application
        }
        else if ((status > 0) && FD_ISSET(l_sock, &readSet)) { //socket ready?
            status = recv(l_sock,
                          (char *)QS::rxPriv_.buf, (int)QS::rxPriv_.end, 0);
            if (status > 0) { // any data received?
                QS::rxPriv_.tail = 0U;
                QS::rxPriv_.head = status; // # bytes received
                QS::rxParse(); // parse all received bytes
            }
        }

        onFlush();
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    rxPriv_.inTestLoop = true;
}

} // namespace QP

