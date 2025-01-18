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

class QEvt; // forward declaration

} // namespace QP

//============================================================================
namespace QP {

class QEQueue {
private:
    QEvt const * volatile m_frontEvt;
    QEvt const * * m_ring;
    QEQueueCtr m_end;
    QEQueueCtr volatile m_head;
    QEQueueCtr volatile m_tail;
    QEQueueCtr volatile m_nFree;
    QEQueueCtr m_nMin;

    // friends...
    friend class QActive;
    friend class QTicker;
    friend class QXMutex;
    friend class QXThread;

public:
    QEQueue() noexcept
      : m_frontEvt(nullptr),
        m_ring(nullptr),
        m_end(0U),
        m_head(0U),
        m_tail(0U),
        m_nFree(0U),
        m_nMin(0U)
    {}
    void init(
        QEvt const * * const qSto,
        std::uint_fast16_t const qLen) noexcept;
    bool post(
        QEvt const * const e,
        std::uint_fast16_t const margin,
        std::uint_fast8_t const qsId) noexcept;
    void postLIFO(
        QEvt const * const e,
        std::uint_fast8_t const qsId) noexcept;
    QEvt const * get(std::uint_fast8_t const qsId) noexcept;
    QEQueueCtr getNFree() const noexcept {
        return m_nFree;
    }
    QEQueueCtr getNMin() const noexcept {
        return m_nMin;
    }
    bool isEmpty() const noexcept {
        return m_frontEvt == nullptr;
    }

private:
    QEQueue(QEQueue const & other) = delete;
    QEQueue & operator=(QEQueue const & other) = delete;
    void postFIFO_(
        QEvt const * const e,
        void const * const sender);
}; // class QEQueue

} // namespace QP

#endif // QEQUEUE_HPP_
