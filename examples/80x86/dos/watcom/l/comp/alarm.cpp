//////////////////////////////////////////////////////////////////////////////
// Product: Orthogonal Component state pattern example
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
#include "bsp.h"
#include "alarm.h"
#include "clock.h"

#include <stdio.h>

Q_DEFINE_THIS_FILE

// FSM definition ------------------------------------------------------------
QState Alarm::initial(Alarm *me, QEvt const *e) {
    (void)e;                  // avoid compiler warning about unused parameter
    me->m_alarm_time = 12*60;
    return Q_TRAN(&Alarm::off);
}
//............................................................................
QState Alarm::off(Alarm *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
                // while in the off state, the alarm is kept in decimal format
            me->m_alarm_time = (me->m_alarm_time/60)*100+me->m_alarm_time%60;
            printf("*** Alarm OFF %02ld:%02ld\n",
                   me->m_alarm_time/100, me->m_alarm_time%100);
            fflush(stdout);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
                         // upon exit, the alarm is converted to binary format
            me->m_alarm_time = (me->m_alarm_time/100)*60+me->m_alarm_time%100;
            return Q_HANDLED();
        }
        case ALARM_ON_SIG: {
                                                            // alarm in range?
            if ((me->m_alarm_time / 100 < 24)
                 && (me->m_alarm_time % 100 < 60))
            {
                return Q_TRAN(&Alarm::on);
            }
            else {         // alarm out of range -- clear and don't transition
                me->m_alarm_time = 0;
                printf("*** Alarm reset %02ld:%02ld\n",
                       me->m_alarm_time/100, me->m_alarm_time%100);
                return Q_HANDLED();
            }
        }
        case ALARM_SET_SIG: {
                         // while setting, the alarm is kept in decimal format
            me->m_alarm_time = (10 * me->m_alarm_time
                              + ((SetEvt const *)e)->digit) % 10000;
            printf("*** Alarm SET %02ld:%02ld\n",
                   me->m_alarm_time/100, me->m_alarm_time%100);
            fflush(stdout);
            return Q_HANDLED();
        }
    }
    return Q_IGNORED();
}
//............................................................................
QState Alarm::on(Alarm *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("*** Alarm ON %02ld:%02ld\n",
                   me->m_alarm_time/60, me->m_alarm_time%60);
            fflush(stdout);
            return Q_HANDLED();
        }
        case ALARM_SET_SIG: {
            printf("*** Cannot set Alarm when it is ON\n");
            fflush(stdout);
            return Q_HANDLED();
        }
        case ALARM_OFF_SIG: {
            return Q_TRAN(&Alarm::off);
        }
        case TIME_SIG: {
            if (((TimeEvt *)e)->current_time == me->m_alarm_time) {
                printf("ALARM!!!\n");
                          // asynchronously post the event to the container AO
                APP_alarmClock->postFIFO(Q_NEW(QEvt, ALARM_SIG));
            }
            return Q_HANDLED();
        }
    }
    return Q_IGNORED();
}
