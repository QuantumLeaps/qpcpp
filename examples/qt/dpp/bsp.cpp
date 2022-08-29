//============================================================================
// Product: BSP for DPP-console example with Qt5
// Last updated for: @qpcpp_7_0_0
// Last updated on  2021-07-19
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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
#include <QCoreApplication>
#include <QTextStream>
#include <QTime>
#include <QTextStream>
//-----------------
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

#ifdef Q_SPY
    enum {
        PHILO_STAT = QP::QS_USER
    };
    static uint8_t const l_time_tick = 0U; // for QS
#endif

Q_DEFINE_THIS_FILE

//............................................................................
static uint32_t l_rnd; // random seed
//static QTextStream l_stdInStream(stdin,   QIODevice::ReadOnly);
static QTextStream l_stdOutStream(stdout, QIODevice::WriteOnly);

//............................................................................
void BSP_init(void) {
    l_stdOutStream << "DPP-Qt console example\n"
                   << "QP " << QP::versionStr << '\n';

    BSP_randomSeed(1234U);

    Q_ALLEGE(QS_INIT((char *)0));
    QS_OBJ_DICTIONARY(&l_time_tick);
    QS_USR_DICTIONARY(PHILO_STAT);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
    QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
    QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records
}
//............................................................................
void BSP_terminate(int) {
    qDebug("terminate");
    QP::QF::stop(); // stop the QF::run() thread
    qApp->quit(); // quit the Qt application *after* the QF::run() has stopped
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    //qDebug("philo[%d] is %s", n, stat);
    l_stdOutStream << "philo[" << n << "] is " << stat << '\n';
}
//............................................................................
void BSP_displayPaused(uint8_t paused) {
    if (paused != 0U) {
        l_stdOutStream << "PAUSED\n";
        //qDebug("PAUSED");
    }
    else {
        l_stdOutStream << "SERVING\n";
        //qDebug("SERVING");
    }
}
//............................................................................
uint32_t BSP_random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);
    return l_rnd >> 8;
}
//............................................................................
void BSP_randomSeed(uint32_t seed) {
    l_rnd = seed;
}

//============================================================================
Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    QS_ASSERTION(module, loc, 10000U); // send assertion info to the QS trace
    qFatal("Assertion failed in module %s, location %d", module, loc);
}

//============================================================================
namespace QP {

//............................................................................
void QF_onClockTick(void) {
    QTimeEvt::TICK_X(0U, &l_time_tick);
}
//............................................................................
void QF::onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC);
    QS_OBJ_DICTIONARY(&l_time_tick);
}
//............................................................................
void QF::onCleanup(void) {
}

#ifdef Q_SPY

#include "qspy.h"

static QTime l_time;

//............................................................................
bool QS::onStartup(void const *) {
    static uint8_t qsBuf[4*1024]; // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    // QS configuration options for this application...
    QSpyConfig const config = {
        QP_VERSION,          // version
        0U,                  // endianness (little)
        QS_OBJ_PTR_SIZE,     // objPtrSize
        QS_FUN_PTR_SIZE,     // funPtrSize
        QS_TIME_SIZE,        // tstampSize
        Q_SIGNAL_SIZE,       // sigSize
        QF_EVENT_SIZ_SIZE,   // evtSize
        QF_EQUEUE_CTR_SIZE,  // queueCtrSize
        QF_MPOOL_CTR_SIZE,   // poolCtrSize
        QF_MPOOL_SIZ_SIZE,   // poolBlkSize
        QF_TIMEEVT_CTR_SIZE, // tevtCtrSize
        { 0U, 0U, 0U, 0U, 0U, 0U} // tstamp
    };
    QSPY_config(&config, nullptr); // no custom parser function

    l_time.start();          // start the time stamp

    return true; // success
}
//............................................................................
void QS::onCleanup(void) {
    QSPY_cleanup();
}
//............................................................................
void QS::onFlush(void) {
    uint16_t nBytes = 1024U;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        QSPY_parse(block, nBytes);
        nBytes = 1024U;
    }
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {
    return (QSTimeCtr)l_time.elapsed();
}

//............................................................................
void QS::onReset(void) {
    //TBD
}
//............................................................................
void QS_onEvent(void) {
    uint16_t nBytes = 1024;
    uint8_t const *block;
    QF_CRIT_ENTRY(dummy);
    if ((block = QS::getBlock(&nBytes)) != (uint8_t *)0) {
        QF_CRIT_EXIT(dummy);
        QSPY_parse(block, nBytes);
    }
    else {
        QF_CRIT_EXIT(dummy);
    }
}
//............................................................................
void QS::onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}

//............................................................................
extern "C" void QSPY_onPrintLn(void) {
    qDebug(QSPY_line);
}

#endif // Q_SPY

} // namespace QP
