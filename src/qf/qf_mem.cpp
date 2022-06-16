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
//! @brief QF/C++ memory management services

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

// unnamed namespace for local definitions with internal linkage
namespace {

Q_DEFINE_THIS_MODULE("qf_mem")

} // unnamed namespace

//============================================================================
namespace QP {

//............................................................................
//! @description
//! Default constructor of a fixed block-size memory pool.
//!
//! @note
//! The memory pool is __not__ ready to use directly after instantiation.
//! To become ready, the QP::QMPool::init() must be called to give the pool
//! memory, size of this memory, and the block size to manage.
//!
QMPool::QMPool(void)
  : m_start(nullptr),
    m_end(nullptr),
    m_free_head(nullptr),
    m_blockSize(0U),
    m_nTot(0U),
    m_nFree(0U),
    m_nMin(0U)
{}

//............................................................................
void QMPool::init(void * const poolSto, std::uint_fast32_t poolSize,
                  std::uint_fast16_t blockSize) noexcept
{
    //! @pre The memory block must be valid and
    //! the poolSize must fit at least one free block and
    //! the blockSize must not be too close to the top of the dynamic range
    Q_REQUIRE_ID(100, (poolSto != nullptr)
        && (poolSize >= static_cast<std::uint_fast32_t>(sizeof(QFreeBlock)))
        && (static_cast<std::uint_fast16_t>(blockSize + sizeof(QFreeBlock))
            > blockSize));

    m_free_head = poolSto;

    // round up the blockSize to fit an integer number of pointers...
    //start with one
    m_blockSize = static_cast<QMPoolSize>(sizeof(QFreeBlock));

    //# free blocks in a memory block
    std::uint_fast16_t nblocks = 1U;
    while (m_blockSize < static_cast<QMPoolSize>(blockSize)) {
        m_blockSize += static_cast<QMPoolSize>(sizeof(QFreeBlock));
        ++nblocks;
    }
    // use rounded-up value
    blockSize = static_cast<std::uint_fast16_t>(m_blockSize);

    // the whole pool buffer must fit at least one rounded-up block
    Q_ASSERT_ID(110, poolSize >= blockSize);

    // chain all blocks together in a free-list...

    // don't count the last block
    poolSize -= static_cast<std::uint_fast32_t>(blockSize);
    m_nTot = 1U; // one (the last) block in the pool

    // start at the head of the free list
    QFreeBlock *fb = static_cast<QFreeBlock *>(m_free_head);

    // chain all blocks together in a free-list...
    while (poolSize >= blockSize) {
        fb->m_next = &fb[nblocks]; // setup the next link
        fb = fb->m_next;  // advance to next block
        // reduce the available pool size
        poolSize -= static_cast<std::uint_fast32_t>(blockSize);
        ++m_nTot;         // increment the number of blocks so far
    }

    fb->m_next = nullptr; // the last link points to NULL
    m_nFree    = m_nTot;  // all blocks are free
    m_nMin     = m_nTot;  // the minimum number of free blocks
    m_start    = poolSto; // the original start this pool buffer
    m_end      = fb;      // the last block in this pool
}

//............................................................................
void *QMPool::get(std::uint_fast16_t const margin,
                  std::uint_fast8_t const qs_id) noexcept
{
    static_cast<void>(qs_id); // unused parameter, if Q_SPY not defined

    QF_CRIT_STAT_
    QF_CRIT_E_();

    // have the than margin?
    QFreeBlock *fb;
    if (m_nFree > static_cast<QMPoolCtr>(margin)) {
        fb = static_cast<QFreeBlock *>(m_free_head);  // get a free block

        // the pool has some free blocks, so a free block must be available
        Q_ASSERT_CRIT_(310, fb != nullptr);

        // put volatile to a temporary to avoid UB
        void * const fb_next = fb->m_next;

        // is the pool becoming empty?
        m_nFree = (m_nFree - 1U); // one free block less
        if (m_nFree == 0U) {
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_CRIT_(320, fb_next == nullptr);

            m_nMin = 0U;// remember that pool got empty
        }
        else {
            // pool is not empty, so the next free block must be in range
            //
            // NOTE: the next free block pointer can fall out of range
            // when the client code writes past the memory block, thus
            // corrupting the next block.
            Q_ASSERT_CRIT_(330, QF_PTR_RANGE_(fb_next, m_start, m_end));

            // is the number of free blocks the new minimum so far?
            if (m_nMin > m_nFree) {
                m_nMin = m_nFree; // remember the minimum so far
            }
        }

        m_free_head = fb_next; // set the head to the next free block

        QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_GET, qs_id)
            QS_TIME_PRE_();        // timestamp
            QS_OBJ_PRE_(this);     // this memory pool
            QS_MPC_PRE_(m_nFree);  // # of free blocks in the pool
            QS_MPC_PRE_(m_nMin);   // min # free blocks ever in the pool
        QS_END_NOCRIT_PRE_()
    }
    // don't have enough free blocks at this point
    else {
        fb = nullptr;

        QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_GET_ATTEMPT, qs_id)
            QS_TIME_PRE_();        // timestamp
            QS_OBJ_PRE_(m_start);  // the memory managed by this pool
            QS_MPC_PRE_(m_nFree);  // the # free blocks in the pool
            QS_MPC_PRE_(margin);   // the requested margin
        QS_END_NOCRIT_PRE_()
    }
    QF_CRIT_X_();

    return fb; // return the block or NULL pointer to the caller
}

//............................................................................
void QMPool::put(void * const b, std::uint_fast8_t const qs_id) noexcept {
    static_cast<void>(qs_id); // unused parameter, if Q_SPY not defined

    //! @pre # free blocks cannot exceed the total # blocks and
    //! the block pointer must be in range to come from this pool.
    //!
    Q_REQUIRE_ID(200, (m_nFree < m_nTot)
                      && QF_PTR_RANGE_(b, m_start, m_end));
    QF_CRIT_STAT_
    QF_CRIT_E_();
    static_cast<QFreeBlock*>(b)->m_next =
        static_cast<QFreeBlock *>(m_free_head); // link into the free list
    m_free_head = b; // set as new head of the free list
    m_nFree = (m_nFree + 1U); // one more free block in this pool

    QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_PUT, qs_id)
        QS_TIME_PRE_();       // timestamp
        QS_OBJ_PRE_(this);    // this memory pool
        QS_MPC_PRE_(m_nFree); // the number of free blocks in the pool
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();
}

//............................................................................
std::uint_fast16_t QF::getPoolMin(std::uint_fast8_t const poolId) noexcept {

    //! @pre the poolId must be in range
    Q_REQUIRE_ID(400, (QF_maxPool_ <= QF_MAX_EPOOL)
                       && (0U < poolId) && (poolId <= QF_maxPool_));
    QF_CRIT_STAT_
    QF_CRIT_E_();
    std::uint_fast16_t const min = static_cast<std::uint_fast16_t>(
        QF_pool_[poolId - 1U].m_nMin);
    QF_CRIT_X_();

    return min;
}

} // namespace QP
