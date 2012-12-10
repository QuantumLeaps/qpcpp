//////////////////////////////////////////////////////////////////////////////
// Product:  QF/C++ port to Win32
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Jun 30, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qf_pkg.h"
#include "qassert.h"

#include <stdio.h>

QP_BEGIN_

Q_DEFINE_THIS_MODULE("qf_port")

// Global objects ------------------------------------------------------------
CRITICAL_SECTION l_win32CritSect;

// Local objects -------------------------------------------------------------
static DWORD WINAPI thread_function(LPVOID arg);
static DWORD l_tickMsec = 10U;    // clock tick in msec (argument for Sleep())
static bool l_running = false;

//............................................................................
void QF::init(void) {
    InitializeCriticalSection(&l_win32CritSect);
}
//............................................................................
void QF_enterCriticalSection(void) {
    EnterCriticalSection(&l_win32CritSect);
}
//............................................................................
void QF_leaveCriticalSection(void) {
    LeaveCriticalSection(&l_win32CritSect);
}
//............................................................................
void QF::stop(void) {
    l_running = false;
}
//............................................................................
void QF::thread_(QActive *act) {
    do {                  // loop until m_thread is cleared in QActive::stop()
        QEvt const *e = act->get_();                       // wait for event
        act->dispatch(e);     // dispatch to the active object's state machine
        gc(e);          // check if the event is garbage, and collect it if so
    } while (act->m_thread != NULL);
    QF::remove_(act);             // remove this object from any subscriptions
    CloseHandle(act->m_osObject);                      // cleanup the OS event
}
//............................................................................
static DWORD WINAPI thread_function(LPVOID me) {         // for CreateThread()
    QF::thread_(static_cast<QActive *>(me));
    return 0;                                                // return success
}
//............................................................................
int16_t QF::run(void) {
    l_running = true;
    onStartup();                                           // startup callback

               // raise the priority of this (main) thread to tick more timely
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    do {
        Sleep(l_tickMsec);                       // wait for the tick interval
        QF_onClockTick();         // clock tick callback (must call QF_TICK())
    } while (l_running);
    onCleanup();                                           // cleanup callback
    QS_EXIT();                                  // cleanup the QSPY connection
    //DeleteCriticalSection(&l_win32CritSect);
    return static_cast<int16_t>(0);                          // return success
}
//............................................................................
void QF_setTickRate(uint32_t ticksPerSec) {
    l_tickMsec = 1000UL / ticksPerSec;
}
//............................................................................
void QActive::start(uint8_t prio,
                    QEvt const *qSto[], uint32_t qLen,
                    void *stkSto, uint32_t stkSize,
                    QEvt const *ie)
{
    Q_REQUIRE((qSto != (QEvt const **)0)  /* queue storage must be provided */
       && (stkSto == (void *)0));        // Windows allocates stack internally

    m_eQueue.init(qSto, (QEQueueCtr)qLen);
    m_osObject = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_prio = prio;
    QF::add_(this);                     // make QF aware of this active object
    init(ie);                                    // execute initial transition

    // NOTE: if stkSize==0, Windows allocates default stack size
    m_thread = CreateThread(NULL, stkSize, &thread_function, this, 0, NULL);
    Q_ASSERT(m_thread != (HANDLE)0);                 // thread must be created

    int p;
    switch (m_prio) {                   // remap QF priority to Win32 priority
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
//............................................................................
void QActive::stop(void) {
    m_thread = NULL;                           // stop the QActive::run() loop
}

QP_END_
