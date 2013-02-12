//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++
// Last Updated for Version: 4.5.04
// Date of the Last Update:  Feb 10, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
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
#ifndef qevt_h
#define qevt_h

/// \file
/// \ingroup qep qf qk
/// \brief QEvt class and basic macros used by all QP components.
///
/// This header file must be included, perhaps indirectly, in all modules
/// (*.cpp files) that use any component of QP/C++ (such as QEP, QF, or QK).

//////////////////////////////////////////////////////////////////////////////
/// \brief The current QP version number
///
/// \return version of the QP as a hex constant constant 0xXYZZ, where X is
/// a 1-digit major version number, Y is a 1-digit minor version number, and
/// ZZ is a 2-digit release number.
#define QP_VERSION      0x4504U

#ifndef Q_ROM
    /// \brief Macro to specify compiler-specific directive for placing a
    /// constant object in ROM.
    ///
    /// Many compilers for Harvard-architecture MCUs provide non-standard
    /// extensions to support placement of objects in different memories.
    /// In order to conserve the precious RAM, QP uses the Q_ROM macro for
    /// all constant objects that can be allocated in ROM.
    ///
    /// To override the following empty definition, you need to define the
    /// Q_ROM macro in the qep_port.h header file. Some examples of valid
    /// Q_ROM macro definitions are: __code (IAR 8051 compiler), code (Keil
    /// Cx51 compiler), PROGMEM (gcc for AVR), __flash (IAR for AVR).
    #define Q_ROM
#endif
#ifndef Q_ROM_VAR            // if NOT defined, provide the default definition
    /// \brief Macro to specify compiler-specific directive for accessing a
    /// constant object in ROM.
    ///
    /// Many compilers for MCUs provide different size pointers for
    /// accessing objects in various memories. Constant objects allocated
    /// in ROM (see #Q_ROM macro) often mandate the use of specific-size
    /// pointers (e.g., far pointers) to get access to ROM objects. The
    /// macro Q_ROM_VAR specifies the kind of the pointer to be used to access
    /// the ROM objects.
    ///
    /// To override the following empty definition, you need to define the
    /// Q_ROM_VAR macro in the qep_port.h header file. An example of valid
    /// Q_ROM_VAR macro definition is: __far (Freescale HC(S)08 compiler).
    #define Q_ROM_VAR
#endif
#ifndef Q_ROM_BYTE
    /// \brief Macro to access a byte allocated in ROM
    ///
    /// Some compilers for Harvard-architecture MCUs, such as gcc for AVR, do
    /// not generate correct code for accessing data allocated in the program
    /// space (ROM). The workaround for such compilers is to explictly add
    /// assembly code to access each data element allocated in the program
    /// space. The macro Q_ROM_BYTE() retrieves a byte from the given ROM
    /// address.
    ///
    /// The Q_ROM_BYTE() macro should be defined for the compilers that
    /// cannot handle correctly data allocated in ROM (such as the gcc).
    /// If the macro is left undefined, the default definition simply returns
    /// the argument and lets the compiler generate the correct code.
    #define Q_ROM_BYTE(rom_var_)   (rom_var_)
#endif

#ifndef Q_SIGNAL_SIZE
    /// \brief The size (in bytes) of the signal of an event. Valid values:
    /// 1, 2, or 4; default 1
    ///
    /// This macro can be defined in the QEP port file (qep_port.h) to
    /// configure the ::QSignal type. When the macro is not defined, the
    /// default of 1 byte is chosen.
    #define Q_SIGNAL_SIZE 2
#endif

#ifndef Q_NNAMESPACE

    /// \brief begin of the namespace QP
    ///
    /// \note defines to nothing if #Q_USE_NAMESPACE is undefined
    #define QP_BEGIN_  namespace QP {

    /// \brief end of the namespace QP
    ///
    /// \note defines to nothing if #Q_USE_NAMESPACE is undefined
    #define QP_END_    }

    /// \brief namespace QP prefix
    ///
    /// \note defines to nothing if #Q_USE_NAMESPACE is undefined
    #define QP_        QP::

#else

    #define QP_BEGIN_
    #define QP_END_
    #define QP_

#endif

QP_BEGIN_

#if (Q_SIGNAL_SIZE == 1)
    typedef uint8_t QSignal;
#elif (Q_SIGNAL_SIZE == 2)
    /// \brief QSignal represents the signal of an event.
    ///
    /// The relationship between an event and a signal is as follows. A signal
    /// in UML is the specification of an asynchronous stimulus that triggers
    /// reactions [<A HREF="http://www.omg.org/docs/ptc/03-08-02.pdf">UML
    /// document ptc/03-08-02</A>], and as such is an essential part of an
    /// event. (The signal conveys the type of the occurrence-what happened?)
    /// However, an event can also contain additional quantitative information
    /// about the occurrence in form of event parameters. Please refer to the
    typedef uint16_t QSignal;
#elif (Q_SIGNAL_SIZE == 4)
    typedef uint32_t QSignal;
#else
    #error "Q_SIGNAL_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

#ifdef Q_EVT_CTOR               // Provide the constructor for the QEvt class?

    //////////////////////////////////////////////////////////////////////////
    /// \brief QEvt base class.
    ///
    /// QEvt represents events without parameters and serves as the base class
    /// for derivation of events with parameters.
    ///
    /// The following example illustrates how to add an event parameter by
    /// inheriting from the QEvt class.
    /// \include qep_qevent.cpp
    class QEvt {
    public:
        QSignal sig;                         ///< signal of the event instance

        QEvt(QSignal const s)           // poolId_ intentionally uninitialized
          : sig(s) {}

#ifdef Q_EVT_VIRTUAL
        virtual ~QEvt() {}                               // virtual destructor
#endif

    private:
        uint8_t poolId_;                     ///< pool ID (0 for static event)
        uint8_t refCtr_;                                ///< reference counter

        friend class QF;
        friend class QTimeEvt;
        friend uint8_t QF_EVT_POOL_ID_ (QEvt const * const e);
        friend uint8_t QF_EVT_REF_CTR_ (QEvt const * const e);
        friend void QF_EVT_REF_CTR_INC_(QEvt const * const e);
        friend void QF_EVT_REF_CTR_DEC_(QEvt const * const e);
    };

#else                                    // QEvt is a POD (Plain Old Datatype)

    struct QEvt {
        QSignal sig;                         ///< signal of the event instance
        uint8_t poolId_;                     ///< pool ID (0 for static event)
        uint8_t refCtr_;                                ///< reference counter
    };

#endif                                                           // Q_EVT_CTOR

QP_END_

//////////////////////////////////////////////////////////////////////////////
/// helper macro to calculate static dimension of a 1-dim array \a array_
#define Q_DIM(array_) (sizeof(array_) / sizeof(array_[0]))

//////////////////////////////////////////////////////////////////////////////
// typedefs for basic numerical types; MISRA-C++ 2008 rule 3-9-2(req).

/// \brief typedef for character strings.
///
/// This typedef specifies character type for exclusive use in character
/// strings. Use of this type, rather than plain 'char', is in compliance
/// with the MISRA-C 2004 Rules 6.1(req), 6.3(adv).
///
typedef char char_t;

/// typedef for 32-bit IEEE 754 floating point numbers
typedef float float32_t;

/// typedef for 64-bit IEEE 754 floating point numbers
typedef double float64_t;

/// typedef for enumerations used for event signals
typedef int enum_t;

/// \brief Perform cast from unsigned integer \a uint_ to pointer
/// of type \a type_.
///
/// This macro encapsulates the cast to (type_ *), which QP ports or
/// application might use to access embedded hardware registers.
/// Such uses can trigger PC-Lint "Note 923: cast from int to pointer"
/// and this macro helps to encapsulate this deviation.
///
#define Q_UINT2PTR_CAST(type_, uint_)  (reinterpret_cast<type_ *>(uint_))

/// \brief Initializer of static constant QEvt instances
///
/// This macro encapsulates the ugly casting of enumerated signals
/// to QSignal and constants for QEvt.poolID and QEvt.refCtr_.
///
#define QEVT_INITIALIZER(sig_) { static_cast<QP_ QSignal>(sig_), \
    static_cast<uint8_t>(0), static_cast<uint8_t>(0) }

//////////////////////////////////////////////////////////////////////////////
// facilities for backwards compatibility
//
#ifndef Q_NQEVENT
//    typedef struct QEvt QEvent;
    #define QEvent QEvt
#endif

#endif                                                               // qevt_h
