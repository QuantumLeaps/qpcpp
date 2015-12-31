//****************************************************************************
// Product: QMsmTst Example, POSIX
// Last updated for version 5.6.0
// Last updated on  2015-12-26
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, www.state-machine.com.
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "qmsmtst.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

namespace QMSMTST {

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static FILE *l_outFile = (FILE *)0;
static struct termios l_oldt;
static void dispatch(QP::QSignal sig);

//............................................................................
extern "C" int main(int argc, char *argv[ ]) {
    if (argc > 1) { // file name provided?
        l_outFile = fopen(argv[1], "w");
    }

    if (l_outFile == (FILE *)0) { // interactive version?
        struct termios newt;
        tcgetattr(STDIN_FILENO, &l_oldt); // save the terminal state
        newt = l_oldt;
        newt.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); // set non-canonical mode

        l_outFile = stdout; // use the stdout as the output file

        printf("QMsmTst example, built on %s at %s\n"
               "QEP: %s.\nPress ESC to quit...\n",
               __DATE__, __TIME__, QP::QEP::getVersion());

        the_msm->init(); // trigger the initial tran. in the test MSM

        for (;;) { // event loop
            printf("\n>");
            int c = getchar(); // get a character from stdin
            printf(": ");

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

            the_msm->dispatch(&e); // dispatch the event
        }
    }
    else { // batch version
        printf("QMsmTst, output saved to %s\n", argv[1]);
        fprintf(l_outFile,
                "QMsmTst example, QEP %s\n", QP::QEP::getVersion());

        the_msm->init(); // trigger the initial tran. in the test MSM

        // dynamic transitions...
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
extern "C" void Q_onAssert(char_t const * const file, int_t line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    exit(-1);
}
//............................................................................
void BSP_display(char_t const *msg) {
    fprintf(l_outFile, msg);
}
//............................................................................
void BSP_terminate(int16_t const result) {
    tcsetattr(STDIN_FILENO, TCSANOW, &l_oldt);
    printf("Bye, Bye!\n");
    exit(result);
}
//............................................................................
static void dispatch(QP::QSignal sig) {
    Q_REQUIRE((A_SIG <= sig) && (sig <= I_SIG));
    fprintf(l_outFile, "\n%c:", 'A' + sig - A_SIG);
    QP::QEvt e = QEVT_INITIALIZER(sig);
    the_msm->dispatch(&e); // dispatch the event
}

} // namespace QMSMTST


//----------------------------------------------------------------------------
#ifdef Q_SPY

#include "qs_port.h"

namespace QP {

//............................................................................
void QF::onStartup(void) {
}
//............................................................................
void QF_onClockTick(void) {
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {
    return static_cast<QSTimeCtr>(clock());
}

} // namespace QP

#endif // Q_SPY

