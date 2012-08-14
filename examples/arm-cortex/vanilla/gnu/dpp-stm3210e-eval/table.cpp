//////////////////////////////////////////////////////////////////////////////
// Product: DPP example
// Last Updated for Version: 4.2.00
// Date of the Last Update:  Jul 15, 2011
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2011 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
class Table : public QActive {
private:
    uint8_t m_fork[N_PHILO];
    uint8_t m_isHungry[N_PHILO];

public:
    Table();

private:
    static QState initial(Table *me, QEvent const *e);
    static QState serving(Table *me, QEvent const *e);
};

#define RIGHT(n_) ((uint8_t)(((n_) + (N_PHILO - 1)) % N_PHILO))
#define LEFT(n_)  ((uint8_t)(((n_) + 1) % N_PHILO))
enum ForkState { FREE, USED };

// Local objects -------------------------------------------------------------
static Table l_table;                                    // local Table object

// Public-scope objects ------------------------------------------------------
QActive * const AO_Table = &l_table;                    // "opaque" AO pointer

//............................................................................
Table::Table() : QActive((QStateHandler)&Table::initial) {
    uint8_t n;
    for (n = 0; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = 0;
    }
}
//............................................................................
QState Table::initial(Table *me, QEvent const *) {

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::serving);

    QS_SIG_DICTIONARY(DONE_SIG,      0);                     // global signals
    QS_SIG_DICTIONARY(EAT_SIG,       0);
    QS_SIG_DICTIONARY(TERMINATE_SIG, 0);

    QS_SIG_DICTIONARY(HUNGRY_SIG,    me);             // signal just for Table

    me->subscribe(DONE_SIG);
    me->subscribe(TERMINATE_SIG);

    return Q_TRAN(&Table::serving);
}
//............................................................................
QState Table::serving(Table *me, QEvent const *e) {
    uint8_t n, m;
    TableEvt *pe;

    switch (e->sig) {
        case HUNGRY_SIG: {
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
        case TERMINATE_SIG: {
            QF::stop();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
