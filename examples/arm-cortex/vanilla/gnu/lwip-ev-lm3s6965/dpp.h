//////////////////////////////////////////////////////////////////////////////
// Product: DPP example with lwIP
// Last Updated for Version: 4.0.03
// Date of the Last Update:  Mar 16, 2009
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#ifndef dpp_h
#define dpp_h

enum DPPSignals {
   EAT_SIG = Q_USER_SIG,        // published by Table to let a philosopher eat
   DONE_SIG,                      // published by Philosopher when done eating
   BTN_DOWN_SIG,       // published by ISR_SysTick when user button is pressed
   BTN_UP_SIG,        // published by ISR_SysTick when user button is released

   DISPLAY_IPADDR_SIG,            // published by lwIPMgr to display IP addres
   DISPLAY_CGI_SIG,                // published by lwIPMgr to display CGI text
   DISPLAY_UDP_SIG,                // published by lwIPMgr to display UDP text
   MAX_PUB_SIG,                                   // the last published signal

   HUNGRY_SIG,             // posted direclty to Table from hungry Philosopher
   SEND_UDP_SIG,            // posted directly to lwIPMgr to send text via UDP
   MAX_SIG                                                  // the last signal
};

struct TableEvt : public QEvent {
    uint8_t philoNum;                                    // philosopher number
};

#define MAX_TEXT_LEN 16
struct TextEvt : public QEvent {
    char text[MAX_TEXT_LEN];                                // text to deliver
};

                                                     // number of philosophers
#define N_PHILO 5

extern QActive * const AO_Philo[N_PHILO];    // "opaque" pointers to Philo  AO
extern QActive * const AO_Table;             // "opaque" pointer to Table   AO
extern QActive * const AO_LwIPMgr;           // "opaque" pointer to LwIPMgr AO

#endif                                                                // dpp_h
