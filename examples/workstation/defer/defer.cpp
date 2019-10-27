//****************************************************************************
// Product: Deferred Event state pattern example
// Last Updated for Version: 6.5.0
// Date of the Last Update:  2019-03-25
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2002-2019 Quantum Leaps, LLC. All rights reserved.
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
// https://www.state-machine.com
// mailto:info@state-machine.com
//****************************************************************************
#include "qpcpp.hpp"
#include "bsp.hpp"

#include <stdio.h> // this example uses printf() to report status

Q_DEFINE_THIS_FILE

using namespace QP;

//............................................................................
enum TServerSignals {
    NEW_REQUEST_SIG = Q_USER_SIG, // the new request signal
    RECEIVED_SIG,   // the request has been received
    AUTHORIZED_SIG, // the request has been authorized
    TERMINATE_SIG   // terminate the application
};
//............................................................................
struct RequestEvt : public QEvt {
    uint8_t ref_num; // reference number of the request
};

class TServer : public QActive { // Transaction Server active object
private:
    QEQueue m_requestQueue; // native QF queue for deferred request events
    QEvt const *m_requestQSto[3]; // storage for the deferred queue buffer
    RequestEvt const *m_activeRequest; // request event being processed

    QTimeEvt m_receivedEvt;    // private time event generator
    QTimeEvt m_authorizedEvt;  // private time event generator

public:
    TServer() // the default ctor
      : QActive(&initial),
        m_receivedEvt(this, RECEIVED_SIG, 0U),
        m_authorizedEvt(this, AUTHORIZED_SIG, 0U)
    {
        m_requestQueue.init(m_requestQSto, Q_DIM(m_requestQSto));
    }

private:
    // hierarchical state machine ...
    Q_STATE_DECL(initial);
    Q_STATE_DECL(idle);
    Q_STATE_DECL(busy);
    Q_STATE_DECL(receiving);
    Q_STATE_DECL(authorizing);
    Q_STATE_DECL(final);
};

// HSM definition ------------------------------------------------------------
Q_STATE_DEF(TServer, initial) {
    (void)e; // unused paramteter
    // no active request yet
    m_activeRequest = static_cast<RequestEvt const *>(0);
    return tran(&idle);
}
//............................................................................
Q_STATE_DEF(TServer, final) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("final-ENTRY;\nBye!Bye!\n");
            QF::stop(); // terminate the application
            return Q_RET_HANDLED;
        }
    }
    return super(&top);
}
//............................................................................
Q_STATE_DEF(TServer, idle) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            printf("idle-ENTRY;\n");
            // recall the request from the requestQueue
            if (recall(&m_requestQueue)) {
                printf("Request recalled\n");
            }
            else {
                printf("No deferred requests\n");
            }
            return Q_RET_HANDLED;
        }
        case NEW_REQUEST_SIG: {
            // create and save a new reference to the request event so that
            // this event will be available beyond this RTC step and won't be
            // recycled.
            Q_NEW_REF(m_activeRequest, RequestEvt);

            printf("Processing request #%d\n",
                   (int)m_activeRequest->ref_num);
            return tran(&receiving);
        }
        case TERMINATE_SIG: {
            return tran(&final);
        }
    }
    return super(&top);
}
//............................................................................
Q_STATE_DEF(TServer, busy) {
    QState status;
    switch (e->sig) {
        case Q_EXIT_SIG: {
            printf("busy-EXIT; done processing request #%d\n",
                   (int)m_activeRequest->ref_num);

            // delete the reference to the active request, because
            // it is now processed.
            Q_DELETE_REF(m_activeRequest);

            status = Q_RET_HANDLED;
            break;
        }
        case NEW_REQUEST_SIG: {
            if (defer(&m_requestQueue, e)) { // could defer?
                printf("Request #%d deferred;\n",
                       (int)((RequestEvt const *)e)->ref_num);
            }
            else {
                printf("Request #%d IGNORED;\n",
                       (int)((RequestEvt const *)e)->ref_num);
                // notify the request sender that his request was denied...
            }
            status = Q_RET_HANDLED;
            break;
        }
        case TERMINATE_SIG: {
            status = tran(&final);
            break;
        }
        default: {
            status = super(&top);
            break;
        }
    }
    return status;
}
//............................................................................
Q_STATE_DEF(TServer, receiving) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // inform about the first stage of processing of the request...
            printf("receiving-ENTRY; active request: #%d\n",
                   (int)m_activeRequest->ref_num);

            // one-shot timeout in 1 second
            m_receivedEvt.armX(BSP_TICKS_PER_SEC, 0U);
            status = Q_RET_HANDLED;
            break;
        }
        case Q_EXIT_SIG: {
            m_receivedEvt.disarm();
            status = Q_RET_HANDLED;
            break;
        }
        case RECEIVED_SIG: {
            status = tran(&authorizing);
            break;
        }
        default: {
            status = super(&busy);
            break;
        }
    }
    return status;
}
//............................................................................
Q_STATE_DEF(TServer, authorizing) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            // inform about the second stage of processing of the request..
            printf("authorizing-ENTRY; active request: #%d\n",
                   (int)m_activeRequest->ref_num);

            // one-shot timeout in 2 seconds
            m_authorizedEvt.armX(2*BSP_TICKS_PER_SEC, 0U);
            return Q_RET_HANDLED;
        }
        case Q_EXIT_SIG: {
            m_authorizedEvt.disarm();
            return Q_RET_HANDLED;
        }
        case AUTHORIZED_SIG: {
            return tran(&idle);
        }
    }
    return super(&busy);
}

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static TServer       l_tserver; // the Transaction Server active object
static QEvt const *l_tserverQSto[10];  // Event queue storage for TServer
static RequestEvt    l_smlPoolSto[10]; // small event pool

//............................................................................
int main(int argc, char *argv[]) {
    printf("Reminder state pattern\nQP version: %s\n"
           "Press n to generate a new request\n"
           "Press ESC to quit...\n",
           QP::versionStr);

    BSP_init(argc, argv); // initialize the BSP

    QF::init(); // initialize the framework and the underlying RTOS/OS


    // publish-subscribe not used, no call to QF::psInit()

    // initialize event pools...
    QF::poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

    // start the active objects...
    l_tserver.start(1, l_tserverQSto, Q_DIM(l_tserverQSto),
                   (void *)0, 0, (QEvt *)0);

    return QF::run(); // run the QF application
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 'n': { // 'n': new request?
            static uint8_t reqCtr = 0U; // count the requests
            RequestEvt *e = Q_NEW(RequestEvt, NEW_REQUEST_SIG);
            e->ref_num = (++reqCtr); // set the reference number
            // post directly to TServer active object
            l_tserver.POST(e, (void *)0); // post directly to TServer AO
            break;
        }
        case '\33': { // ESC pressed?
            static QEvt const terminateEvt = { TERMINATE_SIG, 0U, 0U };
            l_tserver.POST(&terminateEvt, (void *)0);
            break;
        }
    }
}
