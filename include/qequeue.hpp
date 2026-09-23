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
#ifndef QEQUEUE_HPP_
#define QEQUEUE_HPP_

#ifndef QF_EQUEUE_CTR_SIZE
    #define QF_EQUEUE_CTR_SIZE 1U
#endif

namespace QP {

#if (QF_EQUEUE_CTR_SIZE == 1U)
    using QEQueueCtr = std::uint8_t;
#elif (QF_EQUEUE_CTR_SIZE == 2U)
    using QEQueueCtr = std::uint16_t;
#else
    #error QF_EQUEUE_CTR_SIZE defined incorrectly, expected 1U or 2U
#endif

// forward declarations (NOTE must be consistent with "qp.hpp")
class QEvt;

//----------------------------------------------------------------------------
// NOTE must be consistent with "qp.hpp"
struct QEvtPtr {
    QEvt const *e;
};

//----------------------------------------------------------------------------
class QEQueue {
public:
    QEQueue() noexcept;
    void init(
        QEvtPtr * const qSto,
        std::uint_fast16_t const qLen) noexcept;
    bool post(
        QEvt const * const e,
        std::uint_fast16_t const margin,
        std::uint_fast8_t const qsId) noexcept;
    void postLIFO(
        QEvt const * const e,
        std::uint_fast8_t const qsId) noexcept;
    QEvt const * get(std::uint_fast8_t const qsId) noexcept;
    std::uint16_t getFree() const noexcept;
    std::uint16_t getUse() const noexcept;
    std::uint16_t getMin() const noexcept;
    bool isEmpty() const noexcept;
    QEvt const *peekFront() const &;
    QEvt const *peekFront() &&
        // ref-qualified reference (MISRA-C++:2023 Rule 6.8.4)
        = delete;

private:
    QEvtPtr m_frontEvt;
    QEvtPtr * m_ring;
    QEQueueCtr m_end;
    QEQueueCtr m_head;
    QEQueueCtr m_tail;
    QEQueueCtr m_nFree;
    QEQueueCtr m_nMin;

    void postFIFO_(
        QEvt const * const e,
        void const * const sender);

    // friends...
    friend class QActive;
    friend class QTicker;
    friend class QXMutex;
    friend class QXThread;
    friend class QV;
    friend class QK;
    friend class QS;
}; // class QEQueue

} // namespace QP

#endif // QEQUEUE_HPP_
