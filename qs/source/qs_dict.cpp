//****************************************************************************
// Product: QS/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 02, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
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
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
#include "qs_pkg.h"

///
/// \file
/// \ingroup qs
/// \brief QS::sig_dict(), QS::obj_dict(), QS::fun_dict(), and QS::usr_dict()
/// implementation

namespace QP {

//............................................................................
void QS::sig_dict(enum_t const sig, void const * const obj,
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint8_t>(QS_SIG_DICT));
    QS_SIG_(static_cast<QSignal>(sig));
    QS_OBJ_(obj);
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}
//............................................................................
void QS::obj_dict(void const * const obj,
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint8_t>(QS_OBJ_DICT));
    QS_OBJ_(obj);
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}
//............................................................................
void QS::fun_dict(void (* const fun)(void),
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint8_t>(QS_FUN_DICT));
    QS_FUN_(fun);
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}
//............................................................................
void QS::usr_dict(enum_t const rec,
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint8_t>(QS_USR_DICT));
    QS_U8_(static_cast<uint8_t>(rec));
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

}                                                              // namespace QP

