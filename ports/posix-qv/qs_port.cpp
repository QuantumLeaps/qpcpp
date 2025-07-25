//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
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
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C++ software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C++ in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================

// expose features from the 2008 POSIX standard (IEEE Standard 1003.1-2008)
#define _POSIX_C_SOURCE 200809L

#ifndef Q_SPY
    #error Q_SPY must be defined to compile qs_port.cpp
#endif // Q_SPY

#define QP_IMPL        // this is QP implementation
#include "qp_port.hpp" // QP port
#include "qsafe.h"     // QP Functional Safety (FuSa) Subsystem
#include "qs_port.hpp" // QS port

#include "safe_std.h"  // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define QS_TX_SIZE     (8*1024)
#define QS_RX_SIZE     (2*1024)
#define QS_TX_CHUNK    QS_TX_SIZE
#define QS_TIMEOUT_MS  10L

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

namespace { // unnamed local namespace

//DEFINE_THIS_MODULE("qs_port")

// local variables ...........................................................
static int l_sock = INVALID_SOCKET;
static struct timespec const c_timeout = { 0, QS_TIMEOUT_MS*1000000L };

static char *l_rxBuf;
static int   l_rxBufLen;

} // unnamed local namespace

//============================================================================
namespace QP {

//............................................................................
bool QS::onStartup(void const *arg) {
    char hostName[128];
    char const *serviceName = "6601";  // default QSPY server port
    char const *src;
    char *dst;
    int status;

    struct addrinfo *result = NULL;
    struct addrinfo *rp = NULL;
    struct addrinfo hints;
    int sockopt_bool;

    // initialize the QS transmit and receive buffers
    static std::uint8_t qsBuf[QS_TX_SIZE];   // buffer for QS-TX channel
    initBuf(qsBuf, sizeof(qsBuf));

    static std::uint8_t qsRxBuf[QS_RX_SIZE]; // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
    l_rxBuf    = (char *)qsRxBuf;
    l_rxBufLen = (int)sizeof(qsRxBuf);

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

    return true; // success

error:
    return false; // failure
}
//............................................................................
void QS::onCleanup() {
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
    // NOTE:
    // No critical section in QS::onFlush() to avoid nesting of critical
    // sections in case QS::onFlush() is called from Q_onError().

    if (l_sock == INVALID_SOCKET) { // socket NOT initialized?
        FPRINTF_S(stderr, "<TARGET> ERROR   %s\n",
                  "invalid TCP socket");
        QF::stop(); // <== stop and exit the application
        return;
    }

    std::uint16_t nBytes = QS_TX_CHUNK;
    std::uint8_t const *data;
    while ((data = getBlock(&nBytes)) != nullptr) {
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
                break;
            }
        }
        // set nBytes for the next call to QS::getBlock()
        nBytes = QS_TX_CHUNK;
    }
}
//............................................................................
QSTimeCtr QS::onGetTime() {
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);

    // convert to units of 0.1 microsecond
    QSTimeCtr time = (QSTimeCtr)(tspec.tv_sec * 10000000 + tspec.tv_nsec / 100);
    return time;
}

//............................................................................
void QS::doOutput() {
    if (l_sock == INVALID_SOCKET) { // socket NOT initialized?
        FPRINTF_S(stderr, "<TARGET> ERROR   %s\n",
                  "invalid TCP socket");
        QF::stop(); // <== stop and exit the application
        return;
    }

    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    std::uint16_t nBytes = QS_TX_CHUNK;
    std::uint8_t const *data = getBlock(&nBytes);
    QS_CRIT_EXIT();

    if (nBytes > 0U) { // any bytes to send?
        for (;;) { // for-ever until break or return
            int nSent = send(l_sock, reinterpret_cast<char const *>(data),
                             static_cast<int>(nBytes), 0);
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
    }
}
//............................................................................
void QS::doInput(void) {
    int len = recv(l_sock, l_rxBuf, l_rxBufLen, 0);
    if (len > 0) { // any data received?
        QS::rxParseBuf(static_cast<std::uint16_t>(len));
    }
}

} // namespace QP
