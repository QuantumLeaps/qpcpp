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

class XThread1 : public QP::QXThread {
private:
    // NOTE: data needed by this thread should be members of
    // the thread class. That way they are in the memory region
    // accessible from this thread.
    std::uint8_t m_foo;

public:
    static XThread1 inst;
    XThread1();

private:
    static void run(QP::QXThread * const thr);
}; // class XThread1

//............................................................................
QP::QXThread * const TH_XThread1 = &XThread1::inst;

XThread1 XThread1::inst;

XThread1::XThread1()
  : QXThread(&run)
{}

//............................................................................
void XThread1::run(QP::QXThread * const thr) {
    // downcast the generic thr pointer to the specific thread
    auto me = static_cast<XThread1 *>(thr);

    QS_OBJ_DICTIONARY(TH_XThread1);
    QS_OBJ_DICTIONARY(TH_XThread1->getTimeEvt());

    // subscribe to the EAT signal (from the application)
    me->subscribe(APP::EAT_SIG);

    for (;;) {
        QP::QEvt const *e = me->queueGet(BSP::TICKS_PER_SEC/4U);
        if (e) {
            TH_sema.signal(); // signal Thread2
            QP::QF::gc(e); // must explicitly recycle the received event!
        }

        TH_mutex.lock(QP::QXTHREAD_NO_TIMEOUT); // lock the mutex
        BSP::ledOn();
        if (TH_mutex.tryLock()) { // exercise the mutex
            // some floating point code to exercise the VFP...
            float volatile x = 1.4142135F;
            x = x * 1.4142135F;
            QP::QXThread::delay(10U);  // BLOCK while holding a mutex
            TH_mutex.unlock();
        }
        TH_mutex.unlock();
        BSP::ledOff();
    }
}

} // namespace APP
