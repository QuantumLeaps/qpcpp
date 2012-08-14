//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++
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
/// \file
/// \ingroup qf
/// \brief QF::tick() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qf_tick")

//............................................................................
#ifndef Q_SPY
void QF::tick(void) {                                            // see NOTE01
#else
void QF::tick(void const * const sender) {
#endif

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_TICK, null_void, null_void)
        QS_TEC_(static_cast<QTimeEvtCtr>(++QS::tickCtr_)); // the tick counter
    QS_END_NOCRIT_()

    QTimeEvt *prev = null_tevt;
    for (QTimeEvt *t = QF_timeEvtListHead_; t != null_tevt; t = t->m_next) {
        if (t->m_ctr == tc_0) {            // time evt. scheduled for removal?
            if (t == QF_timeEvtListHead_) {
                QF_timeEvtListHead_ = t->m_next;
            }
            else {
                Q_ASSERT(prev != null_tevt);
                prev->m_next = t->m_next;
            }
            QF_EVT_REF_CTR_DEC_(t);                      // mark as not linked
        }
        else {
            --t->m_ctr;
            if (t->m_ctr == tc_0) {                        // about to expire?
                if (t->m_interval != tc_0) {           // periodic time event?
                    t->m_ctr = t->m_interval;            // rearm the time evt
                }
                else {
                    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_AUTO_DISARM, QS::teObj_, t)
                        QS_OBJ_(t);                  // this time event object
                        QS_OBJ_(t->m_act);                // the active object
                    QS_END_NOCRIT_()
                }

                QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_POST, QS::teObj_, t)
                    QS_TIME_();                                   // timestamp
                    QS_OBJ_(t);                       // the time event object
                    QS_SIG_(t->sig);          // the signal of this time event
                    QS_OBJ_(t->m_act);                    // the active object
                QS_END_NOCRIT_()

                QF_CRIT_EXIT_();         // leave crit. section before posting
                           // POST() asserts internally if the queue overflows
                t->m_act->POST(t, sender);
                QF_CRIT_ENTRY_();        // re-enter crit. section to continue

                if (t->m_ctr == tc_0) {             // still marked to expire?
                    if (t == QF_timeEvtListHead_) {
                        QF_timeEvtListHead_ = t->m_next;
                    }
                    else {
                        Q_ASSERT(prev != null_tevt);
                        prev->m_next = t->m_next;
                    }
                    QF_EVT_REF_CTR_DEC_(t);                 // mark as removed
                }
                else {
                    prev = t;
                }
            }
            else {
                prev = t;
            }
        }
    }
    QF_CRIT_EXIT_();
}

QP_END_

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// QF::tick() must always run to completion and never preempt itself.
// In particular, if QF::tick() runs in an ISR, the ISR is not allowed to
// preempt itself. Also, QF::tick() should not be called from two different
// ISRs, which potentially could preempt each other.
//
// NOTE02:
// On many CPUs, the interrupt enabling takes only effect on the next
// machine instruction, which happens to be here interrupt disabling.
// The assignment of a volatile variable requires a few instructions, which
// the compiler cannot optimize away. This ensures that the interrupts get
// actually enabled, so that the interrupt latency stays low.
//
