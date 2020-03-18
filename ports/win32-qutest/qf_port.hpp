/// @file
/// @brief QF/C++ port for QUTEST Unit Test, Win32 with GNU or Visual C++
/// @ingroup qutest
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-01-23
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

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// QUTEST event queue and thread types
#define QF_EQUEUE_TYPE QEQueue
//#define QF_OS_OBJECT_TYPE
//#define QF_THREAD_TYPE

// The maximum number of active objects in the application
#define QF_MAX_ACTIVE        64U

// The number of system clock tick rates
#define QF_MAX_TICK_RATE     2U

// Activate the QF QActive::stop() API
#define QF_ACTIVE_STOP       1

// QF interrupt disable/enable
#define QF_INT_DISABLE()     (++QP::QF_intNest)
#define QF_INT_ENABLE()      (--QP::QF_intNest)

// QF critical section
// QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(dummy) QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)  QF_INT_ENABLE()

// QF_LOG2 not defined -- use the internal LOG2() implementation

#include "qep_port.hpp"  // QEP port
#include "qequeue.hpp"   // QUTEST port uses QEQueue event-queue
#include "qmpool.hpp"    // QUTEST port uses QMPool memory-pool
#include "qf.hpp"        // QF platform-independent public interface

namespace QP {
// interrupt nesting up-down counter
extern uint8_t volatile QF_intNest;
}

// portable "safe" facilities from <stdio.h> and <string.h>
#ifdef _MSC_VER // Microsoft Visual C++

#if (_MSC_VER < 1900) // before Visual Studio 2015

#define snprintf _snprintf
#endif

#define SNPRINTF_S(buf_, len_, format_, ...) \
    _snprintf_s(buf_, len_, _TRUNCATE, format_, ##__VA_ARGS__)

#define STRNCPY_S(dest_, src_, len_) \
    strncpy_s(dest_, len_, src_, _TRUNCATE)

#define FOPEN_S(fp_, fName_, mode_) \
    if (fopen_s(&fp_, fName_, mode_) != 0) { \
        fp_ = static_cast<FILE *>(0); \
    } else ((void)0)

#define CTIME_S(buf_, len_, time_) \
    ctime_s((char *)buf_, len_, time_)

#define SSCANF_S(buf_, format_, ...) \
    sscanf_s(buf_, format_, ##__VA_ARGS__)

#else // other C/C++ compilers (GNU, etc.)

#define SNPRINTF_S(buf_, len_, format_, ...) \
    snprintf(buf_, len_, format_, ##__VA_ARGS__)

#define STRNCPY_S(dest_, src_, len_) strncpy(dest_, src_, len_)

#define FOPEN_S(fp_, fName_, mode_) \
    (fp_ = fopen(fName_, mode_))

#define CTIME_S(buf_, len_, time_) \
    strncpy(static_cast<char *>(buf_), ctime(time_), len_)

#define SSCANF_S(buf_, format_, ...) \
    sscanf(buf_, format_, ##__VA_ARGS__)

#endif // _MSC_VER

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL

    // QUTest scheduler locking (not used)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) ((void)0)
    #define QF_SCHED_UNLOCK_()    ((void)0)

    // native event queue operations
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT_ID(110, (me_)->m_eQueue.m_frontEvt != nullptr)
    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        (QS::rxPriv_.readySet.insert(   \
            static_cast<std::uint_fast8_t>((me_)->m_prio)))

    // native QF event pool operations
    #define QF_EPOOL_TYPE_  QMPool

    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))

    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_)     ((p_).put(e_))

    #include "qf_pkg.hpp" // internal QF interface

#endif // QP_IMPL

#endif // QF_PORT_HPP
