/// @file
/// @brief QP::QMutex::init(), QP::QMutex::lock(), and QP::QMutex::unlock()
/// definitions.
/// @ingroup qk
/// @cond
///***************************************************************************
/// Product: QK/C++
/// Last updated for version 5.6.0
/// Last updated on  2015-12-30
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

//****************************************************************************
/// @description
/// Initialize the QK priority ceiling mutex.
///
/// @param[in]     prioCeiling ceiling priotity of the mutex
///
/// @note The ceiling priority must be unused by any AO. The ceiling
/// priority must be higher than priority of any AO that uses the
/// protected resource.
///
/// @usage
/// @include qk_mux.cpp
///
void QMutex::init(uint_fast8_t const prioCeiling) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    /// @pre the celiling priority of the mutex must not be zero and cannot
    /// exceed the maximum #QF_MAX_ACTIVE. Also, the ceiling priority must
    /// not be already in use. QF requires priority to be __unique__.
    ///
    Q_REQUIRE_ID(100, (static_cast<uint_fast8_t>(0) < prioCeiling)
                       && (prioCeiling <= (uint_fast8_t)QF_MAX_ACTIVE)
              && (QF::active_[prioCeiling] == static_cast<QMActive *>(0)));

    m_prioCeiling = prioCeiling;
    m_lockNest = static_cast<uint_fast8_t>(0);

    // reserve the ceiling priority level for this mutex
    QF::active_[prioCeiling] = reinterpret_cast<QMActive *>(this);

    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// Lock the QK priority ceiling mutex.
///
/// @note This function should be always paired with QXK_mutexUnlock(). The
/// code between QK_mutexLock() and QK_mutexUnlock() should be kept to the
/// minimum.
///
/// @usage
/// @include qk_mux.cpp
///
void QMutex::lock(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    // is the scheduler unloacked?
    if (QK_currPrio_ <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)) {
        QMActive *act = QF::active_[QK_currPrio_];

        /// @pre QMutex_lock() must not be called from ISR level,
        /// the thread priority must not exceed the mutex priority ceiling
        /// and the mutex must be initialized.
        ///
        Q_REQUIRE_ID(200, (!QK_ISR_CONTEXT_()) /* don't call from an ISR! */
            && (act->m_thread <= m_prioCeiling)
            && (QF::active_[m_prioCeiling] != static_cast<QMActive *>(0)));

        // is the mutex available?
        if (m_lockNest == static_cast<uint_fast8_t>(0)) {
            m_lockNest = static_cast<uint_fast8_t>(1);

            // the priority slot must be set to this mutex
            Q_ASSERT_ID(210, QF::active_[m_prioCeiling]
                             == reinterpret_cast<QMActive *>(this));

            // switch the priority of this thread to the ceiling priority
            QF::active_[m_prioCeiling] = act;
            // set to the ceiling
            act->m_prio = m_prioCeiling;

            if (QK_readySet_.hasElement(act->m_thread)) {
                QK_readySet_.remove(act->m_thread);
                QK_readySet_.insert(act->m_prio);
            }
            QK_currPrio_ = act->m_prio;

            QS_BEGIN_NOCRIT_(QS_QK_MUTEX_LOCK,
                             static_cast<void *>(0), static_cast<void *>(0))
                QS_TIME_(); // timestamp
                QS_2U8_(static_cast<uint8_t>(act->m_thread), // the start prio
                        static_cast<uint8_t>(act->m_prio)); // current ceiling
            QS_END_NOCRIT_()
        }
        // is the mutex locked by this thread already (nested locking)?
        else if (QF::active_[m_prioCeiling] == act) {
            ++m_lockNest;
        }
        // the mutex can't be locked by a different AO -- error
        else {
            Q_ERROR_ID(220);
        }
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// Unlock the QK priority ceiling mutex.
///
/// @param[in,out] me      pointer (see @ref oop)
///
/// @note This function should be always paired with QK_mutexLock(). The
/// code between QK_mutexLock() and QK_mutexUnlock() should be kept to the
/// minimum.
///
/// @usage
/// @include qk_mux.cpp
///
void QMutex::unlock(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    // is the scheduler unloacked?
    if (QK_currPrio_ <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)) {
        QMActive *act = QF::active_[QK_currPrio_];

        /// @pre QMutex_unlock() must not be called from ISR level
        /// and the mutex must be owned by this thread.
        ///
        Q_REQUIRE_ID(300, (!QK_ISR_CONTEXT_()) /* don't call from an ISR! */
                          && (m_lockNest > static_cast<uint_fast8_t>(0))
                          && (QF::active_[m_prioCeiling] == act));

        // Unlocking the first nesting level?
        if (m_lockNest == static_cast<uint_fast8_t>(1)) {
            uint_fast8_t p;

            m_lockNest = static_cast<uint_fast8_t>(0);

            // reclaim the ceiling priority for this mutex
            QF::active_[m_prioCeiling] = reinterpret_cast<QMActive *>(this);

            // restore the start priority for this AO...
            act->m_prio  = act->m_thread;
            QK_currPrio_ = act->m_thread;
            if (QK_readySet_.hasElement(m_prioCeiling)) {
                QK_readySet_.remove(m_prioCeiling);
                QK_readySet_.insert(act->m_thread);
            }

            QS_BEGIN_NOCRIT_(QS_QK_MUTEX_UNLOCK,
                             static_cast<void *>(0), static_cast<void *>(0))
                QS_TIME_();                      // timestamp
                QS_2U8_(static_cast<uint8_t>(act->m_thread), // the start prio
                        static_cast<uint8_t>(m_prioCeiling));// curr ceiling
            QS_END_NOCRIT_()

            // find if some other AO has higher priority than the current
            p = QK_schedPrio_();
            if (p != static_cast<uint_fast8_t>(0)) {
                QK_sched_(p);
            }
        }
        // this AO is releasing a nested mutex lock
        else {
            --m_lockNest;
        }
    }
    QF_CRIT_EXIT_();
}

} // namespace QP

