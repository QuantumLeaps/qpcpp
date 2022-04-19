//============================================================================
// Product: Simple Blinky example
// Last updated for version 6.6.0
// Last updated on  2019-10-14
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#ifndef BLINKY_HPP
#define BLINKY_HPP

using namespace QP;

enum BlinkySignals {
    DUMMY_SIG = Q_USER_SIG,
    MAX_PUB_SIG,  // the last published signal

    TIMEOUT_SIG,
    MAX_SIG       // the last signal
};

extern QActive * const AO_Blinky; // opaque pointer

#endif // BLINKY_HPP
