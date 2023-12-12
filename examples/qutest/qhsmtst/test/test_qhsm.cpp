//============================================================================
// Purpose: Fixture for QUTEST
// Last Updated for Version: 7.3.1
// Date of the Last Update:  2023-12-11
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
#include "qhsmtst.hpp"

Q_DEFINE_THIS_FILE

//============================================================================
namespace APP {

enum {
    BSP_DISPLAY = QP::QS_USER,
    CMD,
};

void BSP_display(char const *msg) {
    QS_BEGIN_ID(BSP_DISPLAY, 0U) // app-specific record
        QS_STR(msg);
    QS_END()
}
//............................................................................
void BSP_terminate(int16_t const result) {
    Q_UNUSED_PAR(result);
}

} // namespace APP

//============================================================================
namespace QP {

//............................................................................
void QS::onTestSetup(void) {
}
//............................................................................
void QS::onTestTeardown(void) {
}
//............................................................................
void QS::onCommand(std::uint8_t cmdId, std::uint32_t param1,
                   std::uint32_t param2, std::uint32_t param3)
{
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);

    //PRINTF_S("<TARGET> Command id=%d param=%d\n", (int)cmdId, (int)param);
    switch (cmdId) {
        case 0U: {
            QS_BEGIN_ID(APP::CMD, 0U) // app-specific record
            QS_END()
            break;
        }
        case 1U: {
            bool ret = APP::QHsmTst_isIn(param1);
            QS_BEGIN_ID(APP::CMD, 0U) // app-specific record
                QS_U8(0U, ret ? 1 : 0);
                QS_U8(0U, (uint8_t)param1);
            QS_END()
            break;
       }
       default:
           break;
    }
}

//............................................................................
// callback function to "massage" the event, if necessary
void QS::onTestEvt(QEvt *e) {
    Q_UNUSED_PAR(e);
#ifdef Q_HOST  // is this test compiled for a desktop Host computer?
#else // this test is compiled for an embedded Target system
#endif
}
//............................................................................
// callback function to output the posted QP events (not used here)
void QS::onTestPost(void const *sender, QActive *recipient,
                    QEvt const *e, bool status)
{
    Q_UNUSED_PAR(sender);
    Q_UNUSED_PAR(recipient);
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(status);
}

} // namespace QP

//============================================================================
using namespace APP;

int main(int argc, char *argv[]) {
    static QF_MPOOL_EL(QP::QEvt) smlPoolSto[10]; // small pool

    QP::QF::init(); // initialize the framework and the underlying RT kernel

    // initialize the QS software tracing
    if (!QS_INIT(argc > 1 ? argv[1] : nullptr)) {
        Q_ERROR();
    }

    // initialize event pools...
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // dictionaries...
    QS_OBJ_DICTIONARY(the_sm);
    QS_USR_DICTIONARY(BSP_DISPLAY);
    QS_USR_DICTIONARY(CMD);

    return QP::QF::run();
}

