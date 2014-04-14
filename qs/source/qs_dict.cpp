/// \file
/// \brief QP::QS::sig_dict(), QP::QS::obj_dict(), QP::QS::fun_dict(), and
/// QP::QS::usr_dict() definitions.
/// \ingroup qs
/// \cond
///***************************************************************************
/// Product: QS/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-11
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#define QP_IMPL           // this is QF/QK implementation
#include "qs_port.h"      // QS port

namespace QP {

//****************************************************************************
/// \note This function is only to be used through macro QS_SIG_DICTIONARY()
///
void QS::sig_dict(enum_t const sig, void const * const obj,
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_SIG_DICT));
    QS_SIG_(static_cast<QSignal>(sig));
    QS_OBJ_(obj);
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// \note This function is only to be used through macro QS_OBJ_DICTIONARY()
///
void QS::obj_dict(void const * const obj,
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_OBJ_DICT));
    QS_OBJ_(obj);
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// \note This function is only to be used through macro QS_FUN_DICTIONARY()
///
void QS::fun_dict(void (* const fun)(void),
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_FUN_DICT));
    QS_FUN_(fun);
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

//****************************************************************************
/// \note This function is only to be used through macro QS_USR_DICTIONARY()
///
void QS::usr_dict(enum_t const rec,
                  char_t const Q_ROM * const name)
{
    QS_CRIT_STAT_
    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_USR_DICT));
    QS_U8_(static_cast<uint8_t>(rec));
    QS_STR_ROM_(name);
    endRec();
    QS_CRIT_EXIT_();
    onFlush();
}

} // namespace QP

