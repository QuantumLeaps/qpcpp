//****************************************************************************
// Product: QK/C++ port to 80x86, QK, Open Watcom
// Last Updated for Version: 5.3.1
// Date of the Last Update:  2014-09-21
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, www.state-machine.com.
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
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#ifndef qk_port_h
#define qk_port_h

// QK-specific ISR entry and exit
#define QK_ISR_ENTRY() do { \
    ++QK_intNest_; \
    QF_QS_ISR_ENTRY(QK_intNest_, QK_currPrio_); \
    _enable(); \
} while (0)

#define QK_ISR_EXIT() do { \
    _disable(); \
    outp(0x20, 0x20); \
    QF_QS_ISR_EXIT(QK_intNest_, QK_currPrio_); \
    --QK_intNest_; \
    if (QK_intNest_ == (uint_fast8_t)0) { \
        uint_fast8_t p = QK_schedPrio_(); \
        if (p != (uint_fast8_t)0) { \
            QK_schedExt_(p); \
        } \
    } \
} while (0)

// demonstration of advanced QK features: TLS and extended context switch
struct Lib1_context {  // an example of a library context
    double  x;
};
struct Lib2_context {  // an example of a library context
    double  y;
};
union FPU_context {
    uint32_t align;
    uint8_t  x87[108]; // the x87 FPU context takes 108-bytes
};

struct ThreadContext {
    Lib1_context lib1; // library1 context
    Lib2_context lib2; // library2 context
    FPU_context  fpu;  // the FPU context
};

enum QKTaskFlags {
    QK_LIB1_THREAD = 0x01,
    QK_LIB2_THREAD = 0x02,
    QK_FPU_THREAD  = 0x04
};

// QK thread-local storage
#define QK_TLS(act_) \
    impure_ptr1 = &((ThreadContext *)(act_)->getThread())->lib1; \
    impure_ptr2 = &((ThreadContext *)(act_)->getThread())->lib2

extern Lib1_context * volatile impure_ptr1;
extern Lib2_context * volatile impure_ptr2;

// QK extended context (FPU) save/restore
#define QK_EXT_SAVE(act_)    \
    if (((act_)->getOsObject() & QK_FPU_THREAD) != 0) \
        FPU_save(&((ThreadContext *)(act_)->getThread())->fpu)

#define QK_EXT_RESTORE(act_) \
    if (((act_)->getOsObject() & QK_FPU_THREAD) != 0) \
        FPU_restore(&((ThreadContext *)(act_)->getThread())->fpu)

extern "C" __declspec(__cdecl) void FPU_save(FPU_context *ctx); // in assembly
extern "C" __declspec(__cdecl) void FPU_restore(FPU_context *ctx); // assembly

#include "qk.h"  // QK platform-independent public interface

#endif // qk_port_h

//****************************************************************************
// NOTE01:
// The QK port on x86 is not a pure play, but rather, QK is based in this
// case on DOS.
//
