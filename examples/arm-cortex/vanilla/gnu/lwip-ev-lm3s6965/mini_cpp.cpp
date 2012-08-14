//////////////////////////////////////////////////////////////////////////////
// Minimal Embedded C++ support, no exception handling, no RTTI
// Date of the Last Update:  Sep 10, 2011
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2007-2011 Quantum Leaps, LLC. All rights reserved.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>                   // for prototypes of malloc() and free()

//............................................................................
void *operator new(size_t size) throw() {
    return malloc(size);
}
//............................................................................
void operator delete(void *p) throw() {
    free(p);
}

extern "C" {
//............................................................................
void __cxa_atexit(void (*arg1)(void *), void *arg2, void *arg3) {
}
//............................................................................
void __cxa_guard_acquire() {
}
//............................................................................
void __cxa_guard_release() {
}
//............................................................................
void *__dso_handle = 0;

}                                                                // extern "C"