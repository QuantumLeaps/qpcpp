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

//............................................................................
class AlarmClock : public QActive {  // the AlarmClock container active object
private:
    Alarm    m_alarmComp;                        // Alarm orthogonal component
    QTimeEvt m_timeEvt;                        // private time event generator
    uint32_t m_current_time;                    // the current time in seconds

public:
    AlarmClock()                                               // default ctor
      : QActive((QStateHandler)&AlarmClock::initial),
        m_timeEvt(TIME_SIG) {}

private:
                                             // hierarchical state machine ...
    static QState initial    (AlarmClock *me, QEvt const *e);
    static QState timekeeping(AlarmClock *me, QEvt const *e);
    static QState mode12hr   (AlarmClock *me, QEvt const *e);
    static QState mode24hr   (AlarmClock *me, QEvt const *e);
    static QState final      (AlarmClock *me, QEvt const *e);
};

// HSM definition ------------------------------------------------------------
QState AlarmClock::initial(AlarmClock *me, QEvt const *) {
    me->m_current_time = 0;

                      // take the initial transition in the alarm component...
    me->m_alarmComp.init();

    return Q_TRAN(&AlarmClock::timekeeping);
}
//............................................................................
QState AlarmClock::final(AlarmClock *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("-> final\n");
            printf("\nBye!Bye!\n");
            QF::stop();                                 // stop QF and cleanup
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState AlarmClock::timekeeping(AlarmClock *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
                                              // periodic timeout every second
            me->m_timeEvt.postEvery(me, BSP_TICKS_PER_SEC);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            me->m_timeEvt.disarm();
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&AlarmClock::mode24hr);
        }
        case CLOCK_12H_SIG: {
            return Q_TRAN(&AlarmClock::mode12hr);
        }
        case CLOCK_24H_SIG: {
            return Q_TRAN(&AlarmClock::mode24hr);
        }
        case ALARM_SIG: {
            printf("Wake up!!!\n");
            return Q_HANDLED();
        }
        case ALARM_SET_SIG:
        case ALARM_ON_SIG:
        case ALARM_OFF_SIG: {
            me->m_alarmComp.dispatch(e);     // synchronously dispatch to comp.
            return Q_HANDLED();
        }
        case TERMINATE_SIG: {
            return Q_TRAN(&AlarmClock::final);
        }
    }
    return Q_SUPER(&QHsm::top);
}
//............................................................................
QState AlarmClock::mode24hr(AlarmClock *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("*** 24-hour mode\n");
            return Q_HANDLED();
        }
        case TIME_SIG: {
            if (++me->m_current_time == 24*60) {   // roll over in 24-hr mode?
                me->m_current_time = 0;
            }
            printf("%02ld:%02ld\n",
                  me->m_current_time / 60, me->m_current_time % 60);
            TimeEvt pe;       // temporary synchronous event for the component
            pe.sig = TIME_SIG;
            pe.current_time = me->m_current_time;
            me->m_alarmComp.dispatch(&pe);  // synchronously dispatch to comp.
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&AlarmClock::timekeeping);
}
//............................................................................
QState AlarmClock::mode12hr(AlarmClock *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("*** 12-hour mode\n");
            return Q_HANDLED();
        }
        case TIME_SIG: {
            if (++me->m_current_time == 12*60) {   // roll over in 12-hr mode?
                me->m_current_time = 0;
            }
            uint32_t h = me->m_current_time/60;
            printf("%02ld:%02ld %s\n", (h % 12) ? (h % 12) : 12,
                   me->m_current_time % 60, (h / 12) ? "PM" : "AM");
            TimeEvt pe;       // temporary synchronous event for the component
            pe.sig = TIME_SIG;
            pe.current_time = me->m_current_time;
            me->m_alarmComp.dispatch(&pe);  // synchronously dispatch to comp.
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&AlarmClock::timekeeping);
}

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static AlarmClock    l_alarmClock;             // the AlarmClock active object
static QEvt const *l_alarmClockQSto[10];     // queue storage for AlarmClock
static TimeEvt       l_smlPoolSto[10];                     // small event pool

// Global-scope objects (opaque pointer to the AlarmClock container) ---------
QActive * const APP_alarmClock= &l_alarmClock;     // AlarmClock active object

//............................................................................
int main(int argc, char *argv[]) {
    printf("Reminder state pattern\nQEP version: %s\nQF  version: %s\n"
           "Press 'o' to turn the Alarm ON\n"
           "Press 'f' to turn the Alarm OFF\n"
           "Press '0'..'9' to set the Alarm time\n"
           "Press 'A' to set the Clock in 12-hour mode\n"
           "Press 'B' to set the Clock in 24-hour mode\n"
           "Press ESC to quit...\n",
           QEP::getVersion(), QF::getVersion());

    BSP_init(argc, argv);                                // initialize the BSP

    QF::init();       // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QF::psInit()

                                                  // initialize event pools...
    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

                                                // start the active objects...
    l_alarmClock.start(1, l_alarmClockQSto, Q_DIM(l_alarmClockQSto),
                       (void *)0, 0, (QEvt *)0);

    return QF::run();                                // run the QF application
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 24: {                                     // 'o': Alarm on event?
            l_alarmClock.postFIFO(Q_NEW(QEvt, ALARM_ON_SIG));
            break;
        }
        case 33: {                                    // 'f': Alarm off event?
            l_alarmClock.postFIFO(Q_NEW(QEvt, ALARM_OFF_SIG));
            break;
        }
        case  2:                                                        // '1'
        case  3:                                                        // '2'
        case  4:                                                        // '3'
        case  5:                                                        // '4'
        case  6:                                                        // '5'
        case  7:                                                        // '6'
        case  8:                                                        // '7'
        case  9:                                                        // '8'
        case 10: {                                                      // '9'
            SetEvt *e = Q_NEW(SetEvt, ALARM_SET_SIG);
            e->digit = (uint8_t)(key - 1);
            l_alarmClock.postFIFO(e);
            break;
        }
        case 11: {                                                      // '0'
            SetEvt *e = Q_NEW(SetEvt, ALARM_SET_SIG);
            e->digit = 0;
            l_alarmClock.postFIFO(e);
            break;
        }
        case 30: {                                    // 'a': Clock 12H event?
            l_alarmClock.postFIFO(Q_NEW(QEvt, CLOCK_12H_SIG));
            break;
        }
        case 48: {                                    // 'b': Clock 24H event?
            l_alarmClock.postFIFO(Q_NEW(QEvt, CLOCK_24H_SIG));
            break;
        }
        case 129: {                                            // ESC pressed?
            l_alarmClock.postFIFO(Q_NEW(QEvt, TERMINATE_SIG));
            break;
        }
    }
}
