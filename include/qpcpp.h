/// @file
/// @brief QP/C++ public interface including backwards-compatibility layer
/// @ingroup qep qf qv qk qxk qs
/// @cond
///***************************************************************************
/// Last updated for version 6.2.0
/// Last updated on  2018-03-16
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) 2002-2018 Quantum Leaps. All rights reserved.
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
/// https://www.state-machine.com
/// mailto:info@state-machine.com
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
/// For example, QP_API_VERSION=450 will cause generating the compatibility
/// layer with QP/C++ version 4.5.0 and newer, but not older than 4.5.0.
/// QP_API_VERSION=0 causes generation of the compatibility layer "from the
/// begining of time", which is the maximum backwards compatibilty. This is
/// the default.@n
/// @n
/// Conversely, QP_API_VERSION=9999 means that no compatibility layer should
/// be generated. This setting is useful for checking if an application
/// complies with the latest QP/C++ API.
#define QP_API_VERSION 0

#endif  // QP_API_VERSION

#include "qf_port.h"      // QF/C++ port from the port directory
#include "qassert.h"      // QP assertions
#ifdef Q_SPY              // software tracing enabled?
    #include "qs_port.h"  // QS/C++ port from the port directory
#else
    #include "qs_dummy.h" // QS/C++ dummy (inactive) interface
#endif


/****************************************************************************/
#if (QP_API_VERSION < 540)

/*! @deprecated QFsm state machine;
* instead use: QP::QHsm. Legacy state machines coded in the "QFsm-style" will
* continue to work, but will use the QP::QHsm implementation internally.
* There is no longer any efficiency advantage in using the "QFsm-style"
* state machines.
*
* @note
* For efficiency, the recommended migration path is to use the QP::QMsm
* state machine and the QM modeling tool.
*/
#define QFsm  QHsm

/*! deprecated macro to call in QFsm state-handler when it
* ignores (does not handle) an event (instead use Q_SUPER())
*/
#define Q_IGNORED() (Q_SUPER(&QHsm::top))

// QP API compatibility layer ************************************************
#if (QP_API_VERSION < 500)

//! @deprecated macro for odd 8-bit CPUs.
#define Q_ROM

//! @deprecated macro for odd 8-bit CPUs.
#define Q_ROM_BYTE(rom_var_)   (rom_var_)

//! @deprecated macro for odd 8-bit CPUs.
#define Q_ROM_VAR

#ifdef Q_SPY

    //! @deprecated call to QActive post FIFO operation
    #define postFIFO(e_, sender_) POST((e_), (sender_))

    //! @deprecated call of QF system clock tick (for rate 0)
    #define tick(sender_) TICK_X(static_cast<uint8_t>(0), (sender_))

#else

    #define postFIFO(e_)  POST((e_), dummy)
    #define tick()        TICK_X(static_cast<uint8_t>(0), dummy)

#endif  // Q_SPY

//! @deprecated macro for generating QS-Reset trace record.
#define QS_RESET() ((void)0)

//****************************************************************************
#if (QP_API_VERSION < 450)

namespace QP {

//! deprecated typedef for backwards compatibility
typedef QEvt QEvent;

} // namespace QP

#ifdef Q_SPY
    //! @deprecated call to QF publish operation
    #define publish(e_, sender_)  PUBLISH((e_), (sender_))
#else
    #define publish(e_)   PUBLISH((e_), dummy)
#endif // Q_SPY

#endif // QP_API_VERSION < 450
#endif // QP_API_VERSION < 500
#endif // QP_API_VERSION < 540

#endif // qpcpp_h

