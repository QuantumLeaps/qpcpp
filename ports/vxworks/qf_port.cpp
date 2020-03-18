/// @file
/// @brief QF/C++ port to VxWorks API
/// @ingroup ports
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
///
#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"
#include "qassert.h"
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#include "tickLib.h"        // for tickAnnounce()

// locals ====================================================================
extern "C" {
    static int task_function(
        int a1,
        int a2,
        int a3,
        int a4,
        int a5,
        int a6,
        int a7,
        int a8,
        int a9,
        int a10);

    static void usrClockHook(TASK_ID tid);
}

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

#ifdef Q_SPY
    static constexpr std::uint8_t l_clock_tick = 0U;
#endif

//............................................................................
void QF::init(void) {
}
//............................................................................
int_t QF::run(void) {
    int_t result = -1;
    if (tickAnnounceHookAdd(reinterpret_cast<FUNCPTR>(&usrClockHook)) == OK)
    {
        onStartup(); // the startup callback
        result = 0;  // success
    }
    return result;
}
//............................................................................
void QF::stop(void) {
    onCleanup(); // the cleanup callback
}
//............................................................................
void QF::thread_(QActive *act) {
    // event loop of the active object thread
    for (;;) { // for-ever
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
}
//............................................................................
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par) // see NOTE1
{
    (void)stkSize; // unused paramteter in the VxWorks port

    Q_REQUIRE_ID(200, (prio <= QF_MAX_ACTIVE) /* not exceeding max */
        && (qSto != nullptr) /* queue storage */
        && (qLen > 0U)  /* queue size */
        && (stkSto == nullptr) /* NO stack storage */
        && (stkSize > 0U)); // stack size

    // create the event queue for the AO
    m_eQueue.init(qSto, qLen);

    m_prio = prio;  // save the QF priority
    QF::add_(this); // make QF aware of this active object

    init(par);      // thake the top-most initial tran.
    QS_FLUSH();     // flush the trace buffer to the host

    // synthesize task name of the form tAOxx,
    // where xx is the two-digit QP priority number
    char tname[] = "tAOxx";
    tname[3] = '0' + (m_prio / 10U);
    tname[4] = '0' + (m_prio % 10U);

    // convert the QP-priority to VxWorks priority
    int vx_prio = QF_VX_PRIO_OFFSET + QF_MAX_ACTIVE - prio;

    // spawn a VxWorks thread for the active object
    m_thread = taskSpawn(tname,      // task name
        vx_prio,                     // VxWorks priority
        reinterpret_cast<int>(ie),   // VxWorks taks options, see NOTE1
        static_cast<size_t>(stkSize),
        reinterpret_cast<FUNCPTR>(&task_function),
        reinterpret_cast<int>(this), 0, 0, 0, 0, 0, 0, 0, 0, 0);

    // VxWorks task must be created successfully
    Q_ASSERT_ID(210, m_thread != TASK_ID_NULL);
}

} // namespace QP

// VxWorks stuff in C ========================================================
extern "C" {
//............................................................................
// use exactly the VxWorks task signature
static int task_function(
    int a1,
    int /*a2*/,
    int /*a3*/,
    int /*a4*/,
    int /*a5*/,
    int /*a6*/,
    int /*a7*/,
    int /*a8*/,
    int /*a9*/,
    int /*a10*/)
{
    QP::QF::thread_(reinterpret_cast<QP::QActive *>(a1));
    return 0;
}

//............................................................................
static void usrClockHook(TASK_ID /*tid*/) {
    QP::QF::TICK_X(0U, &QP::l_clock_tick);
}

} // extern "C"

// NOTES: ====================================================================
//
// NOTE1:
// The last parameter of QActive::start() is used to supply the VxWorks task
// options (VX_FP_TASK, VX_PRIVATE_ENV, VX_NO_STACK_FILL, VX_UNBREAKABLE)
// to the AO task. Here is an example of usage:
//
// AO_Table->start(
//      N_PHILO + 1U,
//      l_tableQueueSto, Q_DIM(l_tableQueueSto),
//      nullptr, sizeof(l_tableStk),
//      reinterpret_cast<void *>(VX_FP_TASK | VX_NO_STACK_FILL));
//

