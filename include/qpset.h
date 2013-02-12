//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++
// Last Updated for Version: 4.5.04
// Date of the Last Update:  Feb 04, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#ifndef qpset_h
#define qpset_h

/// \file
/// \ingroup qf qk
/// \brief platform-independent priority sets of 8 or 64 elements.
///
/// This header file must be included in those QF ports that use the
/// cooperative multitasking QF scheduler or the QK.

QP_BEGIN_

//////////////////////////////////////////////////////////////////////////////
// useful lookup tables

#ifndef QF_LOG2

    /// \brief Macro to return (log2(n_) + 1), where \a n_ = 0..255.
    ///
    /// This macro delivers the 1-based number of the most significant 1-bit
    /// of a byte. This macro can be re-implemented in the QP-nano ports,
    /// if the processor supports special instructions, such as CLZ (count
    /// leading zeros).
    ///
    /// If the macro is not defined in the port, the default implementation
    /// uses a lookup table.
    ///
    #define QF_LOG2(n_) (Q_ROM_BYTE(QF_log2Lkup[(n_)]))

    /// \brief Lookup table for (log2(n) + 1), where n is the index
    /// into the table.
    ///
    /// This lookup delivers the 1-based number of the most significant 1-bit
    /// of a byte.
    ///
    extern uint8_t const Q_ROM Q_ROM_VAR QF_log2Lkup[256];

#endif                                                              // QF_LOG2

/// \brief Lookup table for (1 << ((n-1) % 8)), where n is the index
/// into the table.
///
/// \note Index range n = 0..64. The first index (n == 0) should never
/// be used.
extern uint8_t const Q_ROM Q_ROM_VAR QF_pwr2Lkup[65];

/// \brief Lookup table for ~(1 << ((n-1) % 8)), where n is the index
/// into the table.
///
/// \note Index range n = 0..64. The first index (n == 0) should never
/// be used.
extern uint8_t const Q_ROM Q_ROM_VAR QF_invPwr2Lkup[65];

/// \brief Lookup table for (n-1)/8
///
/// \note Index range n = 0..64. The first index (n == 0) should never
/// be used.
extern uint8_t const Q_ROM Q_ROM_VAR QF_div8Lkup[65];

//////////////////////////////////////////////////////////////////////////////
/// \brief Priority Set of up to 8 elements for building various schedulers,
/// but also useful as a general set of up to 8 elements of any kind.
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by scheduling processing. The set is capable
/// of storing up to 8 priority levels.
class QPSet8 {

    //////////////////////////////////////////////////////////////////////////
    /// \brief bimask representing elements of the set
    uint8_t m_bits;

public:

    /// \brief the function evaluates to TRUE if the set is empty,
    /// which means that no active objects are ready to run.
    bool isEmpty(void) const {
        return (m_bits == static_cast<uint8_t>(0));
    }

    /// \brief the function evaluates to TRUE if the set has elements,
    /// which means that some active objects are ready to run.
    bool notEmpty(void) const {
        return (m_bits != static_cast<uint8_t>(0));
    }

    /// \brief the function evaluates to TRUE if the priority set has the
    /// element \a n.
    bool hasElement(uint8_t const n) const {
        return ((m_bits & Q_ROM_BYTE(QF_pwr2Lkup[n]))
                != static_cast<uint8_t>(0));
    }

    /// \brief insert element \a n into the set, n = 1..8
    void insert(uint8_t const n) {
        m_bits |= Q_ROM_BYTE(QF_pwr2Lkup[n]);
    }

    /// \brief remove element \a n from the set, n = 1..8
    void remove(uint8_t const n) {
        m_bits &= Q_ROM_BYTE(QF_invPwr2Lkup[n]);
    }

    /// \brief find the maximum element in the set,
    /// \note returns zero if the set is empty
    uint8_t findMax(void) const {
        return QF_LOG2(m_bits);
    }

    friend class QPSet64;
};

//////////////////////////////////////////////////////////////////////////////
/// \brief Priority Set of up to 64 elements for building various schedulers,
/// but also useful as a general set of up to 64 elements of any kind.
///
/// The priority set represents the set of active objects that are ready to
/// run and need to be considered by scheduling processing. The set is capable
/// of storing up to 64 priority levels.
///
/// The priority set allows to build cooperative multitasking schedulers
/// to manage up to 64 tasks. It is also used in the Quantum Kernel (QK)
/// preemptive scheduler.
///
/// The inherited 8-bit set is used as the 8-element set of of 8-bit subsets
/// Each bit in the super.bits set represents a subset (8-elements)
/// as follows: \n
/// bit 0 in this->m_bits is 1 when subset[0] is not empty \n
/// bit 1 in this->m_bits is 1 when subset[1] is not empty \n
/// bit 2 in this->m_bits is 1 when subset[2] is not empty \n
/// bit 3 in this->m_bits is 1 when subset[3] is not empty \n
/// bit 4 in this->m_bits is 1 when subset[4] is not empty \n
/// bit 5 in this->m_bits is 1 when subset[5] is not empty \n
/// bit 6 in this->m_bits is 1 when subset[6] is not empty \n
/// bit 7 in this->m_bits is 1 when subset[7] is not empty \n
class QPSet64 {

    //////////////////////////////////////////////////////////////////////////
    /// \brief bimask representing 8-element subsets of the set
    uint8_t m_bytes;

    /// \brief bits representing elements in the set as follows: \n
    /// m_bits[0] represent elements  1..8  \n
    /// m_bits[1] represent elements  9..16 \n
    /// m_bits[2] represent elements 17..24 \n
    /// m_bits[3] represent elements 25..32 \n
    /// m_bits[4] represent elements 33..40 \n
    /// m_bits[5] represent elements 41..48 \n
    /// m_bits[6] represent elements 49..56 \n
    /// m_bits[7] represent elements 57..64 \n
    uint8_t m_bits[8];

public:

    /// \brief the function evaluates to TRUE if the set is empty,
    /// which means that no active objects are ready to run.
    bool isEmpty(void) const {
        return (m_bytes == static_cast<uint8_t>(0));
    }

    /// \brief the function evaluates to TRUE if the set has elements,
    /// which means that some active objects are ready to run.
    bool notEmpty(void) const {
        return (m_bytes != static_cast<uint8_t>(0));
    }

    /// \brief the function evaluates to TRUE if the priority set has the
    /// element \a n.
    bool hasElement(uint8_t const n) const {
        uint8_t const m = Q_ROM_BYTE(QF_div8Lkup[n]);
        return ((m_bits[m] & Q_ROM_BYTE(QF_pwr2Lkup[n]))
                != static_cast<uint8_t>(0));
    }

    /// \brief insert element \a n into the set, n = 1..64
    void insert(uint8_t const n) {
        uint8_t m = Q_ROM_BYTE(QF_div8Lkup[n]);
        m_bits[m] |= Q_ROM_BYTE(QF_pwr2Lkup[n]);
        m_bytes |= Q_ROM_BYTE(QF_pwr2Lkup[m + static_cast<uint8_t>(1)]);
    }

    /// \brief remove element \a n from the set, n = 1..64
    void remove(uint8_t const n) {
        uint8_t m = Q_ROM_BYTE(QF_div8Lkup[n]);
        if ((m_bits[m] &= Q_ROM_BYTE(QF_invPwr2Lkup[n]))
             == static_cast<uint8_t>(0))
        {
            m_bytes &=Q_ROM_BYTE(QF_invPwr2Lkup[m + static_cast<uint8_t>(1)]);
        }
    }

    /// \brief find the maximum element in the set,
    /// \note returns zero if the set is empty
    uint8_t findMax(void) const {
        uint8_t n;
        if (m_bytes != static_cast<uint8_t>(0)) {
            n = static_cast<uint8_t>(QF_LOG2(m_bytes)
                                     - static_cast<uint8_t>(1));
            n = static_cast<uint8_t>(QF_LOG2(m_bits[n])
                                     + static_cast<uint8_t>(n << 3));
        }
        else {
            n = static_cast<uint8_t>(0);
        }
        return n;
    }
};

QP_END_

#endif                                                              // qpset_h

