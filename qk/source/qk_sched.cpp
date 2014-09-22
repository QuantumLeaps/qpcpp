/// \file
/// \brief QK_schedPrio_() and QK scheduler QK_sched_() definition.
/// \ingroup qk
/// \cond
///***************************************************************************
/// Product: QK/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-10
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

#define QP_IMPL           // this is QF/QK implementation
#include "qf_port.h"      // QF port
#include "qk_pkg.h"       // QK package-scope internal interface
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

extern "C" {

//****************************************************************************
/// \description
/// This function finds out the priority of the highest-priority active object
/// that (1) has events to process, and (2) has priority that is above the
/// current priority, and (3) has priority that is above the mutex ceiling,
/// if mutex is configured in the port.
///
/// \returns the 1-based priority of the the active object, or zero if
/// no eligible active object is ready to run.
///
/// \attention QK_schedPrio_() must be always called with interrupts
/// __disabled__  and returns with interrupts __disabled__.
///
uint_fast8_t QK_schedPrio_(void) {
    uint_fast8_t p = QK_readySet_.findMax();

    // is the priority below the current preemption threshold?
    if (p <= QK_currPrio_) {
        p = static_cast<uint_fast8_t>(0);
    }
#ifndef QK_NO_MUTEX
    // is the priority below the mutex ceiling??
    else if (p <= QK_ceilingPrio_) {
        p = static_cast<uint_fast8_t>(0);
    }
    else {
        // empty
    }
#endif // QK_NO_MUTEX

    return p;
}

//****************************************************************************
/// \arguments
/// \arg[in] \c p  priority of the next AO to schedule
///
/// \attention QK_sched_() must be always called with interrupts
/// __disabled__  and returns with interrupts __disabled__.
///
/// \note The scheduler might enable interrupts internally, but always
/// returns with interrupts __disabled__.
///
void QK_sched_(uint_fast8_t p) {

    uint_fast8_t pin = QK_currPrio_; // save the initial priority
    QP::QActive *a;
#ifdef QK_TLS // thread-local storage used?
    uint_fast8_t pprev = pin;
#endif
    do {
        a = QP::QF::active_[p]; // obtain the pointer to the AO
        QK_currPrio_ = p; // this becomes the current task priority

#ifdef QK_TLS // thread-local storage used?
        // are we changing threads?
        if (p != pprev) {
            QK_TLS(a); // switch new thread-local storage
            pprev = p;
        }
#endif // QK_TLS

        QS_BEGIN_NOCRIT_(QP::QS_QK_SCHEDULE, QP::QS::priv_.aoObjFilter, a)
            QS_TIME_();  // timestamp
            QS_U8_(p);   // the priority of the active object
            QS_U8_(pin); // the preempted priority
        QS_END_NOCRIT_()

        QF_INT_ENABLE();    // unconditionally enable interrupts

        // perform the run-to-completion (RTS) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and QActive_get_() asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QP::QEvt const *e = a->get_();
        a->dispatch(e);
        QP::QF::gc(e);

        // determine the next highest-priority AO ready to run...
        QF_INT_DISABLE();
        p = QK_readySet_.findMax(); // new highest-prio AO ready to run

        // below the current preemption threshold?
        if (p <= pin) {
            p = static_cast<uint_fast8_t>(0);
        }
#ifndef QK_NO_MUTEX
        // below the mutex ceiling?
        else if (p <= QK_ceilingPrio_) {
            p = static_cast<uint_fast8_t>(0);
        }
        else {
            // empty
        }
#endif // QK_NO_MUTEX

    } while (p != static_cast<uint_fast8_t>(0));

    QK_currPrio_ = pin; // restore the initial priority

#ifdef QK_TLS // thread-local storage used?
    // no extended context for idle loop
    if (pin != static_cast<uint_fast8_t>(0)) {
        a = QP::QF::active_[pin]; // the pointer to the preempted AO
        QK_TLS(a); // restore the original TLS
    }
#endif // QK_TLS
}

} // extern "C"
