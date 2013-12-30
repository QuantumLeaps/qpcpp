//****************************************************************************
// Product: QP/C++
// Last Updated for Version: 5.2.0
// Date of the Last Update:  Dec 27, 2013
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
#ifndef qp_port_h
#define qp_port_h

/// \file
/// \ingroup qep qf qk qs
/// \brief QP/C++ public interface.
///
/// This header file must be included directly or indirectly
/// in all application modules (*.cpp files) that use QP/C++.

#ifndef QP_API_VERSION

/// \brief Macro that specifies the backwards compatibility with the
/// QP/C++ API version.
///
/// For example, QP_API_VERSION=450 will cause generating the compatibility
/// layer with QP/C++ version 4.5.0 and newer, but not older than 4.5.0.
/// QP_API_VERSION=0 causes generation of the compatibility layer "from the
/// begining of time", which is the maximum backwards compatibilty. This is
/// the default.
///
/// Conversely, QP_API_VERSION=9999 means that no compatibility layer should
/// be generated. This setting is useful for checking if an application
/// complies with the latest QP/C++ API.
///
#define QP_API_VERSION 0

#endif                                               // #ifndef QP_API_VERSION


#include "qf_port.h"                    // QF/C++ port from the port directory
#include "qassert.h"                                          // QP assertions

#ifdef Q_SPY                                      // software tracing enabled?
    #include "qs_port.h"                // QS/C++ port from the port directory
#else
    #include "qs_dummy.h"                 // QS/C++ dummy (inactive) interface
#endif

// QP API compatibility layer ------------------------------------------------
#if (QP_API_VERSION < 500)

/// \brief Deprecated macro for odd 8-bit CPUs.
#define Q_ROM_VAR


#ifdef Q_SPY

    /// \brief Deprecated interface for backwards compatibility.
    #define postFIFO(e_, sender_) POST((e_), (sender_))

    /// \brief Deprecated interface for backwards compatibility.
    #define publish(e_, sender_)  PUBLISH((e_), (sender_))

    /// \brief Deprecated interface defined for backwards compatibility
    #define tick(sender_) TICK_X(static_cast<uint8_t>(0), (sender_))

#else

    #define postFIFO(e_)  POST((e_), dummy)
    #define publish(e_)   PUBLISH((e_), dummy)
    #define tick()        TICK_X(static_cast<uint8_t>(0), dummy)

#endif


/// \brief Deprecated macro for generating QS-Reset trace record.
#define QS_RESET() ((void)0)

//............................................................................
#if (QP_API_VERSION < 450)

/// \brief deprecated typedef for backwards compatibility
namespace QP {

typedef QEvt QEvent;

}

#endif                                                 // QP_API_VERSION < 450
#endif                                                 // QP_API_VERSION < 500
//----------------------------------------------------------------------------

#endif                                                            // qp_port_h
