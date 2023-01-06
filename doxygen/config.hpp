//! @file
//! @brief Various macros for configuring and porting QP/C++

//! The preprocessor switch to disable checking assertions
//!
//! @description
//! When defined, Q_NASSERT disables the following macros #Q_ASSERT,
//! #Q_REQUIRE, #Q_ENSURE, #Q_INVARIANT, #Q_ERROR as well as
//! #Q_ASSERT_ID, #Q_REQUIRE_ID, #Q_ENSURE_ID, #Q_INVARIANT_ID, and
//! #Q_ERROR_ID do NOT evaluate the test condition passed as the
//! argument to these macros.
//!
//! @note One notable exception is the macro #Q_ALLEGE, that still
//! evaluates the test condition, but does not report assertion
//! failures when the switch Q_NASSERT is defined.
#define Q_NASSERT

//! Enable the QActive::stop() API in the QF port.
//!
//! @description
//! Defining this macro enables the QActive::stop() API in a given port.
//! This feature should be used with caution, as stopping and re-starting
//! active objects **cleanly** can be tricky.
//!
#define QF_ACTIVE_STOP

//! The preprocessor switch to activate the constructor in QP::QEvt.
//!
//! @description
//! When #Q_EVT_CTOR is defined (typically in the qep_port.hpp header file),
//! QP::QEvt becomes a class with constructor. More importantly, the
//! subclasses of QP::QEvt (custom events) can have non-default constructors.
//! These constructors are then called when events are created (e.g.,
//! with Q_NEW())
//!
//! @sa #Q_EVT_VIRTUAL
#define Q_EVT_CTOR

//! The preprocessor switch to activate the virtual destructor in QP::QEvt.
//!
//! @description
//! This macro only works when #Q_EVT_CTOR is also defined. When also
//! #Q_EVT_VIRTUAL is defined (typically in the qep_port.hpp header
//! file), QP::QEvt becomes a class with a constructor and a virtual
//! destructor. More importantly, the subclasses of QP::QEvt (custom events)
//! can have (virtual) destructors. These destructors are then invoked
//! before recycling the event with QP::QF::gc().
#define Q_EVT_VIRTUAL

//! The preprocessor switch to activate the QS software tracing
//! instrumentation in the code
//!
//! @description
//! When defined, Q_SPY activates the QS software tracing instrumentation.
//! When Q_SPY is not defined, the QS instrumentation in the code does
//! not generate any code.
#define Q_SPY

//! This macro enables calling the QK context-switch callback
//! QF_onContextSw()
#define QF_ON_CONTEXT_SW

//! Macro that should be defined (typically on the compiler's command line)
//! in the Win32-GUI applications that use the @ref win32 or @ref win32-qv
//! ports.
#define WIN32_GUI
