//****************************************************************************
// Product: QF/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 02, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
#include "qf_pkg.h"
#include "qassert.h"

/// \file
/// \ingroup qf
/// \brief QActive::defer() and QActive::recall() implementation.
///

namespace QP {

Q_DEFINE_THIS_MODULE("qa_defer")

//............................................................................
bool QActive::defer(QEQueue * const eq, QEvt const * const e) const {
    return eq->post(e, static_cast<uint_t>(1));       // non-asserting posting
}
//............................................................................
bool QActive::recall(QEQueue * const eq) {
    QEvt const * const e = eq->get();    // try to get evt from deferred queue
    bool const recalled = (e != null_evt);                 // event available?
    if (recalled) {
        this->postLIFO(e);         // post it to the _front_ of the AO's queue

        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();

        if (e->poolId_ != u8_0) {                    // is it a dynamic event?

            // after posting to the AO's queue the event must be referenced
            // at least twice: once in the deferred event queue (eq->get()
            // did NOT decrement the reference counter) and once in the
            // AO's event queue.
            Q_ASSERT(e->refCtr_ > u8_1);

            // we need to decrement the reference counter once, to account
            // for removing the event from the deferred event queue.
            //
            QF_EVT_REF_CTR_DEC_(e);         // decrement the reference counter
        }

        QF_CRIT_EXIT_();
    }
    return recalled;                                     // event not recalled
}

}                                                              // namespace QP


