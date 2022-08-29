//============================================================================
// Product: System test fixture for QXK on the EFM32 target
// Last updated for version 6.9.1
// Last updated on  2020-09-21
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

//============================================================================
namespace {
static void Thread1_run(QP::QXThread * const me) {

    for (;;) {
        BSP::ledOn();
        QP::QXThread::delay(1U); /* BLOCK */
        BSP::ledOff();
        QP::QXThread::delay(1U); /* BLOCK */
    }
}
static QP::QXThread thr1(&Thread1_run, 0U);
}

//============================================================================
namespace QP {

void QS::onTestSetup(void) {
}
//............................................................................
void QS::onTestTeardown(void) {
}
//............................................................................
//! callback function to execute user commands
void QS::onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;

    switch (cmdId) {
       case 0U: {
           break;
       }
       case 1U: {
           break;
       }
       default:
           break;
    }
}

//============================================================================
//! Host callback function to "massage" the event, if necessary
void QS::onTestEvt(QEvt *e) {
    (void)e;
}
//............................................................................
//! callback function to output the posted QP events (not used here)
void QS::onTestPost(void const *sender, QActive *recipient,
                   QEvt const *e, bool status)
{
    (void)sender;
    (void)status;
}

} // namespace QP

//============================================================================
int main() {

    QP::QF::init();  // initialize the framework and the underlying QXK kernel
    BSP::init(); // initialize the Board Support Package

    // dictionaries
    QS_OBJ_DICTIONARY(&thr1);
    QS_OBJ_DICTIONARY(thr1.getTimeEvt());

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS); // all records

    // initialize publish-subscribe...
    static QP::QSubscrList subscrSto[MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[10]; // small pool
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // start the extended thread1
    static QP::QEvt const *test1QueueSto[5];
    static uint64_t test1StackSto[64];
    thr1.start(Q_PRIO(1U, 1U),         // QF-priority/preemption-threshold
               test1QueueSto,          // message queue storage
               Q_DIM(test1QueueSto),   // message length [events]
               test1StackSto,          // stack storage
               sizeof(test1StackSto)); // stack size [bytes]

    return QP::QF::run(); // run the QF application
}
