/// @file
/// @brief QF/C++ port to POSIX API with cooperative QV scheduler (posix-qv)
/// @cond
///***************************************************************************
/// Last updated for version 6.3.2
/// Last updated on  2018-06-16
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
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
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"       // QF package-scope interface
#include "qassert.h"      // QP embedded systems-friendly assertions
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

/* Global objects ==========================================================*/
pthread_mutex_t QF_pThreadMutex_;
QPSet  QV_readySet_;        // QV-ready set of active objects
pthread_cond_t QV_condVar_; // Cond.var. to signal events

// Local objects *************************************************************
static bool l_isRunning;    // flag indicating when QF is running
static struct timespec l_tick;
static int_t l_tickPrio;
enum { NANOSLEEP_NSEC_PER_SEC = 1000000000 }; // see NOTE05

static void *ticker_thread(void *arg);

//****************************************************************************
void QF::init(void) {
    // lock memory so we're never swapped out to disk
    //mlockall(MCL_CURRENT | MCL_FUTURE); // uncomment when supported

    // init the global mutex with the default non-recursive initializer
    pthread_mutex_init(&QF_pThreadMutex_, NULL);

    // clear the internal QF variables, so that the framework can (re)start
    // correctly even if the startup code is not called to clear the
    // uninitialized data (as is required by the C++ Standard).
    extern uint_fast8_t QF_maxPool_;
    QF_maxPool_ = static_cast<uint_fast8_t>(0);
    bzero(&QF::timeEvtHead_[0],
          static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));

    l_tick.tv_sec = 0;
    l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC/100L; // default clock tick
    l_tickPrio = sched_get_priority_min(SCHED_FIFO); // default tick prio
}

//****************************************************************************
int_t QF::run(void) {

    onStartup(); // application-specific startup callback

    // try to set the priority of the ticker thread
    struct sched_param sparam;
    sparam.sched_priority = l_tickPrio;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam) == 0) {
        // success, this application has sufficient privileges
    }
    else {
        // setting priority failed, probably due to insufficient privieges
    }

    l_isRunning = true; // QF is running

    // system clock tick configured?
    if ((l_tick.tv_sec != 0) || (l_tick.tv_nsec != 0)) {
        pthread_attr_t attr;
        struct sched_param param;
        pthread_t idle;

        // SCHED_FIFO corresponds to real-time preemptive priority-based
        // scheduler.
        // NOTE: This scheduling policy requires the superuser priviledges

        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        param.sched_priority = sched_get_priority_min(SCHED_FIFO);

        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&idle, &attr, &ticker_thread, 0) != 0) {
            // Creating the p-thread with the SCHED_FIFO policy failed.
            // Most probably this application has no superuser privileges,
            // so we just fall back to the default SCHED_OTHER policy
            // and priority 0.
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
            param.sched_priority = 0;
            pthread_attr_setschedparam(&attr, &param);
            if (pthread_create(&idle, &attr, &ticker_thread, 0) == 0) {
                return false;
            }
        }
        pthread_attr_destroy(&attr);
    }

    // the combined event-loop and background-loop of the QV kernel */
    QF_INT_DISABLE();
    while (l_isRunning) {

        if (QV_readySet_.notEmpty()) {
            uint_fast8_t p = QV_readySet_.findMax();
            QActive *a = active_[p];
            QF_INT_ENABLE();

            // the active object 'a' must still be registered in QF
            // (e.g., it must not be stopped)
            Q_ASSERT_ID(320, a != static_cast<QActive *>(0));

            // perform the run-to-completion (RTS) step...
            // 1. retrieve the event from the AO's event queue, which by this
            //    time must be non-empty and The "Vanialla" kernel asserts it.
            // 2. dispatch the event to the AO's state machine.
            // 3. determine if event is garbage and collect it if so
            //
            QEvt const *e = a->get_();
            a->dispatch(e);
            gc(e);

            QF_INT_DISABLE();

            if (a->m_eQueue.isEmpty()) { /* empty queue? */
                QV_readySet_.remove(p);
            }
        }
        else {
            // the QV kernel in embedded systems calls here the QV_onIdle()
            // callback. However, the POSIX-QV port does not do busy-waiting
            // for events. Instead, the POSIX-QV port efficiently waits until
            // QP events become available.
            //
            while (QV_readySet_.isEmpty()) {
                pthread_cond_wait(&QV_condVar_, &QF_pThreadMutex_);
            }
        }
    }
    QF_INT_ENABLE();
    onCleanup();  // cleanup callback
    QS_EXIT();    // cleanup the QSPY connection

    pthread_cond_destroy(&QV_condVar_);       // cleanup the condition variable
    pthread_mutex_destroy(&QF_pThreadMutex_); // cleanup the global mutex

    return static_cast<int_t>(0);
}
//****************************************************************************
void QF_setTickRate(uint32_t ticksPerSec, int_t tickPrio) {
    if (ticksPerSec != static_cast<uint32_t>(0)) {
        l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC / ticksPerSec;
    }
    else {
        l_tick.tv_nsec = 0; /* means NO system clock tick */
    }
    l_tickPrio = tickPrio;
}
//****************************************************************************
void QF::stop(void) {
    l_isRunning = false; // terminate the main event-loop thread

    // unblock the event-loop so it can terminate
    QV_readySet_.insert(1);
    pthread_cond_signal(&QV_condVar_);
}
//****************************************************************************
void QActive::start(uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t /*stkSize*/,
                    QEvt const *ie)
{
    Q_REQUIRE_ID(600, (static_cast<uint_fast8_t>(0) < prio) /* priority...*/
        && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE)) /*... in range */
        && (stkSto == static_cast<void *>(0)));    /* statck storage must NOT...
                                                    * ... be provided */

    m_eQueue.init(qSto, qLen);
    m_prio = static_cast<uint8_t>(prio); // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO
    this->init(ie); // execute initial transition (virtual call)
}
//****************************************************************************
void QActive::stop(void) {
    unsubscribeAll();
    QF::remove_(this);
}

//****************************************************************************
static void *ticker_thread(void * /*arg*/) { // for pthread_create()
    while (l_isRunning) { // the clock tick loop...
        QF_onClockTick(); // clock tick callback (must call QF_TICK_X())

        nanosleep(&l_tick, NULL); // sleep for the number of ticks, NOTE05
    }
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
// In some (older) Linux kernels, the POSIX nanosleep() system call might
// deliver only 2*actual-system-tick granularity. To compensate for this,
// you would need to reduce (by 2) the constant NANOSLEEP_NSEC_PER_SEC.
//

