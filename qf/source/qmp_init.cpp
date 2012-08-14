//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 19, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
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
#include "qf_pkg.h"
#include "qassert.h"

/// \file
/// \ingroup qf
/// \brief QMPool::init() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qmp_init")

//............................................................................
void QMPool::init(void * const poolSto, uint32_t const poolSize,
                  QMPoolSize const blockSize)
{
    // The memory block must be valid
    // and the poolSize must fit at least one free block
    // and the blockSize must not be too close to the top of the dynamic range
    Q_REQUIRE((poolSto != null_void)
              && (poolSize >= static_cast<uint32_t>(sizeof(QFreeBlock)))
              && ((blockSize + static_cast<QMPoolSize>(sizeof(QFreeBlock)))
                    > blockSize));

    m_free = poolSto;

                // round up the blockSize to fit an integer number of pointers
    m_blockSize = static_cast<QMPoolSize>(sizeof(QFreeBlock));//start with one
    uint32_t nblocks = static_cast<uint32_t>(1);//# free blocks in a mem block
    while (m_blockSize < blockSize) {
        m_blockSize += static_cast<QMPoolSize>(sizeof(QFreeBlock));
        ++nblocks;
    }

               // the whole pool buffer must fit at least one rounded-up block
    Q_ASSERT(poolSize >= static_cast<uint32_t>(m_blockSize));

                                // chain all blocks together in a free-list...
                                                     // don't chain last block
    uint32_t availSize = poolSize - static_cast<uint32_t>(m_blockSize);
    m_nTot = static_cast<QMPoolCtr>(1);    // one (the last) block in the pool
                                          //start at the head of the free list
    QFreeBlock *fb = static_cast<QFreeBlock *>(m_free);
    while (availSize >= static_cast<uint32_t>(m_blockSize)) {
        fb->m_next = &QF_PTR_AT_(fb, nblocks);          // setup the next link
        fb = fb->m_next;                              // advance to next block
        availSize -= static_cast<uint32_t>(m_blockSize);// less available size
        ++m_nTot;                     // increment the number of blocks so far
    }

    fb->m_next = static_cast<QFreeBlock *>(0); // the last link points to NULL
    m_nFree    = m_nTot;                                // all blocks are free
    m_nMin     = m_nTot;                  // the minimum number of free blocks
    m_start    = poolSto;               // the original start this pool buffer
    m_end      = fb;                            // the last block in this pool

    QS_CRIT_STAT_
    QS_BEGIN_(QS_QF_MPOOL_INIT, QS::mpObj_, m_start)
        QS_OBJ_(m_start);                   // the memory managed by this pool
        QS_MPC_(m_nTot);                         // the total number of blocks
    QS_END_()
}

QP_END_
