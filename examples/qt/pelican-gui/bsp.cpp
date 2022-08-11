//============================================================================
// Product: BSP for PELICAN crossing example for Qt5
// Last updated for version 6.9.1
// Last updated on  2020-09-21
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
#include <QtWidgets>
#include "gui.hpp"
//-----------------
#include "qpcpp.hpp"
#include "pelican.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

//............................................................................
static uint8_t const l_time_tick = 0U; // for QS

//............................................................................
void QP::QF_onClockTick(void) {
    QP::QTimeEvt::TICK_X(0U, &l_time_tick);
}
//............................................................................
void QP::QF::onStartup(void) {
    QP::QF_setTickRate(BSP_TICKS_PER_SEC);
    QS_OBJ_DICTIONARY(&l_time_tick);
}
//............................................................................
Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    QMessageBox::critical(0, "PROBLEM",
        QString("<p>Assertion failed in module <b>%1</b>,"
                "location <b>%2</b></p>")
            .arg(module)
            .arg(loc));
    QS_ASSERTION(module, loc, 10000); // send assertion info to the QS trace
    qFatal("Assertion failed in module %s, location %d", module, loc);
}

//............................................................................
void BSP_init(void) {
    Q_ALLEGE(QS_INIT((char *)0));
    QS_OBJ_DICTIONARY(&l_time_tick);

    // set up the QS filters...
    QS_GLB_FILTER(QP::QS_QF_MPOOL_GET);
}
//............................................................................
void BSP_terminate(int16_t /*result*/) {
    qDebug("terminate");
    QP::QF::stop(); // stop the QF_run() thread
    qApp->quit(); // quit the Qt application *after* the QF_run() has stopped
}
//............................................................................
void BSP_showState(char const *state) {
    qDebug("state:%s", state);
    Gui::instance()->m_stateLabel->setText(state);
}
//............................................................................
void BSP_signalCars(enum BSP_CarsSignal  sig) {
    qDebug("cars:%d", (int)sig);
    switch (sig) {
    case CARS_RED:
        Gui::instance()->m_carsLabel
                ->setPixmap(QPixmap(":/res/cars_red.png"));
        break;
    case CARS_YELLOW:
        Gui::instance()->m_carsLabel
                ->setPixmap(QPixmap(":/res/cars_ylw.png"));
        break;
    case CARS_GREEN:
        Gui::instance()->m_carsLabel
                ->setPixmap(QPixmap(":/res/cars_grn.png"));
        break;
    case CARS_BLANK:
        Gui::instance()->m_carsLabel
                ->setPixmap(QPixmap(":/res/cars_blank.png"));
        break;
    }
}
//............................................................................
void BSP_signalPeds(enum BSP_PedsSignal sig) {
    qDebug("peds:%d", (int)sig);
    switch (sig) {
    case PEDS_DONT_WALK:
        Gui::instance()->m_pedsLabel
                ->setPixmap(QPixmap(":/res/peds_dont.png"));
        break;
    case PEDS_WALK:
        Gui::instance()->m_pedsLabel
                ->setPixmap(QPixmap(":/res/peds_walk.png"));
        break;
    case PEDS_BLANK:
        Gui::instance()->m_pedsLabel
                ->setPixmap(QPixmap(":/res/peds_blank.png"));
        break;
    }
}

//============================================================================
#ifdef Q_SPY

#include "qspy.h"

static QTime l_time;

//............................................................................
static int custParserFun(QSpyRecord * const qrec) {
    int ret = 1; // perform standard QSPY parsing
    switch (qrec->rec) {
        case QP::QS_QF_MPOOL_GET: { // example record to parse
            int nFree;
            (void)QSpyRecord_getUint32(qrec, QS_TIME_SIZE);
            (void)QSpyRecord_getUint64(qrec, QS_OBJ_PTR_SIZE);
            nFree = (int)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE);
            (void)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE); // nMin
            if (QSpyRecord_OK(qrec)) {
                Gui::instance()->m_epoolLabel->setText(QString::number(nFree));
                ret = 0; // don't perform standard QSPY parsing
            }
            break;
        }
    }
    return ret;
}
//............................................................................
bool QP::QS::onStartup(void const * /*arg*/) {
    static uint8_t qsBuf[4*1024];   // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

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
                nullptr,          // matFile,
                nullptr,
                &custParserFun);    // customized parser function

    l_time.start();                 // start the time stamp

    return true; // success
}
//............................................................................
void QP::QS::onCleanup(void) {
    QSPY_stop();
}
//............................................................................
void QP::QS::onFlush(void) {
    uint16_t nBytes = 1024;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        QSPY_parse(block, nBytes);
        nBytes = 1024;
    }
}
//............................................................................
QP::QSTimeCtr QP::QS::onGetTime(void) {
    return (QSTimeCtr)l_time.elapsed();
}

//............................................................................
void QP::QS_onEvent(void) {
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
void QSPY_onPrintLn(void) {
    qDebug(QSPY_line);
}

#endif // Q_SPY

