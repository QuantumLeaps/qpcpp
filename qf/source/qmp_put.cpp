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
/// \brief QMPool::put() implementation.

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qmp_put")

// This macro is specifically and exclusively used for checking the range
// of a block pointer returned to the pool. Such a check must rely on the
// pointer arithmetic not compliant with the MISRA-C++:2008 rules ??? and
// ???. Defining a specific macro for this purpose allows to selectively
// disable the warnings for this particular case.
//
#define QF_PTR_RANGE_(x_, min_, max_)  (((min_) <= (x_)) && ((x_) <= (max_)))

//............................................................................
void QMPool::put(void * const b) {

    Q_REQUIRE(m_nFree < m_nTot);         // adding one free so, # free < total
    Q_REQUIRE(QF_PTR_RANGE_(b, m_start, m_end));         // b must be in range

    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

                                                    // link into the free list
    (static_cast<QFreeBlock*>(b))->m_next = static_cast<QFreeBlock *>(m_free);
    m_free = b;                            // set as new head of the free list
    ++m_nFree;                             // one more free block in this pool

    QS_BEGIN_NOCRIT_(QS_QF_MPOOL_PUT, QS::mpObj_, m_start)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(m_start);                   // the memory managed by this pool
        QS_MPC_(m_nFree);             // the number of free blocks in the pool
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
}

QP_END_
