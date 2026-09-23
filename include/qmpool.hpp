//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005-2026 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License, version 3 or later (GPL-3.0-or-later), or,
// alternatively, under the terms of one of the closed-source Quantum Leaps
// commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
//
// NOTE:
// The GPL does NOT permit distributing this code as part of proprietary
// (closed-source) programs. For such use, please contact Quantum Leaps for
// a commercial license, which is an alternative to the GPL designed
// explicitly for closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QMPOOL_HPP_
#define QMPOOL_HPP_

#ifndef QF_MPOOL_CTR_SIZE
    #define QF_MPOOL_CTR_SIZE 2U
#endif

#define QF_MPOOL_EL(evType_) struct { \
    void * sto_[((sizeof(evType_) - 1U) / sizeof(void *)) + \
                    (sizeof(evType_) < (2U * sizeof(void *)) ? 2U : 1U)]; }

//============================================================================
namespace QP {

using QMPoolSize = std::uint16_t;

#if (QF_MPOOL_CTR_SIZE == 1U)
    using QMPoolCtr = std::uint8_t;
#elif (QF_MPOOL_CTR_SIZE == 2U)
    using QMPoolCtr = std::uint16_t;
#else
    #error QF_MPOOL_CTR_SIZE defined incorrectly, expected 1U or 2U
#endif

//============================================================================
class QMPool {
private:
    void * *m_start;
    void * *m_end;
    void * *m_freeHead;
    QMPoolSize m_blockSize;
    QMPoolCtr m_nTot;
    QMPoolCtr m_nFree;
    QMPoolCtr m_nMin;

public:
    QMPool() noexcept;
    void init(
        void * const poolSto,
        std::uint_fast32_t const poolSize,
        std::uint_fast16_t const blockSize) noexcept;
    void * get(
        std::uint_fast16_t const margin,
        std::uint_fast8_t const qsId) noexcept;
    void put(
        void * const block,
        std::uint_fast8_t const qsId) noexcept;
    QMPoolSize getBlockSize() const noexcept;
    std::uint16_t getUse() const noexcept;
    std::uint16_t getFree() const noexcept;
    std::uint16_t getMin() const noexcept;

#ifdef QF_ISR_API
    void * getFromISR(
        std::uint_fast16_t const margin,
        std::uint_fast8_t const qsId) noexcept;
    void putFromISR(
        void * const b,
        std::uint_fast8_t const qsId) noexcept;
#endif // def QF_ISR_API

private:
    // friends...
    friend class QS;
}; // class QMPool

} // namespace QP

#endif // QMPOOL_HPP_
