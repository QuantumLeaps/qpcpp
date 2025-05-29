//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: LicenseRef-QL-commercial
//
// This software is licensed under the terms of the Quantum Leaps commercial
// licenses. Please contact Quantum Leaps for more information about the
// available licensing options.
//
// RESTRICTIONS
// You may NOT :
// (a) redistribute, encumber, sell, rent, lease, sublicense, or otherwise
//     transfer rights in this software,
// (b) remove or alter any trademark, logo, copyright or other proprietary
//     notices, legends, symbols or labels present in this software,
// (c) plagiarize this software to sidestep the licensing obligations.
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
#include <conio.h>

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
#define QS_TIMEOUT_MS  10

namespace { // unnamed local namespace

//Q_DEFINE_THIS_MODULE("qs_port")

// local variables ...........................................................
static SOCKET l_sock = INVALID_SOCKET;

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
    BOOL   sockopt_bool;
    ULONG  ioctl_opt;
    WSADATA wsaData;

    // initialize the QS transmit and receive buffers
    static std::uint8_t qsBuf[QS_TX_SIZE];   // buffer for QS-TX channel
    initBuf(qsBuf, sizeof(qsBuf));

    static std::uint8_t qsRxBuf[QS_RX_SIZE]; // buffer for QS-RX channel
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
    l_rxBuf    = (char *)qsRxBuf;
    l_rxBufLen = (int)sizeof(qsRxBuf);

    // initialize Windows sockets version 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        FPRINTF_S(stderr, "%s\n",
            "<TARGET> ERROR Windows Sockets cannot be initialized");
        goto error;
    }

    // extract hostName from 'arg' (hostName:port_remote)...
    src = (arg != (void const *)0)
          ? (char const *)arg
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

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        l_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (l_sock != INVALID_SOCKET) {
            if (connect(l_sock, rp->ai_addr,
                        static_cast<int>(rp->ai_addrlen)) == SOCKET_ERROR)
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
    if (ioctlsocket(l_sock, FIONBIO, &ioctl_opt) != NO_ERROR) {
        FPRINTF_S(stderr,
            "<TARGET> ERROR   Failed to set non-blocking socket WASErr=%d\n",
            WSAGetLastError());
        goto error;
    }

    // configure the socket to reuse the address and not to linger
    sockopt_bool = TRUE;
    setsockopt(l_sock, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&sockopt_bool, sizeof(sockopt_bool));
    sockopt_bool = TRUE;
    setsockopt(l_sock, SOL_SOCKET, SO_DONTLINGER,
               (const char *)&sockopt_bool, sizeof(sockopt_bool));

    //PRINTF_S("<TARGET> Connected to QSPY at Host=%s:%d\n",
    //       hostName, port_remote);
    onFlush();

    return true; // success

error:
    return false; // failure
}
//............................................................................
void QS::onCleanup(void) {
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
    WSACleanup();
    //PRINTF_S("%s\n", "<TARGET> Disconnected from QSPY");
}
//............................................................................
void QS::onReset(void) {
    onCleanup();
    exit(0);
}
//............................................................................
void QS::onFlush(void) {
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
        for (;;) { // for-ever until break or return
            int nSent = send(l_sock, (char const *)data, (int)nBytes, 0);
            if (nSent == SOCKET_ERROR) { // sending failed?
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    // sleep for the timeout and then loop back
                    // to send() the SAME data again
                    Sleep(QS_TIMEOUT_MS);
                }
                else { // some other socket error...
                    FPRINTF_S(stderr,
                        "<TARGET> ERROR   sending data over TCP,WASErr=%d\n",
                        err);
                    return;
                }
            }
            else if (nSent < (int)nBytes) { // sent fewer than requested?
                Sleep(QS_TIMEOUT_MS); // sleep for the timeout
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
QSTimeCtr QS::onGetTime(void) {
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return (QSTimeCtr)time.QuadPart;
}

//............................................................................
void QS::doOutput(void) {

    if (l_sock == INVALID_SOCKET) { // socket NOT initialized?
        FPRINTF_S(stderr, "%s\n", "<TARGET> ERROR   invalid TCP socket");
        return;
    }

    std::uint16_t nBytes = QS_TX_CHUNK;
    std::uint8_t const *data;
    QS_CRIT_STAT
    QS_CRIT_ENTRY();
    if ((data = QS::getBlock(&nBytes)) != (uint8_t *)0) {
        QS_CRIT_EXIT();
        for (;;) { // for-ever until break or return
            int nSent = send(l_sock, (char const *)data, (int)nBytes, 0);
            if (nSent == SOCKET_ERROR) { // sending failed?
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    // sleep for the timeout and then loop back
                    // to send() the SAME data again
                    //
                    Sleep(QS_TIMEOUT_MS);
                }
                else { // some other socket error...
                    FPRINTF_S(stderr,
                        "<TARGET> ERROR   sending data over TCP,WASErr=%d\n",
                        err);
                    return;
                }
            }
            else if (nSent < (int)nBytes) { // sent fewer than requested?
                Sleep(QS_TIMEOUT_MS); // sleep for the timeout
                // adjust the data and loop back to send() the rest
                data   += nSent;
                nBytes -= (uint16_t)nSent;
            }
            else {
                break;
            }
        }
    }
    else {
        QS_CRIT_EXIT();
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
