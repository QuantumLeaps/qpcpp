/// @file
/// @brief QP::QXMutex::init(), QP::QXMutex::lock(), and QP::QXMutex::unlock()
/// definitions.
/// @ingroup qxk
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

//****************************************************************************
/// @description
/// Initialize the QXK priority ceiling mutex.
///
/// @param[in]     prioCeiling ceiling priotity of the mutex
///
/// @note The ceiling priority must be unused by any thread. The ceiling
/// priority must be higher than priority of any thread that uses the
/// protected resource.
///
/// @usage
/// @include qxk_mux.cpp
///
void QXMutex::init(uint_fast8_t const prioCeiling) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    /// @pre the celiling priority of the mutex must not be zero and cannot
    /// exceed the maximum #QF_MAX_ACTIVE. Also, the ceiling priority of the
    /// mutex must not be already in use. The priority must be __unique__.
    Q_REQUIRE_ID(100,
        (static_cast<uint_fast8_t>(0) < prioCeiling)
           && (prioCeiling <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
        && (QF::active_[prioCeiling] == static_cast<QMActive *>(0)));

    m_prioCeiling = prioCeiling;
    m_lockNest = static_cast<uint_fast8_t>(0);
    QF::bzero(&m_waitSet, static_cast<uint_fast16_t>(sizeof(m_waitSet)));

    // reserve the ceiling priority level for this mutex
    QF::active_[prioCeiling] = reinterpret_cast<QMActive *>(this);

    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// Lock the QXK priority ceiling mutex.
///
/// @note This function should be always paired with QXK_mutexUnlock(). The
/// code between QXK_mutexLock() and QXK_mutexUnlock() should be kept to the
/// minimum.
///
/// @usage
/// @include qxk_mux.cpp
///
void QXMutex::lock(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    if (QXK_attr_.curr != static_cast<void *>(0)) { // is QXK running?
        QMActive *act = static_cast<QMActive *>(QXK_attr_.curr);

        /// @pre QP::QXMutex::lock() must not be called from ISR level,
        /// the thread priority must not exceed the mutex priority ceiling
        /// and the mutex must be initialized.
        Q_REQUIRE_ID(200, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
            && (act->m_thread.m_startPrio
                <= static_cast<uint_fast8_t>(m_prioCeiling))
            && (QF::active_[m_prioCeiling] != static_cast<QMActive *>(0)));

        // is the mutex available?
        if (m_lockNest == static_cast<uint_fast8_t>(0)) {
            m_lockNest = static_cast<uint_fast8_t>(1);

            // the priority slot must be set to this mutex
            Q_ASSERT_ID(210, QF::active_[m_prioCeiling]
                             == reinterpret_cast<QMActive *>(this));

            // switch the priority of this thread to the ceiling priority
            QF::active_[m_prioCeiling] = act;
            act->m_prio = m_prioCeiling; // set to the ceiling

            QXK_attr_.readySet.remove(act->m_thread.m_startPrio);
            QXK_attr_.readySet.insert(act->m_prio);

            QS_BEGIN_NOCRIT_(QP::QS_QK_MUTEX_LOCK,
                             QP::QS::priv_.aoObjFilter, this)
                QS_TIME_();                    // timestamp
                QS_2U8_(static_cast<uint8_t>(act->m_thread.m_startPrio),
                        static_cast<uint8_t>(act->m_prio));
            QS_END_NOCRIT_()
        }
        // is the mutex locked by this thread already (nested locking)?
        else if (QF::active_[m_prioCeiling] == act) {
            ++m_lockNest;
        }
        // the mutex is locked by a different thread -- block
        else {
            // store the blocking object (this mutex)
            act->m_temp.obj = reinterpret_cast<QMState const *>(this);

            m_waitSet.insert(act->m_prio);
            QXK_attr_.readySet.remove(act->m_prio);

            // multitasking started?
            if (QXK_attr_.curr != static_cast<QMActive *>(0)) {
                QXK_sched_();
            }
        }
    }
    QF_CRIT_EXIT_();
}

//****************************************************************************
/// @description
/// Unlock the QXK priority ceiling mutex.
///
/// @note This function should be always paired with QXK_mutexLock(). The
/// code between QXK_mutexLock() and QXK_mutexUnlock() should be kept to the
/// minimum.
///
/// @usage
/// @include qxk_mux.cpp
///
void QXMutex::unlock(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    if (QXK_attr_.curr != static_cast<void *>(0)) { // is QXK running?
        QMActive *act = static_cast<QMActive *>(QXK_attr_.curr);

        /// @pre QXMutex_unlock() must not be called from ISR level
        /// and the mutex must be owned by this thread.
        Q_REQUIRE_ID(300, (!QXK_ISR_CONTEXT_()) /* don't call from an ISR! */
                          && (m_lockNest > static_cast<uint_fast8_t>(0))
                          && (QF::active_[m_prioCeiling] == act));

        // Unlocking the first nesting level?
        if (m_lockNest == static_cast<uint_fast8_t>(1)) {
            m_lockNest = static_cast<uint_fast8_t>(0);

            // free up the ceiling priority and put the mutex at this slot
            QXK_attr_.readySet.remove(m_prioCeiling);
            QF::active_[m_prioCeiling] = reinterpret_cast<QMActive *>(this);

            // restore the thread priority to the original
            QXK_attr_.readySet.insert(act->m_thread.m_startPrio);
            act->m_prio = act->m_thread.m_startPrio;

            QS_BEGIN_NOCRIT_(QP::QS_QK_MUTEX_UNLOCK,
                             QP::QS::priv_.aoObjFilter, this)
                QS_TIME_();                        // timestamp
                QS_2U8_(static_cast<uint8_t>(act->m_prio),   // original prio
                        static_cast<uint8_t>(m_prioCeiling));// curr ceiling
            QS_END_NOCRIT_()

            // are any threads waiting for the mutex?
            if (m_waitSet.notEmpty()) {
                // find the highest-priority waiting thread
                uint_fast8_t p = m_waitSet.findMax();

                Q_ASSERT_ID(310,
                            p <= static_cast<uint_fast8_t>(m_prioCeiling));

                act = QF::active_[p];

                // this thread is no longer waiting for the mutex
                m_waitSet.remove(p);

                // switch the priority of this thread to the ceiling prio
                QF::active_[m_prioCeiling] = act;
                act->m_prio = static_cast<uint_fast8_t>(m_prioCeiling);

                QXK_attr_.readySet.remove(p);
                QXK_attr_.readySet.insert(act->m_prio);

                QS_BEGIN_NOCRIT_(QP::QS_QK_MUTEX_LOCK,
                                 QP::QS::priv_.aoObjFilter, this)
                    QS_TIME_();  // timestamp
                    QS_2U8_(static_cast<uint8_t>(act->m_prio),// original prio
                        static_cast<uint8_t>(m_prioCeiling)); // curr ceiling
                QS_END_NOCRIT_()
            }

            // multitasking started?
            if (QXK_attr_.curr != static_cast<QMActive *>(0)) {
                QXK_sched_();
            }
        }
        // this thread is releasing a nested lock
        else {
            --m_lockNest;
        }
    }
    QF_CRIT_EXIT_();
}

} // namespace QP
