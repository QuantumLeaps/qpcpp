//============================================================================
// Product: BSP for "start-stop" app with FreeRTOS
// Last Updated for Version: 6.5.0
// Date of the Last Update:  2019-03-22
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
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
#ifndef BSP_HPP
#define BSP_HPP

class BSP {
public:
    enum { TICKS_PER_SEC = configTICK_RATE_HZ };

    static void init(void);
    static void ledOn(void);
    static void ledOff(void);
    static void led2On(void);
    static void led2Off(void);
};

#endif // BSP_HPP
