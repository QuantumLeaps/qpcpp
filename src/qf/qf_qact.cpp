//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_qact")
} // unnamed namespace

namespace QP {

std::array<QActive*, QF_MAX_ACTIVE + 1U> QActive_registry_;

//----------------------------------------------------------------------------
namespace QF {
Attr priv_;
} // namespace QF

//----------------------------------------------------------------------------
QActive::QActive(QStateHandler const initial) noexcept
  : QAsm(),
    m_prio(0U),
    m_pthre(0U)
{
    // NOTE: QActive indirectly inherits the abstract QAsm base class,
    // but it will delegate the state machine behavior to the QHsm class,
    // so the following initiaization is identical as in QHsm ctor:
    m_state.fun = Q_STATE_CAST(&top);
    m_temp.fun  = initial;
}

//............................................................................
void QActive::register_() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    if (m_pthre == 0U) { // preemption-threshold not defined?
        m_pthre = m_prio; // apply the default
    }

    // AO's prio. must be in range
    Q_REQUIRE_INCRIT(100, (0U < m_prio) && (m_prio <= QF_MAX_ACTIVE));

    // the AO must NOT be registered already
    Q_REQUIRE_INCRIT(110, QActive_registry_[m_prio] == nullptr);

    // the AO's prio. must not exceed the preemption threshold
    Q_REQUIRE_INCRIT(130, m_prio <= m_pthre);

#ifndef Q_UNSAFE
    std::uint8_t prev_thre = m_pthre;
    std::uint8_t next_thre = m_pthre;

    for (std::uint8_t p = m_prio - 1U; p > 0U; --p) {
        if (QActive_registry_[p] != nullptr) {
            prev_thre = QActive_registry_[p]->m_pthre;
            break;
        }
    }
    for (std::uint8_t p = m_prio + 1U; p <= QF_MAX_ACTIVE; ++p) {
        if (QActive_registry_[p] != nullptr) {
            next_thre = QActive_registry_[p]->m_pthre;
            break;
        }
    }

    // the preemption threshold of this AO must be between
    // preemption threshold of the previous AO and next AO
    Q_ASSERT_INCRIT(160,
        (prev_thre <= m_pthre) && (m_pthre <= next_thre));

#endif // Q_UNSAFE

    // register the AO at the QF-prio.
    QActive_registry_[m_prio] = this;

    QF_CRIT_EXIT();
}

//............................................................................
void QActive::unregister_() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    std::uint8_t const p = m_prio; // put AO's prio. in a temporary

    // AO's prio. must be in range
    Q_REQUIRE_INCRIT(210, (0U < p) && (p <= QF_MAX_ACTIVE));

    // this AO must be registered at prio. p
    Q_REQUIRE_INCRIT(230, QActive_registry_[p] == this);

    m_state.fun  = nullptr; // invalidate the state
    QActive_registry_[p] = nullptr; // free-up the prio. level

    QF_CRIT_EXIT();
}
//............................................................................
void QActive::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
    // delegate to the QHsm class
    reinterpret_cast<QHsm *>(this)->QHsm::init(e, qsId);
}
//............................................................................
void QActive::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
    // delegate to the QHsm class
    reinterpret_cast<QHsm *>(this)->QHsm::dispatch(e, qsId);
}
//............................................................................
bool QActive::isIn(QStateHandler const stateHndl) noexcept {
    // delegate to the QHsm class
    return reinterpret_cast<QHsm *>(this)->QHsm::isIn(stateHndl);
}
//............................................................................
QStateHandler QActive::childState(QStateHandler const parentHandler) noexcept {
    // delegate to the QHsm class
    return reinterpret_cast<QHsm *>(this)->QHsm::childState(parentHandler);
}
//............................................................................
QActive* QActive::fromRegistry(std::uint_fast8_t const prio) {
    // return the hidden (package scope) registry entry
    return QActive_registry_[prio];
}
//............................................................................
QStateHandler QActive::getStateHandler() const noexcept {
    // delegate to the QHsm class
    return reinterpret_cast<QHsm const *>(this)->QHsm::getStateHandler();
}

//----------------------------------------------------------------------------
#ifndef QF_LOG2
std::uint_fast8_t QF_LOG2(QP::QPSetBits const bitmask) noexcept {
    // look-up table for log2(0..15)
    static constexpr std::array<std::uint8_t, 16U> log2LUT = {
        0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
        4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
    };
    std::uint_fast8_t n = 0U;
    QP::QPSetBits x = bitmask;
    QP::QPSetBits tmp; // temporary for modified bitmask parameter

#if (QF_MAX_ACTIVE > 16U)
    tmp = static_cast<QP::QPSetBits>(x >> 16U);
    if (tmp != 0U) { // x > 2^16?
        n += 16U;
        x = tmp;
    }
#endif
#if (QF_MAX_ACTIVE > 8U)
    tmp = (x >> 8U);
    if (tmp != 0U) { // x > 2^8?
        n += 8U;
        x = tmp;
    }
#endif
    tmp = (x >> 4U);
    if (tmp != 0U) {  // x > 2^4?
        n += 4U;
        x = tmp;
    }
    // x is guaranteed to be in the 0..15 range for the look-up
    return static_cast<std::uint_fast8_t>(n + log2LUT[x]);
}
#endif // ndef QF_LOG2

//----------------------------------------------------------------------------
void QPSet::setEmpty() noexcept {
    m_bits0 = 0U; // clear bitmask for elements 1..32
#if (QF_MAX_ACTIVE > 32U)
    m_bits1 = 0U; // clear bitmask for elements 33..64
#endif
}
//............................................................................
bool QPSet::isEmpty() const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
    return (m_bits0 == 0U);    // check only bitmask for elements 1..32
#else
    return (m_bits0 == 0U)     // bitmask for elements 1..32 empty?
           ? (m_bits1 == 0U)   // check bitmask for for elements 33..64
           : false;             // the set is NOT empty
#endif
}
//............................................................................
bool QPSet::notEmpty() const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
    return (m_bits0 != 0U);     // check only bitmask for elements 1..32
#else
    return (m_bits0 != 0U)      // bitmask for elements 1..32 empty?
           ? true                // the set is NOT empty
           : (m_bits1 != 0U);   // check bitmask for for elements 33..64
#endif
}
//............................................................................
bool QPSet::hasElement(std::uint_fast8_t const n) const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
    // check the bit only in bitmask for elements 1..32
    return (m_bits0 & (static_cast<QPSetBits>(1U) << (n - 1U))) != 0U;
#else
    return (n <= 32U) // which group of elements (1..32 or 33..64)?
        ? ((m_bits0 & (static_cast<QPSetBits>(1U) << (n - 1U)))  != 0U)
        : ((m_bits1 & (static_cast<QPSetBits>(1U) << (n - 33U))) != 0U);
#endif
}
//............................................................................
void QPSet::insert(std::uint_fast8_t const n) noexcept {
#if (QF_MAX_ACTIVE <= 32U)
    // set the bit only in bitmask for elements 1..32
    m_bits0 = (m_bits0 | (static_cast<QPSetBits>(1U) << (n - 1U)));
#else
    if (n <= 32U) { // set the bit in the bitmask for elements 1..32?
        m_bits0 = (m_bits0 | (static_cast<QPSetBits>(1U) << (n - 1U)));
    }
    else { // set the bit in the bitmask for for elements 33..64
        m_bits1 = (m_bits1 | (static_cast<QPSetBits>(1U) << (n - 33U)));
    }
#endif
}
//............................................................................
void QPSet::remove(std::uint_fast8_t const n) noexcept {
#if (QF_MAX_ACTIVE <= 32U)
    // clear the bit only in bitmask for elements 1..32
    m_bits0 = (m_bits0 & static_cast<QPSetBits>(~(1U << (n - 1U))));
#else
    if (n <= 32U) { // clear the bit in the bitmask for elements 1..32?
        (m_bits0 = (m_bits0 & ~(static_cast<QPSetBits>(1U) << (n - 1U))));
    }
    else { // clear the bit in the bitmask for for elements 33..64
        (m_bits1 = (m_bits1 & ~(static_cast<QPSetBits>(1U) << (n - 33U))));
    }
#endif
}
//............................................................................
std::uint_fast8_t QPSet::findMax() const noexcept {
#if (QF_MAX_ACTIVE <= 32U)
    // check only the bitmask for elements 1..32
    return QF_LOG2(m_bits0);
#else
    return (m_bits1 != 0U) // bitmask for elements 32..64 not empty?
        ? (32U + QF_LOG2(m_bits1)) // 32 + log2(bits 33..64)
        : (QF_LOG2(m_bits0));      // log2(bits 1..32)
#endif
}

} // namespace QP
