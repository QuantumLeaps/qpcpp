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
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#if (QF_MAX_EPOOL > 0U)     // mutable events configured?

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_dyn")
} // unnamed namespace

namespace QP {
namespace QF {

//............................................................................
void poolInit(
    void * const poolSto,
    std::uint_fast32_t const poolSize,
    std::uint_fast16_t const evtSize) noexcept
{
    std::uint_fast8_t const poolNum = priv_.maxPool_;

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the maximum of initialized pools so far must be in the configured range
    Q_REQUIRE_INCRIT(100, poolNum < QF_MAX_EPOOL);

    if (poolNum > 0U) { // any event pools already initialized?
        // the last initialized event pool must have event size smaller
        // than the one just being initialized
        // NOTE: QF event pools must be initialized in the increasing order
        // of their event sizes
        Q_REQUIRE_INCRIT(110,
            QF_EPOOL_EVENT_SIZE_(priv_.ePool_[poolNum - 1U]) < evtSize);
    }

    priv_.maxPool_ = static_cast<std::uint8_t>(poolNum + 1U); // one more pool

    QF_CRIT_EXIT();

    // perform the port-dependent initialization of the event-pool
    QF_EPOOL_INIT_(priv_.ePool_[poolNum], poolSto, poolSize, evtSize);

#ifdef Q_SPY
    // generate the QS object-dictionary entry for the initialized pool
    {
        std::array<std::uint8_t, 9U> obj_name = // initial event pool name
            { 'E', 'v', 't', 'P', 'o', 'o', 'l', '?', '\0' };
        // replace the "?" with the one-digit pool number (1-based)
        obj_name[7] = static_cast<std::uint8_t>(
            static_cast<std::uint8_t>('0') + priv_.maxPool_);
        QS::obj_dict_pre_(&priv_.ePool_[poolNum],
                          reinterpret_cast<char *>(&obj_name[0]));
    }
#endif // Q_SPY
}

//............................................................................
std::uint16_t poolGetMaxBlockSize() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    std::uint8_t const maxPool = priv_.maxPool_;

    // the maximum number of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(210, (0U < maxPool) && (maxPool <= QF_MAX_EPOOL));

    // set event size from the port-dependent operation
    std::uint16_t const maxSize =
        QF_EPOOL_EVENT_SIZE_(priv_.ePool_[maxPool - 1U]);
    QF_CRIT_EXIT();

    return maxSize;
}

//............................................................................
#ifdef QF_EPOOL_USE_

std::uint16_t getPoolUse(std::uint_fast8_t const poolNum) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    std::uint8_t const maxPool = priv_.maxPool_;

    // the maximum number of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(310, maxPool <= QF_MAX_EPOOL);

    // the queried poolNum must be one of the initialized pools or 0
    Q_REQUIRE_INCRIT(320, poolNum <= maxPool);
#endif
    std::uint16_t nUse = 0U;
    if (poolNum > 0U) { // event pool number provided?
        // set event pool use from the port-dependent operation
        nUse = QF_EPOOL_USE_(&priv_.ePool_[poolNum - 1U]);
    }
    else { // special case of poolNum==0
        // calculate the sum of used entries in all event pools
        for (std::uint_fast8_t pool = priv_.maxPool_; pool > 0U; --pool) {
            // add the event pool use from the port-dependent operation
            nUse += QF_EPOOL_USE_(&priv_.ePool_[pool - 1U]);
        }
    }

    QF_CRIT_EXIT();

    return nUse;
}
#endif // QF_EPOOL_USE_

//............................................................................
#ifdef QF_EPOOL_FREE_

std::uint16_t getPoolFree(std::uint_fast8_t const poolNum) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    std::uint8_t const maxPool = priv_.maxPool_;

    // the maximum count of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(410, maxPool <= QF_MAX_EPOOL);

    // the poolNum paramter must be in range
    Q_REQUIRE_INCRIT(420, (0U < poolNum) && (poolNum <= maxPool));
#endif
    std::uint16_t const nFree = QF_EPOOL_FREE_(&priv_.ePool_[poolNum - 1U]);

    QF_CRIT_EXIT();

    return nFree;
}
#endif // QF_EPOOL_FREE_

//............................................................................
#ifdef QF_EPOOL_MIN_

std::uint16_t getPoolMin(std::uint_fast8_t const poolNum) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

#ifndef Q_UNSAFE
    std::uint8_t const maxPool = priv_.maxPool_;

    // the maximum count of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(510, maxPool <= QF_MAX_EPOOL);

    // the poolNum paramter must be in range
    Q_REQUIRE_INCRIT(520, (0U < poolNum) && (poolNum <= maxPool));
#endif
    // call port-specific operation for the minimum of free blocks so far
    std::uint16_t const nMin = QF_EPOOL_MIN_(&priv_.ePool_[poolNum - 1U]);

    QF_CRIT_EXIT();

    return nMin;
}
#endif // QF_EPOOL_MIN_

//............................................................................
QEvt * newX_(
    std::uint_fast16_t const evtSize,
    std::uint_fast16_t const margin,
    QSignal const sig) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    std::uint8_t const maxPool = priv_.maxPool_;

    // the maximum count of initialized pools must be in configured range
    Q_REQUIRE_INCRIT(610, maxPool <= QF_MAX_EPOOL);

    // find the pool that fits the requested event size...
    std::uint8_t poolNum = 0U; // zero-based poolNum initially
    for (; poolNum < maxPool; ++poolNum) {
        // call port-specific operation for the event-size in a given pool
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(priv_.ePool_[poolNum])) {
            break; // event pool found
        }
    }

    // event pool must be found, which means that the reqeusted event size
    // fits in one of the initialized pools
    Q_ASSERT_INCRIT(620, poolNum < maxPool);

    ++poolNum; // convert to 1-based poolNum

    QF_CRIT_EXIT();

    // get event e (port-dependent)...
    QEvt *e;
#ifdef Q_SPY
    QF_EPOOL_GET_(priv_.ePool_[poolNum - 1U], e,
                  ((margin != NO_MARGIN) ? margin : 0U),
                  static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum);
#else
    QF_EPOOL_GET_(priv_.ePool_[poolNum - 1U], e,
                  ((margin != NO_MARGIN) ? margin : 0U), 0U);
#endif

    if (e != nullptr) { // was e allocated correctly?
        e->sig      = static_cast<QSignal>(sig); // set the signal
        e->poolNum_ = poolNum;
        e->refCtr_  = 0U; // reference count starts at 0

        QS_CRIT_ENTRY();
        QS_BEGIN_PRE(QS_QF_NEW,
                static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()
        QS_CRIT_EXIT();
    }
    else { // event was not allocated

        QF_CRIT_ENTRY();
        // This assertion means that the event allocation failed,
        // and this failure cannot be tolerated. The most frequent
        // reason is an event leak in the application.
        Q_ASSERT_INCRIT(630, margin != NO_MARGIN);

        QS_BEGIN_PRE(QS_QF_NEW_ATTEMPT,
                static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
            QS_TIME_PRE();        // timestamp
            QS_EVS_PRE(evtSize);  // the size of the event
            QS_SIG_PRE(sig);      // the signal of the event
        QS_END_PRE()

        QF_CRIT_EXIT();
    }

    // if the caller can't tolerate failed allocation (margin != QF_NO_MARGIN),
    // the returned event e is guaranteed to be valid (not NULL).
    return e;
}

//............................................................................
void gc(QEvt const * const e) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the collected event must be valid
    Q_REQUIRE_INCRIT(700, e != nullptr);

    std::uint8_t const poolNum = static_cast<std::uint8_t>(e->poolNum_);
    if (poolNum != 0U) { // is it a pool event (mutable)?

        if (e->refCtr_ > 1U) { // isn't this the last reference?

            QS_BEGIN_PRE(QS_QF_GC_ATTEMPT,
                    static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            QEvt_refCtr_dec_(e); // decrement the ref counter

            QF_CRIT_EXIT();
        }
        else { // this is the last reference to this event, recycle it
#ifndef Q_UNSAFE
            std::uint8_t const maxPool = priv_.maxPool_;

            // the maximum count of initialized pools must be in configured range
            Q_ASSERT_INCRIT(740, maxPool <= QF_MAX_EPOOL);

            // the event poolNum must be one one the initialized event pools
            Q_ASSERT_INCRIT(750, poolNum <= maxPool);
#endif
            QS_BEGIN_PRE(QS_QF_GC,
                    static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
                QS_TIME_PRE();       // timestamp
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_2U8_PRE(poolNum, e->refCtr_);
            QS_END_PRE()

            QF_CRIT_EXIT();

            // call port-specific operation to put the event to a given pool
            // NOTE: casting 'const' away is legit because 'e' is a pool event
#ifdef Q_SPY
            QF_EPOOL_PUT_(priv_.ePool_[poolNum - 1U],
                const_cast<QEvt*>(e),
                static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum);
#else
            QF_EPOOL_PUT_(priv_.ePool_[poolNum - 1U],
                const_cast<QEvt*>(e), 0U);
#endif
        }
    }
    else {
        QF_CRIT_EXIT();
    }
}

//............................................................................
QEvt const * newRef_(
    QEvt const * const e,
    QEvt const * const evtRef) noexcept
{
#ifdef Q_UNSAFE
    Q_UNUSED_PAR(evtRef);
#endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    // the referenced event must be valid
    Q_REQUIRE_INCRIT(800, e != nullptr);

    // the event reference count must not exceed the number of AOs
    // in the system plus each AO possibly holding one event reference
    Q_REQUIRE_INCRIT(820, e->refCtr_ < (2U * QF_MAX_ACTIVE));

    // the event ref must be valid
    Q_REQUIRE_INCRIT(830, evtRef == nullptr);

    std::uint_fast8_t const poolNum = e->poolNum_;

    // the referenced event must be a pool event (not an immutable event)
    Q_ASSERT_INCRIT(840, poolNum != 0U);

    QEvt_refCtr_inc_(e); // increments the ref counter

    QS_BEGIN_PRE(QS_QF_NEW_REF,
            static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of the event
        QS_2U8_PRE(poolNum, e->refCtr_);
    QS_END_PRE()

    QF_CRIT_EXIT();
    Q_UNUSED_PAR(poolNum); // might be unused

    return e;
}

//............................................................................
void deleteRef_(QEvt const * const evtRef) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();

    QEvt const * const e = evtRef;

    // the referenced event must be valid
    Q_REQUIRE_INCRIT(900, e != nullptr);

#ifdef Q_SPY
    std::uint8_t const poolNum = e->poolNum_;

    QS_BEGIN_PRE(QS_QF_DELETE_REF,
            static_cast<std::uint_fast8_t>(QS_ID_EP) + poolNum)
        QS_TIME_PRE();       // timestamp
        QS_SIG_PRE(e->sig);  // the signal of the event
        QS_2U8_PRE(poolNum, e->refCtr_);
    QS_END_PRE()
#endif // def Q_SPY

    QF_CRIT_EXIT();

#if (QF_MAX_EPOOL > 0U)
    gc(e); // recycle the referenced event
#endif
}

} // namespace QF
} // namespace QP

#endif // (QF_MAX_EPOOL > 0U) mutable events configured
