//$file${include::qk.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${include::qk.hpp}
//
// This code has been generated by QM 6.2.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2025-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${include::qk.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef QK_HPP_
#define QK_HPP_

//$declare${QK::QSchedStatus} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QK::QSchedStatus} ........................................................
using QSchedStatus = std::uint_fast8_t;

} // namespace QP
//$enddecl${QK::QSchedStatus} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$declare${QK::QK-base} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QK {

//${QK::QK-base::schedLock} ..................................................
QSchedStatus schedLock(std::uint_fast8_t const ceiling) noexcept;

//${QK::QK-base::schedUnlock} ................................................
void schedUnlock(QSchedStatus const prevCeil) noexcept;

//${QK::QK-base::onIdle} .....................................................
void onIdle();

} // namespace QK
} // namespace QP
//$enddecl${QK::QK-base} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

extern "C" {
//$declare${QK-extern-C} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${QK-extern-C::QK_Attr} ....................................................
class QK_Attr {
public:
    QP::QPSet readySet;
    std::uint_fast8_t actPrio;
    std::uint_fast8_t nextPrio;
    std::uint_fast8_t actThre;
    std::uint_fast8_t lockCeil;
    std::uint_fast8_t intNest;

#ifndef Q_UNSAFE
    QP::QPSet readySet_dis;
#endif // ndef Q_UNSAFE

#ifndef Q_UNSAFE
    std::uint_fast8_t actPrio_dis;
#endif // ndef Q_UNSAFE

#ifndef Q_UNSAFE
    std::uint_fast8_t nextPrio_dis;
#endif // ndef Q_UNSAFE

#ifndef Q_UNSAFE
    std::uint_fast8_t actThre_dis;
#endif // ndef Q_UNSAFE

#ifndef Q_UNSAFE
    std::uint_fast8_t lockCeil_dis;
#endif // ndef Q_UNSAFE
}; // class QK_Attr

//${QK-extern-C::QK_priv_} ...................................................
extern QK_Attr QK_priv_;

//${QK-extern-C::QK_sched_} ..................................................
std::uint_fast8_t QK_sched_() noexcept;

//${QK-extern-C::QK_activate_} ...............................................
void QK_activate_() noexcept;
//$enddecl${QK-extern-C} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
} // extern "C"

//============================================================================
// interface used only for internal implementation, but not in applications
#ifdef QP_IMPL

//$declare${QK-impl} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${QK-impl::QF_SCHED_STAT_} .................................................
#define QF_SCHED_STAT_ QSchedStatus lockStat_;

//${QK-impl::QF_SCHED_LOCK_} .................................................
#define QF_SCHED_LOCK_(ceil_) do { \
    if (QK_ISR_CONTEXT_()) { \
        lockStat_ = 0xFFU; \
    } else { \
        lockStat_ = QK::schedLock((ceil_)); \
    } \
} while (false)

//${QK-impl::QF_SCHED_UNLOCK_} ...............................................
#define QF_SCHED_UNLOCK_() do { \
    if (lockStat_ != 0xFFU) { \
        QK::schedUnlock(lockStat_); \
    } \
} while (false)

//${QK-impl::QACTIVE_EQUEUE_WAIT_} ...........................................
#define QACTIVE_EQUEUE_WAIT_(me_) \
    Q_ASSERT_INCRIT(320, (me_)->m_eQueue.m_frontEvt != nullptr)

//${QK-impl::QACTIVE_EQUEUE_SIGNAL_} .........................................
#ifndef Q_UNSAFE
#define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    QK_priv_.readySet.insert( \
        static_cast<std::uint_fast8_t>((me_)->m_prio)); \
    QK_priv_.readySet.update_(&QK_priv_.readySet_dis); \
    if (!QK_ISR_CONTEXT_()) { \
        if (QK_sched_() != 0U) { \
            QK_activate_(); \
        } \
    } \
} while (false)
#endif // ndef Q_UNSAFE

//${QK-impl::QACTIVE_EQUEUE_SIGNAL_} .........................................
#ifdef Q_UNSAFE
#define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    QK_priv_.readySet.insert( \
        static_cast<std::uint_fast8_t>((me_)->m_prio)); \
    if (!QK_ISR_CONTEXT_()) { \
        if (QK_sched_() != 0U) { \
            QK_activate_(); \
        } \
    } \
} while (false)
#endif // def Q_UNSAFE
//$enddecl${QK-impl} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$declare${QF_EPOOL-impl} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${QF_EPOOL-impl::QF_EPOOL_TYPE_} ...........................................
#define QF_EPOOL_TYPE_ QMPool

//${QF_EPOOL-impl::QF_EPOOL_INIT_} ...........................................
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
    (p_).init((poolSto_), (poolSize_), (evtSize_))

//${QF_EPOOL-impl::QF_EPOOL_EVENT_SIZE_} .....................................
#define QF_EPOOL_EVENT_SIZE_(p_) ((p_).getBlockSize())

//${QF_EPOOL-impl::QF_EPOOL_GET_} ............................................
#define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
    ((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))

//${QF_EPOOL-impl::QF_EPOOL_PUT_} ............................................
#define QF_EPOOL_PUT_(p_, e_, qsId_) ((p_).put((e_), (qsId_)))
//$enddecl${QF_EPOOL-impl} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif // QP_IMPL

#endif // QK_HPP_
