/// @file
/// @brief QF/C++ port to POSIX API (single-threaded, like QV kernel)
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-21
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

// expose features from the 2008 POSIX standard (IEEE Standard 1003.1-2008)
#define _POSIX_C_SOURCE 200809L

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

#include <limits.h>         // for PTHREAD_STACK_MIN
#include <sys/mman.h>       // for mlockall()
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>         // for memcpy() and memset()
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

/* Global objects ==========================================================*/
QPSet  QV_readySet_;        // QV-ready set of active objects
pthread_cond_t QV_condVar_; // Cond.var. to signal events

// Local objects *************************************************************
static pthread_mutex_t l_pThreadMutex; // POSIX mutex for the QF crit. section
static bool l_isRunning;    // flag indicating when QF is running
static struct termios l_tsav; // structure with saved terminal attributes
static struct timespec l_tick;
static int_t l_tickPrio;
enum { NANOSLEEP_NSEC_PER_SEC = 1000000000 }; // see NOTE05

static void *ticker_thread(void *arg);
static void sigIntHandler(int /* dummy */);

//****************************************************************************
void QF::init(void) {
    // lock memory so we're never swapped out to disk
    //mlockall(MCL_CURRENT | MCL_FUTURE); // uncomment when supported

    // init the global mutex with the default non-recursive initializer
    pthread_mutex_init(&l_pThreadMutex, NULL);

    // init the global condition variable with the default initializer
    pthread_cond_init(&QV_condVar_, NULL);

    l_tick.tv_sec = 0;
    l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC/100L; // default clock tick
    l_tickPrio = sched_get_priority_min(SCHED_FIFO); // default tick prio

    // install the SIGINT (Ctrl-C) signal handler
    struct sigaction sig_act;
    sig_act.sa_handler = &sigIntHandler;
    sigaction(SIGINT, &sig_act, NULL);
}

//****************************************************************************
void QF_enterCriticalSection_(void) {
    pthread_mutex_lock(&l_pThreadMutex);
}
//****************************************************************************
void QF_leaveCriticalSection_(void) {
    pthread_mutex_unlock(&l_pThreadMutex);
}

//****************************************************************************
int_t QF::run(void) {
    onStartup(); // application-specific startup callback

    l_isRunning = true; // QF is running

    // system clock tick configured?
    if ((l_tick.tv_sec != 0) || (l_tick.tv_nsec != 0)) {

        pthread_attr_t attr;
        pthread_attr_init(&attr);

        // SCHED_FIFO corresponds to real-time preemptive priority-based
        // scheduler.
        // NOTE: This scheduling policy requires the superuser priviledges
        //
        pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        struct sched_param param;
        param.sched_priority = sched_get_priority_min(SCHED_FIFO);

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
        Q_ASSERT_ID(310, err == 0); /* ticker thread must be created */

        //pthread_attr_getschedparam(&attr, &param);
        //printf("param.sched_priority==%d\n", param.sched_priority);

        pthread_attr_destroy(&attr);
    }

    // the combined event-loop and background-loop of the QV kernel */
    QF_CRIT_STAT_
    QF_CRIT_E_();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_NOCRIT_PRE_(QS_QF_RUN, 0U)
    QS_END_NOCRIT_PRE_()

    while (l_isRunning) {

        if (QV_readySet_.notEmpty()) {
            std::uint_fast8_t p = QV_readySet_.findMax();
            QActive *a = active_[p];
            QF_CRIT_X_();

            // the active object 'a' must still be registered in QF
            // (e.g., it must not be stopped)
            Q_ASSERT_ID(320, a != nullptr);

            // perform the run-to-completion (RTS) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The "Vanialla" kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const *e = a->get_();
            a->dispatch(e, a->m_prio);
            gc(e);

            QF_CRIT_E_();

            if (a->m_eQueue.isEmpty()) { /* empty queue? */
                QV_readySet_.rmove(p);
            }
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the POSIX-QV port does not do busy-waiting
            // for events. Instead, the POSIX-QV port efficiently waits until
            // QP events become available.
            //
            while (QV_readySet_.isEmpty()) {
                pthread_cond_wait(&QV_condVar_, &l_pThreadMutex);
            }
        }
    }
    QF_CRIT_X_();
    onCleanup();  // cleanup callback
    QS_EXIT();    // cleanup the QSPY connection

    pthread_cond_destroy(&QV_condVar_); // cleanup the condition variable
    pthread_mutex_destroy(&l_pThreadMutex); // cleanup the global mutex

    return 0; // return success
}
//****************************************************************************
void QF_setTickRate(std::uint32_t ticksPerSec, int_t tickPrio) {
    if (ticksPerSec != 0U) {
        l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC / ticksPerSec;
    }
    else {
        l_tick.tv_nsec = 0; /* means NO system clock tick */
    }
    l_tickPrio = tickPrio;
}
//............................................................................
void QF::stop(void) {
    l_isRunning = false; // terminate the main event-loop thread

    // unblock the event-loop so it can terminate
    QV_readySet_.insert(1);
    pthread_cond_signal(&QV_condVar_);
}
//............................................................................
void QF_consoleSetup(void) {
    struct termios tio;   // modified terminal attributes

    tcgetattr(0, &l_tsav); // save the current terminal attributes
    tcgetattr(0, &tio);    // obtain the current terminal attributes
    tio.c_lflag &= ~(ICANON | ECHO); // disable the canonical mode & echo
    tcsetattr(0, TCSANOW, &tio);     // set the new attributes
}
//............................................................................
void QF_consoleCleanup(void) {
    tcsetattr(0, TCSANOW, &l_tsav); // restore the saved attributes
}
//............................................................................
int QF_consoleGetKey(void) {
    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);
    if (byteswaiting > 0) {
        char ch;
        (void)read(0, &ch, 1);
        return (int)ch;
    }
    return 0; // no input at this time
}
/*..........................................................................*/
int QF_consoleWaitForKey(void) {
    return getchar();
}

//****************************************************************************
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    (void)stkSize; // unused paramteter in the POSIX port

    Q_REQUIRE_ID(600, (0U < prio)  /* priority...*/
        && (prio <= QF_MAX_ACTIVE) /*.. in range */
        && (stkSto == nullptr));   // statck storage must NOT...
                                    // ... be provided

    m_eQueue.init(qSto, qLen);
    m_prio = static_cast<std::uint8_t>(prio); // set the QF prio of this AO
    QF::add_(this); // make QF aware of this AO

    this->init(par, m_prio); // execute initial transition (virtual call)
    QS_FLUSH(); // flush the QS trace buffer to the host
}
//............................................................................
#ifdef QF_ACTIVE_STOP
void QActive::stop(void) {
    unsubscribeAll(); // unsubscribe from all events

    // make sure the AO is no longer in "ready set"
    QF_CRIT_STAT_
    QF_CRIT_E_();
    QV_readySet_.rmove(m_prio);
    QF_CRIT_X_();

    QF::remove_(this); // remove this AO from QF
}
#endif

//****************************************************************************
static void *ticker_thread(void * /*arg*/) { // for pthread_create()
    while (l_isRunning) { // the clock tick loop...
        nanosleep(&l_tick, NULL); // sleep for the number of ticks, NOTE05
        QF_onClockTick(); // clock tick callback (must call QF_TICK_X())
    }
    return nullptr; // return success
}

//****************************************************************************
static void sigIntHandler(int /* dummy */) {
    QF::onCleanup();
    exit(-1);
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
// three highest p-thread priorities for the ISR-like threads (e.g., I/O),
// and the rest highest-priorities for the active objects.
//
// NOTE05:
// In some (older) Linux kernels, the POSIX nanosleep() system call might
// deliver only 2*actual-system-tick granularity. To compensate for this,
// you would need to reduce the constant NANOSLEEP_NSEC_PER_SEC by factor 2.
//

