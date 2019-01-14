/// @file
/// @brief platform-independent priority sets of 8 or 64 elements.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.3.8
/// Last updated on  2019-01-11
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qpset_h
#define qpset_h

namespace QP {

#if (!defined QF_MAX_ACTIVE) || (QF_MAX_ACTIVE < 1) || (64 < QF_MAX_ACTIVE)
    #error "QF_MAX_ACTIVE not defined or out of range. Valid range is 1..64"
#elif (QF_MAX_ACTIVE <= 8)
    typedef uint8_t QPSetBits;
#elif (QF_MAX_ACTIVE <= 16)
    typedef uint16_t QPSetBits;
#else
    //! bitmask for the internal representation of QPSet elements
    typedef uint32_t QPSetBits;
#endif

//****************************************************************************
// Log-base-2 calculations ...
#ifndef QF_LOG2
    extern "C" uint_fast8_t QF_LOG2(QPSetBits x);
#endif // QF_LOG2

//****************************************************************************
#if (QF_MAX_ACTIVE <= 32)
//! Priority Set of up to 32 elements */
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by the scheduling algorithm. The set is
/// capable of storing up to 32 priority levels.
///
class QPSet {

    QPSetBits volatile m_bits;  //!< bitmask with a bit for each element

public:

    //! Makes the priority set @p me_ empty.
    void setEmpty(void) {
        m_bits = static_cast<QPSetBits>(0);
    }

    //! Evaluates to true if the priority set is empty
    bool isEmpty(void) const {
        return (m_bits == static_cast<QPSetBits>(0));
    }

    //! Evaluates to true if the priority set is not empty
    bool notEmpty(void) const {
        return (m_bits != static_cast<QPSetBits>(0));
    }

    //! the function evaluates to TRUE if the priority set has the element n.
    bool hasElement(uint_fast8_t const n) const {
        return (m_bits & (static_cast<QPSetBits>(1)
                          << (n - static_cast<uint_fast8_t>(1))))
               != static_cast<QPSetBits>(0);
    }

    //! insert element @p n into the set, n = 1..QF_MAX_ACTIVE
    void insert(uint_fast8_t const n) {
        m_bits |= static_cast<QPSetBits>(
            static_cast<QPSetBits>(1) << (n - static_cast<uint_fast8_t>(1)));
    }

    //! remove element @p n from the set, n = 1..QF_MAX_ACTIVE
    void remove(uint_fast8_t const n) {
        m_bits &= static_cast<QPSetBits>(
           ~(static_cast<QPSetBits>(1) << (n - static_cast<uint_fast8_t>(1))));
    }

    uint_fast8_t findMax(void) const {
        return QF_LOG2(m_bits);
    }
};

#else // QF_MAX_ACTIVE > 32

//! Priority Set of up to 64 elements
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by the scheduling algorithm. The set is
/// capable of storing up to 64 priority levels.
///
class QPSet {

    uint32_t volatile m_bits[2]; //!< 2 bitmasks with a bit for each element

public:

    //! Makes the priority set @p me_ empty.
    void setEmpty(void) {
        m_bits[0] = static_cast<uint32_t>(0);
        m_bits[1] = static_cast<uint32_t>(0);
    }

    //! Evaluates to true if the priority set is empty
    // the following logic avoids UB in volatile access for MISRA compliantce
    bool isEmpty(void) const {
        return (m_bits[0] == static_cast<uint32_t>(0))
               ? (m_bits[1] == static_cast<uint32_t>(0))
               : false;
    }

    //! Evaluates to true if the priority set is not empty
    // the following logic avoids UB in volatile access for MISRA compliantce
    bool notEmpty(void) const {
        return (m_bits[0] != static_cast<uint32_t>(0))
               ? true
               : (m_bits[1] != static_cast<uint32_t>(0));
    }

    //! the function evaluates to TRUE if the priority set has the element n.
    bool hasElement(uint_fast8_t const n) const {
        return (n <= static_cast<uint_fast8_t>(32))
            ? ((m_bits[0] & (static_cast<uint32_t>(1)
                             << (n - static_cast<uint_fast8_t>(1))))
               != static_cast<uint32_t>(0))
            : ((m_bits[1] & (static_cast<uint32_t>(1)
                             << (n - static_cast<uint_fast8_t>(33))))
               != static_cast<uint32_t>(0));
    }

    //! insert element @p n into the set, n = 1..64
    void insert(uint_fast8_t const n) {
        if (n <= static_cast<uint_fast8_t>(32)) {
            m_bits[0] |= (static_cast<uint32_t>(1)
                          << (n - static_cast<uint_fast8_t>(1)));
        }
        else {
            m_bits[1] |= (static_cast<uint32_t>(1)
                          << (n - static_cast<uint_fast8_t>(33)));
        }
    }

    //! remove element @p n from the set, n = 1..64
    void remove(uint_fast8_t const n) {
        if (n <= static_cast<uint_fast8_t>(32)) {
            (m_bits[0] &= ~(static_cast<uint32_t>(1)
                            << (n - static_cast<uint_fast8_t>(1))));
        }
        else {
            (m_bits[1] &= ~(static_cast<uint32_t>(1)
                            << (n - static_cast<uint_fast8_t>(33))));
        }
    }

    //! find the maximum element in the set, returns zero if the set is empty
    uint_fast8_t findMax(void) const {
        return (m_bits[1] != static_cast<uint32_t>(0))
            ? (QF_LOG2(m_bits[1])
              + static_cast<uint_fast8_t>(32)) \
            : (QF_LOG2(m_bits[0]));
    }
};

#endif // QF_MAX_ACTIVE

} // namespace QP

#endif // qpset_h

