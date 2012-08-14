//////////////////////////////////////////////////////////////////////////////
// Product: DPP example, QK version
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
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
class Table : public QActive {
private:
    ThreadContext m_context;                                 // thread context
    uint8_t m_fork[N_PHILO];
    uint8_t m_isHungry[N_PHILO];

public:
    Table();

private:
    static QState initial(Table *me, QEvt const *e);
    static QState serving(Table *me, QEvt const *e);

    friend void Table_start(uint8_t p, QEvt const *qSto[], uint32_t qLen);
};

#define RIGHT(n_) ((uint8_t)(((n_) + (N_PHILO - 1)) % N_PHILO))
#define LEFT(n_)  ((uint8_t)(((n_) + 1) % N_PHILO))
enum ForkState { FREE, USED };

// Local objects -------------------------------------------------------------
static Table l_table;                                    // local Table object

// Public-scope objects ------------------------------------------------------
QActive * const AO_Table = &l_table;                    // "opaque" AO pointer

//............................................................................
void Table_start(uint8_t p, QEvt const *qSto[], uint32_t qLen) {
    Table *me = &l_table;

    impure_ptr1 = &me->m_context.lib1;        // initialize reentrant library1
    lib1_reent_init(p);
    impure_ptr2 = &me->m_context.lib2;        // initialize reentrant library2
    lib2_reent_init(p);

    me->start(p, qSto, qLen, &me->m_context,
              (uint8_t)(QK_LIB1_THREAD | QK_LIB2_THREAD | QK_FPU_THREAD));
}
//............................................................................
Table::Table() : QActive((QStateHandler)&Table::initial) {
    uint8_t n;
    for (n = 0; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = 0;
    }
}
//............................................................................
QState Table::initial(Table *me, QEvt const *) {

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::serving);
    QS_SIG_DICTIONARY(HUNGRY_SIG, me);                     // signal for Table

    me->subscribe(DONE_SIG);
    me->subscribe(TERMINATE_SIG);

    return Q_TRAN(&Table::serving);
}
//............................................................................
QState Table::serving(Table *me, QEvt const *e) {
    uint8_t n, m;
    TableEvt *pe;

    switch (e->sig) {
        case HUNGRY_SIG: {
            lib1_test();
            lib2_test();
            n = ((TableEvt const *)e)->philoNum;
                         // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displyPhilStat(n, "hungry  ");
            m = LEFT(n);
            if ((me->m_fork[m] == FREE) && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = me->m_fork[n] = USED;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = n;
                QF::PUBLISH(pe, me);
                BSP_displyPhilStat(n, "eating  ");
            }
            else {
                me->m_isHungry[n] = 1;
            }
            return Q_HANDLED();
        }
        case DONE_SIG: {
            lib1_test();
            lib2_test();
            n = ((TableEvt const *)e)->philoNum;
                         // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displyPhilStat(n, "thinking");
            m = LEFT(n);
                                         // both forks of Phil[n] must be used
            Q_ASSERT((me->m_fork[n] == USED) && (me->m_fork[m] == USED));

            me->m_fork[m] = me->m_fork[n] = FREE;
            m = RIGHT(n);                          // check the right neighbor
            if (me->m_isHungry[m] && (me->m_fork[m] == FREE)) {
                me->m_fork[n] = me->m_fork[m] = USED;
                me->m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF::PUBLISH(pe, me);
                BSP_displyPhilStat(m, "eating  ");
            }
            m = LEFT(n);                            // check the left neighbor
            n = LEFT(m);                     // left fork of the left neighbor
            if (me->m_isHungry[m] && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = me->m_fork[n] = USED;
                me->m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF::PUBLISH(pe, me);
                BSP_displyPhilStat(m, "eating  ");
            }
            return Q_HANDLED();
        }
        case TEST_SIG: {
            lib1_test();
            lib2_test();
            return Q_HANDLED();
        }
        case TERMINATE_SIG: {
            QF::stop();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
