/// @file
/// @brief QP::QActive::defer() and QP::QActive::recall() definitions.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.3.2
/// Last updated on  2018-06-16
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) 2002-2018 Quantum Leaps. All rights reserved.
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
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"       // QF package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

namespace QP {

Q_DEFINE_THIS_MODULE("qf_defer")

//****************************************************************************
/// @description
/// This function is part of the event deferral support. An active object
/// uses this function to defer an event @p e to the QF-supported native
/// event queue @p eq. QF correctly accounts for another outstanding
/// reference to the event and will not recycle the event at the end of
/// the RTC step. Later, the active object might recall one event at a
/// time from the event queue.
///
/// @param[in] eq  pointer to a "raw" thread-safe queue to recall
///                an event from.
/// @param[in] e   pointer to the event to be deferred
///
/// @returns
/// 'true' (success) when the event could be deferred and 'false'
/// (failure) if event deferral failed due to overflowing the queue.
///
/// An active object can use multiple event queues to defer events of
/// different kinds.
///
/// @sa
/// QP::QActive::recall(), QP::QEQueue, QP::QActive::flushDeferred()
///
bool QActive::defer(QEQueue * const eq, QEvt const * const e) const {
    bool status = eq->post(e, static_cast<uint_fast16_t>(0));
    QS_CRIT_STAT_

    QS_BEGIN_(QS_QF_ACTIVE_DEFER, QS::priv_.locFilter[QS::AO_OBJ], this)
        QS_TIME_();      // time stamp
        QS_OBJ_(this);   // this active object
        QS_OBJ_(eq);     // the deferred queue
        QS_SIG_(e->sig); // the signal of the event
        QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
    QS_END_()

    return status;
}

//****************************************************************************
/// @description
/// This function is part of the event deferral support. An active object
/// uses this function to recall a deferred event from a given QF
/// event queue. Recalling an event means that it is removed from the
/// deferred event queue @p eq and posted (LIFO) to the event queue of
/// the active object.
///
/// @param[in]  eq  pointer to a "raw" thread-safe queue to recall
///                 an event from.
///
/// @returns
/// 'true' if an event has been recalled and 'false' if not.
///
/// @note
/// An active object can use multiple event queues to defer events of
/// different kinds.
///
/// @sa
/// QP::QActive::recall(), QP::QEQueue, QP::QActive::postLIFO_()
///
bool QActive::recall(QEQueue * const eq) {
    QEvt const * const e = eq->get(); // try to get evt from deferred queue
    bool recalled;

    // event available?
    if (e != static_cast<QEvt const *>(0)) {
        this->postLIFO(e); // post it to the _front_ of the AO's queue

        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();

        // is it a dynamic event?
        if (e->poolId_ != static_cast<uint8_t>(0)) {

            // after posting to the AO's queue the event must be referenced
            // at least twice: once in the deferred event queue (eq->get()
            // did NOT decrement the reference counter) and once in the
            // AO's event queue.
            Q_ASSERT_CRIT_(210, e->refCtr_ >= static_cast<uint8_t>(2));

            // we need to decrement the reference counter once, to account
            // for removing the event from the deferred event queue.
            QF_EVT_REF_CTR_DEC_(e); // decrement the reference counter
        }

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_RECALL,
                         QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();      // time stamp
            QS_OBJ_(this);   // this active object
            QS_OBJ_(eq);     // the deferred queue
            QS_SIG_(e->sig); // the signal of the event
            QS_2U8_(e->poolId_, e->refCtr_); // pool Id & ref Count
        QS_END_NOCRIT_()

        QF_CRIT_EXIT_();
        recalled = true;
    }
    else {
        QS_CRIT_STAT_

        QS_BEGIN_(QS_QF_ACTIVE_RECALL_ATTEMPT,
                  QS::priv_.locFilter[QS::AO_OBJ], this)
            QS_TIME_();      // time stamp
            QS_OBJ_(this);   // this active object
            QS_OBJ_(eq);     // the deferred queue
        QS_END_()

        recalled = false;
    }
    return recalled;
}

//****************************************************************************
/// @description
/// This function is part of the event deferral support. An active object
/// can use this function to flush a given QF event queue. The function makes
/// sure that the events are not leaked.
///
/// @param[in]  eq  pointer to a "raw" thread-safe queue to flush.
///
/// @returns
/// the number of events actually flushed from the queue.
///
///
/// @sa
/// QP::QActive::defer(), QP::QActive::recall(), QP::QEQueue
///
uint_fast16_t QActive::flushDeferred(QEQueue * const eq) const {
    uint_fast16_t n = static_cast<uint_fast16_t>(0);
    for (QEvt const *e = eq->get();
         e != static_cast<QEvt const *>(0);
         e = eq->get())
    {
        QF::gc(e); // garbage collect
        ++n; // count the flushed event
    }
    return n;
}

} // namespace QP

