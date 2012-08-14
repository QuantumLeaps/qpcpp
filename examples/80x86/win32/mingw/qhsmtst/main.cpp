//////////////////////////////////////////////////////////////////////////////
// Product: QHsmTst Example
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 14, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "qassert.h"
#include "qhsmtst.h"

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

namespace QHSMTST {

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static FILE *l_outFile = (FILE *)0;
static void dispatch(QP::QSignal sig);

//............................................................................
extern "C" int main(int argc, char *argv[ ]) {
    if (argc > 1) {                                     // file name provided?
        l_outFile = fopen(argv[1], "w");
    }

    if (l_outFile == (FILE *)0) {                      // interactive version?
        l_outFile = stdout;

        printf("QHsmTst example, built on %s at %s\n"
               "QEP: %s.\nPress ESC to quit...\n",
               __DATE__, __TIME__, QP::QEP::getVersion());

        the_hsm->init();          // trigger the initial tran. in the test HSM

        for (;;) {                                               // event loop
            printf("\n>");

            int c;
            c = _getche();       // get a character from the console with echo
            printf(": ");

            QP::QEvt e = QEVT_INITIALIZER(0);
            if ('a' <= c && c <= 'i') {                           // in range?
                e.sig = (QP::QSignal)(c - 'a' + A_SIG);
            }
            else if ('A' <= c && c <= 'I') {                      // in range?
                e.sig = (QP::QSignal)(c - 'A' + A_SIG);
            }
            else if (c == '\33') {                             // the ESC key?
                e.sig = TERMINATE_SIG;       // terminate the interactive test
            }
            else {
                e.sig = IGNORE_SIG;
            }

            the_hsm->dispatch(&e);                       // dispatch the event
        }
    }
    else {                                                    // batch version
        printf("QHsmTst, output saved to %s\n", argv[1]);
        fprintf(l_outFile,
                "QHsmTst example, QEP %s\n", QP::QEP::getVersion());

        the_hsm->init();          // trigger the initial tran. in the test HSM

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

    return 0;
}
//............................................................................
extern "C" void Q_onAssert(char const Q_ROM * const file, int line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    _exit(-1);
}
//............................................................................
void BSP_display(char const *msg) {
    fprintf(l_outFile, msg);
}
//............................................................................
void BSP_terminate(int16_t const result) {
    printf("Bye, Bye!");
    _exit(result);
}
//............................................................................
static void dispatch(QP::QSignal sig) {
    Q_REQUIRE((A_SIG <= sig) && (sig <= I_SIG));
    fprintf(l_outFile, "\n%c:", 'A' + sig - A_SIG);
    QP::QEvt e = QEVT_INITIALIZER(sig);
    the_hsm->dispatch(&e);                               // dispatch the event
}

} // namespace QHSMTST
