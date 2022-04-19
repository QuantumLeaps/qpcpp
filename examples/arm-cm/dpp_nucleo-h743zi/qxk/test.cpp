//============================================================================
// DPP example for QXK
// Last updated for version 6.8.0
// Last updated on  2020-01-15
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

namespace DPP {

// local extended-thread objects .............................................
static void Thread1_run(QP::QXThread * const me);
static void Thread2_run(QP::QXThread * const me);

static QP::QXThread l_test1(&Thread1_run, 0U);
static QP::QXThread l_test2(&Thread2_run, 0U);
static QP::QXMutex l_mutex;
static QP::QXSemaphore l_sema;

// global pointer to the test thread .........................................
QP::QXThread * const XT_Test1 = &l_test1;
QP::QXThread * const XT_Test2 = &l_test2;

// Thread-Local Storage for the "extended" threads ...........................
struct TLS_test {
    uint32_t foo;
    uint8_t bar[10];
};
static TLS_test l_tls1;
static TLS_test l_tls2;

static void lib_fun(uint32_t x) {
    QXK_TLS(TLS_test *)->foo = x;
}

//............................................................................
static void Thread1_run(QP::QXThread * const me) {

    QS_OBJ_DICTIONARY(&l_test1);

    // initialize the TLS for Thread1
    me->m_thread = &l_tls1;

    for (;;) {
        l_mutex.lock(QP::QXTHREAD_NO_TIMEOUT); // lock the mutex
        BSP::ledOn();

        if (l_mutex.tryLock()) { // exercise the mutex
            float volatile x;

            // some flating point code to exercise the VFP...
            x = 1.4142135F;
            x = x * 1.4142135F;

            (void)l_sema.signal(); // signal Thread2
            QP::QXThread::delay(10U);  // BLOCK while holding a mutex

            l_mutex.unlock();
        }

        l_mutex.unlock();
        BSP::ledOff();

        QP::QXThread::delay(BSP::TICKS_PER_SEC/7);  // BLOCK

        // publish to Thread2
        //QP::QF::PUBLISH(Q_NEW(QP::QEvt, TEST_SIG), &l_test1);

        // test TLS
        lib_fun(1U);
    }
}

//............................................................................
static void Thread2_run(QP::QXThread * const me) {

    QS_OBJ_DICTIONARY(&l_test2);

    // initialize the semaphore before using it
    // NOTE: Here the semaphore is initialized in the highest-priority thread
    // that uses it. Alternatively, the semaphore can be initialized
    // before any thread runs.
    l_sema.init(0U,  // count==0 (signaling semaphore)
                1U); // max_count==1 (binary semaphore)

    // initialize the mutex before using it
    // NOTE: Here the mutex is initialized in the highest-priority thread
    // that uses it. Alternatively, the mutex can be initialized
    // before any thread runs.
    l_mutex.init(N_PHILO + 6U); // priority-ceiling protocol used
    //l_mutex.init(0U); // alternatively: priority-ceiling NOT used

    // initialize the TLS for Thread2
    me->m_thread = &l_tls2;

    // subscribe to the test signal
    me->subscribe(TEST_SIG);

    for (;;) {
        // wait on a semaphore (BLOCK indefinitely)
        l_sema.wait();

        l_mutex.lock(QP::QXTHREAD_NO_TIMEOUT); // lock the mutex
        QP::QXThread::delay(1U);  // wait more (BLOCK)
        l_mutex.unlock();

        // test TLS
        lib_fun(2U);
    }
}

} // namespace DPP
