//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005-2026 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: LicenseRef-QL-commercial
//
// This software is licensed under the terms of the Quantum Leaps commercial
// license agreement. This header is a notice of that license, not a
// substitute for it; the signed agreement governs in case of conflict.
// Use of this software requires a valid Quantum Leaps commercial or
// evaluation license. Contact Quantum Leaps for licensing options.
//
// RESTRICTIONS
// Unless otherwise agreed in writing with Quantum Leaps, the licensee
// may NOT:
// (a) redistribute, encumber, sell, rent, lease, sublicense, or otherwise
//     transfer rights in this software to any third party,
// (b) remove, obscure, or alter any trademark, copyright, or other
//     proprietary notice, legend, symbol, or label present in this
//     software,
// (c) create derivative works, or modify or repackage this software, for
//     the purpose of concealing its origin or evading the licensing
//     obligations that apply to it,
// (d) use this software, or any portion of it, as input to train,
//     fine-tune, evaluate, or otherwise develop any artificial
//     intelligence or machine learning model, or use such a model to
//     replicate, emulate, or compete with the functionality of this
//     software, whether for internal use or in any product or service
//     offered to a third party.
//
// This software is provided "AS IS," without warranty of any kind, express
// or implied, except as expressly stated in the governing license agreement.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================

#ifndef Q_SPY
    #error Q_SPY must be defined to compile qs_port.cpp
#endif // Q_SPY

#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#include "qs_port.hpp"      // include QS port

#include "safe_std.h"       // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>
#include <time.h>

// Minimum required Windows version is Windows-XP or newer (0x0501)
#ifdef WINVER
#undef WINVER
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define WINVER _WIN32_WINNT_WINXP
#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include <ws2tcpip.h>

#define QS_TX_SIZE     (8*1024)
#define QS_RX_SIZE     (2*1024)
#define QS_TX_CHUNK    QS_TX_SIZE
#define QS_TIMEOUT_MS  10U
#define QS_FLUSH_TOUT_MS   5000L // max wait for QSPY to accept output [ms]
#define QS_EXIT_QSPY_LOST  2     // exit code: the QSPY connection was lost

namespace { // unnamed local namespace

//Q_DEFINE_THIS_MODULE("qs_port")

// local variables ...........................................................
static SOCKET l_sock = INVALID_SOCKET;

static char *l_rxBuf;
static std::size_t l_rxBufLen;

// The connection to QSPY has been lost (e.g., QSPY has terminated, or QSPY
// has replaced this Target by a new one): report it and terminate cleanly
// with the exit code QS_EXIT_QSPY_LOST (consistent with the QUTest port).
static void connectionLost(void) {
    FPRINTF_S(stderr, "%s\n", "<TARGET> ERROR QSPY connection lost");
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
    exit(QS_EXIT_QSPY_LOST);
}

// sends ALL 'len' bytes to QSPY; terminates (connectionLost()) when the
// connection is lost or QSPY does not accept data for QS_FLUSH_TOUT_MS
static void sendAll(std::uint8_t const *data, int len) {
    long waited = 0L; // time waited for QSPY to accept data [ms]
    while (len > 0) {
        int const nSent = send(l_sock, reinterpret_cast<char const *>(data), len, 0);
        if (nSent > 0) { // some data sent?
            data  += nSent;
            len   -= nSent;
            waited = 0L; // progress made
        }
        else if ((nSent == 0) || (WSAGetLastError() == WSAEWOULDBLOCK)) {
            // QSPY does not accept data at this time...
            if (waited >= QS_FLUSH_TOUT_MS) {
                FPRINTF_S(stderr, "%s\n",
                    "<TARGET> ERROR QSPY does not accept data");
                connectionLost();
            }
            // sleep for the timeout and then loop back
            // to send() the SAME data again
            Sleep(QS_TIMEOUT_MS);
            waited += static_cast<long>(QS_TIMEOUT_MS);
        }
        else { // some other socket error (e.g., QSPY is gone)
            FPRINTF_S(stderr,
                "<TARGET> ERROR   sending data over TCP,WSAErr=%d\n",
                WSAGetLastError());
            connectionLost();
        }
    }
}

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

    struct addrinfo *result = nullptr;
    struct addrinfo *rp = nullptr;
    struct addrinfo hints;
    ULONG  ioctl_opt;
    WSADATA wsaData;

    // initialize the QS transmit and receive buffers
    static std::uint8_t qsBuf[QS_TX_SIZE];   // buffer for QS-TX channel
    initBuf(qsBuf, sizeof(qsBuf));

    static std::uint8_t qsRxBuf[QS_RX_SIZE]; // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
    l_rxBuf    = reinterpret_cast<char *>(qsRxBuf);
    l_rxBufLen = sizeof(qsRxBuf);

    // initialize Windows sockets version 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        FPRINTF_S(stderr, "%s\n",
            "<TARGET> ERROR Windows Sockets cannot be initialized");
        goto error;
    }

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
    //PRINTF_S("<TARGET> Connecting to QSPY on Host=%s:%s...\n",
    //         hostName, serviceName);

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

    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        l_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (l_sock != INVALID_SOCKET) {
            if (connect(l_sock, rp->ai_addr, static_cast<int>(rp->ai_addrlen))
                == SOCKET_ERROR)
            {
                closesocket(l_sock);
                l_sock = INVALID_SOCKET;
            }
            break;
        }
    }

    freeaddrinfo(result);

    // socket could not be opened & connected?
    if (l_sock == INVALID_SOCKET) {
        FPRINTF_S(stderr,
            "<TARGET> ERROR   cannot connect to QSPY at host=%s:%s\n",
            hostName, serviceName);
        goto error;
    }

    // set the socket to non-blocking mode
    ioctl_opt = 1;
    if (ioctlsocket(l_sock, static_cast<long int>(FIONBIO), &ioctl_opt)
        != NO_ERROR)
    {
        FPRINTF_S(stderr,
            "<TARGET> ERROR   Failed to set non-blocking socket WSAErr=%d\n",
            WSAGetLastError());
        goto error;
    }

    // configure the socket to reuse the address and not to linger

    //PRINTF_S("<TARGET> Connected to QSPY at Host=%s:%d\n",
    //       hostName, port_remote);
    onFlush();

    return true; // success

error:
    return false; // failure
}
//............................................................................
void QS::onCleanup() {
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
    WSACleanup();
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
        FPRINTF_S(stderr, "%s\n", "<TARGET> ERROR   invalid TCP socket");
        return;
    }

    std::uint16_t nBytes = QS_TX_CHUNK;
    std::uint8_t const *data;
    while ((data = getBlock(&nBytes)) != nullptr) {
        sendAll(data, static_cast<int>(nBytes)); // NOTE: exits if QSPY is lost
        nBytes = QS_TX_CHUNK; // for the next call to getBlock()
    }
}
//............................................................................
QSTimeCtr QS::onGetTime() {
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return static_cast<QSTimeCtr>(time.QuadPart);
}

//............................................................................
void QS::doOutput() {
    if (l_sock == INVALID_SOCKET) { // socket NOT initialized?
        FPRINTF_S(stderr, "%s\n", "<TARGET> ERROR   invalid TCP socket");
        return;
    }

    std::uint16_t nBytes = QS_TX_CHUNK;
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    std::uint8_t const *data = getBlock(&nBytes);
    QS_CRIT_EXIT();

    if (nBytes > 0U) { // any bytes to send?
        sendAll(data, static_cast<int>(nBytes)); // NOTE: exits if QSPY is lost
    }
}
//............................................................................
void QS::doInput() {
    if (l_sock == INVALID_SOCKET) { // socket NOT initialized?
        return;
    }
    int const len = recv(l_sock, l_rxBuf, static_cast<int>(l_rxBufLen), 0);
    if (len > 0) { // any data received?
        QS::rxParseBuf(static_cast<std::uint16_t>(len));
    }
    else if (len == 0) { // QSPY has closed the connection?
        connectionLost();
    }
    else if (WSAGetLastError() != WSAEWOULDBLOCK) {
        connectionLost(); // the connection has failed
    }
    else {
        // no input available at this time
    }
}

} // namespace QP
