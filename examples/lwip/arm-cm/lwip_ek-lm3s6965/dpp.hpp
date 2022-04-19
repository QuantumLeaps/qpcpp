//============================================================================
// Product: DPP example with lwIP
// Last updated for version 6.6.0
// Last updated on  2019-07-30
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#ifndef DPP_HPP
#define DPP_HPP

using namespace QP;

enum DPPSignals {
   EAT_SIG = Q_USER_SIG, // published by Table to let a philosopher eat
   DONE_SIG,             // published by Philosopher when done eating
   BTN_DOWN_SIG,      // published by ISR_SysTick when user button is pressed
   BTN_UP_SIG,        // published by ISR_SysTick when user button is released

   DISPLAY_IPADDR_SIG,   // published by lwIPMgr to display IP addres
   DISPLAY_CGI_SIG,      // published by lwIPMgr to display CGI text
   DISPLAY_UDP_SIG,      // published by lwIPMgr to display UDP text
   MAX_PUB_SIG,          // the last published signal

   HUNGRY_SIG,           // posted direclty to Table from hungry Philosopher
   SEND_UDP_SIG,         // posted directly to lwIPMgr to send text via UDP
   LWIP_DRIVER_GROUP,    // LwIP driver signal group
   LWIP_DRIVER_END = LWIP_DRIVER_GROUP + 10, // reserve 10 signals
   LWIP_SLOW_TICK_SIG,   // slow tick signal for LwIP manager
   MAX_SIG               // the last signal
};

struct TableEvt : public QEvt {
    uint8_t philoNum; // philosopher number
};

#define MAX_TEXT_LEN 16
struct TextEvt : public QEvt {
    char text[MAX_TEXT_LEN]; // text to deliver
};

// number of philosophers
#define N_PHILO 5U

extern QActive * const AO_Philo[N_PHILO]; // "opaque" pointers to Philo  AO
extern QActive * const AO_Table;          // "opaque" pointer to Table   AO
extern QActive * const AO_LwIPMgr;        // "opaque" pointer to LwIPMgr AO

#endif // DPP_HPP
