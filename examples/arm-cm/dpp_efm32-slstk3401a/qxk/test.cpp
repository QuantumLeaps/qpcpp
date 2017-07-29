//****************************************************************************
// DPP example for QXK
// Last updated for version 5.9.6
// Last updated on  2017-07-27
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// https://state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

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

    me->m_thread = &l_tls1; // initialize the TLS for Thread1

    l_mutex.init(3U);

    for (;;) {

        // wait on a semaphore (BLOCK with timeout)
        (void)l_sema.wait(BSP::TICKS_PER_SEC, 0U);
        BSP::ledOn();

        l_mutex.lock(); // exercise the mutex
        // some flating point code to exercise the VFP...
        float volatile x = 1.4142135F;
        x = x * 1.4142135F;
        //QP::QXThread::delay(1U, 0U); // asserts (blocking while holding a mutex)
        l_mutex.unlock();

        QP::QXThread::delay(BSP::TICKS_PER_SEC/7, 0U);  // BLOCK

        // publish to Thread2
        QP::QF::PUBLISH(Q_NEW(QP::QEvt, TEST_SIG), &l_test1);

        // test TLS
        lib_fun(1U);
    }
}

//............................................................................
static void Thread2_run(QP::QXThread * const me) {

    me->m_thread = &l_tls2; // initialize the TLS for Thread2

    // subscribe to the test signal */
    me->subscribe(TEST_SIG);

    // initialize the semaphore before using it
    // NOTE: the semaphore is initialized in the highest-priority thread
    // that uses it. Alternatively, the semaphore can be initialized
    // before any thread runs.
    l_sema.init(0U,   // count==0 (signaling semaphore)
                1U);  // max_count==1 (binary semaphore)

    for (;;) {
        // some flating point code to exercise the VFP...
        float volatile x = 1.4142135F;
        x = x * 1.4142135F;

        // wait on the internal event queue (BLOCK) with timeout
        QP::QEvt const *e = me->queueGet(BSP::TICKS_PER_SEC/2, 0U);
        BSP::ledOff();

        if (e != static_cast<QP::QEvt *>(0)) { // event actually delivered?
            QP::QF::gc(e); // recycle the event manually!
        }
        else {
            QP::QXThread::delay(BSP::TICKS_PER_SEC/2, 0U);  // BLOCK
            l_sema.signal(); // signal Thread1
        }

        // test TLS
        lib_fun(2U);
    }
}

} // namespace DPP
