//****************************************************************************
// Product: QF/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 02, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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
//****************************************************************************
#include "qf_pkg.h"
#include "qassert.h"

/// \file
/// \ingroup qf
/// \brief QMPool::get() and QF::getPoolMargin() implementation.

namespace QP {

Q_DEFINE_THIS_MODULE("qmp_get")

//............................................................................
void *QMPool::get(uint_t const margin) {
    QFreeBlock *fb;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    if (m_nFree > static_cast<QMPoolCtr>(margin)) {   // have the than margin?
        fb = static_cast<QFreeBlock *>(m_free_head);       // get a free block
        Q_ASSERT(fb != static_cast<QFreeBlock *>(0));     // must be available
        m_free_head = fb->m_next;   // adjust list head to the next free block
        --m_nFree;                                      // one free block less

        if (m_nMin > m_nFree) {
            m_nMin = m_nFree;                   // remember the minimum so far
        }

        QS_BEGIN_NOCRIT_(QS_QF_MPOOL_GET, QS::priv_.mpObjFilter, m_start)
            QS_TIME_();                                           // timestamp
            QS_OBJ_(m_start);               // the memory managed by this pool
            QS_MPC_(m_nFree);         // the number of free blocks in the pool
            QS_MPC_(m_nMin); // the mninimum number of free blocks in the pool
        QS_END_NOCRIT_()
    }
    else {
        fb = static_cast<QFreeBlock *>(0);

        QS_BEGIN_NOCRIT_(QS_QF_MPOOL_GET_ATTEMPT,
                         QS::priv_.mpObjFilter, m_start)
            QS_TIME_();                                           // timestamp
            QS_OBJ_(m_start);               // the memory managed by this pool
            QS_MPC_(m_nFree);         // the number of free blocks in the pool
            QS_MPC_(static_cast<QMPoolCtr>(margin));   // the requested margin
        QS_END_NOCRIT_()
    }
    QF_CRIT_EXIT_();

    return fb;               // return the block or NULL pointer to the caller
}
//............................................................................
uint_t QF::getPoolMin(uint_t const poolId) {
    Q_REQUIRE((u_1 <= poolId) && (poolId <= QF_maxPool_));

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    uint_t min = static_cast<uint_t>(QF_pool_[poolId - u_1].m_nMin);
    QF_CRIT_EXIT_();

    return min;
}

}                                                              // namespace QP

