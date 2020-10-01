/// @file
/// @brief QP::QActive services and QF support code
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-18
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

//****************************************************************************
/// @description
/// macro to encapsulate pointer increment, which violates MISRA-C:2004
/// required rule 17.4 (pointer arithmetic used).
///
/// @param[in]  p_  pointer to be incremented.
///
#define QF_PTR_INC_(p_) (++(p_))

namespace QP {

Q_DEFINE_THIS_MODULE("qf_act")

// public objects ************************************************************
QActive *QF::active_[QF_MAX_ACTIVE + 1U]; // to be used by QF ports only

//****************************************************************************
/// @description
/// This function adds a given active object to the active objects managed
/// by the QF framework. It should not be called by the application directly,
/// only through the function QP::QActive::start().
///
/// @param[in]  a  pointer to the active object to add to the framework.
///
/// @note The priority of the active object @p a should be set before calling
/// this function.
///
/// @sa QP::QF::remove_()
///
void QF::add_(QActive * const a) noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(a->m_prio);

    Q_REQUIRE_ID(100, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (active_[p] == nullptr));
    QF_CRIT_STAT_
    QF_CRIT_E_();
    active_[p] = a;  // registger the active object at this priority
    QF_CRIT_X_();
}

//****************************************************************************
/// @description
/// This function removes a given active object from the active objects
/// managed by the QF framework. It should not be called by the QP ports.
///
/// @param[in]  a  pointer to the active object to remove from the framework.
///
/// @note
/// The active object that is removed from the framework can no longer
/// participate in the publish-subscribe event exchange.
///
/// @sa QP::QF::add_()
///
void QF::remove_(QActive * const a) noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(a->m_prio);

    Q_REQUIRE_ID(200, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (active_[p] == a));

    QF_CRIT_STAT_
    QF_CRIT_E_();
    active_[p] = nullptr; // free-up the priority level
    a->m_state.fun = nullptr; // invalidate the state
    QF_CRIT_X_();
}

//****************************************************************************
/// @description
/// Clears a memory buffer by writing zeros byte-by-byte.
///
/// @param[in] start  pointer to the beginning of a memory buffer.
/// @param[in] len    length of the memory buffer to clear (in bytes)
///
/// @note The main application of this function is clearing the internal QF
/// variables upon startup. This is done to avoid problems with non-standard
/// startup code provided with some compilers and toolsets (e.g., TI DSPs or
/// Microchip MPLAB), which does not zero the uninitialized variables, as
/// required by the ANSI C standard.
///
void QF::bzero(void * const start, std::uint_fast16_t const len) noexcept {
    std::uint8_t *ptr = static_cast<std::uint8_t *>(start);
    for (std::uint_fast16_t n = len; n > 0U; --n) {
        *ptr = 0U;
        QF_PTR_INC_(ptr);
    }
}

} // namespace QP

// Log-base-2 calculations ...
#ifndef QF_LOG2

//! function that returns (log2(x) + 1), where @p x is a 32-bit bitmask
///
/// @description
/// This function returns the 1-based number of the most significant 1-bit
/// of a 32-bit bitmask. This function can be replaced in the QP ports, if
/// the CPU has special instructions, such as CLZ (count leading zeros).
///
extern "C" {

    std::uint_fast8_t QF_LOG2(QP::QPSetBits x) noexcept {
        static std::uint8_t const log2LUT[16] = {
            0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
            4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
        };
        std::uint_fast8_t n = 0U;
        QP::QPSetBits t;

#if (QF_MAX_ACTIVE > 16U)
        t = static_cast<QP::QPSetBits>(x >> 16U);
        if (t != 0U) {
            n += 16U;
            x = t;
        }
#endif
#if (QF_MAX_ACTIVE > 8U)
        t = (x >> 8U);
        if (t != 0U) {
            n += 8U;
            x = t;
        }
#endif
        t = (x >> 4U);
        if (t != 0U) {
            n += 4U;
            x = t;
        }
        return n + log2LUT[x];
    }

} // extern "C"

#endif // QF_LOG2

