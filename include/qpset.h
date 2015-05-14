/// @file
/// @brief platform-independent priority sets of 8 or 64 elements.
/// @ingroup qf
/// @cond
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
/// @endcond

#ifndef qpset_h
#define qpset_h

namespace QP {

//****************************************************************************
// useful lookup tables

#ifndef QF_LOG2

    //! Macro to return (log2(n_) + 1), where @p n_ = 0..255.
    /// @description
    /// This macro delivers the 1-based number of the most significant 1-bit
    /// of a byte. This macro can be re-implemented in the QP-nano ports,
    /// if the processor supports special instructions, such as CLZ (count
    /// leading zeros).@n
    /// @n
    /// If the macro is not defined in the port, the default implementation
    /// uses a lookup table.
    #define QF_LOG2(n_) (Q_ROM_BYTE(QF_log2Lkup[(n_)]))

    //! Lookup table for (log2(n) + 1), where n is the index into the table.
    /// @description
    /// This lookup delivers the 1-based number of the most significant 1-bit
    /// of a byte.
    extern uint8_t const Q_ROM QF_log2Lkup[256];

    #define QF_LOG2LKUP 1
#endif  // QF_LOG2

//! Lookup table for (1 << ((n-1) % 8)), where n is the index into the table.
/// @note Index range n = 0..64. The first index (n == 0) should never
/// be used.
extern uint8_t const Q_ROM QF_pwr2Lkup[65];

//! Lookup table for ~(1 << ((n-1) % 8)), where n is the index into the table.
/// @note
/// Index range n = 0..64. The first index (n == 0) should never be used.
extern uint8_t const Q_ROM QF_invPwr2Lkup[65];

//! Lookup table for (n-1)/8
/// @note
/// Index range n = 0..64. The first index (n == 0) should never be used.
extern uint8_t const Q_ROM QF_div8Lkup[65];

//****************************************************************************
//! Priority Set of up to 8 elements for building various schedulers,
//! but also useful as a general set of up to 8 elements of any kind.
/// @description
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by scheduling processing. The set is capable
/// of storing up to 8 priority levels.
class QPSet8 {

    uint_fast8_t volatile m_bits; //!< bimask representing elements of the set

public:

    //! the function evaluates to TRUE if the set is empty, which means that
    //! no active objects are ready to run.
    bool isEmpty(void) const {
        return (m_bits == static_cast<uint_fast8_t>(0));
    }

    //! the function evaluates to TRUE if the set has elements, which means
    //! that some active objects are ready to run.
    bool notEmpty(void) const {
        return (m_bits != static_cast<uint_fast8_t>(0));
    }

    //! the function evaluates to TRUE if the priority set has the element n.
    bool hasElement(uint_fast8_t const n) const {
        return
            ((m_bits & static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_pwr2Lkup[n])))
             != static_cast<uint_fast8_t>(0));
    }

    //! insert element @p n into the set, n = 1..8
    void insert(uint_fast8_t const n) {
        m_bits |= static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_pwr2Lkup[n]));
    }

    //! remove element @p n from the set, n = 1..8
    void remove(uint_fast8_t const n) {
        m_bits &= static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_invPwr2Lkup[n]));
    }

    //! find the maximum element in the set, returns zero if the set is empty
    uint_fast8_t findMax(void) const {
        return static_cast<uint_fast8_t>(QF_LOG2(m_bits));
    }
};

//****************************************************************************
//! Priority Set of up to 64 elements for building various schedulers,
//! but also useful as a general set of up to 64 elements of any kind.
/// @description
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by scheduling processing. The set is capable
/// of storing up to 64 priority levels.@n
/// @n
/// The priority set allows to build cooperative multitasking schedulers
/// to manage up to 64 tasks. It is also used in the Quantum Kernel (QK)
/// preemptive scheduler.
class QPSet64 {

    //! bimask representing 8-element subsets of the set
    /// @description
    /// Each bit in the bytes set represents a subset (8-elements)
    /// as follows: @n
    /// bit 0 in m_bytes is 1 when m_bits[0] is not empty @n
    /// bit 1 in m_bytes is 1 when m_bits[1] is not empty @n
    /// bit 2 in m_bytes is 1 when m_bits[2] is not empty @n
    /// bit 3 in m_bytes is 1 when m_bits[3] is not empty @n
    /// bit 4 in m_bytes is 1 when m_bits[4] is not empty @n
    /// bit 5 in m_bytes is 1 when m_bits[5] is not empty @n
    /// bit 6 in m_bytes is 1 when m_bits[6] is not empty @n
    /// bit 7 in m_bytes is 1 when m_bits[7] is not empty @n
    uint_fast8_t volatile m_bytes;

    //! bits representing elements in the set as follows: @n
    /// @description
    /// m_bits[0] represent elements  1..8  @n
    /// m_bits[1] represent elements  9..16 @n
    /// m_bits[2] represent elements 17..24 @n
    /// m_bits[3] represent elements 25..32 @n
    /// m_bits[4] represent elements 33..40 @n
    /// m_bits[5] represent elements 41..48 @n
    /// m_bits[6] represent elements 49..56 @n
    /// m_bits[7] represent elements 57..64 @n
    uint_fast8_t volatile m_bits[8];

public:

    //! the function evaluates to TRUE if the set is empty, which means
    //! that no active objects are ready to run.
    bool isEmpty(void) const {
        return (m_bytes == static_cast<uint_fast8_t>(0));
    }

    //! the function evaluates to TRUE if the set has elements, which means
    //! that some active objects are ready to run.
    bool notEmpty(void) const {
        return (m_bytes != static_cast<uint_fast8_t>(0));
    }

    //! the function evaluates to TRUE if the priority set has the element n.
    bool hasElement(uint_fast8_t const n) const {
        uint_fast8_t const m =
            static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_div8Lkup[n]));
        return ((m_bits[m]
                  & static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_pwr2Lkup[n])))
               != static_cast<uint_fast8_t>(0));
    }

    //! insert element @p n into the set, n = 1..64
    void insert(uint_fast8_t const n) {
        uint_fast8_t m =
            static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_div8Lkup[n]));
        m_bits[m] |= static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_pwr2Lkup[n]));
        m_bytes   |=
            static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_pwr2Lkup[m
                                          + static_cast<uint_fast8_t>(1)]));
    }

    //! remove element @p n from the set, n = 1..64
    void remove(uint_fast8_t const n) {
        uint_fast8_t m =
            static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_div8Lkup[n]));
        m_bits[m] &= static_cast<uint_fast8_t>(Q_ROM_BYTE(QF_invPwr2Lkup[n]));
        if (m_bits[m] == static_cast<uint_fast8_t>(0)) {
            m_bytes   &= static_cast<uint_fast8_t>(
               Q_ROM_BYTE(QF_invPwr2Lkup[m + static_cast<uint_fast8_t>(1)]));
        }
    }

    //! find the maximum element in the set, returns zero if the set is empty
    uint_fast8_t findMax(void) const {
        uint_fast8_t n;
        if (m_bytes != static_cast<uint_fast8_t>(0)) {
            n = static_cast<uint_fast8_t>(
                    static_cast<uint_fast8_t>(QF_LOG2(m_bytes))
                    - static_cast<uint_fast8_t>(1));
            n = static_cast<uint_fast8_t>(
                    static_cast<uint_fast8_t>(QF_LOG2(m_bits[n]))
                    + static_cast<uint_fast8_t>(n << 3));
        }
        else {
            n = static_cast<uint_fast8_t>(0);
        }
        return n;
    }
};

} // namespace QP

#endif // qpset_h

