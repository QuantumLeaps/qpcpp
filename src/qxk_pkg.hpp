/// @file
/// @brief Internal (package scope) QXK/C++ interface.
/// @ingroup qxk
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-01-16
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef QXK_PKG_HPP
#define QXK_PKG_HPP

namespace QP {

//! timeout signals
enum QXK_Timeouts : std::uint8_t {
    QXK_DELAY_SIG = Q_USER_SIG,
    QXK_QUEUE_SIG,
    QXK_SEMA_SIG
};

} // namespace QP

//****************************************************************************
extern "C" {

//! initialize the private stack of a given AO
void QXK_stackInit_(void *thr, QP::QXThreadHandler const handler,
             void * const stkSto, std::uint_fast16_t const stkSize) noexcept;

//! called when a thread function returns
void QXK_threadRet_(void) noexcept;

} // extern "C"

//! intertnal macro to encapsulate casting of pointers for MISRA deviations
//
/// @description
/// This macro is specifically and exclusively used for casting pointers
/// that are never de-referenced, but only used for internal bookkeeping and
/// checking (via assertions) the correct operation of the QXK kernel.
/// Such pointer casting is not compliant with MISRA C++ Rule 5-2-7
/// as well as other messages (e.g., PC-Lint-Plus warning 826).
/// Defining this specific macro for this purpose allows to selectively
/// disable the warnings for this particular case.
///
#define QXK_PTR_CAST_(type_, ptr_) (reinterpret_cast<type_>(ptr_))

#include "qf_pkg.hpp"  // QF package-scope interface

#endif // QXK_PKG_HPP
