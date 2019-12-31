/// @file
/// @brief QP/C++ public interface including backwards-compatibility layer
/// @ingroup qep qf qv qk qxk qs
/// @cond
///***************************************************************************
/// Last updated for version 6.7.0
/// Last updated on  2019-12-21
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
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

#ifndef qpcpp_h
#define qpcpp_h

/// @description
/// This header file must be included directly or indirectly
/// in all application modules (*.cpp files) that use QP/C++.

#ifndef QP_API_VERSION

//! Macro that specifies the backwards compatibility with the
//! QP/C++ API version.
/// @description
/// For example, QP_API_VERSION=540 will cause generating the compatibility
/// layer with QP/C++ version 5.4.0 and newer, but not older than 5.4.0.
/// QP_API_VERSION=0 causes generation of the compatibility layer "from the
/// begining of time", which is the maximum backwards compatibilty. This is
/// the default.@n
/// @n
/// Conversely, QP_API_VERSION=9999 means that no compatibility layer should
/// be generated. This setting is useful for checking if an application
/// complies with the latest QP/C++ API.
#define QP_API_VERSION 0

#endif  // QP_API_VERSION

#include "qf_port.hpp"      // QF/C++ port from the port directory
#include "qassert.h"        // QP assertions
#ifdef Q_SPY                // software tracing enabled?
    #include "qs_port.hpp"  // QS/C++ port from the port directory
#else
    #include "qs_dummy.hpp" // QS/C++ dummy (inactive) interface
#endif

#endif // qpcpp_h

