//$file${.::clock.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: comp.qm
// File:  ${.::clock.hpp}
//
// This code has been generated by QM 5.2.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${.::clock.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef CLOCK_HPP
#define CLOCK_HPP

using namespace QP;

enum AlarmClockSignals {
    TICK_SIG = Q_USER_SIG, // time tick event
    ALARM_SET_SIG,  // set the alarm
    ALARM_ON_SIG,   // turn the alarm on
    ALARM_OFF_SIG,  // turn the alarm off
    ALARM_SIG,  // alarm event from Alarm component to AlarmClock container
    CLOCK_12H_SIG,  // set the clock in 12H mode
    CLOCK_24H_SIG,  // set the clock in 24H mode
    TIME_SIG,       // time event sent to Alarm (contains current time)
    TERMINATE_SIG   // terminate the application
};

//$declare${Events::SetEvt} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Events::SetEvt} ..........................................................
class SetEvt : public QP::QEvt {
public:
    uint8_t digit;
}; // class SetEvt
//$enddecl${Events::SetEvt} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$declare${Events::TimeEvt} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Events::TimeEvt} .........................................................
class TimeEvt : public QP::QEvt {
public:
    uint32_t current_time;
}; // class TimeEvt
//$enddecl${Events::TimeEvt} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$declare${Components::APP_alarmClock} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Components::APP_alarmClock} ..............................................
extern QActive * const APP_alarmClock;
//$enddecl${Components::APP_alarmClock} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif // CLOCK_HPP
