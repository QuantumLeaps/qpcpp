/// \file
/// \brief QP::QHsm::dispatch() and QP::QHsm::hsm_tran() definitions
/// \ingroup qep
/// \cond
///***************************************************************************
/// Product: QEP/C++
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
#include "qep_port.h"     // QEP port
#include "qep_pkg.h"      // QEP internal interface
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qhsm_dis")

//****************************************************************************
/// \description
/// Dispatches an event for processing to a hierarchical state machine (HSM).
/// The processing of an event represents one run-to-completion (RTC) step.
///
/// \arguments
/// \arg[in] \c e  pointer to the event to be dispatched to the HSM
///
/// \note
/// This state machine must be initialized by calling QP::QHsm::init() exactly
/// __once__ before calling QP::QHsm::dispatch().
///
void QHsm::dispatch(QEvt const * const e) {
    QStateHandler t = m_state.fun;

    /// \pre the state configuration must be stable
    Q_REQUIRE_ID(100, t == m_temp.fun);

    QStateHandler s;
    QState r;
    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_DISPATCH, QS::priv_.smObjFilter, this)
        QS_TIME_();              // time stamp
        QS_SIG_(e->sig);         // the signal of the event
        QS_OBJ_(this);           // this state machine object
        QS_FUN_(t);              // the current state
    QS_END_()

    // process the event hierarchically...
    do {
        s = m_temp.fun;
        r = (*s)(this, e); // invoke state handler s

        // unhandled due to a guard?
        if (r == Q_RET_UNHANDLED) {

            QS_BEGIN_(QS_QEP_UNHANDLED, QS::priv_.smObjFilter, this)
                QS_SIG_(e->sig); // the signal of the event
                QS_OBJ_(this);   // this state machine object
                QS_FUN_(s);      // the current state
            QS_END_()

            r = QEP_TRIG_(s, QEP_EMPTY_SIG_); // find superstate of s
        }
    } while (r == Q_RET_SUPER);

    // transition taken?
    if ((r == Q_RET_TRAN) || (r == Q_RET_TRAN_HIST)) {
        QStateHandler path[MAX_NEST_DEPTH_];

        path[0] = m_temp.fun; // save the target of the transition
        path[1] = t;
        path[2] = s;

        // exit current state to transition source s...
        while (t != s) {
            // exit handled?
            if (QEP_TRIG_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {
                QS_BEGIN_(QS_QEP_STATE_EXIT, QS::priv_.smObjFilter, this)
                    QS_OBJ_(this);   // this state machine object
                    QS_FUN_(t);      // the exited state
                QS_END_()

                (void)QEP_TRIG_(t, QEP_EMPTY_SIG_); // find superstate of t
            }
            t = m_temp.fun; // m_temp.fun holds the superstate
        }

        int_fast8_t ip = hsm_tran(path); // take the HSM transition

        // retrace the entry path in reverse (desired) order...
        for (; ip >= sf8_0; --ip) {
            QEP_ENTER_(path[ip]); // enter path[ip]
        }
        t = path[0];    // stick the target into register
        m_temp.fun = t; // update the next state

        // drill into the target hierarchy...
        while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN) {

            QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
                QS_OBJ_(this);       // this state machine object
                QS_FUN_(t);          // the source (pseudo)state
                QS_FUN_(m_temp.fun); // the target of the transition
            QS_END_()

            ip = sf8_0;
            path[0] = m_temp.fun;
            (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_); // find superstate
            while (m_temp.fun != t) {
                ++ip;
                path[ip] = m_temp.fun;
                (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);// find superstate
            }
            m_temp.fun = path[0];

            // entry path must not overflow
            Q_ASSERT(ip < MAX_NEST_DEPTH_);

             // retrace the entry path in reverse (correct) order...
            do {
                QEP_ENTER_(path[ip]);  // enter path[ip]
                --ip;
            } while (ip >= sf8_0);

            t = path[0];
        }

#ifdef Q_SPY
        if (r == Q_RET_TRAN) {

            QS_BEGIN_(QS_QEP_TRAN, QS::priv_.smObjFilter, this)
                QS_TIME_();          // time stamp
                QS_SIG_(e->sig);     // the signal of the event
                QS_OBJ_(this);       // this state machine object
                QS_FUN_(s);          // the source of the transition
                QS_FUN_(t);          // the new active state
            QS_END_()

        }
        else {

            QS_BEGIN_(QS_QEP_TRAN_HIST, QS::priv_.smObjFilter, this)
                QS_TIME_();          // time stamp
                QS_SIG_(e->sig);     // the signal of the event
                QS_OBJ_(this);       // this state machine object
                QS_FUN_(s);          // the source of the transition
                QS_FUN_(t);          // the new active state
            QS_END_()

        }
#endif // Q_SPY

    }
    // transition not taken
    else {
#ifdef Q_SPY
        if (r == Q_RET_HANDLED) {

            QS_BEGIN_(QS_QEP_INTERN_TRAN, QS::priv_.smObjFilter, this)
                QS_TIME_();          // time stamp
                QS_SIG_(e->sig);     // the signal of the event
                QS_OBJ_(this);       // this state machine object
                QS_FUN_(m_state.fun);// the state that handled the event
            QS_END_()

        }
        else {

            QS_BEGIN_(QS_QEP_IGNORED, QS::priv_.smObjFilter, this)
                QS_TIME_();          // time stamp
                QS_SIG_(e->sig);     // the signal of the event
                QS_OBJ_(this);       // this state machine object
                QS_FUN_(m_state.fun);// the current state
            QS_END_()

        }
#endif
    }

    m_state.fun = t; // change the current active state
    m_temp.fun  = t; // mark the configuration as stable
}

//****************************************************************************
/// \description
/// helper function to execute transition sequence in a hierarchical state
/// machine (HSM).
///
/// \arguments
/// \arg[in,out] \c path array of pointers to state-handler functions
///                      to execute the entry actions
///
/// \returns the depth of the entry path stored in the \c path argument.
////
int_fast8_t QHsm::hsm_tran(QStateHandler (&path)[MAX_NEST_DEPTH_]) {
    int_fast8_t ip = sf8_n1;  // transition entry path index
    int_fast8_t iq;           // helper transition entry path index
    QStateHandler t = path[0];
    QStateHandler s = path[2];
    QState r;
    QS_CRIT_STAT_

    // (a) check source==target (transition to self)
    if (s == t) {
        QEP_EXIT_(s);  // exit the source
        ip = sf8_0;    // cause entering the target
    }
    else {
        (void)QEP_TRIG_(t, QEP_EMPTY_SIG_); // superstate of target
        t = m_temp.fun;

        // (b) check source==target->super
        if (s == t) {
            ip = sf8_0; // cause entering the target
        }
        else {
            (void)QEP_TRIG_(s, QEP_EMPTY_SIG_); // superstate of src

            // (c) check source->super==target->super
            if (m_temp.fun == t) {
                QEP_EXIT_(s);  // exit the source
                ip = sf8_0;    // cause entering the target
            }
            else {
                // (d) check source->super==target
                if (m_temp.fun == path[0]) {
                    QEP_EXIT_(s); // exit the source
                }
                else { // (e) check rest of source==target->super->super..
                       // and store the entry path along the way
                       //
                    iq = sf8_0;  // indicate LCA not found
                    ip = sf8_1;  // cause entering target's superst
                    path[1] = t; // save the superstate of target
                    t = m_temp.fun; // save source->super

                    // find target->super->super
                    r = QEP_TRIG_(path[1], QEP_EMPTY_SIG_);
                    while (r == Q_RET_SUPER) {
                        ++ip;
                        path[ip] = m_temp.fun; // store the entry path

                        // is it the source?
                        if (m_temp.fun == s) {
                            iq = sf8_1; // indicate that LCA found

                            // entry path must not overflow
                            Q_ASSERT(ip < MAX_NEST_DEPTH_);
                            --ip;  // do not enter the source
                            r = Q_RET_HANDLED; // terminate the loop
                        }
                        // it is not the source, keep going up
                        else {
                            r = QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);
                        }
                    }

                    // LCA found yet?
                    if (iq == sf8_0) {
                        // entry path must not overflow
                        Q_ASSERT(ip < MAX_NEST_DEPTH_);

                        QEP_EXIT_(s); // exit the source

                        // (f) check the rest of source->super
                        //                  == target->super->super...
                        //
                        iq = ip;
                        r = Q_RET_IGNORED; // indicate LCA NOT found
                        do {
                            // is this the LCA?
                            if (t == path[iq]) {
                                r = Q_RET_HANDLED; // indicate LCA found
                                // do not enter LCA
                                ip = static_cast<int_fast8_t>(iq - sf8_1);
                                iq = sf8_n1; // cause termination of the loop
                            }
                            else {
                                --iq; // try lower superstate of target
                            }
                        } while (iq >= sf8_0);

                        // LCA not found yet?
                        if (r != Q_RET_HANDLED) {
                            // (g) check each source->super->...
                            // for each target->super...
                            //
                            r = Q_RET_IGNORED; // keep looping
                            do {
                                // exit t unhandled?
                                if (QEP_TRIG_(t, Q_EXIT_SIG) == Q_RET_HANDLED)
                                {
                                    QS_BEGIN_(QS_QEP_STATE_EXIT,
                                              QS::priv_.smObjFilter, this)
                                        QS_OBJ_(this);
                                        QS_FUN_(t);
                                    QS_END_()

                                    (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);
                                }
                                t = m_temp.fun; //  set to super of t
                                iq = ip;
                                do {
                                    // is this LCA?
                                    if (t == path[iq]) {
                                        // do not enter LCA
                                        ip =
                                         static_cast<int_fast8_t>(iq - sf8_1);
                                        iq = sf8_n1;// break out of inner loop
                                        r = Q_RET_HANDLED; // break outer loop
                                    }
                                    else {
                                        --iq;
                                    }
                                } while (iq >= sf8_0);
                            } while (r != Q_RET_HANDLED);
                        }
                    }
                }
            }
        }
    }
    return ip;
}

} // namespace QP

