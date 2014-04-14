//****************************************************************************
// Product:  QF/C++ port to Win32
// Last updated for version 5.3.0
// Last updated on  2014-03-04
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) Quantum Leaps, www.state-machine.com.
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
// Web:   www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

// Global objects ************************************************************
CRITICAL_SECTION l_win32CritSect;

// Local objects *************************************************************
static DWORD WINAPI thread_function(LPVOID arg);
static DWORD l_tickMsec = 10U; // clock tick in msec (argument for Sleep())
static bool l_running;

static DWORD WINAPI thread_function(LPVOID arg);

//****************************************************************************
void QF::init(void) {
    InitializeCriticalSection(&l_win32CritSect);
}
//****************************************************************************
void QF_enterCriticalSection(void) {
    EnterCriticalSection(&l_win32CritSect);
}
//****************************************************************************
void QF_leaveCriticalSection(void) {
    LeaveCriticalSection(&l_win32CritSect);
}
//****************************************************************************
void QF::stop(void) {
    l_running = false;
}
//****************************************************************************
void QF::thread_(QActive *act) {
    // loop until m_thread is cleared in QActive::stop()
    do {
        QEvt const *e = act->get_(); // wait for event
        act->dispatch(e); // dispatch to the active object's state machine
        gc(e); // check if the event is garbage, and collect it if so
    } while (act->m_thread != NULL);
    QF::remove_(act); // remove this object from any subscriptions
    CloseHandle(act->m_osObject); // cleanup the OS event
}
//****************************************************************************
int_t QF::run(void) {
    l_running = true;
    onStartup();  // startup callback

    // if necessary, raise the priority of this (main) thread
    // to tick more timely
    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    do {
        Sleep(l_tickMsec);  // wait for the tick interval
        QF_onClockTick();   // clock tick callback (must call QF_TICK_X())
    } while (l_running);

    onCleanup();            // cleanup callback
    QS_EXIT();              // cleanup the QSPY connection
    //DeleteCriticalSection(&l_win32CritSect);
    return static_cast<int_t>(0); // return success
}
//****************************************************************************
void QF_setTickRate(uint32_t ticksPerSec) {
    l_tickMsec = 1000UL / ticksPerSec;
}
//****************************************************************************
void QActive::start(uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t stkSize,
                    QEvt const *ie)
{
    Q_REQUIRE((qSto != static_cast<QEvt const **>(0)) /*valid queue storage */
       && (stkSto == (void *)0)); // Windows allocates stack internally

    m_prio = prio;  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO

    // create the Win32 event object to block the QEQueue
    m_osObject = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_eQueue.init(qSto, qLen);

    this->init(ie); // execute initial transition (virtual call)

    // stack size not provided?
    if (stkSize == 0U) {
        stkSize = 1024U; // NOTE: will be rounded up to the nearest page
    }
    m_thread = CreateThread(NULL, stkSize, &thread_function, this, 0, NULL);
    Q_ASSERT(m_thread != NULL); // thread must be created

    int p;
    switch (m_prio) { // remap QF priority to Win32 priority
        case 1:
            p = THREAD_PRIORITY_IDLE;
            break;
        case 2:
            p = THREAD_PRIORITY_LOWEST;
            break;
        case 3:
            p = THREAD_PRIORITY_BELOW_NORMAL;
            break;
        case (QF_MAX_ACTIVE - 1):
            p = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        case QF_MAX_ACTIVE:
            p = THREAD_PRIORITY_HIGHEST;
            break;
        default:
            p = THREAD_PRIORITY_NORMAL;
            break;
    }
    SetThreadPriority(m_thread, p);
}
//****************************************************************************
void QActive::stop(void) {
    m_thread = NULL;  // stop the QActive::run() loop
}
//****************************************************************************
static DWORD WINAPI thread_function(LPVOID me) { // for CreateThread()
    QF::thread_(static_cast<QActive *>(me));
    return 0; // return success
}

} // namespace QP
