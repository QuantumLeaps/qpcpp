/// @file
/// @brief QP::QKMutex::init(), QP::QKMutex::lock(), and QP::QKMutex::unlock()
/// definitions.
/// @ingroup qk
/// @cond
///***************************************************************************
/// Product: QK/C++
/// Last updated for version 5.6.4
/// Last updated on  2016-05-04
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

#define QP_IMPL           // this is QF/QK implementation
#include "qf_port.h"      // QF port
#include "qk_pkg.h"       // QK package-scope internal interface
#include "qassert.h"      // QP assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef qk_h
    #error "Source file included in a project NOT based on the QK kernel"
#endif // qk_h

namespace QP {

Q_DEFINE_THIS_MODULE("qk_mutex")

enum {
    MUTEX_UNUSED = 0xFF
};

//****************************************************************************
/// @description
/// Initializes QK priority-ceiling mutex to the specified ceiling priority.
///
/// @param[in] prio        ceiling priority of the mutex
///
/// @note
/// A mutex must be initialized before it can be locked or unlocked.
///
/// @sa QP::QKMutex::lock(), QP::QKMutex::unlock()
///
/// @usage
/// The following example shows how to initialize, lock and unlock QK mutex:
/// @include qk_mux.cpp
///
void QKMutex::init(uint_fast8_t const prio) {
    m_lockPrio = prio;
    m_prevPrio = static_cast<uint_fast8_t>(MUTEX_UNUSED);
}

//****************************************************************************
/// @description
/// This function locks the QK mutex.
///
/// @note
/// A mutex must be initialized before it can be locked or unlocked.
///
/// @note
/// QP::QKMutex::lock() must be always followed by the corresponding
/// QP::QKMutex::unlock().
///
/// @sa QP::QKMutex::init(), QP::QKMutex::unlock()
///
/// @usage
/// The following example shows how to initialize, lock and unlock QK mutex:
/// @include qk_mux.cpp
///
void QKMutex::lock(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    /// @pre scheduler cannot be locked from the ISR context
    /// and the mutex must be unused
    Q_REQUIRE_ID(700, (!QK_ISR_CONTEXT_())
                 && (m_prevPrio == static_cast<uint_fast8_t>(MUTEX_UNUSED)));

    m_prevPrio = QK_lockPrio_;   // save the previous prio
    if (QK_lockPrio_ < m_lockPrio) { // raising the lock prio?
        QK_lockPrio_ = m_lockPrio;
    }

    QS_BEGIN_NOCRIT_(QS_SCHED_LOCK,
                     static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_(); // timestamp
        QS_2U8_(static_cast<uint8_t>(m_prevPrio), /* the previous lock prio */
                static_cast<uint8_t>(QK_lockPrio_)); // the new lock prio
    QS_END_NOCRIT_()

    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// This function unlocks the QK mutex.
///
/// @note
/// A mutex must be initialized before it can be locked or unlocked.
///
/// @note
/// QP::QKMutex::unlock() must always follow the corresponding
/// QP::QKMutex::lock().
///
/// @sa QP::QKMutex::init(), QP::QKMutex::lock()
///
/// @usage
/// The following example shows how to initialize, lock and unlock QK mutex:
/// @include qk_mux.cpp
///
void QKMutex::unlock(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    /// @pre scheduler cannot be unlocked from the ISR context
    /// and the mutex must not be unused
    Q_REQUIRE_ID(800, (!QK_ISR_CONTEXT_())
                 && (m_prevPrio != static_cast<uint_fast8_t>(MUTEX_UNUSED)));

    QS_BEGIN_NOCRIT_(QS_SCHED_UNLOCK,
                     static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_(); // timestamp
        QS_2U8_(static_cast<uint8_t>(m_prevPrio),/* the previouis lock prio */
                static_cast<uint8_t>(QK_lockPrio_)); // the new lock prio
    QS_END_NOCRIT_()

    uint_fast8_t p = m_prevPrio;
    m_prevPrio = static_cast<uint_fast8_t>(MUTEX_UNUSED);

    if (QK_lockPrio_ > p) {
        QK_lockPrio_ = p; // restore the previous lock prio
        p = QK_schedPrio_(); // find the highest-prio AO ready to run
        if (p != static_cast<uint_fast8_t>(0)) { // priority found?
            QK_sched_(p); // schedule any unlocked AOs
        }
    }
    QF_CRIT_EXIT_();
}

} // namespace QP

