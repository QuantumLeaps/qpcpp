//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++
// Last Updated for Version: 4.5.04
// Date of the Last Update:  Feb 09, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
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
#ifndef qf_pkg_h
#define qf_pkg_h

/// \file
/// \ingroup qf
/// \brief Internal (package scope) QF/C++ interface.

#include "qf_port.h"                                                // QF port

#ifndef qpset_h
    #include "qpset.h"                                    // for lookup tables
#endif

/// \brief helper macro to cast const away from an event pointer \a e_
#define QF_EVT_CONST_CAST_(e_) const_cast<QEvt *>(e_)

QP_BEGIN_
                                               // QF-specific critical section
#ifndef QF_CRIT_STAT_TYPE
    /// \brief This is an internal macro for defining the critical section
    /// status type.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// provides the definition of the critical section status variable.
    /// Otherwise this macro is empty.
    /// \sa #QF_CRIT_STAT_TYPE
    ///
    #define QF_CRIT_STAT_

    /// \brief This is an internal macro for entering a critical section.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_ENTRY passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_ENTRY is invoked with a dummy parameter.
    /// \sa #QF_CRIT_ENTRY
    ///
    #define QF_CRIT_ENTRY_()    QF_CRIT_ENTRY(dummy)

    /// \brief This is an internal macro for exiting a critical section.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_EXIT passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_EXIT is invoked with a dummy parameter.
    /// \sa #QF_CRIT_EXIT
    ///
    #define QF_CRIT_EXIT_()     QF_CRIT_EXIT(dummy)

#else
    #define QF_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QF_CRIT_ENTRY_()    QF_CRIT_ENTRY(critStat_)
    #define QF_CRIT_EXIT_()     QF_CRIT_EXIT(critStat_)
#endif

// package-scope objects -----------------------------------------------------
extern QTimeEvt *QF_timeEvtListHead_;  ///< head of linked list of time events
extern QF_EPOOL_TYPE_ QF_pool_[QF_MAX_EPOOL];        ///< allocate event pools
extern uint8_t QF_maxPool_;                  ///< # of initialized event pools
extern QSubscrList *QF_subscrList_;             ///< the subscriber list array
extern enum_t QF_maxSignal_;                 ///< the maximum published signal

//............................................................................
/// \brief Structure representing a free block in the Native QF Memory Pool
/// \sa ::QMPool
struct QFreeBlock {
    QFreeBlock *m_next;
};

//////////////////////////////////////////////////////////////////////////////
// internal helper inline functions

/// \brief access to the poolId_ of an event \a e
inline uint8_t QF_EVT_POOL_ID_(QEvt const * const e) { return e->poolId_; }

/// \brief access to the refCtr_ of an event \a e
inline uint8_t QF_EVT_REF_CTR_(QEvt const * const e) { return e->refCtr_; }

/// \brief increment the refCtr_ of an event \a e
inline void QF_EVT_REF_CTR_INC_(QEvt const * const e) {
    ++(QF_EVT_CONST_CAST_(e))->refCtr_;
}

/// \brief decrement the refCtr_ of an event \a e
inline void QF_EVT_REF_CTR_DEC_(QEvt const * const e) {
    --(QF_EVT_CONST_CAST_(e))->refCtr_;
}

//////////////////////////////////////////////////////////////////////////////
// internal frequently used srongly-typed constants

uint8_t  const u8_0 = static_cast<uint8_t>(0); ///< \brief constant (uint8_t)0
uint8_t  const u8_1 = static_cast<uint8_t>(1); ///< \brief constant (uint8_t)1
int8_t   const s8_0 = static_cast<int8_t>(0);  ///< \brief constant  (int8_t)0
int8_t   const s8_1 = static_cast<int8_t>(1);  ///< \brief constant  (int8_t)1
QTimeEvtCtr const tc_0 = static_cast<QTimeEvtCtr>(0);

void     * const null_void = static_cast<void *>(0);
QEvt const * const null_evt  = static_cast<QEvt const *>(0);
QTimeEvt * const null_tevt = static_cast<QTimeEvt *>(0);
QActive  * const null_act  = static_cast<QActive *>(0);

QP_END_

/// \brief access element at index \a i_ from the base pointer \a base_
#define QF_PTR_AT_(base_, i_) (base_[i_])

//////////////////////////////////////////////////////////////////////////////
#ifdef Q_SPY                                   // QS software tracing enabled?

    #include "qs_port.h"                                    // include QS port

    #if (QF_EQUEUE_CTR_SIZE == 1)

        /// \brief Internal QS macro to output an unformatted event queue
        /// counter data element
        /// \note the counter size depends on the macro #QF_EQUEUE_CTR_SIZE.
        #define QS_EQC_(ctr_)       QS::u8_(ctr_)
    #elif (QF_EQUEUE_CTR_SIZE == 2)
        #define QS_EQC_(ctr_)       QS::u16_(ctr_)
    #elif (QF_EQUEUE_CTR_SIZE == 4)
        #define QS_EQC_(ctr_)       QS::u32_(ctr_)
    #else
        #error "QF_EQUEUE_CTR_SIZE not defined"
    #endif


    #if (QF_EVENT_SIZ_SIZE == 1)

        /// \brief Internal QS macro to output an unformatted event size
        /// data element
        /// \note the event size depends on the macro #QF_EVENT_SIZ_SIZE.
        #define QS_EVS_(size_)      QS::u8_(size_)
    #elif (QF_EVENT_SIZ_SIZE == 2)
        #define QS_EVS_(size_)      QS::u16_(size_)
    #elif (QF_EVENT_SIZ_SIZE == 4)
        #define QS_EVS_(size_)      QS::u32_(size_)
    #endif


    #if (QF_MPOOL_SIZ_SIZE == 1)

        /// \brief Internal QS macro to output an unformatted memory pool
        /// block-size data element
        /// \note the block-size depends on the macro #QF_MPOOL_SIZ_SIZE.
        #define QS_MPS_(size_)      QS::u8_(size_)
    #elif (QF_MPOOL_SIZ_SIZE == 2)
        #define QS_MPS_(size_)      QS::u16_(size_)
    #elif (QF_MPOOL_SIZ_SIZE == 4)
        #define QS_MPS_(size_)      QS::u32_(size_)
    #endif

    #if (QF_MPOOL_CTR_SIZE == 1)

        /// \brief Internal QS macro to output an unformatted memory pool
        /// block-counter data element
        /// \note the counter size depends on the macro #QF_MPOOL_CTR_SIZE.
        #define QS_MPC_(ctr_)       QS::u8_(ctr_)
    #elif (QF_MPOOL_CTR_SIZE == 2)
        #define QS_MPC_(ctr_)       QS::u16_(ctr_)
    #elif (QF_MPOOL_CTR_SIZE == 4)
        #define QS_MPC_(ctr_)       QS::u32_(ctr_)
    #endif


    #if (QF_TIMEEVT_CTR_SIZE == 1)

        /// \brief Internal QS macro to output an unformatted time event
        /// tick-counter data element
        /// \note the counter size depends on the macro #QF_TIMEEVT_CTR_SIZE.
        #define QS_TEC_(ctr_)       QS::u8_(ctr_)
    #elif (QF_TIMEEVT_CTR_SIZE == 2)
        #define QS_TEC_(ctr_)       QS::u16_(ctr_)
    #elif (QF_TIMEEVT_CTR_SIZE == 4)
        #define QS_TEC_(ctr_)       QS::u32_(ctr_)
    #endif

#else

    #include "qs_dummy.h"                   // disable the QS software tracing

#endif                                                                // Q_SPY

#endif                                                             // qf_pkg_h
