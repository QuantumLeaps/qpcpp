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
/// \brief QF::new_() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qf_new")

//............................................................................
QEvt *QF::new_(QEvtSize const evtSize, enum_t const sig) {
                    // find the pool id that fits the requested event size ...
    uint8_t idx = u8_0;
    while (evtSize
           > static_cast<QEvtSize>(QF_EPOOL_EVENT_SIZE_(QF_pool_[idx])))
    {
        ++idx;
        Q_ASSERT(idx < QF_maxPool_);     // cannot run out of registered pools
    }

    QS_CRIT_STAT_
    QS_BEGIN_(QS_QF_NEW, null_void, null_void)
        QS_TIME_();                                               // timestamp
        QS_EVS_(evtSize);                             // the size of the event
        QS_SIG_(static_cast<QSignal>(sig));         // the signal of the event
    QS_END_()

    QEvt *e;
    QF_EPOOL_GET_(QF_pool_[idx], e);
    Q_ASSERT(e != static_cast<QEvt *>(0));// pool must not run out of events

    e->sig     = static_cast<QSignal>(sig);       // set signal for this event
    e->poolId_ = static_cast<uint8_t>(idx + u8_1); // store pool ID in the evt
    e->refCtr_ = u8_0;                       // set the reference counter to 0

    return e;
}

QP_END_
