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
//! @brief QF/C++ port to POSIX-QV (single-threaded)

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

static bool l_isRunning;       // flag indicating when QF is running
static struct termios l_tsav;  // structure with saved terminal attributes
static struct timespec l_tick; // structure for the clock tick
static int_t l_tickPrio;       // priority of the ticker thread

constexpr long NSEC_PER_SEC {1000000000L};
constexpr long DEFAULT_TICKS_PER_SEC {100L};

//============================================================================
static void *ticker_thread(void *arg); // prototype
static void *ticker_thread(void *arg) { // for pthread_create()
    Q_UNUSED_PAR(arg);

    // system clock tick must be configured
    Q_REQUIRE_ID(100, l_tick.tv_nsec != 0);

    // get the absolute monotonic time for no-drift sleeping
    static struct timespec next_tick;
    clock_gettime(CLOCK_MONOTONIC, &next_tick);

    // round down nanoseconds to the nearest configured period
    next_tick.tv_nsec = (next_tick.tv_nsec / l_tick.tv_nsec) * l_tick.tv_nsec;

    while (l_isRunning) { // the clock tick loop...

        // advance to the next tick (absolute time)
        next_tick.tv_nsec += l_tick.tv_nsec;
        if (next_tick.tv_nsec >= NSEC_PER_SEC) {
            next_tick.tv_nsec -= NSEC_PER_SEC;
            next_tick.tv_sec  += 1;
        }

        // sleep without drifting till next_tick (absolute), see NOTE03
        if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                            &next_tick, NULL) == 0) // success?
        {
            // clock tick callback (must call QTimeEvt::TICK_X())
            QP::QF::onClockTick();
        }
    }
    return nullptr; // return success
}
//............................................................................
static void sigIntHandler(int dummy); // prototype
static void sigIntHandler(int dummy) {
    Q_UNUSED_PAR(dummy);
    QP::QF::onCleanup();
    exit(-1);
}

} // unnamed local namespace

// Global objects ============================================================
namespace QP {
namespace QF {

QPSet readySet_;
QPSet readySet_dis_;
pthread_cond_t condVar_; // cond.var. to signal events

//============================================================================
// QF functions

// NOTE: initialize the critical section mutex as non-recursive,
// but check that nesting of critical sections never occurs
// (see QF::enterCriticalSection_()/QF::leaveCriticalSection_()
static pthread_mutex_t l_critSectMutex_ = PTHREAD_MUTEX_INITIALIZER;
static int_t l_critSectNest;   // critical section nesting up-down counter

//............................................................................
void enterCriticalSection_() {
    if (l_isRunning) {
        pthread_mutex_lock(&l_critSectMutex_);
        Q_ASSERT_INCRIT(100, l_critSectNest == 0); // NO nesting of crit.sect!
        ++l_critSectNest;
    }
}
//............................................................................
void leaveCriticalSection_() {
    if (l_isRunning) {
        Q_ASSERT_INCRIT(200, l_critSectNest == 1); // crit.sect must ballace!
        if ((--l_critSectNest) == 0) {
           pthread_mutex_unlock(&l_critSectMutex_);
        }
    }
}

//............................................................................
void init() {
    // init the global condition variable with the default initializer
    pthread_cond_init(&condVar_, NULL);

    readySet_.setEmpty();
#ifndef Q_UNSAFE
    readySet_.update_(&readySet_dis_);
#endif

    // lock memory so we're never swapped out to disk
    //mlockall(MCL_CURRENT | MCL_FUTURE); // un-comment when supported

    l_tick.tv_sec = 0;
    l_tick.tv_nsec = NSEC_PER_SEC / DEFAULT_TICKS_PER_SEC; // default rate
    l_tickPrio = sched_get_priority_min(SCHED_FIFO); // default ticker prio

    // install the SIGINT (Ctrl-C) signal handler
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = &sigIntHandler;
    sigaction(SIGINT, &sig_act, NULL);
}

//............................................................................
int run() {
    l_isRunning = true; // QF is running

    onStartup(); // application-specific startup callback

    QF_CRIT_STAT
    // system clock tick configured?
    if ((l_tick.tv_sec != 0) || (l_tick.tv_nsec != 0)) {

        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // SCHED_FIFO corresponds to real-time preemptive priority-based
        // scheduler.
        // NOTE: This scheduling policy requires the superuser priviledges
        pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        struct sched_param param;
        param.sched_priority = l_tickPrio;

        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        pthread_t ticker;
        int err = pthread_create(&ticker, &attr, &ticker_thread, 0);
        if (err != 0) {
            // Creating the p-thread with the SCHED_FIFO policy failed.
            // Most probably this application has no superuser privileges,
            // so we just fall back to the default SCHED_OTHER policy
            // and priority 0.
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
            param.sched_priority = 0;
            pthread_attr_setschedparam(&attr, &param);
            err = pthread_create(&ticker, &attr, &ticker_thread, 0);
        }
        QF_CRIT_ENTRY();
        Q_ASSERT_INCRIT(310, err == 0); // ticker thread must be created
        QF_CRIT_EXIT();

        //pthread_attr_getschedparam(&attr, &param);
        //printf("param.sched_priority==%d\n", param.sched_priority);

        pthread_attr_destroy(&attr);
    }

    // the combined event-loop and background-loop of the QV kernel
    QF_CRIT_ENTRY();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    while (l_isRunning) {
        Q_ASSERT_INCRIT(300, readySet_.verify_(&readySet_dis_));

        // find the maximum priority AO ready to run
        if (readySet_.notEmpty()) {
            std::uint_fast8_t p = readySet_.findMax();
            QActive *a = QActive::registry_[p];

            // the active object 'a' must still be registered in QF
            // (e.g., it must not be stopped)
            Q_ASSERT_INCRIT(320, a != nullptr);
            QF_CRIT_EXIT();

            QEvt const *e = a->get_();
            // dispatch event (virtual call)
            a->dispatch(e, a->getPrio());
            QF::gc(e);

            QF_CRIT_ENTRY();
            if (a->getEQueue().isEmpty()) { // empty queue?
                readySet_.remove(p);
#ifndef Q_UNSAFE
                readySet_.update_(&readySet_dis_);
#endif
            }
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the POSIX-QV port does not do busy-waiting
            // for events. Instead, the POSIX-QV port efficiently waits until
            // QP events become available.
            while (readySet_.isEmpty()) {
                Q_ASSERT_INCRIT(390, l_critSectNest == 1);
                --l_critSectNest;

                pthread_cond_wait(&condVar_, &l_critSectMutex_);

                Q_ASSERT_INCRIT(391, l_critSectNest == 0);
                ++l_critSectNest;
            }
        }
    }
    QF_CRIT_EXIT();
    onCleanup();  // cleanup callback
    QS_EXIT();    // cleanup the QSPY connection

    pthread_cond_destroy(&condVar_); // cleanup the condition variable
    pthread_mutex_destroy(&l_critSectMutex_); // cleanup the global mutex

    return 0; // return success
}
//............................................................................
void stop() {
    l_isRunning = false; // terminate the main event-loop thread

    // unblock the event-loop so it can terminate
    readySet_.insert(1U);
#ifndef Q_UNSAFE
    readySet_.update_(&readySet_dis_);
#endif
    pthread_cond_signal(&condVar_);
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

// QActive functions =========================================================
void QActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    // no per-AO stack needed for this port
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(600, stkSto == nullptr);
    QF_CRIT_EXIT();

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = 0U; // preemption-threshold (not used in this port)
    register_(); // make QF aware of this AO

    m_eQueue.init(qSto, qLen);

    // top-most initial tran. (virtual call)
    this->init(par, m_prio);
    QS_FLUSH(); // flush the QS trace buffer to the host
}

//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    unsubscribeAll(); // unsubscribe from all events

    // make sure the AO is no longer in "ready set"
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF::readySet_.remove(m_prio);
#ifndef Q_UNSAFE
    QF::readySet_.update_(&QF::readySet_dis_);
#endif
    QF_CRIT_EXIT();

    unregister_();
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
// and the remaining highest-priorities for the active objects.
//

