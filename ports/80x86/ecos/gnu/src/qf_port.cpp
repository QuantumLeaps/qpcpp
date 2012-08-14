//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++  port to 80x86, eCos, GNU compiler
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

Q_DEFINE_THIS_MODULE(qf_port)


// local objects -------------------------------------------------------------
static cyg_handle_t l_tick_alarmH;

//............................................................................
char const Q_ROM * Q_ROM_VAR QF::getPortVersion(void) {
    static const char Q_ROM Q_ROM_VAR version[] =  "4.4.02";
    return version;
}
//............................................................................
void QF::init(void) {
}
//............................................................................
void QF::stop(void) {
    cyg_alarm_disable(l_tick_alarmH);
    cyg_alarm_delete (l_tick_alarmH);

    onCleanup();                                           // cleanup callback
}
//............................................................................
int16_t QF::run(void) {
    static cyg_alarm alarm;                // memory for the eCos alarm object
    cyg_handle_t counterH;
    cyg_handle_t clockH = cyg_real_time_clock();

    cyg_clock_to_counter(clockH, &counterH);
    cyg_alarm_create(counterH,
                     ((*)(cyg_handle_t, cyg_addrword_t))&QF_tick,
                     0,
                     &l_tick_alarmH,
                     &alarm);

    onStartup();                             // configure and start interrupts

                                                   // kick off the QF tick ...
    cyg_alarm_initialize(l_tick_alarmH, cyg_current_time() + 1, 1);
    return static_cast<int16_t>(0);                          // return success
}
//............................................................................
static void thread_function(cyg_addrword_t data) {
    ((QActive *)data)->run();                         // run the active object
}
//............................................................................
void QActive::start(uint8_t prio,
                    QEvt const *qSto[], uint32_t qLen,
                    void *stkSto, uint32_t stkSize,
                    QEvt const *ie)
{
    // create and assign the mail box to the QActive object...
    //
    cyg_mbox_create(&m_eQueue.handle, &m_eQueue.mbox);
    Q_ASSERT(m_eQueue.handle != 0);

    m_prio = prio;
    QF_add_(this);                      // make QF aware of this active object
    init(ie);                                    // execute initial transition

    QS_FLUSH();                          // flush the trace buffer to the host

    // create the actual eCos thread, note this ASSIGNS m_thread
    // with the newly created thread's handle ...
    //
    cyg_thread_create(QF_MAX_ACTIVE - prio,
                     &thread_function,
                     (cyg_addrword_t)this,
                     "AO",                             // active object thread
                     stkSto, stkSize,
                     &m_thread.hanlde, &m_thread.thread);

    // crank it up - this will invoke thread_function()
    //
    cyg_thread_resume(m_thread.handle);
}
//............................................................................
#ifndef Q_SPY
void QActive::postFIFO(QEvt const *e) {
#else
void QActive::postFIFO(QEvt const *e, void const *sender) {
#endif

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS_aoObj_, me)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(sender);                                  // the sender object
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(me);                         // this active object (recipient)
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOCRIT_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {                 // is it a pool event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }
    QF_CRIT_EXIT_();
    Q_ALLEGE(cyg_mbox_tryput(m_eQueue.handle, e));
}
//............................................................................
void QActive::postLIFO(QEvt const *e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS_aoObj_, me)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(me);                                     // this active object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
        QS_EQC_(0);                    // min number of free entries (unknown)
    QS_END_NOCRIT_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {                 // is it a pool event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }
    QF_CRIT_EXIT_();
    Q_ALLEGE(cyg_mbox_tryput(m_eQueue.handle, e));
}
//............................................................................
QEvt const * QActive::get_(void) {
    QEvt const * e  = (QEvt const *)cyg_mbox_get(m_eQueue.handle);
    QS_CRIT_STAT_

    Q_ASSERT(e != (QEvt *)0);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS_aoObj_, me)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(me);                                     // this active object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(0);                        // number of free entries (unknown)
    QS_END_()

    return e;
}
//............................................................................
void QActive::stop_(void) {
    m_running = (uint8_t)0;                            // stop the thread loop
}

