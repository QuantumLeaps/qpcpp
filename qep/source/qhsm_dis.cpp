//////////////////////////////////////////////////////////////////////////////
// Product: QEP/C++
// Last Updated for Version: 4.5.01
// Date of the Last Update:  Jun 13, 2012
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
#include "qep_pkg.h"
#include "qassert.h"

/// \file
/// \ingroup qep
/// \brief QHsm::dispatch() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qhsm_dis")

//............................................................................
void QHsm::dispatch(QEvt const * const e) {
    QStateHandler t = m_state;

    Q_REQUIRE(t == m_temp);          // the state configuration must be stable

    QStateHandler s;
    QState r;
    QS_CRIT_STAT_

    QS_BEGIN_(QS_QEP_DISPATCH, QS::smObj_, this)
        QS_TIME_();                                              // time stamp
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(t);                                       // the current state
    QS_END_()

    do {                                // process the event hierarchically...
        s = m_temp;
        r = (*s)(this, e);                           // invoke state handler s

        if (r == Q_RET_UNHANDLED) {               // unhandled due to a guard?

            QS_BEGIN_(QS_QEP_UNHANDLED, QS::smObj_, this)
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(s);                               // the current state
            QS_END_()

            r = QEP_TRIG_(s, QEP_EMPTY_SIG_);          // find superstate of s
        }
    } while (r == Q_RET_SUPER);

    if (r == Q_RET_TRAN) {                                // transition taken?
        QStateHandler path[QEP_MAX_NEST_DEPTH_];
        int8_t ip = s8_n1;                      // transition entry path index
        int8_t iq;                       // helper transition entry path index
#ifdef Q_SPY
        QStateHandler src = s;       // save the transition source for tracing
#endif

        path[0] = m_temp;                 // save the target of the transition
        path[1] = t;

        while (t != s) {       // exit current state to transition source s...
            if (QEP_TRIG_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {   //exit handled?
                QS_BEGIN_(QS_QEP_STATE_EXIT, QS::smObj_, this)
                    QS_OBJ_(this);                // this state machine object
                    QS_FUN_(t);                            // the exited state
                QS_END_()

                (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);    // find superstate of t
            }
            t = m_temp;                         // m_temp holds the superstate
        }

        t = path[0];                               // target of the transition

        if (s == t) {         // (a) check source==target (transition to self)
            QEP_EXIT_(s);                                   // exit the source
            ip = s8_0;                                     // enter the target
        }
        else {
            (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);        // superstate of target
            t = m_temp;
            if (s == t) {                   // (b) check source==target->super
                ip = s8_0;               // enter the target
            }
            else {
                (void)QEP_TRIG_(s, QEP_EMPTY_SIG_);       // superstate of src
                                     // (c) check source->super==target->super
                if (m_temp == t) {
                    QEP_EXIT_(s);                           // exit the source
                    ip = s8_0;           // enter the target
                }
                else {
                                            // (d) check source->super==target
                    if (m_temp == path[0]) {
                        QEP_EXIT_(s);                       // exit the source
                    }
                    else { // (e) check rest of source==target->super->super..
                           // and store the entry path along the way
                           //
                        iq = s8_0; // indicate LCA not found
                        ip = s8_1; // enter target's superst
                        path[1] = t;          // save the superstate of target
                        t = m_temp;                      // save source->super
                                                  // find target->super->super
                        r = QEP_TRIG_(path[1], QEP_EMPTY_SIG_);
                        while (r == Q_RET_SUPER) {
                            ++ip;
                            path[ip] = m_temp;         // store the entry path
                            if (m_temp == s) {            // is it the source?
                                                    // indicate that LCA found
                                iq = s8_1;
                                               // entry path must not overflow
                                Q_ASSERT(ip < QEP_MAX_NEST_DEPTH_);
                                --ip;               // do not enter the source
                                r = Q_RET_HANDLED;       // terminate the loop
                            }
                            else {      // it is not the source, keep going up
                                r = QEP_TRIG_(m_temp, QEP_EMPTY_SIG_);
                            }
                        }
                        if (iq == s8_0) {  // LCA found yet?

                                               // entry path must not overflow
                            Q_ASSERT(ip < QEP_MAX_NEST_DEPTH_);

                            QEP_EXIT_(s);                   // exit the source

                            // (f) check the rest of source->super
                            //                  == target->super->super...
                            //
                            iq = ip;
                            r = Q_RET_IGNORED;       // indicate LCA NOT found
                            do {
                                if (t == path[iq]) {       // is this the LCA?
                                    r = Q_RET_HANDLED;   // indicate LCA found
                                                           // do not enter LCA
                                    ip = static_cast<int8_t>(iq - s8_1);
                                                         // terminate the loop
                                    iq = s8_n1;
                                }
                                else {
                                    --iq;    // try lower superstate of target
                                }
                            } while (iq >= s8_0);

                            if (r != Q_RET_HANDLED) {    // LCA not found yet?
                                // (g) check each source->super->...
                                // for each target->super...
                                //
                                r = Q_RET_IGNORED;             // keep looping
                                do {
                                                          // exit t unhandled?
                                    if (QEP_TRIG_(t, Q_EXIT_SIG)
                                        == Q_RET_HANDLED)
                                    {
                                        QS_BEGIN_(QS_QEP_STATE_EXIT,
                                                  QS::smObj_, this)
                                            QS_OBJ_(this);
                                            QS_FUN_(t);
                                        QS_END_()

                                        (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);
                                    }
                                    t = m_temp;          //  set to super of t
                                    iq = ip;
                                    do {
                                        if (t == path[iq]) {   // is this LCA?
                                                           // do not enter LCA
                                            ip = static_cast<int8_t>(iq-s8_1);
                                                // break out of the inner loop
                                            iq = s8_n1;
                                            r = Q_RET_HANDLED;  // break outer
                                        }
                                        else {
                                            --iq;
                                        }
                                    } while (iq >= s8_0);
                                } while (r != Q_RET_HANDLED);
                            }
                        }
                    }
                }
            }
        }
                       // retrace the entry path in reverse (desired) order...
        for (; ip >= s8_0; --ip) {
            QEP_ENTER_(path[ip]);                            // enter path[ip]
        }
        t = path[0];                         // stick the target into register
        m_temp = t;                                   // update the next state

                                         // drill into the target hierarchy...
        while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN) {

            QS_BEGIN_(QS_QEP_STATE_INIT, QS::smObj_, this)
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(t);                        // the source (pseudo)state
                QS_FUN_(m_temp);              // the target of the transition
            QS_END_()

            ip = s8_0;
            path[0] = m_temp;
            (void)QEP_TRIG_(m_temp, QEP_EMPTY_SIG_);       // find superstate
            while (m_temp != t) {
                ++ip;
                path[ip] = m_temp;
                (void)QEP_TRIG_(m_temp, QEP_EMPTY_SIG_);   // find superstate
            }
            m_temp = path[0];
                                               // entry path must not overflow
            Q_ASSERT(ip < QEP_MAX_NEST_DEPTH_);

            do {       // retrace the entry path in reverse (correct) order...
                QEP_ENTER_(path[ip]);                        // enter path[ip]
                --ip;
            } while (ip >= s8_0);

            t = path[0];
        }

        QS_BEGIN_(QS_QEP_TRAN, QS::smObj_, this)
            QS_TIME_();                                          // time stamp
            QS_SIG_(e->sig);                        // the signal of the event
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_(src);                      // the source of the transition
            QS_FUN_(t);                                // the new active state
        QS_END_()

    }
    else {                                             // transition not taken
#ifdef Q_SPY
        if (r == Q_RET_HANDLED) {

            QS_BEGIN_(QS_QEP_INTERN_TRAN, QS::smObj_, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state);          // the state that handled the event
            QS_END_()

        }
        else {

            QS_BEGIN_(QS_QEP_IGNORED, QS::smObj_, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(m_state);                         // the current state
            QS_END_()

        }
#endif
    }

    m_state = t;                            // change the current active state
    m_temp  = t;                           // mark the configuration as stable
}

QP_END_
