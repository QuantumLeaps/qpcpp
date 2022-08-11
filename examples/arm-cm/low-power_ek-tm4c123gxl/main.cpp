//============================================================================
// DPP example
// Last updated for version 6.4.0
// Last updated on  2019-02-23
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
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
#include "qpcpp.hpp"
#include "low_power.hpp"
#include "bsp.hpp"

//............................................................................
int main() {
    static QP::QSubscrList subscrSto[MAX_PUB_SIG];

    static QP::QEvt const *blinky0QueueSto[10]; // queue storage for Blinky0
#ifdef QXK_HPP // QXK kernel?
    static uint32_t const *xblinky1Stack[64]; // stack for XBlinky1
#else
    static QP::QEvt const *blinky1QueueSto[10]; // queue storage for Blinky1
#endif

    QP::QF::init();  // initialize the framework and the underlying RT kernel

    BSP_init(); // initialize the BSP

    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto)); // init publish-subscribe

    // initialize event pools...
    //QP::QF::poolInit(smlPoolSto,
    //                 sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the xthreads/active objects...
    AO_Blinky0->start(1U, // priority
                      blinky0QueueSto, Q_DIM(blinky0QueueSto),
                      0, 0U, 0);

#ifdef QXK_HPP // QXK kernel?
    XSEM_sw1.init(0U, 1U); /* binary signaling semaphore */
    XT_Blinky1.start(2U,     /* unique QP priority of the AO */
                  0, 0U, /* event queue (not used) */
                  xblinky1Stack,  /* stack storage (must provide in QXK) */
                  sizeof(xblinky1Stack), /* stack size [bytes] */
                  0);     /* initial event (or 0) */
#else // QV or QK kernels
    AO_Blinky1->start(2U, // priority
                    blinky1QueueSto, Q_DIM(blinky1QueueSto),
                    0, 0U, 0);
#endif

    return QP::QF::run(); // run the QF application
}
