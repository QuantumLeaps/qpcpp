//============================================================================
// System test fixture for QK kernel on the EFM32 target
// Last updated for version 7.1.1
// Last updated on  2022-09-05
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

namespace { // unnamed namespace

Q_DEFINE_THIS_FILE

enum { NumB = 3 };

//............................................................................
// AO ObjB
class ObjB : public QP::QActive {
public:
    static ObjB inst[NumB];

public:
    ObjB() : QActive(&initial) {}

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(active);
}; // class ObjB

Q_STATE_DEF(ObjB, initial) {
    for (std::uint8_t n = 0U; n < NumB; ++n) {
        QS_OBJ_ARR_DICTIONARY(&ObjB::inst[n], n);
    }
    QS_FUN_DICTIONARY(&ObjB::initial);
    QS_FUN_DICTIONARY(&ObjB::active);

    return tran(&active);
}

Q_STATE_DEF(ObjB, active) {
    QP::QState status_;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            status_ = Q_RET_HANDLED;
            break;
        }
        case TRIG_SIG: {
            BSP::trigISR();
            status_ = Q_RET_HANDLED;
            break;
        }
        case TEST_SIG: {
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
//............................................................................
// AO ObjA
class ObjA : public QP::QActive {
public:
    static ObjA inst;

public:
    ObjA() : QActive(&initial) {}

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(active);
}; // class ObjA

Q_STATE_DEF(ObjA, initial) {
    subscribe(TEST_SIG);

    QS_OBJ_DICTIONARY(&ObjA::inst);
    QS_FUN_DICTIONARY(&ObjA::initial);
    QS_FUN_DICTIONARY(&ObjA::active);

    return tran(&active);
}

Q_STATE_DEF(ObjA, active) {
    QP::QState status_;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            status_ = Q_RET_HANDLED;
            break;
        }
        case TRIG_SIG: {
            BSP::trigISR();
            status_ = Q_RET_HANDLED;
            break;
        }
        case TEST_SIG: {
            static QP::QEvt const tste = { TEST_SIG, 0U, 0U };
            BSP::ledOn();
            ObjB::inst[2].POST(&tste, this);
            ObjB::inst[1].POST(&tste, this);
            ObjB::inst[0].POST(&tste, this);
            BSP::ledOff();
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

} // unnamed namespace

//============================================================================
ObjB ObjB::inst[NumB];
ObjA ObjA::inst;

int main() {
    QP::QF::init(); // initialize the framework and the underlying QXK kernel
    BSP::init(); // initialize the Board Support Package

    // dictionaries
    QS_FUN_DICTIONARY(&QP::QHsm::top);
    QS_SIG_DICTIONARY(TEST_SIG, nullptr);
    QS_SIG_DICTIONARY(TRIG_SIG, nullptr);

    static std::uint16_t pspec[NumB + 1] = {
        Q_PRIO(1U, 0U),
        Q_PRIO(2U, 0U),
        Q_PRIO(3U, 0U),
        Q_PRIO(4U, 0U)
    };
    QS_OBJ_DICTIONARY(pspec);

    // initialize publish-subscribe...
    static QP::QSubscrList subscrSto[MAX_PUB_SIG];
    QP::QF::psInit(subscrSto, Q_DIM(subscrSto));

    // initialize event pools...
    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[10]; // small pool
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // pause execution of the test and wait for the test script to continue
    QS_TEST_PAUSE();

    static QP::QEvt const *aoB_queueSto[NumB][5];
    for (std::uint8_t n = 0U; n < NumB; ++n) {
        ObjB::inst[n].start(
            pspec[n],                 // QP priority spec
            aoB_queueSto[n],          // event queue storage
            Q_DIM(aoB_queueSto[n]),   // event length [events]
            nullptr,                  // no stack storage
            0U);                      // zero stack size [bytes]
    }

    static QP::QEvt const *aoA_queueSto[5];
    ObjA::inst.start(
        pspec[NumB],             // QP priority spec
        aoA_queueSto,            // event queue storage
        Q_DIM(aoA_queueSto),     // event length [events]
        nullptr,                 // no stack storage
        0U);                     // zero stack size [bytes]

    return QP::QF::run(); // run the QF application
}

//============================================================================
namespace QP {

//............................................................................
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
//............................................................................
//! Host callback function to "massage" the event, if necessary
void QS::onTestEvt(QEvt *e) {
    Q_UNUSED_PAR(e);
}
//............................................................................
//! callback function to output the posted QP events (not used here)
void QS::onTestPost(void const *sender, QActive *recipient,
                   QEvt const *e, bool status)
{
    Q_UNUSED_PAR(sender);
    Q_UNUSED_PAR(status);
}

} // namespace QP
