//============================================================================
// Product: QMsmTst Example
// Last Updated for Version: 7.3.0
// Date of the Last Update:  2023-09-06
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
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

#include "qpcpp.hpp"     // QP/C++ framework
#include "qmsmtst.hpp"   // QMsmTst state machine

#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>   // for exit()

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static FILE *l_outFile = (FILE *)0;
static void dispatch(QP::QSignal sig);

//............................................................................
int main(int argc, char *argv[ ]) {

    if (argc > 1) { // file name provided?
        FOPEN_S(l_outFile, argv[1], "w");
    }

    if (l_outFile == (FILE *)0) { // interactive version?
        l_outFile = stdout;

        PRINTF_S("QMsmTst example, built on %s at %s\n"
               "QEP: %s.\nEnter x or X quit...\n",
               __DATE__, __TIME__, QP_VERSION_STR);

        APP::the_sm->init(0U); // trigger the initial tran. in the test SM

        for (;;) { // event loop
            PRINTF_S("\n%s", ">>>");
            char inp[4];
            scanf("%1s", inp); // input the event

            QP::QSignal sig = 0U;
            if ('a' <= inp[0] && inp[0] <= 'i') { // in range?
                sig = (QP::QSignal)(inp[0] - 'a' + APP::A_SIG);
            }
            else if ('A' <= inp[0] && inp[0] <= 'I') { // in range?
                sig = (QP::QSignal)(inp[0] - 'A' + APP::A_SIG);
            }
            else if ((inp[0] == 'x') || (inp[0] == 'X')) { // x or X?
                sig = APP::TERMINATE_SIG; // terminate the interactive test
            }
            else {
                sig = APP::IGNORE_SIG;
            }

            QP::QEvt const e(sig);
            APP::the_sm->dispatch(&e, 0U); // dispatch the event
        }
    }
    else { // batch version
        PRINTF_S("QMsmTst, output saved to %s\n", argv[1]);
        FPRINTF_S(l_outFile,
                "QMsmTst example, QP %s\n", QP_VERSION_STR);

        APP::the_sm->init(0U); // trigger the initial tran. in the test SM

        // dynamic transitions
        dispatch(APP::A_SIG);
        dispatch(APP::B_SIG);
        dispatch(APP::D_SIG);
        dispatch(APP::E_SIG);
        dispatch(APP::I_SIG);
        dispatch(APP::F_SIG);
        dispatch(APP::I_SIG);
        dispatch(APP::I_SIG);
        dispatch(APP::F_SIG);
        dispatch(APP::A_SIG);
        dispatch(APP::B_SIG);
        dispatch(APP::D_SIG);
        dispatch(APP::D_SIG);
        dispatch(APP::E_SIG);
        dispatch(APP::G_SIG);
        dispatch(APP::H_SIG);
        dispatch(APP::H_SIG);
        dispatch(APP::C_SIG);
        dispatch(APP::G_SIG);
        dispatch(APP::C_SIG);
        dispatch(APP::C_SIG);

        fclose(l_outFile);
    }

    return 0;
}
//............................................................................
extern "C" Q_NORETURN Q_onError(char const * const file, int_t const  line) {
    FPRINTF_S(stderr, "Assertion failed in %s, line %d", file, line);
    exit(-1);
}
//............................................................................
static void dispatch(QP::QSignal sig) {
    Q_REQUIRE((APP::A_SIG <= sig) && (sig <= APP::I_SIG));
    FPRINTF_S(l_outFile, "\n%c:", 'A' + sig - APP::A_SIG);
    QP::QEvt e(sig);
    APP::the_sm->dispatch(&e, 0U); // dispatch the event
}

namespace APP {
//............................................................................
void BSP_display(char const *msg) {
    FPRINTF_S(l_outFile, "%s",  msg);
}
//............................................................................
void BSP_terminate(int16_t const result) {
    PRINTF_S("%s", "Bye, Bye!");
    exit(result);
}

} // namespace APP
