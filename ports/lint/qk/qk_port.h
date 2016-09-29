/// @file
/// @brief QK/C++ port to Lint, Generic C++ compiler
/// @cond
///***************************************************************************
/// Last updated for version 5.7.2
/// Last updated on  2016-09-26
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// http://www.state-machine.com
/// mailto:info@state-machine.com

///***************************************************************************
/// @endcond

#ifndef qk_port_h
#define qk_port_h

//lint -save -e1960    MISRA-C++:2008 Rule 7-3-1, Global declaration


//****************************************************************************
// QK extended context (FPU) save/restore

//! Define the method for saving the extended context (e.g.,
//! the context of a floating-point co-processor)
/// @note This is just an example of #QK_EXT_SAVE macro. You need to define
/// the macro appropriately for the co-processor you're using. This macro
/// is only used in the extended QK scheduler QK_scheduleExt_(). If you define
/// #QK_EXT_SAVE, you also need to provide #QK_EXT_RESTORE.
#define QK_EXT_SAVE(act_)   \
    FPU_save(static_cast<void *>((act_)->getThread()))

//! Define the method for restoring the extended context (e.g.,
/// the context of a floating-point co-processor).
/// @note This is just an example of #QK_EXT_RESTORE macro. You need to define
/// the macro appropriately for the co-processor you're using. This macro
/// is only used in the extended QK scheduler QK_scheduleExt_(). If you define
/// #QK_EXT_RESTORE, you also need to provide #QK_EXT_SAVE.
#define QK_EXT_RESTORE(act_) \
    FPU_restore(static_cast<void *>((act_)->getThread()))

//****************************************************************************
// QK interrupt entry and exit

//! Define the ISR entry sequence, if the compiler supports writing
//! interrupts in C++.
/// @note This is just an example of #QK_ISR_ENTRY. You need to define
/// the macro appropriately for the CPU/compiler you're using. Also, some
/// QK ports will not define this macro, but instead will provide ISR
/// skeleton code in assembly.
#define QK_ISR_ENTRY() do { \
    ++QK_attr_.intNest; \
} while (false)


//! Define the ISR exit sequence, if the compiler supports writing
//! interrupts in C++.
/// @note This is just an example of #QK_ISR_EXIT. You need to define
/// the macro appropriately for the CPU/compiler you're using. Also, some
/// QK ports will not define this macro, but instead will provide ISR
/// skeleton code in assembly.
#define QK_ISR_EXIT()     do { \
    --QK_attr_.intNest; \
    if (QK_attr_.intNest == static_cast<uint_fast8_t>(0)) { \
        if (QK_sched_() != static_cast<uint_fast8_t>(0)) { \
            QK_activate_(); \
        } \
    } \
    else { \
        Q_ERROR(); \
    } \
} while (false)

extern "C" {

void FPU_save(void *ctx);     // defined in assembly
void FPU_restore(void *ctx);  // defined in assembly
extern void *impure_ptr;

} // extern "C"

//lint -restore

#include "qk.h" // QK platform-independent public interface

#endif // qk_port_h
