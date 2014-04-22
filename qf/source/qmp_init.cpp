/// \file
/// \brief QP::QMPool::QMPool() and QP::QMPool::init() definitions.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-10
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
/// \endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qmp_init")

//****************************************************************************
/// \description
/// Default constructor of a fixed block-size memory pool.
///
/// \note The memory pool is __not__ ready to use directly after instantiation.
/// To become ready, the QP::QMPool::init() must be called to give the pool
/// memory, size of this memory, and the block size to manage.
///
QMPool::QMPool(void)
  : m_start(null_void),
    m_end(null_void),
    m_free_head(null_void),
    m_blockSize(static_cast<QMPoolSize>(0)),
    m_nTot(static_cast<QMPoolCtr>(0)),
    m_nFree(static_cast<QMPoolCtr>(0)),
    m_nMin(static_cast<QMPoolCtr>(0))
{}

//****************************************************************************
/// \description
/// Initialize a fixed block-size memory pool by providing it with the pool
/// memory to manage, size of this memory, and the block size.
///
/// \arguments
/// \arg[in] \c poolSto  pointer to the memory buffer for pool storage
/// \arg[in] \c poolSize size of the storage buffer in bytes
/// \arg[in] \c blockSize fixed-size of the memory blocks in bytes
///
/// \attention
/// The caller of QP::QMPool::init() must make sure that the \c poolSto
/// pointer is properly __aligned__. In particular, it must be possible to
/// efficiently store a pointer at the location pointed to by \c poolSto.
/// Internally, the QP::QMPool::init() function rounds up the block size
/// \c blockSize so that it can fit an integer number of pointers.
/// This is done to achieve proper alignment of the blocks within the pool.
///
/// \note Due to the rounding of block size the actual capacity of the pool
/// might be less than (\c poolSize / \c blockSize). You can check the
/// capacity of the pool by calling the QP::QF::getPoolMin() function.
///
/// \note This function is __not__ protected by a critical section, because
/// it is intended to be called only during the initialization of the system,
/// when interrupts are not allowed yet.
///
/// \note Many QF ports use memory pools to implement the event pools.
///
void QMPool::init(void * const poolSto, uint_fast16_t poolSize,
                  uint_fast16_t blockSize)
{
    /// \pre The memory block must be valid and
    /// the poolSize must fit at least one free block and
    /// the blockSize must not be too close to the top of the dynamic range
    Q_REQUIRE_ID(100, (poolSto != null_void)
              && (poolSize >= static_cast<uint_fast16_t>(sizeof(QFreeBlock)))
              && ((blockSize + static_cast<uint_fast16_t>(sizeof(QFreeBlock)))
                    > blockSize));

    m_free_head = poolSto;

    // round up the blockSize to fit an integer number of pointers...
    m_blockSize = static_cast<QMPoolSize>(sizeof(QFreeBlock));//start with one
    uint_fast16_t nblocks = uf16_1; //# free blocks in a memory block
    while (m_blockSize < static_cast<QMPoolSize>(blockSize)) {
        m_blockSize += static_cast<QMPoolSize>(sizeof(QFreeBlock));
        ++nblocks;
    }
    // use rounded-up value
    blockSize = static_cast<uint_fast16_t>(m_blockSize);

    // the whole pool buffer must fit at least one rounded-up block
    Q_ASSERT_ID(110, poolSize >= blockSize);

    // chain all blocks together in a free-list...
    poolSize -= blockSize; // don't count the last block
    m_nTot = static_cast<QMPoolCtr>(1); // one (the last) block in the pool

    // start at the head of the free list
    QFreeBlock *fb = static_cast<QFreeBlock *>(m_free_head);

    // chain all blocks together in a free-list...
    while (poolSize >= blockSize) {
        fb->m_next = &QF_PTR_AT_(fb, nblocks); // setup the next link
        fb = fb->m_next;        // advance to next block
        poolSize -= blockSize;  // reduce the available pool size
        ++m_nTot;               // increment the number of blocks so far
    }

    fb->m_next = static_cast<QFreeBlock *>(0); // the last link points to NULL
    m_nFree    = m_nTot;   // all blocks are free
    m_nMin     = m_nTot;   // the minimum number of free blocks
    m_start    = poolSto;  // the original start this pool buffer
    m_end      = fb;       // the last block in this pool

    QS_CRIT_STAT_
    QS_BEGIN_(QS_QF_MPOOL_INIT, QS::priv_.mpObjFilter, m_start)
        QS_OBJ_(m_start);  // the memory managed by this pool
        QS_MPC_(m_nTot);   // the total number of blocks
    QS_END_()
}

} // namespace QP

