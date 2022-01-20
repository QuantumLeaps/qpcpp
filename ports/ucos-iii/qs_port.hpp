/// @file
/// @brief QS/C++ port to uC/OS-III and 32-bit CPUs
/// @cond
///***************************************************************************
// Last updated for version 6.6.0
// Last updated on  2019-10-14
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef QS_PORT_HPP
#define QS_PORT_HPP

#define QS_TIME_SIZE        4U
#define QS_OBJ_PTR_SIZE     4U
#define QS_FUN_PTR_SIZE     4U

//****************************************************************************
// NOTE: QS might be used with or without other QP components, in which case
// the separate definitions of the macros QF_CRIT_STAT_TYPE, QF_CRIT_ENTRY,
// and QF_CRIT_EXIT are needed. In this port QS is configured to be used with
// the QF framework, by simply including "qf_port.hpp" *before* "qs.hpp".
//
#include "qf_port.hpp" // use QS with QF

#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    #define QS_CRIT_STAT_    CPU_SR cpu_sr;
    #define QS_CRIT_E_()     CPU_CRITICAL_ENTER()
    #define QS_CRIT_X_()     CPU_CRITICAL_EXIT(); QS_REC_DONE()
#endif // CPU_CFG_CRITICAL_METHOD

#include "qs.hpp"      // QS platform-independent public interface

#endif // QS_PORT_HPP
