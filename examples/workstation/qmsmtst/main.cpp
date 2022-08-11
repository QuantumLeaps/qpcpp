//============================================================================
// Product: QHsmTst Example
// Last updated for: @ref qpcpp_7_0_0
// Last updated on: 2021-12-18
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2021 Quantum Leaps, LLC. All rights reserved.
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
#include "qmsmtst.hpp"

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>

using namespace QP;

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static FILE *l_outFile = (FILE *)0;
static void dispatch(QP::QSignal sig);

//............................................................................
int main(int argc, char *argv[ ]) {

    QF::init();
    QF::onStartup();

    Q_ALLEGE(QS_INIT(argv));
    QS_OBJ_DICTIONARY(the_sm);
    QS_SIG_DICTIONARY(A_SIG, nullptr);
    QS_SIG_DICTIONARY(B_SIG, nullptr);
    QS_SIG_DICTIONARY(C_SIG, nullptr);
    QS_SIG_DICTIONARY(D_SIG, nullptr);
    QS_SIG_DICTIONARY(E_SIG, nullptr);
    QS_SIG_DICTIONARY(F_SIG, nullptr);
    QS_SIG_DICTIONARY(G_SIG, nullptr);
    QS_SIG_DICTIONARY(H_SIG, nullptr);
    QS_SIG_DICTIONARY(I_SIG, nullptr);
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);
    QS_GLB_FILTER(-QP::QS_QF_TICK);

    if (argc > 1) { // file name provided?
        l_outFile = fopen(argv[1], "w");
    }

    if (l_outFile == (FILE *)0) { // interactive version?
        l_outFile = stdout;

        PRINTF_S("QMsmTst example, built on %s at %s\n"
               "QEP: %s.\nPress ESC to quit...\n",
               __DATE__, __TIME__, QP_VERSION_STR);

        the_sm->init(0U); // trigger the initial tran. in the test HSM

        for (;;) { // event loop
            PRINTF_S("\n%c", '>');
            QS_OUTPUT(); // handle the QS output

            int c;
            c = (uint8_t)QP::QF::consoleWaitForKey();
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

            the_sm->dispatch(&e, 0U); // dispatch the event
        }
    }
    else { // batch version
        PRINTF_S("QMsmTst, output saved to %s\n", argv[1]);
        FPRINTF_S(l_outFile,
                "QMsmTst example, QEP %s\n", QP_VERSION_STR);

        the_sm->init(0U); // trigger the initial tran. in the test HSM

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
    the_sm->dispatch(&e, 0U); // dispatch the event
    QS_OUTPUT(); // handle the QS output
}

namespace QP {

//----------------------------------------------------------------------------
void QF::onStartup(void) {
    QF::consoleSetup();
}
//............................................................................
void QF::onCleanup(void) {
    QF::consoleCleanup();
}
//............................................................................
void QF::onClockTick(void) {
}

//----------------------------------------------------------------------------
#ifdef Q_SPY

//! callback function to execute user commands (dummy definition)
void QS::onCommand(uint8_t cmdId,
     uint32_t param1, uint32_t param2, uint32_t param3)
{
    /* unused parameters */
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP

