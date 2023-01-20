//============================================================================
// QXThread example
// Last updated for version 7.3.0
// Last updated on  2023-08-12
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
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
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"             // QP/C++ real-time embedded framework
#include "dpp.hpp"               // DPP Application interface
#include "bsp.hpp"               // Board Support Package

//----------------------------------------------------------------------------
// unnamed namespace for local definitions with internal linkage
namespace {
//Q_DEFINE_THIS_FILE
} // unnamed namespace

//----------------------------------------------------------------------------
namespace APP {

//............................................................................
QP::QXSemaphore TH_sema;
QP::QXMutex TH_mutex;

//............................................................................
class XThread2 : public QP::QXThread {
private:
    // NOTE: data needed by this thread should be members of
    // the thread class. That way they are in the memory region
    // accessible from this thread.
    std::uint8_t m_foo;

public:
    static XThread2 inst;
    XThread2();

private:
    static void run(QP::QXThread * const thr);
}; // class XThread2

//............................................................................
QP::QXThread * const TH_XThread2 = &XThread2::inst;

XThread2 XThread2::inst;

XThread2::XThread2()
  : QXThread(&run)
{}

//............................................................................
void XThread2::run(QP::QXThread * const thr) {
    // downcast the generic thr pointer to the specific thread
    //auto me = static_cast<XThread2 *>(thr);

    QS_OBJ_DICTIONARY(TH_XThread2);
    QS_OBJ_DICTIONARY(TH_XThread2->getTimeEvt());
    QS_OBJ_DICTIONARY(&TH_sema);
    QS_OBJ_DICTIONARY(&TH_mutex);

    // initialize the semaphore before using it
    // NOTE: Here the semaphore is initialized in the highest-priority thread
    // that uses it. Alternatively, the semaphore can be initialized
    // before any thread runs.
    TH_sema.init(0U,  // count==0 (signaling semaphore)
                1U); // max_count==1 (binary semaphore)

    // initialize the mutex before using it
    // NOTE: Here the mutex is initialized in the highest-priority thread
    // that uses it. Alternatively, the mutex can be initialized
    // before any thread runs.
    TH_mutex.init(APP::N_PHILO + 6U); // priority-ceiling mutex
    //l_mutex.init(0U); // alternatively: priority-ceiling NOT used

    for (;;) {
        // wait on a semaphore (BLOCK indefinitely)
        TH_sema.wait();

        TH_mutex.lock(QP::QXTHREAD_NO_TIMEOUT); // lock the mutex
        QP::QXThread::delay(5U);  // wait more (BLOCK)
        TH_mutex.unlock();
    }
}

} // namespace APP
