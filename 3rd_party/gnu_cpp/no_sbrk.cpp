//****************************************************************************
// Product: Dummy heap management to reduce the codesize, GNU toolset
// Last Updated for Version: 6.0.3
// Date of the Last Update:  2017-12-01
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
// Web  : https://state-machine.com
// Email: info@state-machine.com
//****************************************************************************

extern "C" void assert_failed(char const *module, int loc); // prototype
extern "C" char *_sbrk(int); // prototype

extern "C" {

char *_sbrk(int) {
    assert_failed("_sbrk", 0);
    return (char *)-1;
}

} // extern "C"

