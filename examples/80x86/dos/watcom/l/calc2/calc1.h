//****************************************************************************
// Product: Calculator Example with inheritance of the whole state model
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 15, 2012
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
#ifndef calc1_h
#define calc1_h

using namespace QP;

enum Calc1Signals {
    C_SIG = Q_USER_SIG,
    CE_SIG,
    DIGIT_0_SIG,
    DIGIT_1_9_SIG,
    POINT_SIG,
    OPER_SIG,
    EQUALS_SIG,
    OFF_SIG,

    MAX_CALC1_SIG
};

struct CalcEvt : public QEvt {
    uint8_t key_code;                                       // code of the key
};


// Calculator HSM class for inheritance --------------------------------------
class Calc1 : public QHsm {
protected:
    double  m_operand1;                              // the value of operand 1
    uint8_t m_operator;                                // operator key entered

public:
    Calc1(void) : QHsm((QStateHandler)&Calc1::initial) {}              // ctor

protected:
    static QState initial(Calc1 *me, QEvt const *e);  // initial pseudostate

    static QState on       (Calc1 *me, QEvt const *e);
    static QState error    (Calc1 *me, QEvt const *e);
    static QState ready    (Calc1 *me, QEvt const *e);
    static QState result   (Calc1 *me, QEvt const *e);
    static QState begin    (Calc1 *me, QEvt const *e);
    static QState negated1 (Calc1 *me, QEvt const *e);
    static QState operand1 (Calc1 *me, QEvt const *e);
    static QState zero1    (Calc1 *me, QEvt const *e);
    static QState int1     (Calc1 *me, QEvt const *e);
    static QState frac1    (Calc1 *me, QEvt const *e);
    static QState opEntered(Calc1 *me, QEvt const *e);
    static QState negated2 (Calc1 *me, QEvt const *e);
    static QState operand2 (Calc1 *me, QEvt const *e);
    static QState zero2    (Calc1 *me, QEvt const *e);
    static QState int2     (Calc1 *me, QEvt const *e);
    static QState frac2    (Calc1 *me, QEvt const *e);
    static QState final    (Calc1 *me, QEvt const *e);

    static QStateHandler   state_on;
    static QStateHandler   state_error;
    static QStateHandler   state_ready;
    static QStateHandler   state_result;
    static QStateHandler   state_begin;
    static QStateHandler   state_negated1;
    static QStateHandler   state_operand1;
    static QStateHandler   state_zero1;
    static QStateHandler   state_int1;
    static QStateHandler   state_frac1;
    static QStateHandler   state_opEntered;
    static QStateHandler   state_negated2;
    static QStateHandler   state_operand2;
    static QStateHandler   state_zero2;
    static QStateHandler   state_int2;
    static QStateHandler   state_frac2;
    static QStateHandler   state_final;
};

#endif                                                              // calc1_h
