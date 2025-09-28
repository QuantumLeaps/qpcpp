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
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_mem")
} // unnamed namespace

namespace QP {

//............................................................................
QMPool::QMPool() noexcept
  : m_start(nullptr),    // no storage initially
    m_end(nullptr),      // no storage
    m_freeHead(nullptr), // no storage
    m_blockSize(0U),     // zero block
    m_nTot(0U),          // no blocks in the pool
    m_nFree(0U),         // no free blocks
    m_nMin(0U)           // minimum free blocks so far
{}
//............................................................................
void QMPool::init(
    void * const poolSto,
    std::uint_fast32_t const poolSize,
    std::uint_fast16_t const blockSize) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the pool storage must be provided
    Q_REQUIRE_INCRIT(100, poolSto != nullptr);

    m_start    = static_cast<void * *>(poolSto);
    m_freeHead = m_start;

    // find # free links in a memory block, see NOTE1
    m_blockSize = static_cast<QMPoolSize>(2U * sizeof(void *));
    std::uint_fast16_t index = 2U; // index of the next block
    while (m_blockSize < static_cast<QMPoolSize>(blockSize)) {
        m_blockSize += static_cast<QMPoolSize>(sizeof(void *));
        ++index;
    }

    // the pool buffer must fit at least one rounded-up block
    Q_ASSERT_INCRIT(110, poolSize >= m_blockSize);

    // start at the head of the free list
    void * *pfb = m_freeHead; // pointer to free block
    std::uint32_t nTot = 1U; // the last block already in the list

    // chain all blocks together in a free-list...
    for (std::uint_fast32_t size = poolSize - m_blockSize;
         size >= static_cast<std::uint_fast32_t>(m_blockSize);
         size -= static_cast<std::uint_fast32_t>(m_blockSize))
    {
        pfb[0] = &pfb[index]; // set the next link to next free block
#ifndef Q_UNSAFE
        pfb[1] = pfb[0]; // update Duplicate Storage (NOT inverted)
#endif
        pfb = static_cast<void * *>(pfb[0]); // advance to the next block
        ++nTot; // one more free block in the pool
    }
    pfb[0] = nullptr; // the last link points to NULL

    // the total number of blocks must fit in the configured dynamic range
#if (QF_MPOOL_CTR_SIZE == 1U)
    Q_ASSERT_INCRIT(160, nTot < 0xFFU);
#elif (QF_MPOOL_CTR_SIZE == 2U)
    Q_ASSERT_INCRIT(160, nTot < 0xFFFFU);
#endif

    m_nTot  = static_cast<QMPoolCtr>(nTot);
    m_nFree = m_nTot; // all blocks are free
    m_end   = pfb;    // the last block in this pool
    m_nMin  = m_nTot; // the minimum # free blocks

#ifndef Q_UNSAFE
    pfb[1] = pfb[0]; // update Duplicate Storage (NOT inverted)
#endif

    QF_CRIT_EXIT();
}

//............................................................................
void * QMPool::get(
    std::uint_fast16_t const margin,
    std::uint_fast8_t const qsId) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // get members into temporaries
    void * *pfb     = m_freeHead; // pointer to free block
    QMPoolCtr nFree = m_nFree;    // get member into temporary

    // have more free blocks than the requested margin?
    if (nFree > static_cast<QMPoolCtr>(margin)) {
        // the free block pointer must be valid
        Q_ASSERT_INCRIT(330, pfb != nullptr);

        // fast temporary
        void * * const pfb_next = static_cast<void * *>(pfb[0]);

        // the free block must have integrity (Duplicate Storage, NOT inverted)
        Q_INVARIANT_INCRIT(342, pfb_next == pfb[1]);

        --nFree; // one less free block
        if (nFree == 0U) { // is the pool becoming empty?
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_INCRIT(350, pfb_next == nullptr);

            m_nFree = 0U; // no more free blocks
            m_nMin  = 0U; // remember that the pool got empty
        }
        else { // the pool is NOT empty

            // the next free-block pointer must be in range
            Q_ASSERT_INCRIT(360,
                (m_start <= pfb_next) && (pfb_next <= m_end));

            m_nFree = nFree; // update the original
            if (m_nMin > nFree) { // is this the new minimum?
                m_nMin = nFree; // remember the minimum so far
            }
        }

        m_freeHead = pfb_next; // set the head to the next free block

        // change the allocated block contents so that it is different
        // than a free block inside the pool.
        pfb[0] = &m_end[1]; // invalid location beyond the end
#ifndef Q_UNSAFE
        pfb[1] = nullptr; // invalidate the duplicate storage
#endif

        QS_BEGIN_PRE(QS_QF_MPOOL_GET, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(this);      // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(m_nMin);    // min # free blocks ever in the pool
        QS_END_PRE()
    }
    else { // don't have enough free blocks at this point
        pfb = nullptr;

        QS_BEGIN_PRE(QS_QF_MPOOL_GET_ATTEMPT, qsId)
            QS_TIME_PRE();         // timestamp
            QS_OBJ_PRE(this);      // this memory pool
            QS_MPC_PRE(nFree);     // # free blocks in the pool
            QS_MPC_PRE(margin);    // the requested margin
        QS_END_PRE()
    }

    QF_CRIT_EXIT();

    return static_cast<void *>(pfb); // return the block or nullptr
}

//............................................................................
void QMPool::put(
    void * const block,
    std::uint_fast8_t const qsId) noexcept
{
#ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
#endif

    void * * const pfb = static_cast<void * *>(block); // ptr to free block

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the block returned to the pool must be valid
    Q_REQUIRE_INCRIT(400, pfb != nullptr);

    // the block must be in range of this pool (block from a different pool?)
    Q_REQUIRE_INCRIT(410, (m_start <= pfb) && (pfb <= m_end));

    // the block must NOT be in the pool already (double free?)
    // NOTE: a block in the pool already matches the duplicate storage,
    // so the block not in the pool must NOT match the Duplicate Storage
    Q_INVARIANT_INCRIT(422, pfb[0] != pfb[1]);

    // get members into temporaries
    void * * const freeHead = m_freeHead;
    QMPoolCtr nFree = m_nFree;

    // the number of free blocks must be below the total because
    // one more block is just being returned to the pool
    Q_REQUIRE_INCRIT(450, nFree < m_nTot);

    ++nFree; // one more free block in this pool

    m_freeHead = pfb; // set as new head of the free list
    m_nFree    = nFree;
    pfb[0]     = freeHead; // link into the list
#ifndef Q_UNSAFE
    pfb[1] = freeHead;  // update Duplicate Storage (NOT inverted)
#endif

    QS_BEGIN_PRE(QS_QF_MPOOL_PUT, qsId)
        QS_TIME_PRE();         // timestamp
        QS_OBJ_PRE(this);      // this memory pool
        QS_MPC_PRE(nFree);     // the # free blocks in the pool
    QS_END_PRE()

    QF_CRIT_EXIT();
}

//............................................................................
std::uint16_t QMPool::getUse() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return static_cast<std::uint16_t>(m_nTot - m_nFree);
}
//............................................................................
std::uint16_t QMPool::getFree() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return static_cast<std::uint16_t>(m_nFree);
}
//............................................................................
std::uint16_t QMPool::getMin() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return static_cast<std::uint16_t>(m_nMin);
}
//............................................................................
QMPoolSize QMPool::getBlockSize() const noexcept {
    // NOTE: this function does NOT apply critical section, so it can
    // be safely called from an already established critical section.
    return m_blockSize;
}

} // namespace QP

//============================================================================
// NOTE1:
// The memory buffer for the pool is organized as an array of void* pointers
// (see void * data type). These pointers are used to form a linked-list
// of free blocks in the pool. The first location pfb[0] is the actual link.
// The second location pfb[1] is used in SafeQP as the redundant Duplicate
// Storage (NOT inverted) for the link at pfb[0]. Therefore, the minimum
// number of void* pointers (void * data type) inside a memory block is 2.
