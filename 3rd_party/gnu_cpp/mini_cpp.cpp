//****************************************************************************
// Minimal Embedded C++ support, no exception handling, no RTTI
// Last Updated for Version: 5.8.2
// Date of the Last Update:  2017-02-06
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

#include <stdlib.h> // for prototypes of malloc() and free()

//............................................................................
void *operator new(size_t size) throw() {
    return malloc(size);
}
//............................................................................
void operator delete(void *p) throw() {
    free(p);
}
//............................................................................
void operator delete(void *p, unsigned int) throw() {
    free(p);
}

extern "C" {
//............................................................................
void __cxa_atexit(void (*arg1)(void *), void *arg2, void *arg3) {
    (void)arg1;
    (void)arg2;
    (void)arg3;
}
//............................................................................
void __cxa_guard_acquire() {
}
//............................................................................
void __cxa_guard_release() {
}
//............................................................................
void *__dso_handle = static_cast<void *>(0);

} // extern "C"
