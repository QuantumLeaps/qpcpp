//============================================================================
// Product: System test fixture for QK on the EFM32 target
// Last updated for version 7.1.2
// Last updated on  2022-10-06
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps. All rights reserved.
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

namespace {
//============================================================================
// AO ObjB
enum { NUM_B = 3 };

//............................................................................
// AO ObjB
class ObjB : public QP::QActive {
public:
    static ObjB inst[NUM_B];

public:
    ObjB() : QActive(&initial) {}

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(active);
}; // class ObjB

Q_STATE_DEF(ObjB, initial) {
    static bool registered = false; // starts off with 0, per C-standard
    if (!registered) {
        registered = true;
        QS_FUN_DICTIONARY(&ObjB::initial);
        QS_FUN_DICTIONARY(&ObjB::active);
    }
    subscribe(TEST1_SIG);
    subscribe(TEST2_SIG);
    return tran(&active);
}

Q_STATE_DEF(ObjB, active) {
    QP::QState status_;
    switch (e->sig) {
        case TEST0_SIG: {
            BSP::trace(this, "TEST0 1of2");
            BSP::trigISR();
            BSP::trace(this, "TEST0 2of2");
            status_ = Q_RET_HANDLED;
            break;
        }
        case TEST1_SIG: {
            static QP::QEvt const t2 = { TEST2_SIG, 0U, 0U };
            BSP::trace(this, "TEST1 1of2");
            QActive::PUBLISH(&t2, this);
            BSP::trace(this, "TEST1 2of2");
            status_ = Q_RET_HANDLED;
            break;
        }
        case TEST2_SIG: {
            BSP::trace(this, "TEST2 1of1");
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

ObjB ObjB::inst[NUM_B];

} // unnamed namespace

//============================================================================
int main() {

    QP::QF::init();  // initialize the framework and the underlying QXK kernel
    BSP::init(); // initialize the Board Support Package

    // initialize publish-subscribe...
    static QP::QSubscrList subscrSto[MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[10]; // small pool
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // dictionaries
    QS_SIG_DICTIONARY(TEST0_SIG,   nullptr);
    QS_SIG_DICTIONARY(TEST1_SIG,   nullptr);
    QS_SIG_DICTIONARY(TEST2_SIG,   nullptr);
    QS_SIG_DICTIONARY(TEST3_SIG,   nullptr);

    // priority specifications for ObjBs...
    static QP::QPrioSpec pspecB[NUM_B];
    QS_OBJ_DICTIONARY(pspecB);

    std::uint8_t n;

    for (n = 0U; n < NUM_B; ++n) {
        QS_OBJ_ARR_DICTIONARY(&ObjB::inst[n], n);
    }

    // pause execution of the test and wait for the test script to continue
    // NOTE:
    // this pause gives the test-script a chance to poke pspecB and pspecX
    // variables to start the threads with the desired prio-specifications.
    QS_TEST_PAUSE();

    static QP::QEvt const *aoB_queueSto[NUM_B][5];
    for (n = 0U; n < NUM_B; ++n) {
        if (pspecB[n] != 0U) {
            ObjB::inst[n].start(pspecB[n],        // QF-prio/p-thre.
                         aoB_queueSto[n],         // event queue storage
                         Q_DIM(aoB_queueSto[n]),  // event length [events]
                         nullptr,                 // no stack storage
                         0U);                     // zero stack size [bytes]
        }
    }

    return QP::QF::run(); // run the QF application
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
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
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
{}

} // namespace QP
