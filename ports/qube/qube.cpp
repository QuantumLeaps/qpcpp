//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005-2026 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License, version 3 or later (GPL-3.0-or-later), or,
// alternatively, under the terms of one of the closed-source Quantum Leaps
// commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
//
// NOTE:
// The GPL does NOT permit distributing this code as part of proprietary
// (closed-source) programs. For such use, please contact Quantum Leaps for
// a commercial license, which is an alternative to the GPL designed
// explicitly for closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
#ifndef Q_SPY
    #error Q_SPY must be defined to compile qube.cpp
#endif // Q_SPY

#define QP_IMPL           // this is QP implementation
#include "qp_port.hpp"    // QP port
#include "qp_pkg.hpp"     // QP package-scope interface
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#include "qs_port.hpp"    // QS port
#include "qs_pkg.hpp"     // QS package-scope interface

#include <cstddef>        // for size_t used in "qspy.h"
#include "qspy.h"         // QSPY parser interface (C)

#include "safe_std.h"     // portable "safe" <stdio.h>/<string.h> facilities
#include <cstdlib>        // for std::exit(), std::system()
#include <cstdio>         // for std::fputs(), std::fgets()
#include <cstring>        // for std::strcmp(), std::memset()

// QSPY object dictionary (defined in qspy.c)
extern "C" {
    extern Dictionary QSPY_objDict;
}

namespace { // unnamed namespace

Q_DEFINE_THIS_MODULE("qube")

QP::QActive *l_currAO;
char l_currAO_name[QS_DNAME_LEN_MAX];

std::uint8_t const l_qube = 0U; // QS object representing Qube (sender)

char l_line[QS_LINE_LEN_MAX]; // last line entered by the user
char l_cmd[QS_DNAME_LEN_MAX]; // last command part of the line

char const *HELP_STR =
    "Commands:\n"
    "<Enter>      exit\n"
    "?            help\n"
    "help         help\n"
    "curr <name>  set the current AO\n"
    "SIG [params] dispatch a given event to the current AO";

// terminal colors
#define B_BLUE     "\x1b[44m"
#define F_GRAY     "\x1b[30;1m"
#define F_YELLOW   "\x1b[33m"
#define F_BYELLOW  "\x1b[33;1m"

#define COLOR_DFLT "\x1b[K\x1b[0m"
#define COLOR_APP  B_BLUE F_BYELLOW

//............................................................................
bool cmd_evt(char const * const cmd, char const * const params) {
    QP::QSignal const sig = static_cast<QP::QSignal>(
        QSPY_findSig(cmd, static_cast<ObjType>(
            reinterpret_cast<std::uintptr_t>(l_currAO))));
    if (sig != 0U) {
        if (l_currAO != nullptr) {
            QP::QEvt const * const e = QP::Qube::onGenEvt(sig, params);
            if (e != nullptr) {
                l_currAO->POST(e, &l_qube);
            }
            else {
                FPRINTF_S(stderr, "ERROR: %s\n",
                          "Event could not be generated");
            }
        }
        else {
            FPRINTF_S(stderr, "ERROR: %s\n",
                      "No current Active Object found");
        }
    }
    else {
        FPRINTF_S(stderr, "ERROR: Dictionary for Event-sig=%s not found\n",
                  cmd);
    }
    return true;
}
//............................................................................
bool cmd_help(char const * const cmd, char const * const params) {
    Q_UNUSED_PAR(cmd);
    Q_UNUSED_PAR(params);

    PRINTF_S("\n%s", HELP_STR);
    return true;
}
//............................................................................
bool cmd_curr(char const * const cmd, char const * const params) {
    Q_UNUSED_PAR(cmd);

    KeyType const key = QSPY_findObj(params);
    if (key != KEY_NOT_FOUND) {
        l_currAO = reinterpret_cast<QP::QActive *>(
            static_cast<std::uintptr_t>(key));
        STRNCPY_S(l_currAO_name, sizeof(l_currAO_name), params);
    }
    else {
        FPRINTF_S(stderr,
                  "ERROR: Dictionary for Active Object '%s' not found\n",
                  params);
    }
    return true;
}
//............................................................................
void parse_input() {
    static struct {
        char const *name;
        bool (*handler)(char const * const cmd, char const * const params);
    } const commands[] = {
        { "A..Z",    &cmd_evt    }, // not used directly
        { "?",       &cmd_help   },
        { "help",    &cmd_help   },
        { "curr",    &cmd_curr   },
    };
    char *lin = l_line; // pointer within the current line
    char *cmd = l_cmd;  // pointer within the current command

    // find the beginning of parameters in the current line
    // copy over the command part into l_cmd[].
    for (;
         (*lin != '\n') && (lin < &l_line[Q_DIM(l_line) - 1U])
             && (cmd < &l_cmd[Q_DIM(l_cmd)]);
         ++lin, ++cmd)
    {
        if ((*lin == ' ') || (*lin == ',')) { // separator?
            ++lin; // skip the separator
            break;
        }
        else {
            *cmd = *lin;
        }
    }
    *cmd = '\0';   // zero-terminate the command
    char const * const params = lin; // start of the parameters

    // scan to the end of line and replace the terminating '\n' with '\0'
    for (; (*lin != '\n') && (lin < &l_line[Q_DIM(l_line) - 1U]); ++lin) {
    }
    *lin = '\0'; // zero-terminate

    bool ok = false;
    if (('A' <= l_cmd[0]) && (l_cmd[0] <= 'Z')) { // A..Z ?
        STRNCPY_S(cmd, static_cast<std::size_t>(&l_cmd[Q_DIM(l_cmd)] - cmd),
                  "_SIG");
        ok = cmd_evt(l_cmd, params);
    }
    else { // other command
        // search existing command (past cmd_evt)...
        for (std::size_t i = 1U; i < Q_DIM(commands); ++i) {
            if (std::strcmp(commands[i].name, l_cmd) == 0) {
                ok = (*commands[i].handler)(l_cmd, params);
                break;
            }
        }
    }
    if (!ok) {
        FPRINTF_S(stderr, "ERROR: command '%s' not recognized\n", l_cmd);
        PRINTF_S("\n%s\n", HELP_STR);
    }
}

} // unnamed namespace

namespace QP {

// Global objects ============================================================
QPSet QF_readySet_;
std::uint_fast8_t QF_intLock_;

//............................................................................
void Qube::setAO(QActive * const ao) noexcept {
    l_currAO = ao;
}

// QF functions ==============================================================
namespace QF {

//............................................................................
void QF_INIT_CFGCHK_() {
#ifdef _WIN32
    std::system("color"); // to support color output
#endif

    PRINTF_S("Qube %s (c) state-machine.com\n\n%s\n\n",
             QP_VERSION_STR, HELP_STR);

    QF_readySet_.setEmpty();
    l_currAO_name[0] = '\0';

    std::fputs(COLOR_APP, stdout);

    // initialize the QS software tracing...
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }
    // function dictionaries for the standard API
    QS_FUN_DICTIONARY(&QActive::post_);
    QS_FUN_DICTIONARY(&QActive::postLIFO);
    QS::obj_dict_pre_(&l_qube, "Qube"); // the event sender
}
//............................................................................
void stop() {
    QS::onReset();
}
//............................................................................
void onCleanup() {
    std::fputs(COLOR_DFLT "\n", stdout);
    PRINTF_S("%s\n", "Qube exit");
}
//............................................................................
int_t run() {
    QS_CRIT_STAT
    QS_CRIT_ENTRY();

    // produce the QS_QF_RUN trace record
    QS_BEGIN_PRE(QS_QF_RUN, 0U)
    QS_END_PRE()

    QS_CRIT_EXIT();

    // handle all events posted so far
    // NOTE: lambda inside QF::run() for access to the private QActive data
    auto const handleEvts = []() {
        while (QF_readySet_.notEmpty()) {
            std::uint_fast8_t const p = QF_readySet_.findMax();
            QActive * const a = QActive_registry_[p];

            QEvt const * const e = a->get_(); // queue not empty
            QS_FLUSH();
            a->dispatch(e, a->getPrio()); // virtual call
            QS_FLUSH();
#if (QF_MAX_EPOOL > 0U)
            gc(e); // check if the event is garbage, and collect it if so
            QS_FLUSH();
#endif
            if (a->m_eQueue.isEmpty()) { // empty queue?
                QF_readySet_.remove(p);
            }
        }
    };
    handleEvts();
    QS_FLUSH(); // output the startup trace (e.g., initial tran.) now

    if (l_currAO == nullptr) { // current AO not set?
        // take the highest-priority registered AO
        for (std::uint_fast8_t p = QF_MAX_ACTIVE; p != 0U; --p) {
            if (QActive_registry_[p] != nullptr) {
                l_currAO = QActive_registry_[p];
                break;
            }
        }
    }
    // still not found?
    if (l_currAO != nullptr) {
        char const * const name = Dictionary_get(&QSPY_objDict,
            static_cast<KeyType>(reinterpret_cast<std::uintptr_t>(l_currAO)),
            nullptr);
        if (name != nullptr) {
            STRNCPY_S(l_currAO_name, sizeof(l_currAO_name), name);
        }
    }
    else {
        STRNCPY_S(l_currAO_name, sizeof(l_currAO_name), "?");
        PRINTF_S("%s\n", "No current Active Object found");
    }

    std::fputs(COLOR_DFLT "\n", stdout);

    // the event-loop...
    for (;;) {
        PRINTF_S("%s>", l_currAO_name);
        if (std::fgets(l_line, sizeof(l_line), stdin) != nullptr) {
            if (l_line[0] == '\n') { // <Enter>?
                break;
            }
            parse_input();

            std::fputs(COLOR_APP, stdout);
            handleEvts();
            std::fputs(COLOR_DFLT "\n", stdout);
        }
        else { // end of input (e.g., input redirected from a file)
            break;
        }
    }
    onCleanup();
    return 0; // return no error
}

} // namespace QF

//============================================================================
void QActive::start(QPrioSpec const prioSpec,
    QEvtPtr * const qSto, std::uint_fast16_t const qLen,
    void * const stkSto, std::uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    m_prio  = static_cast<std::uint8_t>(prioSpec & 0xFFU); // QF-priority
    m_pthre = static_cast<std::uint8_t>(prioSpec >> 8U); // preempt.-thre.
    register_(); // make QF aware of this active object

    m_eQueue.init(qSto, qLen); // initialize the built-in queue

    this->init(par, m_prio); // the top-most initial tran. (virtual)
}
//............................................................................
#ifdef QACTIVE_CAN_STOP
void QActive::stop() {
    if (QActive_subscrList_ != nullptr) {
        unsubscribeAll(); // unsubscribe from all events
    }
    unregister_(); // un-register this active object
}
#endif // def QACTIVE_CAN_STOP

//============================================================================
bool QS::onStartup(void const * arg) {
    Q_UNUSED_PAR(arg);

    // initialize the QS transmit buffer
    static std::uint8_t qsBuf[4 * 1024]; // buffer for QS-TX channel
    initBuf(qsBuf, sizeof(qsBuf));

    // QS configuration options for this application...
    QSpyConfig config;
    std::memset(&config, 0, sizeof(config));
    config.qpVersion    = QP_VERSION;
    config.qpType       = 2U; // QP/C++
    config.qpDate       = 0U; // unknown
    config.endianness   = 0U; // little
    config.objPtrSize   = QS_OBJ_PTR_SIZE;
    config.funPtrSize   = QS_FUN_PTR_SIZE;
    config.tstampSize   = QS_TIME_SIZE;
    config.sigSize      = 2U;
    config.evtSize      = 2U; // QF_EVENT_SIZ_SIZE fixed at 2
    config.queueCtrSize = QF_EQUEUE_CTR_SIZE;
    config.poolCtrSize  = QF_MPOOL_CTR_SIZE;
    config.poolBlkSize  = 2U; // QF_MPOOL_SIZ_SIZE fixed at 2
    config.tevtCtrSize  = QF_TIMEEVT_CTR_SIZE;

    QSPY_config(&config, nullptr); // no custom parser function

    return true; // success
}
//............................................................................
void QS::onCleanup() {
}
//............................................................................
void QS::onReset() {
    QF::onCleanup();
    std::exit(0);
}
//............................................................................
// NOTE:
// No critical section in QS::onFlush() to avoid nesting of critical sections
// in case QS::onFlush() is called from Q_onError().
void QS::onFlush() {
    for (;;) {
        std::uint16_t nBytes = 1024U;
        std::uint8_t const * const block = getBlock(&nBytes);
        if (block != nullptr) {
            QSPY_parse(block, nBytes);
        }
        else {
            break;
        }
    }
}
//............................................................................
QSTimeCtr QS::onGetTime() {
    static QSTimeCtr time_ctr = 0U;
    ++time_ctr;
    return time_ctr;
}

} // namespace QP

//============================================================================
extern "C" {

//............................................................................
// callback from the QSPY parser (C code) to print the last line of output
void QSPY_onPrintLn(void) {
    PRINTF_S("%s\n", QSPY_line);
}
//............................................................................
Q_NORETURN Q_onError(char const * const module, int_t const id) {
    FPRINTF_S(stderr, "ERROR in %s:%d", module, id);
    QS_ASSERTION(module, id, 0U); // report assertion to QS
    QP::QF::onCleanup();
    QS_EXIT();
    std::exit(-1);
}

} // extern "C"
