//============================================================================
// Product: BSP for DPP example (console) for POSIX (multithreaded) port
// Last updated for version 7.3.1
// Last updated on  2023-11-18
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
#include "qpcpp.hpp"        // QP/C++ real-time embedded framework
#include "dpp.hpp"          // DPP Application interface
#include "bsp.hpp"          // Board Support Package

#include "safe_std.h"       // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>
#include <sys/select.h>

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static std::uint32_t l_rnd; // random seed

#ifdef Q_SPY

    // QSpy source IDs
    static QP::QSpyId const l_clock_tick = { QP::QS_AP_ID };

    enum AppRecords { // application-specific trace records
        PHILO_STAT = QP::QS_USER,
        PAUSED_STAT,
    };

#endif

} // unnamed namespace

//============================================================================
// Error handler

extern "C" {

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    QS_ASSERTION(module, id, 10000U); // report assertion to QS
    FPRINTF_S(stderr, "ERROR in %s:%d", module, id);
    QP::QF::onCleanup();
    QS_EXIT();
    exit(-1);
}
//............................................................................
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

} // extern "C"

//============================================================================
namespace BSP {

//............................................................................
void init(int argc, char **argv) {
    Q_UNUSED_PAR(argc);
    Q_UNUSED_PAR(argv);

    PRINTF_S("Dining Philosophers Problem example"
           "\nQP %s\n"
           "Press 'p' to pause\n"
           "Press 's' to serve\n"
           "Press ESC to quit...\n",
           QP_VERSION_STR);

    BSP::randomSeed(1234U);

    if (!QS_INIT(argc > 1 ? argv[1] : nullptr)) {
        Q_ERROR();
    }

    QS_OBJ_DICTIONARY(&l_clock_tick);

    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(PAUSED_STAT);

    QS_ONLY(APP::produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_ALL_RECORDS);
    QS_GLB_FILTER(-QP::QS_QF_TICK);     // exclude the tick record
    QS_LOC_FILTER(-(APP::N_PHILO + 3)); // exclude prio. of AO_Ticker0
}
//............................................................................
void start() {
    // initialize event pools
    static QF_MPOOL_EL(APP::TableEvt) smlPoolSto[2*APP::N_PHILO];
    QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // initialize publish-subscribe
    static QP::QSubscrList subscrSto[APP::MAX_PUB_SIG];
    QP::QActive::psInit(subscrSto, Q_DIM(subscrSto));

    // start AOs/threads...
    static QP::QEvt const *philoQueueSto[APP::N_PHILO][10];
    for (std::uint8_t n = 0U; n < APP::N_PHILO; ++n) {
        APP::AO_Philo[n]->start(
            n + 3U,                  // QP prio. of the AO
            philoQueueSto[n],        // event queue storage
            Q_DIM(philoQueueSto[n]), // queue length [events]
            nullptr, 0U);            // no stack storage
    }

    static QP::QEvt const *tableQueueSto[APP::N_PHILO];
    APP::AO_Table->start(
        APP::N_PHILO + 7U,       // QP prio. of the AO
        tableQueueSto,           // event queue storage
        Q_DIM(tableQueueSto),    // queue length [events]
        nullptr, 0U);            // no stack storage
}
//............................................................................
void displayPhilStat(std::uint8_t n, char const *stat) {
    PRINTF_S("Philosopher %2d is %s\n", (int)n, stat);

    // application-specific record
    QS_BEGIN_ID(PHILO_STAT, APP::AO_Table->getPrio())
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void displayPaused(uint8_t paused) {
    PRINTF_S("Paused is %s\n", paused ? "ON" : "OFF");
}
//............................................................................
std::uint32_t random() { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    std::uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time

    return (rnd >> 8);
}
//............................................................................
void randomSeed(std::uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
void terminate(std::int16_t result) {
    Q_UNUSED_PAR(result);
    QP::QF::stop();
}

} // namespace BSP

//============================================================================
namespace QP {

//............................................................................
void QF::onStartup() {
    consoleSetup();

    // disable the standard clock-tick service by setting tick-rate to 0
    setTickRate(0U, 10U); // zero tick-rate / ticker thread prio.
}
//............................................................................
void QF::onCleanup() {
    PRINTF_S("\n%s\n", "Bye! Bye!");
    consoleCleanup();
}
//............................................................................
void QF::onClockTick() {

    // NOTE:
    // The standard clock-tick service has been DISABLED in QF::onStartup()
    // by setting the clock tick rate to zero.
    // Therefore QF::onClockTick() must implement an alternative waiting
    // mechanism for the clock period. This particular implementation is
    // based on the select() system call to block for the desired timeout.

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = (1000000/BSP::TICKS_PER_SEC);
    select(0, NULL, NULL, NULL, &tv); // block for the timevalue

    QTimeEvt::TICK_X(0U, &l_clock_tick); // process time events at rate 0

    QS_RX_INPUT(); // handle the QS-RX input
    QS_OUTPUT();   // handle the QS output

    switch (consoleGetKey()) {
        case '\33': { // ESC pressed?
            BSP::terminate(0);
            break;
        }
        case 'p': {
            static QEvt const pauseEvt(APP::PAUSE_SIG);
            QActive::PUBLISH(&pauseEvt, &l_clock_tick);
            break;
        }
        case 's': {
            static QEvt const serveEvt(APP::SERVE_SIG);
            QActive::PUBLISH(&serveEvt, &l_clock_tick);
            break;
        }
        default: {
            break;
        }
    }
}

//============================================================================
#ifdef Q_SPY

//............................................................................
void QS::onCommand(std::uint8_t cmdId, std::uint32_t param1,
                   std::uint32_t param2, std::uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

#endif // Q_SPY

} // namespace QP

