/// \file
/// \brief QP::QF::tickX_() and QP::QF::noTimeEvtsActiveX() definitions.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-09
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qf_tick")

// Package-scope objects *****************************************************
QTimeEvt QF::timeEvtHead_[QF_MAX_TICK_RATE]; // heads of time event lists

//****************************************************************************
/// \description
/// This function must be called periodically from a time-tick ISR or from
/// a task so that QF can manage the timeout events assigned to the given
/// system clock tick rate.
///
/// \arguments
/// \arg[in]  \c tickRate  system clock tick rate serviced in this call.
///
/// \note this function should be called only via the macro TICK_X()
///
/// \note the calls to QP::QF::tickX_() with different tick rate argument can
/// preempt each other. For example, higher clock tick rates might be
/// serviced from interrupts while others from tasks (active objects).
///
/// \sa QP::QTimeEvt.
///
#ifndef Q_SPY
void QF::tickX_(uint8_t const tickRate)
#else
void QF::tickX_(uint8_t const tickRate, void const * const sender)
#endif
{
    QTimeEvt *prev = &timeEvtHead_[tickRate];
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_TICK, null_void, null_void)
        QS_TEC_(static_cast<QTimeEvtCtr>(++prev->m_ctr)); // tick ctr
        QS_U8_(tickRate);                                 // tick rate
    QS_END_NOCRIT_()

    // scan the linked-list of time events at this rate...
    for (;;) {
        QTimeEvt *t = prev->m_next; // advance down the time evt. list

        // end of the list?
        if (t == null_tevt) {
            if (timeEvtHead_[tickRate].m_act != null_void) {
                Q_ASSERT(prev != null_tevt); // sanity check
                prev->m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = null_void;
                t = prev->m_next; // switch to the new list
            }
            else {
                break; // all currently armed time evts. processed
            }
        }

        // time evt. scheduled for removal?
        if (t->m_ctr == tc_0) {
            prev->m_next = t->m_next;
            t->refCtr_ &= u8_0x7F; // mark as unlinked
            // do NOT advance the prev pointer
            QF_CRIT_EXIT_(); // exit crit. section to reduce latency

            // prevent merging critical sections, see NOTE1 below
            QF_CRIT_EXIT_NOP();
        }
        else {
            --t->m_ctr;

            // is time evt about to expire?
            if (t->m_ctr == tc_0) {
                QActive *act = t->toActive(); // temporary for volatile

                // periodic time evt?
                if (t->m_interval != tc_0) {
                    t->m_ctr = t->m_interval; // rearm the time event
                    prev = t; // advance to this time event
                }
                // one-shot time event: automatically disarm
                else {
                    prev->m_next = t->m_next;
                    t->refCtr_ &= u8_0x7F; // mark as unlinked
                    // do NOT advance the prev pointer

                    QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_AUTO_DISARM,
                                     QS::priv_.teObjFilter, t)
                        QS_OBJ_(t);        // this time event object
                        QS_OBJ_(act);      // the target AO
                        QS_U8_(tickRate);  // tick rate
                    QS_END_NOCRIT_()
                }

                QS_BEGIN_NOCRIT_(QS_QF_TIMEEVT_POST, QS::priv_.teObjFilter, t)
                    QS_TIME_();            // timestamp
                    QS_OBJ_(t);            // the time event object
                    QS_SIG_(t->sig);       // signal of this time event
                    QS_OBJ_(act);          // the target AO
                    QS_U8_(tickRate);      // tick rate
                QS_END_NOCRIT_()

                QF_CRIT_EXIT_(); // exit crit. section before posting

                (void)act->POST(t, sender); // asserts if queue overflows
            }
            else {
                prev = t; // advance to this time event
                QF_CRIT_EXIT_(); // exit crit. section to reduce latency

                // prevent merging critical sections, see NOTE1 below
                QF_CRIT_EXIT_NOP();
            }
        }
        QF_CRIT_ENTRY_(); // re-enter crit. section to continue
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
// NOTE1:
// In some QF ports the critical section exit takes effect only on the next
// machine instruction. If this case, the next instruction is another entry
// to a critical section, the critical section won't be really exited, but
// rather the two adjacent critical sections would be merged.
//
// The QF_CRIT_EXIT_NOP() macro contains minimal code required
// to prevent such merging of critical sections in QF ports,
// in which it can occur.


//****************************************************************************
/// \description
/// Find out if any time events are armed at the given clock tick rate.
///
/// \arguments
/// \arg[in]  \c tickRate  system clock tick rate to find out about.
///
/// \returns 'true' if no time events are armed at the given tick rate and
/// 'false' otherwise.
///
/// \note This function should be called in critical section.
///
bool QF::noTimeEvtsActiveX(uint8_t const tickRate) {
    Q_REQUIRE(tickRate < static_cast<uint8_t>(QF_MAX_TICK_RATE));
    bool inactive;
    if (timeEvtHead_[tickRate].m_next == null_tevt) {
        inactive = false;
    }
    else if (timeEvtHead_[tickRate].m_act == null_void) {
        inactive = false;
    }
    else {
        inactive = true;
    }
    return inactive;
}

} // namespace QP
