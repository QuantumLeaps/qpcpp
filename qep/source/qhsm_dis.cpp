//****************************************************************************
// Product: QEP/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 27, 2013
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
#include "qep_pkg.h"
#include "qassert.h"

/// \file
/// \ingroup qep
/// \brief QHsm::dispatch() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qhsm_dis")

//............................................................................
void QHsm::dispatch(QEvt const * const e) {
    QStateHandler t = m_state.fun;

    Q_REQUIRE(t == m_temp.fun);      // the state configuration must be stable

    QStateHandler s;
    QState r;
    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_DISPATCH, QS::priv_.smObjFilter, this)
        QS_TIME_();                                              // time stamp
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(t);                                       // the current state
    QS_END_()

    do {                                // process the event hierarchically...
        s = m_temp.fun;
        r = (*s)(this, e);                           // invoke state handler s

        if (r == Q_RET_UNHANDLED) {               // unhandled due to a guard?

            QS_BEGIN_(QS_QEP_UNHANDLED, QS::priv_.smObjFilter, this)
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(s);                               // the current state
            QS_END_()

            r = QEP_TRIG_(s, QEP_EMPTY_SIG_);          // find superstate of s
        }
    } while (r == Q_RET_SUPER);

    if (r == Q_RET_TRAN) {                                // transition taken?
        QStateHandler path[MAX_NEST_DEPTH];

        path[0] = m_temp.fun;             // save the target of the transition
        path[1] = t;
        path[2] = s;

        while (t != s) {       // exit current state to transition source s...
            if (QEP_TRIG_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {   //exit handled?
                QS_BEGIN_(QS_QEP_STATE_EXIT, QS::priv_.smObjFilter, this)
                    QS_OBJ_(this);                // this state machine object
                    QS_FUN_(t);                            // the exited state
                QS_END_()

                (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);    // find superstate of t
            }
            t = m_temp.fun;                 // m_temp.fun holds the superstate
        }

        int_t ip = hsm_tran(path);                  // take the HSM transition

                       // retrace the entry path in reverse (desired) order...
        for (; ip >= s_0; --ip) {
            QEP_ENTER_(path[ip]);                            // enter path[ip]
        }
        t = path[0];                         // stick the target into register
        m_temp.fun = t;                               // update the next state

                                         // drill into the target hierarchy...
        while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN) {

            QS_BEGIN_(QS_QEP_STATE_INIT, QS::priv_.smObjFilter, this)
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(t);                        // the source (pseudo)state
                QS_FUN_(m_temp.fun);           // the target of the transition
            QS_END_()

            ip = s_0;
            path[0] = m_temp.fun;
            (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);    // find superstate
            while (m_temp.fun != t) {
                ++ip;
                path[ip] = m_temp.fun;
                (void)QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);// find superstate
            }
            m_temp.fun = path[0];
                                               // entry path must not overflow
            Q_ASSERT(ip < MAX_NEST_DEPTH);

            do {       // retrace the entry path in reverse (correct) order...
                QEP_ENTER_(path[ip]);                        // enter path[ip]
                --ip;
            } while (ip >= s_0);

            t = path[0];
        }

        QS_BEGIN_(QS_QEP_TRAN, QS::priv_.smObjFilter, this)
            QS_TIME_();                                          // time stamp
            QS_SIG_(e->sig);                        // the signal of the event
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_(s);                        // the source of the transition
            QS_FUN_(t);                                // the new active state
        QS_END_()

    }
    else {                                             // transition not taken
#ifdef Q_SPY
        if (r == Q_RET_HANDLED) {

            QS_BEGIN_(QS_QEP_INTERN_TRAN, QS::priv_.smObjFilter, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state.fun);      // the state that handled the event
            QS_END_()

        }
        else {

            QS_BEGIN_(QS_QEP_IGNORED, QS::priv_.smObjFilter, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state.fun);                     // the current state
            QS_END_()

        }
#endif
    }

    m_state.fun = t;                        // change the current active state
    m_temp.fun  = t;                       // mark the configuration as stable
}
//............................................................................
int_t QHsm::hsm_tran(QStateHandler (&path)[MAX_NEST_DEPTH]) {
    int_t ip = s_n1;                            // transition entry path index
    int_t iq;                            // helper transition entry path index
    QStateHandler t = path[0];
    QStateHandler s = path[2];
    QState r;
    QS_CRIT_STAT_

    if (s == t) {             // (a) check source==target (transition to self)
        QEP_EXIT_(s);                                       // exit the source
        ip = s_0;                                          // enter the target
    }
    else {
        (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);            // superstate of target
        t = m_temp.fun;
        if (s == t) {                       // (b) check source==target->super
            ip = s_0;                                      // enter the target
        }
        else {
            (void)QEP_TRIG_(s, QEP_EMPTY_SIG_);           // superstate of src
                                     // (c) check source->super==target->super
            if (m_temp.fun == t) {
                QEP_EXIT_(s);                               // exit the source
                ip = s_0;                                  // enter the target
            }
            else {
                                            // (d) check source->super==target
                if (m_temp.fun == path[0]) {
                    QEP_EXIT_(s);                           // exit the source
                }
                else { // (e) check rest of source==target->super->super..
                       // and store the entry path along the way
                       //
                    iq = s_0;                        // indicate LCA not found
                    ip = s_1;                        // enter target's superst
                    path[1] = t;              // save the superstate of target
                    t = m_temp.fun;                      // save source->super
                                                  // find target->super->super
                    r = QEP_TRIG_(path[1], QEP_EMPTY_SIG_);
                    while (r == Q_RET_SUPER) {
                        ++ip;
                        path[ip] = m_temp.fun;         // store the entry path
                        if (m_temp.fun == s) {            // is it the source?
                                                    // indicate that LCA found
                            iq = s_1;
                                               // entry path must not overflow
                            Q_ASSERT(ip < MAX_NEST_DEPTH);
                            --ip;                   // do not enter the source
                            r = Q_RET_HANDLED;           // terminate the loop
                        }
                        else {      // it is not the source, keep going up
                            r = QEP_TRIG_(m_temp.fun, QEP_EMPTY_SIG_);
                        }
                    }
                    if (iq == s_0) {                         // LCA found yet?
                                               // entry path must not overflow
                        Q_ASSERT(ip < MAX_NEST_DEPTH);

                        QEP_EXIT_(s);                       // exit the source

                        // (f) check the rest of source->super
                        //                  == target->super->super...
                        //
                        iq = ip;
                        r = Q_RET_IGNORED;           // indicate LCA NOT found
                        do {
                            if (t == path[iq]) {           // is this the LCA?
                                r = Q_RET_HANDLED;       // indicate LCA found
                                                           // do not enter LCA
                                ip = static_cast<int_t>(iq - s_1);
                                                         // terminate the loop
                                iq = s_n1;
                            }
                            else {
                                --iq;        // try lower superstate of target
                            }
                        } while (iq >= s_0);

                        if (r != Q_RET_HANDLED) {        // LCA not found yet?
                            // (g) check each source->super->...
                            // for each target->super...
                            //
                            r = Q_RET_IGNORED;                 // keep looping
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
                                t = m_temp.fun;          //  set to super of t
                                iq = ip;
                                do {
                                    if (t == path[iq]) {       // is this LCA?
                                                           // do not enter LCA
                                        ip = static_cast<int_t>(iq - s_1);
                                                // break out of the inner loop
                                        iq = s_n1;
                                        r = Q_RET_HANDLED;      // break outer
                                    }
                                    else {
                                        --iq;
                                    }
                                } while (iq >= s_0);
                            } while (r != Q_RET_HANDLED);
                        }
                    }
                }
            }
        }
    }
    return ip;
}

}                                                              // namespace QP

