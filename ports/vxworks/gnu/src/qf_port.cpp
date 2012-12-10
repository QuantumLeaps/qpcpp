//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port to VxWorks, GNU compiler
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 19, 2012
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
#include "qf_pkg.h"
#include "qassert.h"

#include "taskLib.h"                                        // for taskSpawn()
#include "msgQLib.h"    // msgQCreate()/msgQDelete(), msgQSend()/msgQReceive()
#include "tickLib.h"                                     // for tickAnnounce()

Q_DEFINE_THIS_MODULE(qf_port)

// local objects -------------------------------------------------------------

//............................................................................
extern "C" void usrClock(void) {      // called from the system clock tick ISR
    tickAnnounce();                     // perform all VxWorks tme bookkeeping
    QF::tick();                                 // perform QF time bookkeeping
}

//............................................................................
const char Q_ROM * Q_ROM_VAR QF::getPortVersion(void) {
    static char const Q_ROM Q_ROM_VAR version[] = "4.4.02";
    return version;
}
//............................................................................
void QF::init(void) {
}
//............................................................................
int16_t QF::run(void) {
    onStartup();                                           // startup callback
    return static_cast<int16_t>(0);                          // return success
}
//............................................................................
void QF::stop(void) {
    onCleanup();                                           // cleanup callback
}
//............................................................................
                                     // use exactly the VxWorks task signature
extern "C" void task_function(int me,
                              int, int, int, int, int, int, int, int, int)
{
    ((QActive *)me)->run();
}
//............................................................................
void QActive::start(uint8_t prio,
                    QEvt const *qSto[], uint32_t qLen,
                    void *stkSto, uint32_t stkSize,
                    QEvt const *ie)
{
    Q_REQUIRE((qSto == (QEvt const **)0)      // VxWorks allocates Queue mem
              && (stkSto == (void *)0));        // VxWorks allocates Stack mem

    m_eQueue = msgQCreate(qLen, sizeof(QEvt *), MSG_Q_FIFO);
    Q_ASSERT(m_eQueue != 0);                          // VxWorks queue created
    m_prio = prio;                                     // save the QF priority
    QF::add_(this);                     // make QF aware of this active object
    init(ie);                                    // execute initial transition
    m_thread = taskSpawn((char *)0,
                         QF_MAX_ACTIVE - m_prio,           // VxWorks priority
                         VX_FP_TASK,        // taks options -- need to adjust!
                         (int)stkSize,
                         &task_function,
                         (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    Q_ASSERT(m_thread != ERROR);                       // VxWorks task created
}
//............................................................................
#ifndef Q_SPY
void QActive::postFIFO(QEvt const *e) {
#else
void QActive::postFIFO(QEvt const *e, void const *sender) {
#endif

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(sender);                                  // the sender object
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                       // this active object (recipient)
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOCRIT_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {                 // is it a pool event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }
    QF_CRIT_EXIT_();
    Q_ALLEGE(msgQSend(m_eQueue, (char *)&e, sizeof(e),
                      NO_WAIT, MSG_PRI_NORMAL) == OK);
}
//............................................................................
void QActive::postLIFO(QEvt const *e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOCRIT_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {                 // is it a pool event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }
    QF_CRIT_EXIT_();
    Q_ALLEGE(msgQSend(m_eQueue, (char *)&e, sizeof(e),
                      NO_WAIT, MSG_PRI_URGENT) == OK);
}
//............................................................................
QEvt const *QActive::get_(void) {
    QEvt const *e;
    QS_CRIT_STAT_

    Q_ALLEGE(msgQReceive(m_eQueue, (char *)&e, sizeof(e), WAIT_FOREVER)
             == sizeof(e));

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
    QS_END_()

    return e;
}
//............................................................................
void QActive::stop(void) {
    m_running = (uint8_t)0;             // stop the loop inside QActive::run()
    msgQDelete(m_eQueue);                          // delete the VxWorks queue
}
