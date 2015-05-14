//****************************************************************************
// Product: DPP example with lwIP and direct screen output
// Last Updated for Version: 5.4.0
// Date of the Last Update:  2015-05-12
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
// Web  : http://www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qpcpp.h"  // QP/C++ API
#include "dpp.h"    // application events and active objects
#include "bsp.h"    // Board Support Package header file

extern "C" {
    #include "rit128x96x4.h" // RITEK 128x96x4 OLED used in Rev C-D boards
}
#include <stdio.h>

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
class Table : public QActive {
    uint8_t m_fork[N_PHILO];
    uint8_t m_isHungry[N_PHILO];
    uint8_t m_displayOn;
    uint8_t m_udpCtr;
    QTimeEvt m_te_DISPLAY_TIMEOUT;

public:
    Table(); // ctor

private:
    static QState initial(Table *me, QEvt const *e);
    static QState serving(Table *me, QEvt const *e);

private:
    void displayInit(void);
    void displayOn(void);
    void displayOff(void);
    void displayPhilStat(uint8_t n, char const *stat);
    void displyIPAddr(char const *ip_addr);
    void displyUdpText(char const *text);
    void displyCgiText(char const *text);
};

#define RIGHT(n_) ((uint8_t)(((n_) + (N_PHILO - 1)) % N_PHILO))
#define LEFT(n_)  ((uint8_t)(((n_) + 1) % N_PHILO))
enum ForkState { FREE, USED };

#ifdef Q_SPY
enum AppRecords { // application-specific QS trace records
    PHILO_STAT = QS_USER,
    CGI_TEXT,
    UDP_TEXT,
};
#endif

// Local objects -------------------------------------------------------------
static Table l_table;  // the single instance of the Table active object

enum TablePrivateSignals {
   DISPLAY_TIMEOUT_SIG = MAX_SIG
};

#define DISPLAY_TIMEOUT (BSP_TICKS_PER_SEC * 30)

// Global-scope objects ------------------------------------------------------
QActive * const AO_Table = (QActive *)&l_table; // "opaque" AO pointer

//............................................................................
Table::Table()
   : QActive((QStateHandler)&Table::initial),
      m_te_DISPLAY_TIMEOUT(DISPLAY_TIMEOUT_SIG)

{
    uint8_t n;
    for (n = 0; n < N_PHILO; ++n) {
        m_fork[n] = FREE;
        m_isHungry[n] = 0;
    }
    m_udpCtr = 0;
}
//............................................................................
QState Table::initial(Table *me, QEvt const *e) {
    (void)e; // unused parameter

    me->displayInit(); // Initialize the OLED display

    // subscribe to published events...
    me->subscribe(DONE_SIG);
    me->subscribe(BTN_DOWN_SIG);
    me->subscribe(DISPLAY_IPADDR_SIG);
    me->subscribe(DISPLAY_CGI_SIG);
    me->subscribe(DISPLAY_UDP_SIG);

    QS_OBJ_DICTIONARY(&l_table);
    QS_FUN_DICTIONARY(&QHsm::top);
    QS_FUN_DICTIONARY(&Table::initial);
    QS_FUN_DICTIONARY(&Table::serving);

    // global signals...
    QS_SIG_DICTIONARY(DONE_SIG,           static_cast<void *>(0));
    QS_SIG_DICTIONARY(EAT_SIG,            static_cast<void *>(0));
    QS_SIG_DICTIONARY(DISPLAY_IPADDR_SIG, static_cast<void *>(0));
    QS_SIG_DICTIONARY(DISPLAY_CGI_SIG,    static_cast<void *>(0));
    QS_SIG_DICTIONARY(DISPLAY_UDP_SIG,    static_cast<void *>(0));

    // signals just for Table...
    QS_SIG_DICTIONARY(HUNGRY_SIG,          me);
    QS_SIG_DICTIONARY(DISPLAY_TIMEOUT_SIG, me);

    return Q_TRAN(&Table::serving);
}
//............................................................................
QState Table::serving(Table *me, QEvt const *e) {
    uint8_t n, m;
    TableEvt *pe;

    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->m_te_DISPLAY_TIMEOUT.postEvery((QActive *)me,
                                               DISPLAY_TIMEOUT);
            me->displayOn();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_te_DISPLAY_TIMEOUT.disarm();
            me->displayOff();
            return Q_HANDLED();
        }
        case HUNGRY_SIG: {
            n = ((TableEvt const *)e)->philoNum;
            // philo ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            me->displayPhilStat(n, "hungry  ");
            m = LEFT(n);
            if ((me->m_fork[m] == FREE) && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = me->m_fork[n] = USED;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = n;
                QF::PUBLISH(pe, me);
                me->displayPhilStat(n, "eating  ");
            }
            else {
                me->m_isHungry[n] = 1;
            }
            return Q_HANDLED();
        }
        case DONE_SIG: {
            n = ((TableEvt const *)e)->philoNum;
            // philo ID must be in range and he must be not hungry
            Q_ASSERT((n < N_PHILO) && (!me->m_isHungry[n]));

            me->displayPhilStat(n, "thinking");
            m = LEFT(n);
            // both forks of Phil[n] must be used
            Q_ASSERT((me->m_fork[n] == USED) && (me->m_fork[m] == USED));

            me->m_fork[m] = me->m_fork[n] = FREE;
            m = RIGHT(n); // check the right neighbor
            if (me->m_isHungry[m] && (me->m_fork[m] == FREE)) {
                me->m_fork[n] = me->m_fork[m] = USED;
                me->m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF::PUBLISH(pe, me);
                me->displayPhilStat(m, "eating  ");
            }
            m = LEFT(n);  // check the left neighbor
            n = LEFT(m);  // left fork of the left neighbor
            if (me->m_isHungry[m] && (me->m_fork[n] == FREE)) {
                me->m_fork[m] = me->m_fork[n] = USED;
                me->m_isHungry[m] = 0;
                pe = Q_NEW(TableEvt, EAT_SIG);
                pe->philoNum = m;
                QF::PUBLISH(pe, me);
                me->displayPhilStat(m, "eating  ");
            }
            return Q_HANDLED();
        }
        case BTN_DOWN_SIG: {
            me->displayOn();
            return Q_HANDLED();
        }
        case DISPLAY_TIMEOUT_SIG: {
            me->displayOff();
            return Q_HANDLED();
        }
        case DISPLAY_IPADDR_SIG: {
            me->displyIPAddr(((TextEvt *)e)->text);
            return Q_HANDLED();
        }
        case DISPLAY_CGI_SIG: {
            me->displyCgiText(((TextEvt *)e)->text);
            return Q_HANDLED();
        }
        case DISPLAY_UDP_SIG: {
            TextEvt *te;

            me->displyUdpText(((TextEvt *)e)->text);
            ++me->m_udpCtr;

            te = Q_NEW(TextEvt, SEND_UDP_SIG);
            snprintf(te->text, Q_DIM(te->text), "%s-%d",
                     ((TextEvt const *)e)->text, (int)me->m_udpCtr);
            AO_LwIPMgr->POST(te, me); // post directly

            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}

// helper functions for the display ..........................................
//............................................................................
void Table::displayInit(void) {
    RIT128x96x4Init(1000000);
    RIT128x96x4StringDraw("QP-lwIP Demo",       4*6, 0*8,  9);
    RIT128x96x4StringDraw("IP :",               0*6, 2*8,  5);
    RIT128x96x4StringDraw("DPP: 0 ,1 ,2 ,3 ,4", 0*6, 4*8,  5);
    RIT128x96x4StringDraw("CGI:",               0*6, 6*8,  5);
    RIT128x96x4StringDraw("UDP:",               0*6, 8*8,  5);
    RIT128x96x4StringDraw("state-machine.com",  2*6,10*8,  9);
    m_displayOn = 1;
}
//............................................................................
void Table::displayOn(void) {
    m_te_DISPLAY_TIMEOUT.rearm(DISPLAY_TIMEOUT);
    if (!m_displayOn) {
        m_displayOn = 1;
        RIT128x96x4DisplayOn();
    }
}
//............................................................................
void Table::displayOff(void) {
    if (m_displayOn) {
        m_displayOn = 0;
        RIT128x96x4DisplayOff();
    }
}
//............................................................................
void Table::displayPhilStat(uint8_t n, char const *stat) {
    if (m_displayOn) {
        char str[2];
        str[0] = stat[0];
        str[1] = '\0';
        RIT128x96x4StringDraw(str, (6*6 + 3*6*n), 4*8, 15);
    }
    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);  // Philosopher number
        QS_STR(stat); // Philosopher status
    QS_END()
}
//............................................................................
void Table::displyIPAddr(char const *ip_addr) {
    displayOn(); // make sure the screen is on
    RIT128x96x4StringDraw("               ", 5*6, 2*8, 15); // wipe clean
    RIT128x96x4StringDraw(ip_addr,           5*6, 2*8, 15);
}
//............................................................................
void Table::displyCgiText(char const *text) {
    displayOn(); // make sure the screen is on
    RIT128x96x4StringDraw("               ", 5*6, 6*8, 15); // wipe clean
    RIT128x96x4StringDraw(text,              5*6, 6*8, 15);

    QS_BEGIN(CGI_TEXT, 0) // application-specific record begin
        QS_STR(text);     // User text
    QS_END()
}
//............................................................................
void Table::displyUdpText(char const *text) {
    displayOn(); // make sure the screen is on
    RIT128x96x4StringDraw("               ", 5*6, 6*8, 15); // wipe clean
    RIT128x96x4StringDraw(text,              5*6, 6*8, 15);

    QS_BEGIN(UDP_TEXT, 0) // application-specific record begin
        QS_STR(text);     // User text
    QS_END()
}

