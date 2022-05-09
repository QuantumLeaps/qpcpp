//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
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
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2021-12-23
//! @version Last updated for: @ref qpcpp_7_0_0
//!
//! @file
//! @brief QF/C++ port example, Generic C++11 compiler
//! @description
//! This is an example QP/C++ port with the documentation for the main
//! items, such as configuration macros, functions, and includes.

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

//lint -save -e1960    MISRA-C++:2008 Rule 7-3-1, Global declaration

//! The maximum number of active objects in the application.
//!
//! This macro *must* be defined in the QF port and should be in range
//! of 1U..63U, inclusive. The value of this macro determines the maximum
//! priority level of an active object in the system. Not all priority
//! levels must be used, but the maximum priority cannot exceed
//! #QF_MAX_ACTIVE.
//!
//! @note Once you choose a certain value of #QF_MAX_ACTIVE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MAX_ACTIVE              32U

//! The maximum number of clock tick rates in the application.
//! @description
//! This macro can be defined in the QF ports and should be in range
//! of 1U..15U, inclusive. The value of this macro determines the maximum
//! number of clock tick rates for time events (::QTimeEvt).
//!
//! If the macro is not defined, the default value is 1U.
//!
//! @note Once you choose a certain value of #QF_MAX_TICK_RATE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.h header file and henceforth include
//! this header file in all builds.
#define QF_MAX_TICK_RATE            1U

//! The maximum number of event pools in the application.
//!
//! This macro should be defined in the QF port and should be in range
//! of 1..255, inclusive. The value of this macro determines the maximum
//! number of event pools in the system. Not all all these event pools must
//! actually be used by the applications, but the maximum number of pools
//! cannot exceed #QF_MAX_EPOOL.
//!
//! If the macro is not defined, the default value is 3
//!
//! @note Once you choose a certain value of #QF_MAX_EPOOL, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MAX_EPOOL                3U

//! The size (in bytes) of the event-size representation in the QF.
//! Valid values: 1U, 2U, or 4U; default 2U
//!
//! @description
//! This macro can be defined in the QF ports to configure the QP::QEvtSize
//! type. If the macro is not defined, the default of 2 byte will be chosen in
//! qf.hpp. The valid #QF_EVENT_SIZ_SIZE values of 1, 2, or 4, correspond
//! to QP::QEvtSize of uint8_t, uint16_t, and uint32_t, respectively. The
//! QP::QEvtSize data type determines the dynamic range of event-sizes in
//! your application.
//! @sa QP::QF::poolInit(), QP::QF::new_()
//!
//! @note Once you choose a certain value of #QF_EVENT_SIZ_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_EVENT_SIZ_SIZE           2U

//! The size (in bytes) of the ring-buffer counters used in the native
//! QF event queue implementation. Valid values: 1U, 2U, or 4U; default 1U
//! @description
//! This macro can be defined in the QF ports to configure the QP::QEQueueCtr
//! type. If the macro is not defined, the default of 1 byte will be chosen
//! in qequeue.hpp. The valid #QF_EQUEUE_CTR_SIZE values of 1U, 2U, or 4U,
//! correspond to QP::QEQueueCtr of uint8_t, uint16_t, and uint32_t,
//! respectively. The QP::QEQueueCtr data type determines the dynamic range
//! of numerical values of ring-buffer counters inside event queues, or,
//! in other words, the maximum number of events that the native QF event
//! queue can manage.
//! @sa QP::QEQueue
//!
//! @note Once you choose a certain value of #QF_EQUEUE_CTR_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_EQUEUE_CTR_SIZE          1U

//! The size (in bytes) of the block-size representation in the
//! native QF event pool. Valid values: 1U, 2U, or 4U; default 2U.
//! #QF_EVENT_SIZ_SIZE.
//!@description
//! This macro can be defined in the QF ports to configure the QP::QMPoolSize
//! type. If the macro is not defined, the default of #QF_EVENT_SIZ_SIZE
//! will be chosen in qmpool.hpp, because the memory pool is primarily used for
//! implementing event pools.
//!
//! The valid #QF_MPOOL_SIZ_SIZE values of 1U, 2U, or 4U, correspond to
//! QP::QMPoolSize of uint8_t, uint16_t, and uint32_t, respectively. The
//! QP::QMPoolSize data type determines the dynamic range of block-sizes that
//! the native QP::QMPool can hanle.
//! @sa #QF_EVENT_SIZ_SIZE, QP::QMPool
//!
//! @note Once you choose a certain value of #QF_MPOOL_SIZ_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MPOOL_SIZ_SIZE           2U

//! The size (in bytes) of the block-counter representation in the
//! native QF event pool. Valid values: 1U, 2U, or 4U; default 2U.
//! @description
//! This macro can be defined in the QF ports to configure the QP::QMPoolCtr
//! type. If the macro is not defined, the default of 2 bytes will be chosen
//! in qmpool.hpp. The valid #QF_MPOOL_CTR_SIZE values of 1, 2, or 4, correspond
//! to QP::QMPoolSize of uint8_t, uint16_t, and uint32_t, respectively. The
//! QP::QMPoolCtr data type determines the dynamic range of block-counters that
//! the native QP::QMPool can handle, or, in other words, the maximum number
//! of blocks that the native QF event pool can manage.
//! @sa QP::QMPool
//!
//! @note Once you choose a certain value of #QF_MPOOL_CTR_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_MPOOL_CTR_SIZE           2U

//! The size (in bytes) of the time event -counter representation
//! in the QP::QTimeEvt class. Valid values: 1U, 2U, or 4U; default 2U.
//! @description
//! This macro can be defined in the QF ports to configure the internal tick
//! counters of Time Events. If the macro is not defined, the default of 2
//! bytes will be chosen in qf.hpp. The valid #QF_TIMEEVT_CTR_SIZE values of
//! 1U, 2U, or 4U, correspond to tick counters of uint8_t, uint16_t, and
//! uint32_t, respectively. The tick counter representation determines the
//! dynamic range of time delays that a Time Event can handle.
//! @sa QP::QTimeEvt
//!
//! @note Once you choose a certain value of #QF_TIMEEVT_CTR_SIZE, you must
//! consistently use the same value in building all the QP component libraries
//! and your own application code. The consistency is guaranteed if you define
//! this macro only once in the qf_port.hpp header file and henceforth include
//! this header file in all builds.
#define QF_TIMEEVT_CTR_SIZE         2U

//! Define the interrupt disabling policy.
//! @description
//! This macro encapsulates platform-specific way of disabling interrupts
//! from C++ for a given CPU and compiler.
//!
//! @note the #QF_INT_DISABLE macro should always be used in pair with the
//! macro #QF_INT_ENABLE.
//!
#define QF_INT_DISABLE()            intDisable()

//! Define the interrupt enabling policy.
//! @description
//! This macro encapsulates platform-specific way of enabling interrupts
//! from "C" for a given CPU and compiler.
//!
//! @note the #QF_INT_DISABLE macro should always be used in pair with the
//! macro #QF_INT_ENABLE.
//!
#define QF_INT_ENABLE()             intEnable()

//! Define the type of the critical section status.
//! @description
//! Defining this macro configures the "saving and restoring critical section
//! status" policy. Conversely, if this macro is not defined, the simple
//! "unconditional critical section exit" is used.
//!
#define QF_CRIT_STAT_TYPE           unsigned

//! Define the critical section entry policy.
//!
//! This macro enters a critical section (often by means of disabling
//! interrupts). When the "saving and restoring critical section status"
//! policy is used, the macro sets the \a status_ argument to the critical
//! section status just before the entry. When the policy of "unconditional
//! critical section exit" is used, the macro does not use the \a status_
//! argument.
//!
//! @note the #QF_CRIT_ENTRY macro should always be used in pair with the
//! macro #QF_CRIT_EXIT.
//!
#define QF_CRIT_ENTRY(stat_)        ((stat_) = critEntry())

//! Define the critical section exit policy.
//! @description
//! This macro enters a critical section (often by means of disabling
//! interrupts). When the "saving and restoring critical section status"
//! policy is used, the macro restores the critical section status from the
//! @a status_ argument. When the policy of "unconditional critical section
//! exit" is used, the macro does not use the \a status argument and
//! exits the critical section unconditionally (often by means of enabling
//! interrupts).
//!
//! @note the #QF_CRIT_ENTRY macro should always be used in pair with the
//! macro #QF_CRIT_EXIT.
//!
#define QF_CRIT_EXIT(stat_)         critExit(stat_)

//! Enable the QActive::stop() API in the QF port.
//!
//! @description
//! Defining this macro enables the QActive::stop() API in a given port.
//! This feature should be used with caution, as stopping and re-starting
//! active objects **cleanly** can be tricky.
//!
#define QF_ACTIVE_STOP

//lint -restore

#include "qep_port.hpp"   // QEP port
#include "qxk_port.hpp"   // QXK port
#include "qf.hpp"         // QF platform-independent public interface
#include "qxthread.hpp"   // QXK naked thread

extern "C" {

void intDisable(void);
void intEnable(void);

QF_CRIT_STAT_TYPE critEntry(void);
void critExit(QF_CRIT_STAT_TYPE stat);

} // extern "C"

#endif // QF_PORT_HPP
