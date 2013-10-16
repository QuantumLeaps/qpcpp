//****************************************************************************
// Product: Calc1ulator Example with inheritance of the whole state model
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
#include "qp_port.h"                           // the port of the QP framework
#include "bsp.h"                                      // board support package
#include "calc1.h"


// state variables -----------------------------------------------------------
QStateHandler Calc1::state_on        = (QStateHandler)&Calc1::on;
QStateHandler Calc1::state_error     = (QStateHandler)&Calc1::error;
QStateHandler Calc1::state_ready     = (QStateHandler)&Calc1::ready;
QStateHandler Calc1::state_result    = (QStateHandler)&Calc1::result;
QStateHandler Calc1::state_begin     = (QStateHandler)&Calc1::begin;
QStateHandler Calc1::state_negated1  = (QStateHandler)&Calc1::negated1;
QStateHandler Calc1::state_operand1  = (QStateHandler)&Calc1::operand1;
QStateHandler Calc1::state_zero1     = (QStateHandler)&Calc1::zero1;
QStateHandler Calc1::state_int1      = (QStateHandler)&Calc1::int1;
QStateHandler Calc1::state_frac1     = (QStateHandler)&Calc1::frac1;
QStateHandler Calc1::state_opEntered = (QStateHandler)&Calc1::opEntered;
QStateHandler Calc1::state_negated2  = (QStateHandler)&Calc1::negated2;
QStateHandler Calc1::state_operand2  = (QStateHandler)&Calc1::operand2;
QStateHandler Calc1::state_zero2     = (QStateHandler)&Calc1::zero2;
QStateHandler Calc1::state_int2      = (QStateHandler)&Calc1::int2;
QStateHandler Calc1::state_frac2     = (QStateHandler)&Calc1::frac2;
QStateHandler Calc1::state_final     = (QStateHandler)&Calc1::final;


// HSM definition ------------------------------------------------------------
QState Calc1::initial(Calc1 *me, QEvt const * /* e */) {
    BSP_clear();
    return Q_TRAN(state_on);
}
//............................................................................
QState Calc1::on(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("on-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("on-EXIT;");
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            BSP_message("on-INIT;");
            return Q_TRAN(state_ready);
        }
        case C_SIG: {
            BSP_clear();
            return Q_TRAN(state_on);                     // transition-to-self
        }
        case OFF_SIG: {
            return Q_TRAN(state_final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Calc1::error(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("error-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("error-EXIT;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(state_on);
}
//............................................................................
QState Calc1::ready(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("ready-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("ready-EXIT;");
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            BSP_message("ready-INIT;");
            return Q_TRAN(state_begin);
        }
        case DIGIT_0_SIG: {
            BSP_clear();
            return Q_TRAN(state_zero1);
        }
        case DIGIT_1_9_SIG: {
            BSP_clear();
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_int1);
        }
        case POINT_SIG: {
            BSP_clear();
            BSP_insert((int)'0');
            BSP_insert((int)'.');
            return Q_TRAN(state_frac1);
        }
        case OPER_SIG: {
            me->m_operand1 = BSP_get_value();
            me->m_operator = ((CalcEvt const *)e)->key_code;
            return Q_TRAN(state_opEntered);
        }
    }
    return Q_SUPER(state_on);
}
//............................................................................
QState Calc1::result(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("result-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("result-EXIT;");
            return Q_HANDLED();
        }
    }
    return Q_SUPER(state_ready);
}
//............................................................................
QState Calc1::begin(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("begin-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("begin-EXIT;");
            return Q_HANDLED();
        }
        case OPER_SIG: {
            if (((CalcEvt const *)e)->key_code == KEY_MINUS) {
                return Q_TRAN(state_negated1);
            }
            break;
        }
    }
    return Q_SUPER(state_ready);
}
//............................................................................
QState Calc1::negated1(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("negated1-ENTRY;");
            BSP_negate();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("negated1-EXIT;");
            return Q_HANDLED();
        }
        case OPER_SIG: {
            if (((CalcEvt const *)e)->key_code == KEY_MINUS) {
                ;                                         // explicitly ignore
                return Q_HANDLED();                           // event handled
            }
            break;
        }
        case CE_SIG: {
            BSP_clear();
            return Q_TRAN(state_begin);
        }
        case DIGIT_0_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_zero1);
        }
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_int1);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_frac1);
        }
    }
    return Q_SUPER(state_on);
}
//............................................................................
QState Calc1::negated2(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("negated2-ENTRY;");
            BSP_negate();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("negated2-EXIT;");
            return Q_HANDLED();
        }
        case OPER_SIG: {
            if (((CalcEvt const *)e)->key_code == KEY_MINUS) {
                ;                                         // explicitly ignore
                return Q_HANDLED();                           // event handled
            }
            break;
        }
        case CE_SIG: {
            BSP_clear();
            return Q_TRAN(state_opEntered);
        }
        case DIGIT_0_SIG: {
            return Q_TRAN(state_zero2);
        }
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_int2);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_frac2);
        }
    }
    return Q_SUPER(state_on);
}
//............................................................................
QState Calc1::operand1(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("operand1-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("operand1-EXIT;");
            return Q_HANDLED();
        }
        case CE_SIG: {
            BSP_clear();
            return Q_TRAN(state_begin);
        }
        case OPER_SIG: {
            me->m_operand1 = BSP_get_value();
            me->m_operator = ((CalcEvt const *)e)->key_code;
            return Q_TRAN(state_opEntered);
        }
        case EQUALS_SIG: {
            return Q_TRAN(state_result);
        }
    }
    return Q_SUPER(state_on);
}
//............................................................................
QState Calc1::zero1(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("zero1-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("zero1-EXIT;");
            return Q_HANDLED();
        }
        case DIGIT_0_SIG: {
            ;                                             // explicitly ignore
            return Q_HANDLED();
        }
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_int1);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_frac1);
        }
    }
    return Q_SUPER(state_operand1);
}
//............................................................................
QState Calc1::int1(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("int1-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("int1-EXIT;");
            return Q_HANDLED();
        }
        case DIGIT_0_SIG:                        // intentionally fall through
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_HANDLED();
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_frac1);
        }
    }
    return Q_SUPER(state_operand1);
}
//............................................................................
QState Calc1::frac1(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("frac1-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("frac1-EXIT;");
            return Q_HANDLED();
        }
        case DIGIT_0_SIG:                        // intentionally fall through
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_HANDLED();
        }
        case POINT_SIG: {
            ;                                             // explicitly ignore
            return Q_HANDLED();
        }
    }
    return Q_SUPER(state_operand1);
}
//............................................................................
QState Calc1::opEntered(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("opEntered-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("opEntered-EXIT;");
            return Q_HANDLED();
        }
        case OPER_SIG: {
            if (((CalcEvt const *)e)->key_code == KEY_MINUS) {
                BSP_clear();
                return Q_TRAN(state_negated2);
            }
            break;
        }
        case DIGIT_0_SIG: {
            BSP_clear();
            return Q_TRAN(state_zero2);
        }
        case DIGIT_1_9_SIG: {
            BSP_clear();
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_int2);
        }
        case POINT_SIG: {
            BSP_clear();
            BSP_insert((int)'0');
            BSP_insert((int)'.');
            return Q_TRAN(state_frac2);
        }
    }
    return Q_SUPER(state_on);
}
//............................................................................
QState Calc1::operand2(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("operand2-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("operand2-EXIT;");
            return Q_HANDLED();
        }
        case CE_SIG: {
            BSP_clear();
            return Q_TRAN(state_opEntered);
        }
        case OPER_SIG: {
            if (BSP_eval(me->m_operand1, me->m_operator, BSP_get_value())) {
                me->m_operand1 = BSP_get_value();
                me->m_operator = ((CalcEvt const *)e)->key_code;
                return Q_TRAN(state_opEntered);
            }
            else {
                return Q_TRAN(state_error);
            }
            return Q_HANDLED();
        }
        case EQUALS_SIG: {
            if (BSP_eval(me->m_operand1, me->m_operator, BSP_get_value())) {
                return Q_TRAN(state_result);
            }
            else {
                return Q_TRAN(state_error);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(state_on);
}
//............................................................................
QState Calc1::zero2(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("zero2-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("zero2-EXIT;");
            return Q_HANDLED();
        }
        case DIGIT_0_SIG: {
            ;                                             // explicitly ignore
            return Q_HANDLED();
        }
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_int2);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_frac2);
        }
    }
    return Q_SUPER(state_operand2);
}
//............................................................................
QState Calc1::int2(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("int2-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("int2-EXIT;");
            return Q_HANDLED();
        }
        case DIGIT_0_SIG:                        // intentionally fall through
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_HANDLED();
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(state_frac2);
        }
    }
    return Q_SUPER(state_operand2);
}
//............................................................................
QState Calc1::frac2(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("frac2-ENTRY;");
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            BSP_message("frac2-EXIT;");
            return Q_HANDLED();
        }
        case DIGIT_0_SIG:                        // intentionally fall through
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_HANDLED();
        }
        case POINT_SIG: {
            ;                                             // explicitly ignore
            return Q_HANDLED();
        }
   }
    return Q_SUPER(state_operand2);
}
//............................................................................
QState Calc1::final(Calc1 *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("final-ENTRY;");
            BSP_exit();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}

