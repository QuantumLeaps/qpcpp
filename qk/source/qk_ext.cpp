//////////////////////////////////////////////////////////////////////////////
// Product: QK/C++
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
#include "qk_pkg.h"
//#include "qassert.h"

/// \file
/// \ingroup qk
/// \brief QK_schedExt_() implementation.

QP_BEGIN_

//Q_DEFINE_THIS_MODULE("qk_ext")

QP_END_

//............................................................................
// NOTE: the QK scheduler is entered and exited with interrupts DISABLED.
// QK_schedExt_() is extern "C", so it does not belong to the QP namespace.
//
extern "C" {

void QK_schedExt_(uint8_t p) {

    uint8_t const pin = QK_currPrio_;             // save the initial priority
    QP_ QActive *a;
#ifdef QK_TLS                                    // thread-local storage used?
    uint8_t pprev = pin;
#endif
#ifdef QK_EXT_SAVE                            // extended context-switch used?
    if (pin != static_cast<uint8_t>(0)) { // no extended context for idle loop
        a = QP_ QF::active_[pin];           // the pointer to the preempted AO
        QK_EXT_SAVE(a);                           // save the extended context
    }
#endif
    do {
        a = QP_ QF::active_[p];                // obtain the pointer to the AO
        QK_currPrio_ = p;            // this becomes the current task priority

#ifdef QK_TLS                                    // thread-local storage used?
        if (p != pprev) {                          // are we changing threads?
            QK_TLS(a);                      // switch new thread-local storage
            pprev = p;
        }
#endif
        QS_BEGIN_NOCRIT_(QP_ QS_QK_SCHEDULE, QP_ QS::aoObj_, a)
            QS_TIME_();                                           // timestamp
            QS_U8_(p);                    // the priority of the active object
            QS_U8_(pin);                             // the preempted priority
        QS_END_NOCRIT_()

        QF_INT_ENABLE();                  // unconditionally enable interrupts

        QP_ QEvt const *e = a->get_();     // get the next event for this AO
        a->dispatch(e);                               // dispatch e to this AO
        QP_ QF::gc(e);              // garbage collect the event, if necessary

                        // determine the next highest-priority AO ready to run
        QF_INT_DISABLE();                                // disable interrupts
        p = QK_readySet_.findMax();

#ifdef QK_NO_MUTEX
    } while (p > pin);             // is the new priority higher than initial?
#else                                   // QK priority-ceiling mutexes allowed
    } while ((p > pin) && (p > QK_ceilingPrio_));
#endif

    QK_currPrio_ = pin;                        // restore the initial priority

#if defined(QK_TLS) || defined(QK_EXT_RESTORE)
    if (pin != static_cast<uint8_t>(0)) { // no extended context for idle loop
        a = QP_ QF::active_[pin];           // the pointer to the preempted AO
#ifdef QK_TLS                                    // thread-local storage used?
        QK_TLS(a);                                 // restore the original TLS
#endif
#ifdef QK_EXT_RESTORE                         // extended context-switch used?
        QK_EXT_RESTORE(a);                     // restore the extended context
#endif
    }
#endif
}

}                                                                // extern "C"
