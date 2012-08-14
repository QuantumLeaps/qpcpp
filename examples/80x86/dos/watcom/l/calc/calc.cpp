//////////////////////////////////////////////////////////////////////////////
// Product:  Calculator Example
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
#include "qp_port.h"                           // the port of the QP framework
#include "bsp.h"                                      // board support package
#include "calc.h"

class Calc : public QHsm {
private:
    double  m_operand1;                              // the value of operand 1
    uint8_t m_operator;                                // operator key entered

public:
    Calc() : QHsm((QStateHandler)&Calc::initial) {                     // ctor
    }

private:
    static QState initial  (Calc *me, QEvt const *e); // initial pseudostate
    static QState on       (Calc *me, QEvt const *e);       // state handler
    static QState error    (Calc *me, QEvt const *e);       // state handler
    static QState ready    (Calc *me, QEvt const *e);       // state handler
    static QState result   (Calc *me, QEvt const *e);       // state handler
    static QState begin    (Calc *me, QEvt const *e);       // state handler
    static QState negated1 (Calc *me, QEvt const *e);       // state handler
    static QState operand1 (Calc *me, QEvt const *e);       // state handler
    static QState zero1    (Calc *me, QEvt const *e);       // state handler
    static QState int1     (Calc *me, QEvt const *e);       // state handler
    static QState frac1    (Calc *me, QEvt const *e);       // state handler
    static QState opEntered(Calc *me, QEvt const *e);       // state handler
    static QState negated2 (Calc *me, QEvt const *e);       // state handler
    static QState operand2 (Calc *me, QEvt const *e);       // state handler
    static QState zero2    (Calc *me, QEvt const *e);       // state handler
    static QState int2     (Calc *me, QEvt const *e);       // state handler
    static QState frac2    (Calc *me, QEvt const *e);       // state handler
    static QState final    (Calc *me, QEvt const *e);       // state handler
};

// Local objects -------------------------------------------------------------
static Calc l_calc;                                 // the calculator instance

// Global objects ------------------------------------------------------------
QHsm * const the_calc = &l_calc;

// HSM definition ------------------------------------------------------------
QState Calc::initial(Calc *me, QEvt const * /* e */) {
    BSP_clear();
    return Q_TRAN(&Calc::on);
}
//............................................................................
QState Calc::on(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::ready);
        }
        case C_SIG: {
            BSP_clear();
            return Q_TRAN(&Calc::on);                    // transition-to-self
        }
        case OFF_SIG: {
            return Q_TRAN(&Calc::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState Calc::error(Calc *me, QEvt const *e) {
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
    return Q_SUPER(&Calc::on);
}
//............................................................................
QState Calc::ready(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::begin);
        }
        case DIGIT_0_SIG: {
            BSP_clear();
            return Q_TRAN(&Calc::zero1);
        }
        case DIGIT_1_9_SIG: {
            BSP_clear();
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::int1);
        }
        case POINT_SIG: {
            BSP_clear();
            BSP_insert((int)'0');
            BSP_insert((int)'.');
            return Q_TRAN(&Calc::frac1);
        }
        case OPER_SIG: {
            me->m_operand1 = BSP_get_value();
            me->m_operator = ((CalcEvt const *)e)->key_code;
            return Q_TRAN(&Calc::opEntered);
        }
    }
    return Q_SUPER(&Calc::on);
}
//............................................................................
QState Calc::result(Calc *me, QEvt const *e) {
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
    return Q_SUPER(&Calc::ready);
}
//............................................................................
QState Calc::begin(Calc *me, QEvt const *e) {
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
                return Q_TRAN(&Calc::negated1);
            }
            break;
        }
    }
    return Q_SUPER(&Calc::ready);
}
//............................................................................
QState Calc::negated1(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::begin);
        }
        case DIGIT_0_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::zero1);
        }
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::int1);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::frac1);
        }
    }
    return Q_SUPER(&Calc::on);
}
//............................................................................
QState Calc::negated2(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::opEntered);
        }
        case DIGIT_0_SIG: {
            return Q_TRAN(&Calc::zero2);
        }
        case DIGIT_1_9_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::int2);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::frac2);
        }
    }
    return Q_SUPER(&Calc::on);
}
//............................................................................
QState Calc::operand1(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::begin);
        }
        case OPER_SIG: {
            me->m_operand1 = BSP_get_value();
            me->m_operator = ((CalcEvt const *)e)->key_code;
            return Q_TRAN(&Calc::opEntered);
        }
        case EQUALS_SIG: {
            return Q_TRAN(&Calc::result);
        }
    }
    return Q_SUPER(&Calc::on);
}
//............................................................................
QState Calc::zero1(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::int1);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::frac1);
        }
    }
    return Q_SUPER(&Calc::operand1);
}
//............................................................................
QState Calc::int1(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::frac1);
        }
    }
    return Q_SUPER(&Calc::operand1);
}
//............................................................................
QState Calc::frac1(Calc *me, QEvt const *e) {
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
    return Q_SUPER(&Calc::operand1);
}
//............................................................................
QState Calc::opEntered(Calc *me, QEvt const *e) {
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
                return Q_TRAN(&Calc::negated2);
            }
            break;
        }
        case DIGIT_0_SIG: {
            BSP_clear();
            return Q_TRAN(&Calc::zero2);
        }
        case DIGIT_1_9_SIG: {
            BSP_clear();
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::int2);
        }
        case POINT_SIG: {
            BSP_clear();
            BSP_insert((int)'0');
            BSP_insert((int)'.');
            return Q_TRAN(&Calc::frac2);
        }
    }
    return Q_SUPER(&Calc::on);
}
//............................................................................
QState Calc::operand2(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::opEntered);
        }
        case OPER_SIG: {
            if (BSP_eval(me->m_operand1, me->m_operator, BSP_get_value())) {
                me->m_operand1 = BSP_get_value();
                me->m_operator = ((CalcEvt const *)e)->key_code;
                return Q_TRAN(&Calc::opEntered);
            }
            else {
                return Q_TRAN(&Calc::error);
            }
            return Q_HANDLED();
        }
        case EQUALS_SIG: {
            if (BSP_eval(me->m_operand1, me->m_operator, BSP_get_value())) {
                return Q_TRAN(&Calc::result);
            }
            else {
                return Q_TRAN(&Calc::error);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Calc::on);
}
//............................................................................
QState Calc::zero2(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::int2);
        }
        case POINT_SIG: {
            BSP_insert(((CalcEvt const *)e)->key_code);
            return Q_TRAN(&Calc::frac2);
        }
    }
    return Q_SUPER(&Calc::operand2);
}
//............................................................................
QState Calc::int2(Calc *me, QEvt const *e) {
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
            return Q_TRAN(&Calc::frac2);
        }
    }
    return Q_SUPER(&Calc::operand2);
}
//............................................................................
QState Calc::frac2(Calc *me, QEvt const *e) {
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
    return Q_SUPER(&Calc::operand2);
}
//............................................................................
QState Calc::final(Calc *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            BSP_message("final-ENTRY;");
            BSP_exit();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}

