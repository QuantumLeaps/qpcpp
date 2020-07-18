/// @file
/// @brief platform-independent priority sets of 8 or 64 elements.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.8.2
/// Last updated on  2020-07-18
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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

//****************************************************************************
// Log-base-2 calculations ...
#ifndef QF_LOG2
    extern "C" std::uint_fast8_t QF_LOG2(QPSetBits x) noexcept;
#endif // QF_LOG2

//****************************************************************************
#if (QF_MAX_ACTIVE <= 32)
//! Priority Set of up to 32 elements */
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by the scheduling algorithm. The set is
/// capable of storing up to 32 priority levels. QP::QPSet is specifically
/// declared as a POD (Plain Old Data) for ease of initialization and
/// interfacing with plain "C" code.
///
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
        m_bits |= (1U << (n - 1U));
    }

    //! remove element @p n from the set, n = 1..QF_MAX_ACTIVE
    /// @note
    /// intentionally misspelled ("rmove") to avoid collision with
    /// the C++ standard library facility "remove"
    void rmove(std::uint_fast8_t const n) noexcept {
        m_bits &=
           static_cast<QPSetBits>(~(static_cast<QPSetBits>(1) << (n - 1U)));
    }

    std::uint_fast8_t findMax(void) const noexcept {
        return QF_LOG2(m_bits);
    }
};

#else // QF_MAX_ACTIVE > 32U

//! Priority Set of up to 64 elements
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by the scheduling algorithm. The set is
/// capable of storing up to 64 priority levels. QP::QPSet is specifically
/// declared as a POD (Plain Old Data) for ease of initialization and
/// interfacing with plain "C" code.
///
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
            m_bits[0] |= (static_cast<std::uint32_t>(1) << (n - 1U));
        }
        else {
            m_bits[1] |= (static_cast<std::uint32_t>(1) << (n - 33U));
        }
    }

    //! remove element @p n from the set, n = 1..64
    /// @note
    /// intentionally misspelled ("rmove") to avoid collision with
    /// the C++ standard library facility "remove"
    void rmove(std::uint_fast8_t const n) noexcept {
        if (n <= 32U) {
            (m_bits[0] &= ~(static_cast<std::uint32_t>(1) << (n - 1U)));
        }
        else {
            (m_bits[1] &= ~(static_cast<std::uint32_t>(1) << (n - 33U)));
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

