//****************************************************************************
// Product: QHsmTst Example
// Last Updated for Version: 6.9.1
// Date of the Last Update:  2020-09-22
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps, LLC. All rights reserved.
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
//****************************************************************************
#include "qpcpp.hpp"
#include "qhsmtst.hpp"

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>

using namespace QP;

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static FILE *l_outFile = (FILE *)0;
static void dispatch(QP::QSignal sig);

//............................................................................
int main(int argc, char *argv[ ]) {

#ifdef Q_SPY
    uint8_t qsBuf[128];
    QS::initBuf(qsBuf, sizeof(qsBuf));
#endif

    QF::init();
    QF::onStartup();

    if (argc > 1) { // file name provided?
        l_outFile = fopen(argv[1], "w");
    }

    if (l_outFile == (FILE *)0) { // interactive version?
        l_outFile = stdout;

        PRINTF_S("QHsmTst example, built on %s at %s\n"
               "QEP: %s.\nPress ESC to quit...\n",
               __DATE__, __TIME__, QP_VERSION_STR);

        the_hsm->init(0U); // trigger the initial tran. in the test HSM

        for (;;) { // event loop
            PRINTF_S("\n>", "");

            int c;
            c = (uint8_t)QP::QF_consoleWaitForKey();
            PRINTF_S("%c: ", (c >= ' ') ? c : 'X');

            QP::QEvt e = QEVT_INITIALIZER(0);
            if ('a' <= c && c <= 'i') { // in range?
                e.sig = (QP::QSignal)(c - 'a' + A_SIG);
            }
            else if ('A' <= c && c <= 'I') { // in range?
                e.sig = (QP::QSignal)(c - 'A' + A_SIG);
            }
            else if (c == '\33') { // the ESC key?
                e.sig = TERMINATE_SIG; // terminate the interactive test
            }
            else {
                e.sig = IGNORE_SIG;
            }

            the_hsm->dispatch(&e, 0U); // dispatch the event
        }
    }
    else { // batch version
        PRINTF_S("QHsmTst, output saved to %s\n", argv[1]);
        FPRINTF_S(l_outFile,
                "QHsmTst example, QEP %s\n", QP::QEP::getVersion());

        the_hsm->init(0U); // trigger the initial tran. in the test HSM

        // dynamic transitions
        dispatch(A_SIG);
        dispatch(B_SIG);
        dispatch(D_SIG);
        dispatch(E_SIG);
        dispatch(I_SIG);
        dispatch(F_SIG);
        dispatch(I_SIG);
        dispatch(I_SIG);
        dispatch(F_SIG);
        dispatch(A_SIG);
        dispatch(B_SIG);
        dispatch(D_SIG);
        dispatch(D_SIG);
        dispatch(E_SIG);
        dispatch(G_SIG);
        dispatch(H_SIG);
        dispatch(H_SIG);
        dispatch(C_SIG);
        dispatch(G_SIG);
        dispatch(C_SIG);
        dispatch(C_SIG);

        fclose(l_outFile);
    }

    QF::onCleanup();
    return 0;
}

//............................................................................
void BSP_display(char const *msg) {
    FPRINTF_S(l_outFile, "%s",  msg);
}
//............................................................................
void BSP_terminate(int16_t const result) {
    PRINTF_S("%s", "Bye, Bye!");
    QF::onCleanup();
    exit(result);
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const file, int_t const  line) {
    FPRINTF_S(stderr, "Assertion failed in %s, line %d", file, line);
    QF::onCleanup();
    exit(-1);
}
//............................................................................
static void dispatch(QP::QSignal sig) {
    Q_REQUIRE((A_SIG <= sig) && (sig <= I_SIG));
    FPRINTF_S(l_outFile, "\n%c:", 'A' + sig - A_SIG);
    QP::QEvt e = QEVT_INITIALIZER(sig);
    the_hsm->dispatch(&e, 0U); // dispatch the event
}

namespace QP {

//----------------------------------------------------------------------------
void QF::onStartup(void) {
    QF_consoleSetup();
}
//............................................................................
void QF::onCleanup(void) {
    QF_consoleCleanup();
}
//............................................................................
void QF_onClockTick(void) {
}

//----------------------------------------------------------------------------
#ifdef Q_SPY

//! callback function to execute user commands
void QS::onCommand(uint8_t cmdId,
     uint32_t param1, uint32_t param2, uint32_t param3)
{
    switch (cmdId) {
       case 0U: {
           break;
       }
       default:
           break;
    }

    /* unused parameters */
    (void)param1;
    (void)param2;
    (void)param3;
}


#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

