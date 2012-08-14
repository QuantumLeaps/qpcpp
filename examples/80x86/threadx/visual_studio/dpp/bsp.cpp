//////////////////////////////////////////////////////////////////////////////
// Product: DPP example
// Last Updated for Version: 4.4.00
// Date of the Last Update:  Apr 13, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

Q_DEFINE_THIS_FILE

// local variables -----------------------------------------------------------
static uint8_t l_running;

#ifdef Q_SPY

enum QSUserRecords {
    PHILO_STAT = QP::QS_USER
};

static SOCKET  l_sock = INVALID_SOCKET;
#endif

//............................................................................
void BSP_init(int argc, char *argv[]) {
    char const *hostAndPort = "localhost:6601";

    printf("Dining Philosopher Problem example"
           "\nQEP %s\nQF  %s\n"
           "Press ESC to quit...\n",
           QP::QEP::getVersion(),
           QP::QF::getVersion());
    if (argc > 1) {                                         // port specified?
        hostAndPort = argv[1];
    }
    if (!QS_INIT(hostAndPort)) {
        printf("\nUnable to open QSpy socket\n");
        exit(-1);
    }
}
//............................................................................
void QP::QF::onStartup(void) {
}
//............................................................................
void QP::QF::onCleanup(void) {
    QS_EXIT();
    exit(0);
}
//............................................................................
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    printf("Philosopher %2d is %s\n", (int)n, stat);

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    QP::QF::stop();
}

//----------------------------------------------------------------------------
#ifdef Q_SPY                                            // define QS callbacks

bool QP::QS::onStartup(void const *arg) {
    static uint8_t qsBuf[1024];                   // 1K buffer for Quantum Spy
    static WSADATA wsaData;
    char host[64];
    char const *src;
    char *dst;
    USHORT port = 6601;                                        // default port
    ULONG ioctl_opt = 1;
    struct sockaddr_in servAddr;
    struct hostent *server;

    initBuf(qsBuf, sizeof(qsBuf));

    // initialize Windows sockets
    if (WSAStartup(MAKEWORD(2,0), &wsaData) == SOCKET_ERROR) {
        printf("Windows Sockets cannot be initialized.");
        return (uint8_t)0;
    }

    src = (char const *)arg;
    dst = host;
    while ((*src != '\0') && (*src != ':') && (dst < &host[sizeof(host)])) {
        *dst++ = *src++;
    }
    *dst = '\0';
    if (*src == ':') {
        port = (USHORT)strtoul(src + 1, NULL, 10);
    }


    l_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);          // TCP socket
    if (l_sock == INVALID_SOCKET){
        printf("Socket cannot be created.\n"
               "Windows socket error 0x%08X.",
               WSAGetLastError());
        return (uint8_t)0;
    }

    server = gethostbyname(host);
    if (server == NULL) {
        printf("QSpy host name cannot be resolved.\n"
               "Windows socket error 0x%08X.",
               WSAGetLastError());
        return (uint8_t)0;
    }
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    memcpy(&servAddr.sin_addr, server->h_addr, server->h_length);
    servAddr.sin_port = htons(port);
    if (connect(l_sock, (struct sockaddr *)&servAddr, sizeof(servAddr))
        == SOCKET_ERROR)
    {
        printf("Socket cannot be connected to the QSpy server.\n"
               "Windows socket error 0x%08X.",
               WSAGetLastError());
        QS_EXIT();
        return (uint8_t)0;
    }

    // Set the socket to non-blocking mode
    if (ioctlsocket(l_sock, FIONBIO, &ioctl_opt) == SOCKET_ERROR) {
        printf("Socket configuration failed.\n"
               "Windows socket error 0x%08X.",
               WSAGetLastError());
        QS_EXIT();
        return (uint8_t)0;
    }

    QS_FILTER_ON(QS_ALL_RECORDS);
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);
    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QK_SCHEDULE);
                                                     // object dictionaries...
    return true;                                                    // success
}
//............................................................................
void QP::QS::onCleanup(void) {
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
    }
    WSACleanup();
}
//............................................................................
void QP::QS::onFlush(void) {
    uint16_t nBytes = 1000;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        send(l_sock, (char const *)block, nBytes, 0);
        nBytes = 1000;
    }
}
//............................................................................
QP::QSTimeCtr QP::QS::onGetTime(void) {
    return static_cast<QP::QSTimeCtr>(clock());
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------
