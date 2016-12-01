//****************************************************************************
// Product: DPP example (console)
// Last Updated for Version: 5.8.0
// Date of the Last Update:  2016-11-30
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
// http://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

#include <conio.h>
#include <stdio.h>

#ifdef Q_SPY
    #include <time.h>
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h> // Win32 API

    #include "qspy.h"    // QSPY interface
#endif


//****************************************************************************
namespace DPP {

Q_DEFINE_THIS_FILE

// local variables -----------------------------------------------------------
static uint32_t l_rnd; // random seed

#ifdef Q_SPY
    enum {
        PHILO_STAT = QP::QS_USER
    };
    static bool l_isRunning;
    static uint8_t const l_clock_tick = 0U;
#endif

//............................................................................
void BSP::init(void) {
    printf("Dining Philosopher Problem example"
           "\nQP %s\n"
           "Press 'p' to pause\n"
           "Press 's' to serve\n"
           "Press ESC to quit...\n",
           QP::versionStr);

    BSP::randomSeed(1234U);
    Q_ALLEGE(QS_INIT((void *)0));
    QS_OBJ_DICTIONARY(&l_clock_tick); // must be called *after* QF::init()
    QS_USR_DICTIONARY(PHILO_STAT);
}
//............................................................................
void BSP::terminate(int16_t result) {
    (void)result;
#ifdef Q_SPY
    l_isRunning = false; // stop the QS output thread
#endif
    QP::QF::stop(); // stop the main "ticker thread"
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    printf("Philosopher %2d is %s\n", (int)n, stat);

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    printf("Paused is %s\n", paused ? "ON" : "OFF");
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);
    return l_rnd >> 8;
}
//............................................................................
void BSP::randomSeed(uint32_t seed) {
    l_rnd = seed;
}

} // namespace DPP


//****************************************************************************
namespace QP {

//............................................................................
void QF::onStartup(void) {
    QF_setTickRate(DPP::BSP::TICKS_PER_SEC); // set the desired tick rate
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QF_onClockTick(void) {
    QF::TICK_X(0U, &DPP::l_clock_tick); // process time events at rate 0

    if (_kbhit()) {  // any key pressed?
        int ch = _getch();
        if (ch == '\33') { // see if the ESC key pressed
            DPP::BSP::terminate(0);
        }
        else if (ch == 'p') {
            QF::PUBLISH(Q_NEW(QEvt, DPP::PAUSE_SIG), &DPP::l_clock_tick);
        }
        else if (ch == 's') {
            QF::PUBLISH(Q_NEW(QEvt, DPP::SERVE_SIG), &DPP::l_clock_tick);
        }
    }
}
//............................................................................
extern "C" void Q_onAssert(char const * const module, int loc) {
    QS_ASSERTION(module, loc, 10000U); // report assertion to QS
    fprintf(stderr, "Assertion failed in %s, location %d", module, loc);
    QF::stop();
}

//----------------------------------------------------------------------------
#ifdef Q_SPY // define QS callbacks

//............................................................................
static DWORD WINAPI idleThread(LPVOID par) { // signature for CreateThread()
    (void)par;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
    DPP::l_isRunning = true;
    while (DPP::l_isRunning) {
        uint16_t nBytes = 256;
        uint8_t const *block;
        QF_CRIT_ENTRY(dummy);
        block = QS::getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);
        if (block != (uint8_t *)0) {
            QSPY_parse(block, nBytes);
        }
        Sleep(50); // wait for a while
    }
    return 0; // return success
}
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[4*1024]; // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    (void)arg;
    QSPY_config(QP_VERSION,         // version
                QS_OBJ_PTR_SIZE,    // objPtrSize
                QS_FUN_PTR_SIZE,    // funPtrSize
                QS_TIME_SIZE,       // tstampSize
                Q_SIGNAL_SIZE,      // sigSize,
                QF_EVENT_SIZ_SIZE,  // evtSize
                QF_EQUEUE_CTR_SIZE, // queueCtrSize
                QF_MPOOL_CTR_SIZE,  // poolCtrSize
                QF_MPOOL_SIZ_SIZE,  // poolBlkSize
                QF_TIMEEVT_CTR_SIZE,// tevtCtrSize
                (void *)0,          // matFile,
                (void *)0,
                (QSPY_CustParseFun)0); // customized parser function

    // set up the QS filters...
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

    QS_FILTER_ON(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_ON(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_ON(QS_QF_PUBLISH);

    QS_FILTER_ON(DPP::PHILO_STAT);

    return CreateThread(NULL, 1024, &idleThread, (void *)0, 0, NULL)
             != (HANDLE)0; // return the status of creating the idle thread
}
//............................................................................
void QS::onCleanup(void) {
    DPP::l_isRunning = false;
    QSPY_stop();
}
//............................................................................
void QS::onFlush(void) {
    for (;;) {
        uint16_t nBytes = 1024U;
        uint8_t const *block;

        QF_CRIT_ENTRY(dummy);
        block = getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);

        if (block != static_cast<uint8_t const *>(0)) {
            QSPY_parse(block, nBytes);
            nBytes = 1024U;
        }
        else {
            break;
        }
    }
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {
    return (QSTimeCtr)clock();
}
//............................................................................
extern "C" void QSPY_onPrintLn(void) {
    fputs(QSPY_line, stdout);
    fputc('\n', stdout);
}
#endif // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP
