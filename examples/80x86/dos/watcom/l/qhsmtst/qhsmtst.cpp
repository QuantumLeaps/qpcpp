//////////////////////////////////////////////////////////////////////////////
// Product: QHsmTst Example
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 20, 2012
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
#include "qep_port.h"
#include "qhsmtst.h"

// local-scope objects .......................................................
class QHsmTst : public QHsm {
    int8_t m_foo;                                   // extended state variable

public:
    QHsmTst() : QHsm((QStateHandler)&QHsmTst::initial) {               // ctor
    }

protected:
    static QState initial(QHsmTst *me, QEvt const *e);// initial pseudostate
    static QState s      (QHsmTst *me, QEvt const *e);      // state-handler
    static QState s1     (QHsmTst *me, QEvt const *e);      // state-handler
    static QState s11    (QHsmTst *me, QEvt const *e);      // state-handler
    static QState s2     (QHsmTst *me, QEvt const *e);      // state-handler
    static QState s21    (QHsmTst *me, QEvt const *e);      // state-handler
    static QState s211   (QHsmTst *me, QEvt const *e);      // state-handler
};

static QHsmTst l_hsmtst;      // the sole instance of the state machine object

// global-scope definitions --------------------------------------------------
extern QHsm * const the_hsm = &l_hsmtst;                 // the opaque pointer

//............................................................................
QState QHsmTst::initial(QHsmTst *me, QEvt const *) {
    BSP_display("top-INIT;");
    me->m_foo = 0;                       // initialize extended state variable
    return Q_TRAN(&QHsmTst::s2);
}

//............................................................................
QState QHsmTst::s(QHsmTst *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_display("s-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_display("s-EXIT;");
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            BSP_display("s-INIT;");
            return Q_TRAN(&QHsmTst::s11);
        }
        case E_SIG: {
            BSP_display("s-E;");
            return Q_TRAN(&QHsmTst::s11);
        }
        case I_SIG: {                      // internal transition with a guard
            if (me->m_foo) {
                BSP_display("s-I;");
                me->m_foo = 0;
                return Q_HANDLED();
            }
            break;
        }
        case TERMINATE_SIG: {
            BSP_exit();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState QHsmTst::s1(QHsmTst *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_display("s1-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_display("s1-EXIT;");
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            BSP_display("s1-INIT;");
            return Q_TRAN(&QHsmTst::s11);
        }
        case A_SIG: {
            BSP_display("s1-A;");
            return Q_TRAN(&QHsmTst::s1);
        }
        case B_SIG: {
            BSP_display("s1-B;");
            return Q_TRAN(&QHsmTst::s11);
        }
        case C_SIG: {
            BSP_display("s1-C;");
            return Q_TRAN(&QHsmTst::s2);
        }
        case D_SIG: {                               // transition with a gurad
            if (!me->m_foo) {
                BSP_display("s1-D;");
                me->m_foo = 1;
                return Q_TRAN(&QHsmTst::s);
            }
            break;
        }
        case F_SIG: {
            BSP_display("s1-F;");
            return Q_TRAN(&QHsmTst::s211);
        }
        case I_SIG: {                                   // internal transition
            BSP_display("s1-I;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsmTst::s);
}
//............................................................................
QState QHsmTst::s11(QHsmTst *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_display("s11-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_display("s11-EXIT;");
            return Q_HANDLED();
        }
        case D_SIG: {                               // transition with a gurad
            if (me->m_foo) {
                BSP_display("s11-D;");
                me->m_foo = 0;
                return Q_TRAN(&QHsmTst::s1);
            }
            break;
        }
        case G_SIG: {
            BSP_display("s11-G;");
            return Q_TRAN(&QHsmTst::s211);
        }
        case H_SIG: {
            BSP_display("s11-H;");
            return Q_TRAN(&QHsmTst::s);
        }
    }
    return Q_SUPER(&QHsmTst::s1);
}
//............................................................................
QState QHsmTst::s2(QHsmTst *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_display("s2-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_display("s2-EXIT;");
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            BSP_display("s2-INIT;");
            return Q_TRAN(&QHsmTst::s211);
        }
        case C_SIG: {
            BSP_display("s2-C;");
            return Q_TRAN(&QHsmTst::s1);
        }
        case F_SIG: {
            BSP_display("s2-F;");
            return Q_TRAN(&QHsmTst::s11);
        }
        case I_SIG: {                      // internal transition with a guard
            if (!me->m_foo) {
                BSP_display("s2-I;");
                me->m_foo = (uint8_t)1;
                return Q_HANDLED();
            }
            break;
        }
    }
    return Q_SUPER(&QHsmTst::s);
}
//............................................................................
QState QHsmTst::s21(QHsmTst *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_display("s21-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_display("s21-EXIT;");
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            BSP_display("s21-INIT;");
            return Q_TRAN(&QHsmTst::s211);
        }
        case A_SIG: {
            BSP_display("s21-A;");
            return Q_TRAN(&QHsmTst::s21);
        }
        case B_SIG: {
            BSP_display("s21-B;");
            return Q_TRAN(&QHsmTst::s211);
        }
        case G_SIG: {
            BSP_display("s21-G;");
            return Q_TRAN(&QHsmTst::s1);
        }
    }
    return Q_SUPER(&QHsmTst::s2);
}
//............................................................................
QState QHsmTst::s211(QHsmTst *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_display("s211-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_display("s211-EXIT;");
            return Q_HANDLED();
        }
        case D_SIG: {
            BSP_display("s211-D;");
            return Q_TRAN(&QHsmTst::s21);
        }
        case H_SIG: {
            BSP_display("s211-H;");
            return Q_TRAN(&QHsmTst::s);
        }
    }
    return Q_SUPER(&QHsmTst::s21);
}
