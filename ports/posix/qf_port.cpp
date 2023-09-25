//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2023-11-30
//! @version Last updated for: @ref qpc_7_3_1
//!
//! @file
//! @brief QF/C++ port to POSIX (multithreaded with P-threads)

// expose features from the 2008 POSIX standard (IEEE Standard 1003.1-2008)
#define _POSIX_C_SOURCE 200809L

#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#include <limits.h>         // for PTHREAD_STACK_MIN
#include <sys/mman.h>       // for mlockall()
#include <sys/ioctl.h>
#include <time.h>           // for clock_nanosleep()
#include <string.h>         // for memcpy() and memset()
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

namespace { // unnamed local namespace

Q_DEFINE_THIS_MODULE("qf_port")

// Local objects =============================================================
// initialize the startup mutex with default non-recursive initializer
static pthread_mutex_t l_startupMutex = PTHREAD_MUTEX_INITIALIZER;

static bool l_isRunning;      // flag indicating when QF is running
static struct termios l_tsav; // structure with saved terminal attributes
static struct timespec l_tick;
static int_t l_tickPrio;

constexpr long NSEC_PER_SEC {1000000000L};
constexpr long DEFAULT_TICKS_PER_SEC {100L};

static void sigIntHandler(int dummy); // prototype
static void sigIntHandler(int dummy) {
    Q_UNUSED_PAR(dummy);
    QP::QF::onCleanup();
    exit(-1);
}

static void *ao_thread(void *arg); // prototype
static void *ao_thread(void *arg) { // thread routine for all AOs
    QP::QActive::evtLoop_(static_cast<QP::QActive *>(arg));
    return nullptr; // return success
}

} // unnamed local namespace

//============================================================================
namespace QP {
namespace QF {

// NOTE: initialize the critical section mutex as non-recursive,
// but check that nesting of critical sections never occurs
// (see QF::enterCriticalSection_()/QF::leaveCriticalSection_()
pthread_mutex_t critSectMutex_ = PTHREAD_MUTEX_INITIALIZER;
int_t critSectNest_;

//............................................................................
void enterCriticalSection_() {
    pthread_mutex_lock(&critSectMutex_);
    Q_ASSERT_INCRIT(100, critSectNest_ == 0); // NO nesting of crit.sect!
    ++critSectNest_;
}
//............................................................................
void leaveCriticalSection_() {
    Q_ASSERT_INCRIT(200, critSectNest_ == 1); // crit.sect. must ballace!
    if ((--critSectNest_) == 0) {
        pthread_mutex_unlock(&critSectMutex_);
    }
}

//............................................................................
void init() {
    // lock memory so we're never swapped out to disk
    //mlockall(MCL_CURRENT | MCL_FUTURE); // un-comment when supported

    // lock the startup mutex to block any active objects started before
    // calling QF::run()
    pthread_mutex_lock(&l_startupMutex);

    l_tick.tv_sec = 0;
    l_tick.tv_nsec = NSEC_PER_SEC / DEFAULT_TICKS_PER_SEC; // default tick
    l_tickPrio = sched_get_priority_min(SCHED_FIFO); // default ticker prio

    // install the SIGINT (Ctrl-C) signal handler
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = &sigIntHandler;
    sigaction(SIGINT, &sig_act, NULL);
}

//............................................................................
int run() {

    onStartup(); // application-specific startup callback

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    // try to set the priority of the ticker thread, see NOTE01
    struct sched_param sparam;
    sparam.sched_priority = l_tickPrio;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam) == 0) {
        // success, this application has sufficient privileges
    }
    else {
        // setting priority failed, probably due to insufficient privileges
    }

    // exit the startup critical section to unblock any active objects
    // started before calling QF_run()
    pthread_mutex_unlock(&l_startupMutex);
    l_isRunning = true;

    // The provided clock tick service configured?
    if ((l_tick.tv_sec != 0) || (l_tick.tv_nsec != 0)) {

        // get the absolute monotonic time for no-drift sleeping
        static struct timespec next_tick;
        clock_gettime(CLOCK_MONOTONIC, &next_tick);

        // round down nanoseconds to the nearest configured period
        next_tick.tv_nsec
            = (next_tick.tv_nsec / l_tick.tv_nsec) * l_tick.tv_nsec;

        while (l_isRunning) { // the clock tick loop...

            // advance to the next tick (absolute time)
            next_tick.tv_nsec += l_tick.tv_nsec;
            if (next_tick.tv_nsec >= NSEC_PER_SEC) {
                next_tick.tv_nsec -= NSEC_PER_SEC;
                next_tick.tv_sec  += 1;
            }

            // sleep without drifting till next_time (absolute), see NOTE03
            if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                                &next_tick, NULL) == 0) // success?
            {
                // clock tick callback (must call QTimeEvt::TICK_X() once)
                onClockTick();
            }
        }
    }
    else { // The provided system clock tick NOT configured

        while (l_isRunning) { // the clock tick loop...

            // In case the application intentionally DISABLED the provided
            // system clock, the QF_onClockTick() callback is used to let
            // the application implement the alternative tick service.
            // In that case the QF_onClockTick() must internally WAIT
            // for the desired clock period before calling QTIMEEVT_TICK_X().
            onClockTick();
        }
    }
    onCleanup(); // cleanup callback
    QS_EXIT();   // cleanup the QSPY connection

    pthread_mutex_destroy(&l_startupMutex);
    pthread_mutex_destroy(&critSectMutex_);

    return 0; // return success
}
//............................................................................
void stop() {
    l_isRunning = false; // terminate the main (ticker) thread
}
//............................................................................
void setTickRate(std::uint32_t ticksPerSec, int tickPrio) {
    if (ticksPerSec != 0U) {
        l_tick.tv_nsec = NSEC_PER_SEC / ticksPerSec;
    }
    else {
        l_tick.tv_nsec = 0U; // means NO system clock tick
    }
    l_tickPrio = tickPrio;
}

//............................................................................
void consoleSetup() {
    struct termios tio;   // modified terminal attributes

    tcgetattr(0, &l_tsav); // save the current terminal attributes
    tcgetattr(0, &tio);    // obtain the current terminal attributes
    tio.c_lflag &= ~(ICANON | ECHO); // disable the canonical mode & echo
    tcsetattr(0, TCSANOW, &tio);     // set the new attributes
}
//............................................................................
void consoleCleanup() {
    tcsetattr(0, TCSANOW, &l_tsav); // restore the saved attributes
}
//............................................................................
int consoleGetKey() {
    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);
    if (byteswaiting > 0) {
        char ch;
        read(0, &ch, 1);
        return (int)ch;
    }
    return 0; // no input at this time
}
//............................................................................
int consoleWaitForKey() {
    return static_cast<int>(getchar());
}

} // namespace QF

//============================================================================
void QActive::evtLoop_(QActive *act) {
    // block this thread until the startup mutex is unlocked from QF::run()
    pthread_mutex_lock(&l_startupMutex);
    pthread_mutex_unlock(&l_startupMutex);

#ifdef QACTIVE_CAN_STOP
    act->m_thread = true;
    while (act->m_thread)
#else
    for (;;) // for-ever
#endif
    {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e, act->m_prio); // dispatch to the HSM
        QF::gc(e); // check if the event is garbage, and collect it if so
    }
#ifdef QACTIVE_CAN_STOP
    act->unregister_(); // remove this object from QF
#endif
}

//============================================================================
void QActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // p-threads allocate stack internally
    Q_REQUIRE_ID(800, stkSto == nullptr);

    pthread_cond_init(&m_osObject, 0);
    m_eQueue.init(qSto, qLen);

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used in this port)
    register_(); // register this AO

    // top-most initial tran. (virtual call)
    this->init(par, m_prio);
    QS_FLUSH(); // flush the trace buffer to the host

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // SCHED_FIFO corresponds to real-time preemptive priority-based scheduler
    // NOTE: This scheduling policy requires the superuser privileges
    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

    // priority of the p-thread, see NOTE04
    struct sched_param param;
    param.sched_priority = m_prio
                           + (sched_get_priority_max(SCHED_FIFO)
                              - QF_MAX_ACTIVE - 3U);
    pthread_attr_setschedparam(&attr, &param);

    pthread_attr_setstacksize(&attr, (stkSize < PTHREAD_STACK_MIN
                                      ? PTHREAD_STACK_MIN
                                      : stkSize));
    pthread_t thread;
    int err = pthread_create(&thread, &attr, &ao_thread, this);
    if (err != 0) {
        // Creating p-thread with the SCHED_FIFO policy failed. Most likely
        // this application has no superuser privileges, so we just fall
        // back to the default SCHED_OTHER policy and priority 0.
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        param.sched_priority = 0;
        pthread_attr_setschedparam(&attr, &param);
        err = pthread_create(&thread, &attr, &ao_thread, this);
    }
    Q_ASSERT_ID(810, err == 0); // AO thread must be created

    //pthread_attr_getschedparam(&attr, &param);
    //printf("param.sched_priority==%d\n", param.sched_priority);

    pthread_attr_destroy(&attr);
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    unsubscribeAll(); // unsubscribe this AO from all events
    m_thread = false; // stop the thread loop (see QF::thread_)
}
#endif

} // namespace QP

//============================================================================
// NOTE01:
// In Linux, the scheduler policy closest to real-time is the SCHED_FIFO
// policy, available only with superuser privileges. QF::run() attempts to set
// this policy as well as to maximize its priority, so that the ticking
// occurs in the most timely manner (as close to an interrupt as possible).
// However, setting the SCHED_FIFO policy might fail, most probably due to
// insufficient privileges.
//
// NOTE03:
// Any blocking system call, such as clock_nanosleep() system call can
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
// three highest p-thread priorities for the ISR-like threads (e.g., I/O),
// and the rest highest-priorities for the active objects.
//

