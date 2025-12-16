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
#ifndef QK_HPP_
#define QK_HPP_

//============================================================================
namespace QP {

using QSchedStatus = std::uint8_t;

//----------------------------------------------------------------------------
class QK {
public:
    QP::QPSet readySet;
    std::uint8_t actPrio;
    std::uint8_t nextPrio;
    std::uint8_t actThre;
    std::uint8_t lockCeil;
    std::uint8_t intNest;


    static QSchedStatus schedLock(std::uint8_t const ceiling) noexcept;
    static void schedUnlock(QSchedStatus const prevCeil) noexcept;
    static void onIdle();

    static std::uint_fast8_t sched_() noexcept;
    static std::uint_fast8_t sched_act_(
                QActive const* const act,
                std::uint_fast8_t const pthre_in) noexcept;
    static void activate_();

    static QK priv_;

}; // class QK

} // namespace QP

//============================================================================
// interface used only for internal implementation, but not in applications

#ifdef QP_IMPL
//! @cond INTERNAL

// scheduler locking for QK...
#define QF_SCHED_STAT_ QSchedStatus lockStat_;
#define QF_SCHED_LOCK_(ceil_) do { \
    if (QK_ISR_CONTEXT_()) { \
        lockStat_ = 0xFFU; \
    } else { \
        lockStat_ = QK::schedLock((ceil_)); \
    } \
} while (false)

#define QF_SCHED_UNLOCK_() do { \
    if (lockStat_ != 0xFFU) { \
        QK::schedUnlock(lockStat_); \
    } \
} while (false)

// QActive event queue customization for QK...
#define QACTIVE_EQUEUE_WAIT_(me_) (static_cast<void>(0))
#define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    QK::priv_.readySet.insert( \
        static_cast<std::uint_fast8_t>((me_)->m_prio)); \
    if (!QK_ISR_CONTEXT_()) { \
        if (QK::sched_() != 0U) { \
            QK::activate_(); \
        } \
    } \
} while (false)

// QMPool operations
#define QF_EPOOL_TYPE_  QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (p_).init((poolSto_), (poolSize_), (evtSize_))
#define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
    ((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))
#define QF_EPOOL_PUT_(p_, e_, qsId_) ((p_).put((e_), (qsId_)))
#define QF_EPOOL_USE_(ePool_)   ((ePool_)->getUse())
#define QF_EPOOL_FREE_(ePool_)  ((ePool_)->getFree())
#define QF_EPOOL_MIN_(ePool_)   ((ePool_)->getMin())

//! @endcond
#endif // QP_IMPL

#endif // QK_HPP_
