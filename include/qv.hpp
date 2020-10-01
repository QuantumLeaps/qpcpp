/// @file
/// @brief QV/C++ platform-independent public interface.
/// @ingroup qv
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-15
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef QV_HPP
#define QV_HPP

#include "qequeue.hpp" // QV kernel uses the native QF event queue
#include "qmpool.hpp"  // QV kernel uses the native QF memory pool
#include "qpset.hpp"   // QV kernel uses the native QF priority set

//****************************************************************************
// QF configuration for QK

// QV event-queue used for AOs
#define QF_EQUEUE_TYPE       QEQueue


//****************************************************************************
namespace QP {

//! QV services.
/// @description
/// This class groups together QV services. It has only static members and
/// should not be instantiated.
///
// @note The QV ready set, etc. belong conceptually to the QV class (as static
/// class members). However, to avoid C++ potential name-mangling problems in
/// assembly language, these elements are defined outside of the QK class and
/// use the extern "C" linkage specification.
class QV {
public:

    //! QV idle callback (customized in BSPs for QK)
    /// @description
    /// QV::onIdle() must be called with interrupts DISABLED because
    /// the determination of the idle condition (no events in the
    /// queues) can change at any time by an interrupt posting events
    /// to a queue. QV::onIdle() MUST enable interrupts internally,
    /// perhaps at the same time as putting the CPU into a power-saving
    /// mode.
    ///
    /// @sa QP::QK::onIdle()
    static void onIdle(void);
};

} // namespace QP

//****************************************************************************
extern "C" {
    extern QP::QPSet QV_readySet_;  //!< ready set of AOs
} // extern "C"

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    // QV-specific scheduler locking (not needed in QV)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) (static_cast<void>(0))
    #define QF_SCHED_UNLOCK_()    (static_cast<void>(0))

    // QV-specific native event queue operations...
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT_ID(110, (me_)->m_eQueue.m_frontEvt != nullptr)
    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        (QV_readySet_.insert(static_cast<std::uint_fast8_t>((me_)->m_prio)))

    // QV-specific native QF event pool operations...
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qs_id_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qs_id_))))
    #define QF_EPOOL_PUT_(p_, e_, qs_id_) ((p_).put((e_), (qs_id_)))

#endif // QP_IMPL

#endif // QV_HPP

