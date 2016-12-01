/// @file
/// @brief QP::QXMutex::init(), QP::QXMutex::lock(), and QP::QXMutex::unlock()
/// definitions.
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Product: QK/C++
/// Last updated for version 5.8.0
/// Last updated on  2016-11-19
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qxk_pkg.h"      // QXK package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef qxk_h
    #error "Source file included in a project NOT based on the QXK kernel"
#endif // qxk_h

namespace QP {

Q_DEFINE_THIS_MODULE("qxk_mutex")

enum {
    MUTEX_UNUSED = 0xFF
};

//****************************************************************************
/// @description
/// Initializes QXK priority-ceiling mutex to the specified ceiling priority.
///
/// @param[in] prio        ceiling priority of the mutex
///
/// @note
/// A mutex must be initialized before it can be locked or unlocked.
///
/// @sa QP::QXMutex::lock(), QP::QXMutex::unlock()
///
/// @usage
/// The following example shows how to initialize, lock and unlock QXK mutex:
/// @include qxk_mux.cpp
///
void QXMutex::init(uint_fast8_t const prio) {
    m_lockPrio = prio;
    m_prevPrio = static_cast<uint_fast8_t>(MUTEX_UNUSED);
}

//****************************************************************************
/// @description
/// This function locks the QXK mutex.
///
/// @note
/// A mutex must be initialized before it can be locked or unlocked.
///
/// @note
/// QP::QXMutex::lock() must be always followed by the corresponding
/// QP::QXMutex::unlock().
///
/// @attention
/// A thread holding a mutex __cannot block__.
///
/// @sa QP::QXMutex::init(), QP::QXMutex::unlock()
///
/// @usage
/// The following example shows how to initialize, lock and unlock QXK mutex:
/// @include qxk_mux.cpp
///
void QXMutex::lock(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    /// @pre scheduler cannot be locked from the ISR context
    /// and the mutex must be unused
    Q_REQUIRE_ID(700, (!QXK_ISR_CONTEXT_())
                 && (m_prevPrio == static_cast<uint_fast8_t>(MUTEX_UNUSED)));

    m_prevPrio   = QXK_attr_.lockPrio;   // save previous lock prio
    m_prevHolder = QXK_attr_.lockHolder; // save previous lock holder

    if (QXK_attr_.lockPrio < m_lockPrio) { // raising the lock prio?
        QXK_attr_.lockPrio = m_lockPrio;
    }
    QXK_attr_.lockHolder =
        (QXK_attr_.curr != static_cast<void *>(0))
        ? static_cast<QActive volatile *>(QXK_attr_.curr)->m_prio
        : static_cast<uint_fast8_t>(0);

    QS_BEGIN_NOCRIT_(QS_SCHED_LOCK,
        static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_(); // timestamp
        QS_2U8_(static_cast<uint8_t>(m_prevPrio), /* the previouis lock prio */
                static_cast<uint8_t>(QXK_attr_.lockPrio)); // new lock priority
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// This function unlocks the QXK mutex.
///
/// @note
/// A mutex must be initialized before it can be locked or unlocked.
///
/// @note
/// QP::QXMutex::unlock() must always follow the corresponding
/// QP::QXMutex::lock().
///
/// @sa QP::QXMutex::init(), QP::QXMutex::lock()
///
/// @usage
/// The following example shows how to initialize, lock and unlock QXK mutex:
/// @include qxk_mux.cpp
///
void QXMutex::unlock(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    /// @pre scheduler cannot be unlocked from the ISR context
    /// and the mutex must NOT be unused
    Q_REQUIRE_ID(800, (!QXK_ISR_CONTEXT_())
                 && (m_prevPrio != static_cast<uint_fast8_t>(MUTEX_UNUSED)));

    uint_fast8_t p = m_prevPrio; // the previouis lock prio
    m_prevPrio = static_cast<uint_fast8_t>(MUTEX_UNUSED);
    QXK_attr_.lockHolder = m_prevHolder; // restore previous lock holder

    QS_BEGIN_NOCRIT_(QS_SCHED_UNLOCK,
        static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_(); // timestamp
        QS_2U8_(static_cast<uint8_t>(p), /* the previouis lock prio */
                static_cast<uint8_t>(QXK_attr_.lockPrio)); // new lock prio
    QS_END_NOCRIT_()

    if (QXK_attr_.lockPrio > p) {
        QXK_attr_.lockPrio = p; // restore the previous lock prio
        // find the highest-prio thread ready to run
        if (QXK_sched_() != static_cast<uint_fast8_t>(0)) { // priority found?
            QXK_activate_(); // activate any unlocked basic threads
        }
    }
    QF_CRIT_EXIT_();
}

} // namespace QP
