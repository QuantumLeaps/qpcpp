/// @file
/// @brief QF/C++ memory management services
/// @cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-04-29
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

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"       // QF package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY


namespace QP {

Q_DEFINE_THIS_MODULE("qf_mem")

//****************************************************************************
/// @description
/// Default constructor of a fixed block-size memory pool.
///
/// @note The memory pool is __not__ ready to use directly after instantiation.
/// To become ready, the QP::QMPool::init() must be called to give the pool
/// memory, size of this memory, and the block size to manage.
///
QMPool::QMPool(void)
  : m_start(static_cast<void *>(0)),
    m_end(static_cast<void *>(0)),
    m_free_head(static_cast<void *>(0)),
    m_blockSize(static_cast<QMPoolSize>(0)),
    m_nTot(static_cast<QMPoolCtr>(0)),
    m_nFree(static_cast<QMPoolCtr>(0)),
    m_nMin(static_cast<QMPoolCtr>(0))
{}

//****************************************************************************
/// @description
/// Initialize a fixed block-size memory pool by providing it with the pool
/// memory to manage, size of this memory, and the block size.
///
/// @param[in] poolSto  pointer to the memory buffer for pool storage
/// @param[in] poolSize size of the storage buffer in bytes
/// @param[in] blockSize fixed-size of the memory blocks in bytes
///
/// @attention
/// The caller of QP::QMPool::init() must make sure that the @p poolSto
/// pointer is properly __aligned__. In particular, it must be possible to
/// efficiently store a pointer at the location pointed to by @p poolSto.
/// Internally, the QP::QMPool::init() function rounds up the block size
/// @p blockSize so that it can fit an integer number of pointers.
/// This is done to achieve proper alignment of the blocks within the pool.
///
/// @note Due to the rounding of block size the actual capacity of the pool
/// might be less than (@p poolSize / @p blockSize). You can check the
/// capacity of the pool by calling the QP::QF::getPoolMin() function.
///
/// @note This function is __not__ protected by a critical section, because
/// it is intended to be called only during the initialization of the system,
/// when interrupts are not allowed yet.
///
/// @note Many QF ports use memory pools to implement the event pools.
///
void QMPool::init(void * const poolSto, uint_fast32_t poolSize,
                  uint_fast16_t blockSize)
{
    /// @pre The memory block must be valid and
    /// the poolSize must fit at least one free block and
    /// the blockSize must not be too close to the top of the dynamic range
    Q_REQUIRE_ID(100, (poolSto != static_cast<void *>(0))
        && (poolSize >= static_cast<uint_fast32_t>(sizeof(QFreeBlock)))
        && (static_cast<uint_fast16_t>(
               blockSize + static_cast<uint_fast16_t>(sizeof(QFreeBlock)))
            > blockSize));

    m_free_head = poolSto;

    // round up the blockSize to fit an integer number of pointers...
    //start with one
    m_blockSize = static_cast<QMPoolSize>(sizeof(QFreeBlock));

    //# free blocks in a memory block
    uint_fast16_t nblocks = static_cast<uint_fast16_t>(1);
    while (m_blockSize < static_cast<QMPoolSize>(blockSize)) {
        m_blockSize += static_cast<QMPoolSize>(sizeof(QFreeBlock));
        ++nblocks;
    }
    // use rounded-up value
    blockSize = static_cast<uint_fast16_t>(m_blockSize);

    // the whole pool buffer must fit at least one rounded-up block
    Q_ASSERT_ID(110, poolSize >= static_cast<uint_fast32_t>(blockSize));

    // chain all blocks together in a free-list...

    // don't count the last block
    poolSize -= static_cast<uint_fast32_t>(blockSize);
    m_nTot = static_cast<QMPoolCtr>(1); // one (the last) block in the pool

    // start at the head of the free list
    QFreeBlock *fb = static_cast<QFreeBlock *>(m_free_head);

    // chain all blocks together in a free-list...
    while (poolSize >= static_cast<uint_fast32_t>(blockSize)) {
        fb->m_next = &QF_PTR_AT_(fb, nblocks); // setup the next link
        fb = fb->m_next;  // advance to next block
        // reduce the available pool size
        poolSize -= static_cast<uint_fast32_t>(blockSize);
        ++m_nTot;         // increment the number of blocks so far
    }

    fb->m_next = static_cast<QFreeBlock *>(0); // the last link points to NULL
    m_nFree    = m_nTot;  // all blocks are free
    m_nMin     = m_nTot;  // the minimum number of free blocks
    m_start    = poolSto; // the original start this pool buffer
    m_end      = fb;      // the last block in this pool

    QS_CRIT_STAT_
    QS_BEGIN_(QS_QF_MPOOL_INIT, QS::priv_.mpObjFilter, m_start)
        QS_OBJ_(m_start);  // the memory managed by this pool
        QS_MPC_(m_nTot);   // the total number of blocks
    QS_END_()
}

//****************************************************************************
/// @description
/// Recycle a memory block to the fixed block-size memory pool.
///
/// @param[in]  b  pointer to the memory block that is being recycled
///
/// @attention
/// The recycled block must be allocated from the __same__ memory pool
/// to which it is returned.
///
/// @note This function can be called from any task level or ISR level.
///
/// @sa QP::QMPool::get()
///
void QMPool::put(void * const b) {

    /// @pre # free blocks cannot exceed the total # blocks and
    /// the block pointer must be in range to come from this pool.
    ///
    Q_REQUIRE_ID(200, (m_nFree < m_nTot)
                      && QF_PTR_RANGE_(b, m_start, m_end));
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    static_cast<QFreeBlock*>(b)->m_next =
        static_cast<QFreeBlock *>(m_free_head); // link into the free list
    m_free_head = b; // set as new head of the free list
    ++m_nFree;       // one more free block in this pool

    QS_BEGIN_NOCRIT_(QS_QF_MPOOL_PUT, QS::priv_.mpObjFilter, m_start)
        QS_TIME_();       // timestamp
        QS_OBJ_(m_start); // the memory managed by this pool
        QS_MPC_(m_nFree); // the number of free blocks in the pool
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// The function allocates a memory block from the pool and returns a pointer
/// to the block back to the caller.
///
/// @param[in] margin  the minimum number of unused blocks still available
///                    in the pool after the allocation.
///
/// @note This function can be called from any task level or ISR level.
///
/// @note The memory pool must be initialized before any events can
/// be requested from it. Also, the QP::QMPool::get() function uses internally
/// a QF critical section, so you should be careful not to call it from within
/// a critical section when nesting of critical section is not supported.
///
/// @attention
/// An allocated block must be later returned back to the same pool
/// from which it has been allocated.
///
/// @sa QP::QMPool::put()
///
void *QMPool::get(uint_fast16_t const margin) {
    QFreeBlock *fb;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    // have the than margin?
    if (m_nFree > static_cast<QMPoolCtr>(margin)) {
        fb = static_cast<QFreeBlock *>(m_free_head);  // get a free block

        // the pool has some free blocks, so a free block must be available
        Q_ASSERT_ID(310, fb != static_cast<QFreeBlock *>(0));

        void *fb_next = fb->m_next; // put volatile to a temporary to avoid UB

        // is the pool becoming empty?
        --m_nFree;  // one free block less
        if (m_nFree == static_cast<QMPoolCtr>(0)) {
            // pool is becoming empty, so the next free block must be NULL
            Q_ASSERT_ID(320, fb_next == static_cast<QFreeBlock *>(0));

            m_nMin = static_cast<QMPoolCtr>(0);// remember that pool got empty
        }
        else {
            // pool is not empty, so the next free block must be in range
            //
            // NOTE: the next free block pointer can fall out of range
            // when the client code writes past the memory block, thus
            // corrupting the next block.
            Q_ASSERT_ID(330, QF_PTR_RANGE_(fb_next, m_start, m_end));

            // is the number of free blocks the new minimum so far?
            if (m_nMin > m_nFree) {
                m_nMin = m_nFree; // remember the minimum so far
            }
        }

        m_free_head = fb_next; // adjust list head to the next free block

        QS_BEGIN_NOCRIT_(QS_QF_MPOOL_GET, QS::priv_.mpObjFilter, m_start)
            QS_TIME_();        // timestamp
            QS_OBJ_(m_start);  // the memory managed by this pool
            QS_MPC_(m_nFree);  // the number of free blocks in the pool
            QS_MPC_(m_nMin);   // the mninimum # free blocks in the pool
        QS_END_NOCRIT_()
    }
    else {
        fb = static_cast<QFreeBlock *>(0);

        QS_BEGIN_NOCRIT_(QS_QF_MPOOL_GET_ATTEMPT,
                         QS::priv_.mpObjFilter, m_start)
            QS_TIME_();        // timestamp
            QS_OBJ_(m_start);  // the memory managed by this pool
            QS_MPC_(m_nFree);  // the # free blocks in the pool
            QS_MPC_(margin);   // the requested margin
        QS_END_NOCRIT_()
    }
    QF_CRIT_EXIT_();

    return fb; // return the block or NULL pointer to the caller
}

//****************************************************************************
/// @description
/// This function obtains the minimum number of free blocks in the given
/// event pool since this pool has been initialized by a call to
/// QP::QF::poolInit().
///
/// @param[in] poolId  event pool ID in the range 1..QF_maxPool_, where
///                    QF_maxPool_ is the number of event pools initialized
///                    with the function QP::QF::poolInit().
///
/// @returns the minimum number of unused blocks in the given event pool.
///
uint_fast16_t QF::getPoolMin(uint_fast8_t const poolId) {

    /// @pre the poolId must be in range
    Q_REQUIRE_ID(400, (static_cast<uint_fast8_t>(1) <= poolId)
                       && (poolId <= QF_maxPool_));

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    uint_fast16_t min = static_cast<uint_fast16_t>(
        QF_pool_[poolId - static_cast<uint_fast8_t>(1)].m_nMin);
    QF_CRIT_EXIT_();

    return min;
}

} // namespace QP

