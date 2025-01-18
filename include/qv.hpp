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
#ifndef QV_HPP_
#define QV_HPP_

namespace QP {
namespace QV {

//============================================================================
class Attr {
public:
    QPSet readySet;
    std::uint_fast8_t schedCeil;
}; // class Attr

extern QV::Attr priv_;

void schedDisable(std::uint_fast8_t const ceiling);

void schedEnable();

void onIdle();

} // namespace QV
} // namespace QP

//============================================================================
// interface used only for internal implementation, but not in applications
#ifdef QP_IMPL

// scheduler locking for QV (not needed)...
#define QF_SCHED_STAT_
#define QF_SCHED_LOCK_(dummy) (static_cast<void>(0))
#define QF_SCHED_UNLOCK_()    (static_cast<void>(0))

// QActive event queue customization for QV...
#define QACTIVE_EQUEUE_WAIT_(me_) (static_cast<void>(0))
#define QACTIVE_EQUEUE_SIGNAL_(me_) \
    (QV::priv_.readySet.insert((me_)->m_prio))

// QF event pool customization for QV...
#define QF_EPOOL_TYPE_ QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (p_).init((poolSto_), (poolSize_), (evtSize_))
#define QF_EPOOL_EVENT_SIZE_(p_) ((p_).getBlockSize())
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
    ((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))
#define QF_EPOOL_PUT_(p_, e_, qsId_) ((p_).put((e_), (qsId_)))

#endif // QP_IMPL

#endif // QV_HPP_
