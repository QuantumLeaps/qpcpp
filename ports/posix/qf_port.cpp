/// @file
/// @brief QF/C++ port to POSIX/P-threads
/// @cond
///***************************************************************************
/// Last updated for version 5.4.0
/// Last updated on  2015-04-14
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

#include <limits.h>      // for PTHREAD_STACK_MIN
#include <sys/mman.h>    // for mlockall()
#include <sys/select.h>

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Global-scope objects ------------------------------------------------------
pthread_mutex_t QF_pThreadMutex_ = PTHREAD_MUTEX_INITIALIZER;

// Local-scope objects -------------------------------------------------------
static long int l_tickUsec = 10000UL; // clock tick in usec (for tv_usec)
static bool l_running;

static void *ao_thread(void *arg); // thread routine for all AOs

//............................................................................
void QF::init(void) {
    // lock memory so we're never swapped out to disk
    //mlockall(MCL_CURRENT | MCL_FUTURE); // uncomment when supported
}
//............................................................................
int_t QF::run(void) {
    onStartup(); // invoke startup callback

    // try to maximize the priority of this thread, see NOTE01
    struct sched_param sparam;
    sparam.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam) == 0) {
        // success, this application has sufficient privileges
    }
    else {
        // setting priority failed, probably due to insufficient privieges
    }

    struct timeval timeout = { 0 }; // timeout for select()
    l_running = true;
    while (l_running) {
        QF_onClockTick(); // clock tick callback (must call QF_TICK_X())

        timeout.tv_usec = l_tickUsec;
        select(0, 0, 0, 0, &timeout); // sleep for the full tick , NOTE05
    }
    onCleanup(); // invoke cleanup callback
    pthread_mutex_destroy(&QF_pThreadMutex_);
    return static_cast<int_t>(0); // return success
}
//............................................................................
void QF_setTickRate(uint32_t ticksPerSec) {
    l_tickUsec = 1000000UL / ticksPerSec;
}
//............................................................................
void QF::stop(void) {
    l_running = false; // stop the loop in QF::run()
}
//............................................................................
void QF::thread_(QMActive *act) {
    // loop until m_thread is cleared in QActive::stop()
    do {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    } while (act->m_thread != static_cast<uint8_t>(0));

    QF::remove_(act); // remove this object from any subscriptions
    pthread_cond_destroy(&act->m_osObject); // cleanup the condition variable
}
//............................................................................
void QMActive::start(uint_fast8_t prio,
                     QEvt const *qSto[], uint_fast16_t qLen,
                     void *stkSto, uint_fast16_t stkSize,
                     QEvt const *ie)
{
    // p-threads allocate stack internally
    Q_REQUIRE_ID(600, stkSto == static_cast<void *>(0));

    pthread_cond_init(&m_osObject, 0);

    m_eQueue.init(qSto, qLen);
    m_prio = static_cast<uint8_t>(prio); // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO
    this->init(ie); // execute initial transition (virtual call)

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // SCHED_FIFO corresponds to real-time preemptive priority-based scheduler
    // NOTE: This scheduling policy requires the superuser privileges
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    // see NOTE04
    struct sched_param param;
    param.sched_priority = prio
                           + (sched_get_priority_max(SCHED_FIFO)
                              - QF_MAX_ACTIVE - 3);

    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // stack size not provided?
    if (stkSize == 0U) {
        stkSize = (uint_fast16_t)PTHREAD_STACK_MIN; // the minimum
    }
    pthread_t thread;
    if (pthread_create(&thread, &attr, &ao_thread, this) != 0) {

        // Creating the p-thread with the SCHED_FIFO policy failed.
        // Most probably this application has no superuser privileges,
        // so we just fall back to the default SCHED_OTHER policy
        // and priority 0.

        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        param.sched_priority = 0;
        pthread_attr_setschedparam(&attr, &param);
        Q_ALLEGE(pthread_create(&thread, &attr, &ao_thread, this)== 0);
    }
    pthread_attr_destroy(&attr);
    m_thread = static_cast<uint8_t>(1);
}
//............................................................................
void QMActive::stop(void) {
    m_thread = static_cast<uint8_t>(0); // stop the QF::thread_() loop
}

//............................................................................
static void *ao_thread(void *arg) { // the expected POSIX signature
    QF::thread_(static_cast<QMActive *>(arg));
    return static_cast<void *>(0); // return success
}

} // namespace QP

//****************************************************************************
// NOTE01:
// In Linux, the scheduler policy closest to real-time is the SCHED_FIFO
// policy, available only with superuser privileges. QF::run() attempts to set
// this policy as well as to maximize its priority, so that the ticking
// occurrs in the most timely manner (as close to an interrupt as possible).
// However, setting the SCHED_FIFO policy might fail, most probably due to
// insufficient privileges.
//
// NOTE02:
// On some Linux systems nanosleep() might actually not deliver the finest
// time granularity. For example, on some Linux implementations, nanosleep()
// could not block for shorter intervals than 20ms, while the underlying
// clock tick period was only 10ms. Sometimes, the select() system call can
// provide a finer granularity.
//
// NOTE03:
// Any blocking system call, such as nanosleep() or select() system call can
// be interrupted by a signal, such as ^C from the keyboard. In this case this
// QF port breaks out of the event-loop and returns to main() that exits and
// terminates all spawned p-threads.
//
// NOTE04:
// According to the man pages (for pthread_attr_setschedpolicy) the only value
// supported in the Linux p-threads implementation is PTHREAD_SCOPE_SYSTEM,
// meaning that the threads contend for CPU time with all processes running on
// the machine. In particular, thread priorities are interpreted relative to
// the priorities of all other processes on the machine.
//
// This is good, because it seems that if we set the priorities high enough,
// no other process (or thread running within) can gain control over the CPU.
//
// However, QF limits the number of priority levels to QF_MAX_ACTIVE.
// Assuming that a QF application will be real-time, this port reserves the
// three highest Linux priorities for the ISR-like threads (e.g., the ticker,
// I/O), and the rest highest-priorities for the active objects.
//
// NOTE05:
// The select() system call seems to deliver the finest time granularity of
// 1 clock tick. The timeout value passed to select() is rounded up to the
// nearest tick (10 ms on desktop Linux). The timeout cannot be too short,
// because the system might choose to busy-wait for very short timeouts.
// An alternative, POSIX nanosleep() system call seems to deliver only 20ms
// granularity.
//
// Here the select() call is used not just as a fairly portable way to sleep
// with subsecond precision. The select() call is also used to detect any
// characters typed on the console.
//
// Also according to man pages, on Linux, the function select() modifies
// timeout to reflect the amount of time not slept; most other implementations
// do not do this. This causes problems both when Linux code which reads
// timeout is ported to other operating systems, and when code is ported to
// Linux that reuses a struct timeval for multiple selects in a loop without
// reinitializing it. Here the microsecond part of the structure is re-
// initialized before each select() call.
//
