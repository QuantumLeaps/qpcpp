/// @file
/// @brief QXK/C++ preemptive kernel counting semaphore implementation
/// @ingroup qxk
/// @cond
////**************************************************************************
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
////**************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qxk_pkg.h"      // QXK package-scope internal interface
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

Q_DEFINE_THIS_MODULE("qxk_sema")

//****************************************************************************
void QXSemaphore::init(uint_fast16_t const count) {
    m_count = count;
    QF::bzero(&m_waitSet, static_cast<uint_fast16_t>(sizeof(m_waitSet)));
}

//****************************************************************************
bool QXSemaphore::wait(uint_fast16_t const nTicks,
                       uint_fast8_t const tickRate)
{
    QF_CRIT_STAT_


    QF_CRIT_ENTRY_();
    QXThread *thr = static_cast<QXThread *>(QXK_attr_.curr);

    Q_REQUIRE_ID(100, (!QXK_ISR_CONTEXT_()) /* can't block inside an ISR */
        /* this must be a "naked" thread (no state) */
        && (thr->m_state.act == static_cast<QActionHandler>(0)));

    if (m_count > static_cast<uint_fast16_t>(0)) {
        --m_count;
    }
    else {
        thr->m_temp.obj = reinterpret_cast<QMState const *>(this);
        thr->teArm_(static_cast<QSignal>(QXK_SEMA_SIG), nTicks, tickRate);
        m_waitSet.insert(thr->m_prio);
        QXK_attr_.readySet.remove(thr->m_prio);
        QXK_sched_();
    }
    QF_CRIT_EXIT_();

    // signal of non-zero means that the time event has not expired
    return (thr->m_timeEvt.sig != static_cast<QSignal>(0));
}

//****************************************************************************
void QXSemaphore::signal(void) {
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();
    if (m_waitSet.notEmpty()) {
        uint_fast8_t p = m_waitSet.findMax();
        QXK_attr_.readySet.insert(p);
        m_waitSet.remove(p);

        (void)static_cast<QXThread *>(QF::active_[p])->teDisarm_();

        // the waitSet is not empty, so the semaphore count must be zero
        Q_ASSERT_ID(910, m_count == static_cast<uint_fast16_t>(0));

        // not inside ISR? Multitasking started?
        if ((!QXK_ISR_CONTEXT_())
            && (QXK_attr_.curr != static_cast<void *>(0)))
        {
            QXK_sched_();
        }
    }
    else {
        ++m_count;
    }
    QF_CRIT_EXIT_();
}

} // namespace QP

