//============================================================================
// Test fixture for DPP example
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "bsp.hpp"
#include "dpp.hpp"

namespace { // unnamed namespace

// instantiate dummy collaborator AOs...
static QP::QActiveDummy Table_dummy;

} // unnamed namespace

QP::QActive * const APP::AO_Table = &Table_dummy;

int main() {
    QP::QF::init();  // initialize the framework and the underlying RT kernel
    BSP::init();     // initialize the BSP

    // object dictionaries...
    QS_OBJ_DICTIONARY(&Table_dummy);

    // pause execution of the test and wait for the test script to continue
    QS_TEST_PAUSE();

    // initialize event pools...
    static QF_MPOOL_EL(APP::TableEvt) smlPoolSto[2*APP::N_PHILO];
    QP::QF::poolInit(smlPoolSto,
                     sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe...
    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // start the active objects...
    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10];
    std::uint8_t n = 2;
    APP::AO_Philo[n]->start(
        n + 1U,   // QP priority
        philoQueueSto[n], Q_DIM(philoQueueSto[n]),
        nullptr, 0U);

    APP::AO_Table->start(
        APP::N_PHILO + 1U, // QP priority of the dummy
        nullptr, 0U, nullptr, 0U);

    return QP::QF::run(); // run the QF application
}


namespace QP {

//............................................................................
void QS::onTestSetup(void) {
}
//............................................................................
void QS::onTestTeardown(void) {
}

//............................................................................
// callback function to execute user commands
void QS::onCommand(uint8_t cmdId,
                   uint32_t param1, uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;

    switch (cmdId) {
       case 0U: {
           QEvt const e_pause(APP::PAUSE_SIG);
           APP::AO_Table->dispatch(&e_pause,
                              static_cast<std::uint_fast8_t>(param1));
           break;
       }
       case 1U: {
           QEvt const e_serve(APP::SERVE_SIG);
           APP::AO_Table->dispatch(&e_serve,
                              static_cast<std::uint_fast8_t>(param1));
           break;
       }
       default:
           break;
    }
}

//============================================================================
// callback function to "massage" the event, if necessary
void QS::onTestEvt(QP::QEvt *e) {
    (void)e;
#ifdef Q_HOST  // is this test compiled for a desktop Host computer?
#else // this test is compiled for an embedded Target system
#endif
}
//............................................................................
// callback function to output the posted QP events (not used here)
void QS::onTestPost(void const *sender, QP::QActive *recipient,
                    QP::QEvt const *e, bool status)
{
    (void)sender;
    (void)status;
    switch (e->sig) {
        case APP::EAT_SIG:
        case APP::DONE_SIG:
        case APP::HUNGRY_SIG:
            QS_BEGIN_ID(QUTEST_ON_POST, 0U) // app-specific record
                QS_SIG(e->sig, recipient);
                QS_U8(0, Q_EVT_CAST(APP::TableEvt)->philoId);
            QS_END()
            break;
        default:
            break;
    }
}

} // namespace QP
