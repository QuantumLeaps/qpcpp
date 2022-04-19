//============================================================================
// Product: DPP example with emWin/uC/GUI, NO Window Manager
// Last updated for version 6.8.0
// Last updated on  2020-01-22
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2005-2020 Quantum Leaps, LLC. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

extern "C" {
    #include "GUI.h"
    #include "GUI_SIM.h"
    #include "DIALOG.h"
}

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
class Table : public QActive {
private:
    uint8_t m_fork[N_PHILO];
    uint8_t m_isHungry[N_PHILO];

public:
    Table();

private:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(ready);
    Q_STATE_DECL(serving);
    Q_STATE_DECL(paused);
};

#define RIGHT(n_) ((uint8_t)(((n_) + (N_PHILO - 1)) % N_PHILO))
#define LEFT(n_)  ((uint8_t)(((n_) + 1) % N_PHILO))
enum m_forkState { FREE, USED };

// Local objects -------------------------------------------------------------
static Table l_table; // local Table object

#ifdef Q_SPY
enum QSUserRecords {
    PHILO_STAT = QS_USER,
    TABLE_STAT
};
#endif

// Public-scope objects ------------------------------------------------------
QActive * const AO_Table = &l_table; // "opaque" AO pointer


// GUI definition ============================================================
enum YCoord {
    PHILO_0_Y =   0,
    PHILO_1_Y =  20,
    PHILO_2_Y =  40,
    PHILO_3_Y =  60,
    PHILO_4_Y =  80,
    TABLE_Y   = 100,
    STATE_X   = 100
};

static int l_xOrg = 80;
static int l_yOrg = 60;
static char const l_thinking[] = "thinking";
static char const *l_philoStat[N_PHILO] = {
    l_thinking, l_thinking, l_thinking, l_thinking, l_thinking
};
static int const l_philoY[N_PHILO] = {
    0, 20, 40, 60, 80
};
static char const *l_tableState = " ";
static int const l_tableY = 100;

//............................................................................
static void displyPhilStat(uint8_t n, char const *stat) {
    GUI_SetBkColor(GUI_GRAY);
    GUI_SetColor(GUI_BLACK);
    GUI_SetFont(&GUI_Font13_ASCII);

    l_philoStat[n] = stat;
    GUI_DispStringAt(stat,  l_xOrg + STATE_X, l_yOrg + l_philoY[n]);

    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
static void displyTableStat(char const *stat) {
    GUI_SetBkColor(GUI_GRAY);
    GUI_SetColor(GUI_BLACK);
    GUI_SetFont(&GUI_Font13_ASCII);

    l_tableState = stat;
    GUI_DispStringAt(stat,  l_xOrg + STATE_X, l_yOrg + l_tableY);

    QS_BEGIN_ID(TABLE_STAT, AO_Table->m_prio) // app-specific record begin
        QS_STR(stat); // Philosopher status
    QS_END()
}

//............................................................................
static void renderDppScreen(void) {
    GUI_SetBkColor(GUI_GRAY);
    GUI_Clear();
    GUI_SetColor(GUI_BLACK);
    GUI_SetFont(&GUI_Font24_ASCII);
    GUI_DispStringHCenterAt("Dining Philosophers - Demo", 160, 5);
    GUI_SetFont(&GUI_Font13_ASCII);

    GUI_DispStringAt("Philosopher 0",  l_xOrg, l_yOrg + l_philoY[0]);
    GUI_DispStringAt("Philosopher 1",  l_xOrg, l_yOrg + l_philoY[1]);
    GUI_DispStringAt("Philosopher 2",  l_xOrg, l_yOrg + l_philoY[2]);
    GUI_DispStringAt("Philosopher 3",  l_xOrg, l_yOrg + l_philoY[3]);
    GUI_DispStringAt("Philosopher 4",  l_xOrg, l_yOrg + l_philoY[4]);
    GUI_DispStringAt("Table",          l_xOrg, l_yOrg + l_tableY);

    displyPhilStat(0, l_philoStat[0]);
    displyPhilStat(1, l_philoStat[1]);
    displyPhilStat(2, l_philoStat[2]);
    displyPhilStat(3, l_philoStat[3]);
    displyPhilStat(4, l_philoStat[4]);
}
//............................................................................
static void moveDppScreen(int dx, int dy) {
    l_xOrg += dx;
    l_yOrg += dy;

    renderDppScreen();
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
Q_STATE_DEF(Table, initial) {

    GUI_Init(); // initialize the embedded GUI

    subscribe(DONE_SIG);
    subscribe(PAUSE_SIG);

    return tran(&ready);
}
//............................................................................
Q_STATE_DEF(Table, ready) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            renderDppScreen();
            return Q_RET_HANDLED;
        }
        case Q_INIT_SIG: {
            return tran(&serving);
        }

        // ... hardkey events ...
        case KEY_LEFT_REL_SIG: { // hardkey LEFT released
            moveDppScreen(-5, 0);
            return Q_RET_HANDLED;
        }
        case KEY_RIGHT_REL_SIG: { // hardkey RIGHT released
            moveDppScreen(5, 0);
            return Q_RET_HANDLED;
        }
        case KEY_DOWN_REL_SIG: { // hardkey DOWN released
            moveDppScreen(0, 5);
            return Q_RET_HANDLED;
        }
        case KEY_UP_REL_SIG: {   // hardkey UP released
            moveDppScreen(0, -5);
            return Q_RET_HANDLED;
        }
    }
    return super(&top);
}
//............................................................................
Q_STATE_DEF(Table, serving) {
    uint8_t n, m;
    TableEvt *pe;

    switch (e->sig) {
        case Q_ENTRY_SIG: {
            displyTableStat("serving");
            for (n = 0; n < N_PHILO; ++n) { // give permissions to eat...
                if (m_isHungry[n]
                    && (m_fork[LEFT(n)] == FREE)
                        && (m_fork[n] == FREE))
                {
                    m_fork[LEFT(n)] = m_fork[n] = USED;
                    pe = Q_NEW(TableEvt, EAT_SIG);
                    pe->philoNum = n;
                    QF::PUBLISH(pe, me);
                    m_isHungry[n] = 0;
                    displyPhilStat(n, "eating  ");
                }
            }
            return Q_RET_HANDLED;
        }
        case HUNGRY_SIG: {
            n = ((TableEvt *)e)->philoNum;
            Q_ASSERT(n < N_PHILO && !m_isHungry[n]);
            displyPhilStat(n, "hungry  ");
            m = LEFT(n);
            if (m_fork[m] == FREE && m_fork[n] == FREE) {
                m_fork[m] = m_fork[n] = USED;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = n;
                QF::PUBLISH(pe, me);
                displyPhilStat(n, "eating  ");
            }
            else {
                m_isHungry[n] = 1;
            }
            return Q_RET_HANDLED;
        }
        case DONE_SIG: {
            n = ((TableEvt *)e)->philoNum;
            Q_ASSERT(n < N_PHILO);
            displyPhilStat(n, "thinking");
            m_fork[LEFT(n)] = m_fork[n] = FREE;
            m = RIGHT(n); // check the right neighbor
            if (m_isHungry[m] && m_fork[m] == FREE) {
                m_fork[n] = m_fork[m] = USED;
                m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF::PUBLISH(pe, me);
                displyPhilStat(m, "eating  ");
            }
            m = LEFT(n); // check the left neighbor
            n = LEFT(m);
            if (m_isHungry[m] && m_fork[n] == FREE) {
                m_fork[m] = m_fork[n] = USED;
                m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF::PUBLISH(pe, me);
                displyPhilStat(m, "eating  ");
            }
            return Q_RET_HANDLED;
        }
        case PAUSE_SIG: // "Toggle" button pressed
        case KEY_CENTER_PRESS_SIG: { // hardkey CENTER pressed
            return tran(&paused);
        }
    }
    return super(&ready);
}
//............................................................................
Q_STATE_DEF(Table, paused) {
    uint8_t n;

    switch (e->sig) {
        case Q_ENTRY_SIG: {
            displyTableStat("paused");
            return Q_RET_HANDLED;
        }
        case HUNGRY_SIG: {
            n = ((TableEvt *)e)->philoNum;
            Q_ASSERT(n < N_PHILO && !m_isHungry[n]);
            displyPhilStat(n, "hungry  ");
            m_isHungry[n] = 1;
            return Q_RET_HANDLED;
        }
        case DONE_SIG: {
            n = ((TableEvt *)e)->philoNum;
            Q_ASSERT(n < N_PHILO);
            displyPhilStat(n, "thinking");
            m_fork[LEFT(n)] = m_fork[n] = FREE;
            return Q_RET_HANDLED;
        }
        case PAUSE_SIG: // "Toggle" button pressed
        case KEY_CENTER_REL_SIG: { // hardkey CENTER released
            return tran(&serving);
        }
    }
    return super(&ready);
}
