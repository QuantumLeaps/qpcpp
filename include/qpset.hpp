//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief platform-independent priority sets of 8 or 64 elements.

#ifndef QPSET_HPP
#define QPSET_HPP

#ifndef QF_MAX_ACTIVE
    // default value when NOT defined
    #define QF_MAX_ACTIVE 32U
#endif

namespace QP {

#if (QF_MAX_ACTIVE < 1U) || (64U < QF_MAX_ACTIVE)
    #error "QF_MAX_ACTIVE out of range. Valid range is 1U..64U"
#elif (QF_MAX_ACTIVE <= 8U)
    using QPSetBits = std::uint8_t;
#elif (QF_MAX_ACTIVE <= 16U)
    using QPSetBits = std::uint16_t;
#else
    //! bitmask for the internal representation of QPSet elements
    using QPSetBits = std::uint32_t;
#endif

//============================================================================
// Log-base-2 calculations ...
#ifndef QF_LOG2

//! function that returns (log2(x) + 1), where @p x is a 32-bit bitmask
//!
//! @description
//! This function returns the 1-based number of the most significant 1-bit
//! of a 32-bit bitmask. This function can be replaced in the QP ports, if
//! the CPU has special instructions, such as CLZ (count leading zeros).
//!
extern "C" std::uint_fast8_t QF_LOG2(QPSetBits x) noexcept;

#endif // QF_LOG2

//============================================================================
#if (QF_MAX_ACTIVE <= 32)
//! Priority Set of up to 32 elements */
//!
//! @description
//! The priority set represents the set of active objects that are ready to
//! run and need to be considered by the scheduling algorithm. The set is
//! capable of storing up to 32 priority levels. QP::QPSet is specifically
//! declared as a POD (Plain Old Data) for ease of initialization and
//! interfacing with plain "C" code.
//!
struct QPSet {

    QPSetBits volatile m_bits;  //!< bitmask with a bit for each element

    //! Makes the priority set @p me_ empty.
    void setEmpty(void) noexcept {
        m_bits = 0U;
    }

    //! Evaluates to true if the priority set is empty
    bool isEmpty(void) const noexcept {
        return (m_bits == 0U);
    }

    //! Evaluates to true if the priority set is not empty
    bool notEmpty(void) const noexcept {
        return (m_bits != 0U);
    }

    //! the function evaluates to TRUE if the priority set has the element n.
    bool hasElement(std::uint_fast8_t const n) const noexcept {
        return (m_bits & (1U << (n - 1U))) != 0U;
    }

    //! insert element @p n into the set, n = 1..QF_MAX_ACTIVE
    void insert(std::uint_fast8_t const n) noexcept {
        m_bits = (m_bits | (1U << (n - 1U)));
    }

    //! remove element @p n from the set, n = 1..QF_MAX_ACTIVE
    //!
    //! @note
    //! intentionally misspelled ("rmove") to avoid collision with
    //! the C++ standard library facility "remove"
    void rmove(std::uint_fast8_t const n) noexcept {
        m_bits = (m_bits &
           static_cast<QPSetBits>(~(static_cast<QPSetBits>(1) << (n - 1U))));
    }

    std::uint_fast8_t findMax(void) const noexcept {
        return QF_LOG2(m_bits);
    }
};

#else // QF_MAX_ACTIVE > 32U

//! Priority Set of up to 64 elements
//!
//! @description
//! The priority set represents the set of active objects that are ready to
//! run and need to be considered by the scheduling algorithm. The set is
//! capable of storing up to 64 priority levels. QP::QPSet is specifically
//! declared as a POD (Plain Old Data) for ease of initialization and
//! interfacing with plain "C" code.
//!
struct QPSet {

    //! Two 32-bit bitmasks with a bit for each element
    std::uint32_t volatile m_bits[2];

    //! Makes the priority set @p me_ empty.
    void setEmpty(void) noexcept {
        m_bits[0] = 0U;
        m_bits[1] = 0U;
    }

    //! Evaluates to true if the priority set is empty
    // the following logic avoids UB in volatile access for MISRA compliantce
    bool isEmpty(void) const noexcept {
        return (m_bits[0] == 0U) ? (m_bits[1] == 0U) : false;
    }

    //! Evaluates to true if the priority set is not empty
    // the following logic avoids UB in volatile access for MISRA compliantce
    bool notEmpty(void) const noexcept {
        return (m_bits[0] != 0U) ? true : (m_bits[1] != 0U);
    }

    //! the function evaluates to TRUE if the priority set has the element n.
    bool hasElement(std::uint_fast8_t const n) const noexcept {
        return (n <= 32U)
            ? ((m_bits[0] & (static_cast<std::uint32_t>(1) << (n - 1U)))
                  != 0U)
            : ((m_bits[1] & (static_cast<std::uint32_t>(1) << (n - 33U)))
                  != 0U);
    }

    //! insert element @p n into the set, n = 1..64
    void insert(std::uint_fast8_t const n) noexcept {
        if (n <= 32U) {
            m_bits[0] = (m_bits[0]
                | (static_cast<std::uint32_t>(1) << (n - 1U)));
        }
        else {
            m_bits[1] = (m_bits[1]
                | (static_cast<std::uint32_t>(1) << (n - 33U)));
        }
    }

    //! remove element @p n from the set, n = 1..64
    //!
    //! @note
    //! intentionally misspelled ("rmove") to avoid collision with
    //! the C++ standard library facility "remove"
    void rmove(std::uint_fast8_t const n) noexcept {
        if (n <= 32U) {
            (m_bits[0] = (m_bits[0]
                & ~(static_cast<std::uint32_t>(1) << (n - 1U))));
        }
        else {
            (m_bits[1] = (m_bits[1]
                & ~(static_cast<std::uint32_t>(1) << (n - 33U))));
        }
    }

    //! find the maximum element in the set, returns zero if the set is empty
    std::uint_fast8_t findMax(void) const noexcept {
        return (m_bits[1] != 0U)
            ? (QF_LOG2(m_bits[1]) + 32U)
            : (QF_LOG2(m_bits[0]));
    }
};

#endif // QF_MAX_ACTIVE

} // namespace QP

#endif // QPSET_HPP
