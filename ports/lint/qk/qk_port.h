/// \file
/// \brief QK/C++ port to Lint, Generic C++ compiler
/// \cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-03-14
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#ifndef qk_port_h
#define qk_port_h

//lint -save -e1960    MISRA-C++:2008 Rule 7-3-1, Global declaration

/// \file
/// \ingroup qk
/// \brief QK/C++ port to QK for a "generic" C++ compiler.
///
/// \note This is just an EXAMPLE of a QK port used for "linting" the QK.

//****************************************************************************
// QK extended context (FPU) save/restore

/// \brief Define the method for saving the extended context (e.g.,
/// the context of a floating-point co-processor).
///
/// \note This is just an example of #QK_EXT_SAVE macro. You need to define
/// the macro appropriately for the co-processor you're using. This macro
/// is only used in the extended QK scheduler QK_scheduleExt_(). If you define
/// #QK_EXT_SAVE, you also need to provide #QK_EXT_RESTORE.
#define QK_EXT_SAVE(act_)   \
    FPU_save(static_cast<void *>((act_)->getThread()))

/// \brief Define the method for restoring the extended context (e.g.,
/// the context of a floating-point co-processor).
///
/// \note This is just an example of #QK_EXT_RESTORE macro. You need to define
/// the macro appropriately for the co-processor you're using. This macro
/// is only used in the extended QK scheduler QK_scheduleExt_(). If you define
/// #QK_EXT_RESTORE, you also need to provide #QK_EXT_SAVE.
#define QK_EXT_RESTORE(act_) \
    FPU_restore(static_cast<void *>((act_)->getThread()))

//****************************************************************************
// QK interrupt entry and exit

/// \brief Define the ISR entry sequence, if the compiler supports writing
/// interrupts in C++.
///
/// \note This is just an example of #QK_ISR_ENTRY. You need to define
/// the macro appropriately for the CPU/compiler you're using. Also, some
/// QK ports will not define this macro, but instead will provide ISR
/// skeleton code in assembly.
#define QK_ISR_ENTRY() do { \
    ++QK_intNest_; \
    QF_QS_ISR_ENTRY(QK_intNest_, QK_currPrio_); \
} while (false)


/// \brief Define the ISR exit sequence, if the compiler supports writing
/// interrupts in C++.
///
/// \note This is just an example of #QK_ISR_EXIT. You need to define
/// the macro appropriately for the CPU/compiler you're using. Also, some
/// QK ports will not define this macro, but instead will provide ISR
/// skeleton code in assembly.
#define QK_ISR_EXIT() do { \
    send End-Of-Interrupt instruction to the Interrupt Controller; \
    QF_QS_ISR_EXIT(QK_intNest_, QK_currPrio_); \
    --QK_intNest_; \
    if (QK_intNest_ == static_cast<uint8_t>(0)) { \
        QK_scheduleExt_(); \
    } \
} while (false)

//****************************************************************************
// Thread-Local-Storage switching

/// \brief Define the method for switching the Thread-Local-Storage for
/// for a given thread.
///
/// \note This is just an example of #QK_TLS macro. You need to define
/// the macro appropriately for the runtime library you're using. This macro
/// is optional and you don't need to define it. The macro is used in both
/// the regular QK scheduler QK_sched_() and the extended QK scheduler
/// QK_schedExt_().
#define QK_TLS(act_)        \
    (impure_ptr = static_cast<void *>((act_)->getThread()))

extern "C" {

void FPU_save(void *ctx);     // defined in assembly
void FPU_restore(void *ctx);  // defined in assembly
extern void *impure_ptr;

} // extern "C"

//lint -restore

#include "qk.h" // QK platform-independent public interface

#endif // qk_port_h
