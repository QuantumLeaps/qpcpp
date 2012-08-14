//////////////////////////////////////////////////////////////////////////////
// Product: DPP example
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
class Table : public QP::QActive {
private:
    uint8_t m_fork[N_PHILO];
    uint8_t m_isHungry[N_PHILO];

public:
    Table();

private:
    static QP::QState initial(Table *me, QP::QEvt const *e);
    static QP::QState serving(Table *me, QP::QEvt const *e);
};

#define RIGHT(n_) (static_cast<uint8_t>(((n_) + (N_PHILO - 1U)) % N_PHILO))
#define LEFT(n_)  (static_cast<uint8_t>(((n_) + 1U) % N_PHILO))
enum ForkState { FREE, USED };

// Local objects -------------------------------------------------------------
static Table l_table;                                    // local Table object

// Public-scope objects ------------------------------------------------------
QP::QActive * const AO_Table = &l_table;                // "opaque" AO pointer

//............................................................................
Table::Table() : QActive(Q_STATE_CAST(&Table::initial)) {
    uint8_t n;
    for (n = 0U; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = 0U;
    }
}
//............................................................................
QP::QState Table::initial(Table *me, QP::QEvt const *) {

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::serving);

    // global signals...
    QS_SIG_DICTIONARY(DONE_SIG,      static_cast<void *>(0));
    QS_SIG_DICTIONARY(EAT_SIG,       static_cast<void *>(0));
    QS_SIG_DICTIONARY(TERMINATE_SIG, static_cast<void *>(0));

    QS_SIG_DICTIONARY(HUNGRY_SIG,    me);             // signal just for Table

    me->subscribe(DONE_SIG);
    me->subscribe(TERMINATE_SIG);

    return Q_TRAN(&Table::serving);
}
//............................................................................
QP::QState Table::serving(Table *me, QP::QEvt const *e) {
    TableEvt *pe;
    switch (e->sig) {
        case HUNGRY_SIG: {
            uint8_t n = (static_cast<TableEvt const *>(e))->philoNum;
                         // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displyPhilStat(n, "hungry  ");
            uint8_t m = LEFT(n);
            if ((me->m_fork[m] == FREE) && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = me->m_fork[n] = USED;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = n;
                QP::QF::PUBLISH(pe, me);
                BSP_displyPhilStat(n, "eating  ");
            }
            else {
                me->m_isHungry[n] = 1;
            }
            return Q_HANDLED();
        }
        case DONE_SIG: {
            uint8_t n = ((TableEvt const *)e)->philoNum;
                         // phil ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            BSP_displyPhilStat(n, "thinking");
            uint8_t m = LEFT(n);
                                         // both forks of Phil[n] must be used
            Q_ASSERT((me->m_fork[n] == USED) && (me->m_fork[m] == USED));

            me->m_fork[m] = me->m_fork[n] = FREE;
            m = RIGHT(n);                          // check the right neighbor
            if (me->m_isHungry[m] && (me->m_fork[m] == FREE)) {
                me->m_fork[n] = me->m_fork[m] = USED;
                me->m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QP::QF::PUBLISH(pe, me);
                BSP_displyPhilStat(m, "eating  ");
            }
            m = LEFT(n);                            // check the left neighbor
            n = LEFT(m);                     // left fork of the left neighbor
            if (me->m_isHungry[m] && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = me->m_fork[n] = USED;
                me->m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QP::QF::PUBLISH(pe, me);
                BSP_displyPhilStat(m, "eating  ");
            }
            return Q_HANDLED();
        }
        case TERMINATE_SIG: {
            QP::QF::stop();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
