//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Version 8.0.2
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
#ifndef QMPOOL_HPP_
#define QMPOOL_HPP_

#ifndef QF_MPOOL_SIZ_SIZE
    #define QF_MPOOL_SIZ_SIZE 2U
#endif
#ifndef QF_MPOOL_CTR_SIZE
    #define QF_MPOOL_CTR_SIZE 2U
#endif

#define QF_MPOOL_EL(evType_) struct { \
    void * sto_[((sizeof(evType_) - 1U) / sizeof(void *)) + \
                    (sizeof(evType_) < (2U * sizeof(void *)) ? 2U : 1U)]; \
}

//============================================================================
namespace QP {

#if (QF_MPOOL_SIZ_SIZE == 1U)
    using QMPoolSize = std::uint8_t;
#elif (QF_MPOOL_SIZ_SIZE == 2U)
    using QMPoolSize = std::uint16_t;
#elif (QF_MPOOL_SIZ_SIZE == 4U)
    using QMPoolSize = std::uint32_t;
#else
    #error QF_MPOOL_SIZ_SIZE defined incorrectly, expected 1U, 2U, or 4U
#endif

#if (QF_MPOOL_CTR_SIZE == 1U)
    using QMPoolCtr = std::uint8_t;
#elif (QF_MPOOL_CTR_SIZE == 2U)
    using QMPoolCtr = std::uint16_t;
#elif (QF_MPOOL_CTR_SIZE == 4U)
    using QMPoolCtr = std::uint32_t;
#else
    #error QF_MPOOL_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U
#endif

//============================================================================
class QMPool {
private:
    void * * m_start;
    void * * m_end;
    void * * volatile m_freeHead;
    QMPoolSize m_blockSize;
    QMPoolCtr m_nTot;
    QMPoolCtr volatile m_nFree;
    QMPoolCtr m_nMin;

#ifndef Q_UNSAFE
    std::uintptr_t m_freeHead_dis;
    QMPoolCtr m_nFree_dis;
#endif // ndef Q_UNSAFE

public:
    QMPool()
      : m_start(nullptr),
        m_end(nullptr),
        m_freeHead(nullptr),
        m_blockSize(0U),
        m_nTot(0U),
        m_nFree(0U),
        m_nMin(0U)
    #ifndef Q_UNSAFE
       ,m_freeHead_dis(static_cast<std::uintptr_t>(~0U)),
        m_nFree_dis(static_cast<QEQueueCtr>(~0U))
    #endif
    {}
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
    QMPoolSize getBlockSize() const noexcept {
        return m_blockSize;
    }
    QMPoolCtr getNMin() const noexcept {
        #ifndef Q_UNSAFE
        return m_nMin;
        #else
        return 0U;
        #endif
    }
    QMPoolCtr getNFree() const noexcept {
        return m_nFree;
    }

private:
    QMPool(QEQueue const & other) = delete;
    QMPool & operator=(QMPool const & other) = delete;

public:

#ifdef QF_ISR_API
    void * getFromISR(
        std::uint_fast16_t const margin,
        std::uint_fast8_t const qsId) noexcept;
#endif // def QF_ISR_API

#ifdef QF_ISR_API
    void putFromISR(
        void * const b,
        std::uint_fast8_t const qsId) noexcept;
#endif // def QF_ISR_API
}; // class QMPool

} // namespace QP

#endif // QMPOOL_HPP_
