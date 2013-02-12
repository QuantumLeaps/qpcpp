//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++
// Last Updated for Version: 4.5.04
// Date of the Last Update:  Jan 16, 2013
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
#ifndef qs_h
#define qs_h

/// \file
/// \ingroup qep qf qk qs
/// \brief QS/C++ platform-independent public interface.
/// This header file must be included directly or indirectly
/// in all modules (*.cpp files) that use QS/C++.

#ifndef Q_SPY
    #error "Q_SPY must be defined to include qs.h"
#endif

#ifndef Q_ROM                      // provide the default if Q_ROM NOT defined
    #define Q_ROM
#endif
#ifndef Q_ROM_VAR              // provide the default if Q_ROM_VAR NOT defined
    #define Q_ROM_VAR
#endif
#ifndef Q_ROM_BYTE            // provide the default if Q_ROM_BYTE NOT defined
    #define Q_ROM_BYTE(rom_var_)   (rom_var_)
#endif

#ifndef QS_TIME_SIZE

    /// \brief The size (in bytes) of the QS time stamp. Valid values: 1, 2,
    /// or 4; default 4.
    ///
    /// This macro can be defined in the QS port file (qs_port.h) to
    /// configure the ::QSTimeCtr type. Here the macro is not defined so the
    /// default of 4 byte is chosen.
    #define QS_TIME_SIZE 4
#endif

//////////////////////////////////////////////////////////////////////////////
QP_BEGIN_

/// \brief Quantum Spy record types.
///
/// The following constants specify the QS record types used in QP components.
/// You can specify your own record types starting from the ::QS_USER offset.
/// Currently, the maximum of all records cannot exceed 256.
/// \sa QS::filterOn()/#QS_FILTER_ON and QS::filterOff()/#QS_FILTER_OFF

enum QSpyRecords {
    QS_QP_RESET,                 ///< reset the QP (start of a new QS session)

    // QEP records
    QS_QEP_STATE_ENTRY,                               ///< a state was entered
    QS_QEP_STATE_EXIT,                                 ///< a state was exited
    QS_QEP_STATE_INIT,         ///< an initial transition was taken in a state
    QS_QEP_INIT_TRAN,           ///< the top-most initial transition was taken
    QS_QEP_INTERN_TRAN,                  ///< an internal transition was taken
    QS_QEP_TRAN,                           ///< a regular transition was taken
    QS_QEP_IGNORED,             ///< an event was ignored (silently discarded)
    QS_QEP_DISPATCH,          ///< an event was dispatched (begin of RTC step)
    QS_QEP_UNHANDLED,               ///< an event was unhandled due to a guard

    // QF records
    QS_QF_ACTIVE_ADD,                ///< an AO has been added to QF (started)
    QS_QF_ACTIVE_REMOVE,         ///< an AO has been removed from QF (stopped)
    QS_QF_ACTIVE_SUBSCRIBE,                  ///< an AO subscribed to an event
    QS_QF_ACTIVE_UNSUBSCRIBE,              ///< an AO unsubscribed to an event
    QS_QF_ACTIVE_POST_FIFO,  ///< an event was posted (FIFO) directly to an AO
    QS_QF_ACTIVE_POST_LIFO,  ///< an event was posted (LIFO) directly to an AO
    QS_QF_ACTIVE_GET, ///< an AO got an event and its queue is still not empty
    QS_QF_ACTIVE_GET_LAST,      ///< an AO got an event and its queue is empty
    QS_QF_EQUEUE_INIT,                     ///< an event queue was initialized
    QS_QF_EQUEUE_POST_FIFO,     ///< an event was posted (FIFO) to a raw queue
    QS_QF_EQUEUE_POST_LIFO,     ///< an event was posted (LIFO) to a raw queue
    QS_QF_EQUEUE_GET,              ///< get an event and queue still not empty
    QS_QF_EQUEUE_GET_LAST,              ///< get the last event from the queue
    QS_QF_MPOOL_INIT,                       ///< a memory pool was initialized
    QS_QF_MPOOL_GET,        ///< a memory block was removed from a memory pool
    QS_QF_MPOOL_PUT,         ///< a memory block was returned to a memory pool
    QS_QF_PUBLISH,       ///< an event was truly published to some subscribers
    QS_QF_RESERVED8,
    QS_QF_NEW,                                         ///< new event creation
    QS_QF_GC_ATTEMPT,                          ///< garbage collection attempt
    QS_QF_GC,                                          ///< garbage collection
    QS_QF_TICK,                                     ///< QF::tick() was called
    QS_QF_TIMEEVT_ARM,                             ///< a time event was armed
    QS_QF_TIMEEVT_AUTO_DISARM,      ///< a time event expired and was disarmed
    QS_QF_TIMEEVT_DISARM_ATTEMPT,///< an attempt to disarm a disarmed QTimeEvt
    QS_QF_TIMEEVT_DISARM,           ///< true disarming of an armed time event
    QS_QF_TIMEEVT_REARM,                         ///< rearming of a time event
    QS_QF_TIMEEVT_POST,      ///< a time event posted itself directly to an AO
    QS_QF_TIMEEVT_CTR,                 ///< a time event counter was requested
    QS_QF_CRIT_ENTRY,                        ///< critical section was entered
    QS_QF_CRIT_EXIT,                          ///< critical section was exited
    QS_QF_ISR_ENTRY,                                   ///< an ISR was entered
    QS_QF_ISR_EXIT,                                     ///< an ISR was exited
    QS_QF_INT_DISABLE,                           ///< interrupts were disabled
    QS_QF_INT_ENABLE,                             ///< interrupts were enabled
    QS_QF_RESERVED4,
    QS_QF_RESERVED3,
    QS_QF_RESERVED2,
    QS_QF_RESERVED1,
    QS_QF_RESERVED0,

    // QK records
    QS_QK_MUTEX_LOCK,                             ///< the QK mutex was locked
    QS_QK_MUTEX_UNLOCK,                         ///< the QK mutex was unlocked
    QS_QK_SCHEDULE,      ///< the QK scheduler scheduled a new task to execute
    QS_QK_RESERVED6,
    QS_QK_RESERVED5,
    QS_QK_RESERVED4,
    QS_QK_RESERVED3,
    QS_QK_RESERVED2,
    QS_QK_RESERVED1,
    QS_QK_RESERVED0,

    // Miscellaneous QS records
    QS_SIG_DIC,                                   ///< signal dictionary entry
    QS_OBJ_DIC,                                   ///< object dictionary entry
    QS_FUN_DIC,                                 ///< function dictionary entry
    QS_USR_DIC,                           ///< user QS record dictionary entry
    QS_RESERVED4,
    QS_RESERVED3,
    QS_RESERVED2,
    QS_RESERVED1,
    QS_RESERVED0,
    QS_ASSERT,                                ///< assertion fired in the code

    // User records
    QS_USER                ///< the first record available for user QS records
};

/// \brief Specification of all QS records for the QS::filterOn() and
/// QS::filterOff()
uint8_t const QS_ALL_RECORDS = static_cast<uint8_t>(0xFF);

/// \brief Constant representing End-Of-Data condition returned from the
/// QS::getByte() function.
uint16_t const QS_EOD  = static_cast<uint16_t>(0xFFFF);


#if (QS_TIME_SIZE == 1)
    typedef uint8_t QSTimeCtr;
    #define QS_TIME_()   (QP_ QS::u8_(QP_ QS::onGetTime()))
#elif (QS_TIME_SIZE == 2)
    typedef uint16_t QSTimeCtr;
    #define QS_TIME_()   (QP_ QS::u16_(QP_ QS::onGetTime()))
#elif (QS_TIME_SIZE == 4)

    /// \brief The type of the QS time stamp
    ///
    /// This type determines the dynamic range of QS time stamps
    typedef uint32_t QSTimeCtr;

    /// \brief Internal macro to output time stamp to the QS record
    #define QS_TIME_()   (QP_ QS::u32_(QP_ QS::onGetTime()))
#else
    #error "QS_TIME_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

/// \brief Quantum Spy logging facilities
///
/// This class groups together QS services. It has only static members and
/// should not be instantiated.
class QS {
public:

    /// \brief Get the current version of QS
    ///
    /// \return version of the QS as a constant 6-character string of the form
    /// x.y.zz, where x is a 1-digit major version number, y is a 1-digit
    /// minor version number, and zz is a 2-digit release number.
    static char_t const Q_ROM * Q_ROM_VAR getVersion(void);

    /// \brief Initialize the QS data buffer.
    ///
    /// This function should be called from QS_init() to provide QS with the
    /// data buffer. The first argument \a sto[] is the address of the memory
    /// block, and the second argument \a stoSize is the size of this block
    /// in bytes. Currently the size of the QS buffer cannot exceed 64KB.
    ///
    /// QS can work with quite small data buffers, but you will start losing
    /// data if the buffer is too small for the bursts of logging activity.
    /// The right size of the buffer depends on the data production rate and
    /// the data output rate. QS offers flexible filtering to reduce the data
    /// production rate.
    ///
    /// \note If the data output rate cannot keep up with the production rate,
    /// QS will start overwriting the older data with newer data. This is
    /// consistent with the "last-is-best" QS policy. The record sequence
    /// counters and check sums on each record allow to easily detect data
    /// loss.
    static void initBuf(uint8_t sto[], uint32_t const stoSize);

    /// \brief Turn the global Filter on for a given record type \a rec.
    ///
    /// This function sets up the QS filter to enable the record type \a rec.
    /// The argument #QS_ALL_RECORDS specifies to filter-on all records.
    /// This function should be called indirectly through the macro
    /// #QS_FILTER_ON.
    ///
    /// \note Filtering based on the record-type is only the first layer of
    /// filtering. The second layer is based on the object-type. Both filter
    /// layers must be enabled for the QS record to be inserted into the QS
    /// buffer.
    /// \sa QS_filterOff(), #QS_FILTER_SM_OBJ, #QS_FILTER_AO_OBJ,
    /// #QS_FILTER_MP_OBJ, #QS_FILTER_EQ_OBJ, and #QS_FILTER_TE_OBJ.
    static void filterOn(uint8_t const rec);

    /// \brief Turn the global Filter off for a given record type \a rec.
    ///
    /// This function sets up the QS filter to disable the record type \a rec.
    /// The argument #QS_ALL_RECORDS specifies to suppress all records.
    /// This function should be called indirectly through the macro
    /// #QS_FILTER_OFF.
    ///
    /// \note Filtering records based on the record-type is only the first
    /// layer of filtering. The second layer is based on the object-type.
    /// Both filter layers must be enabled for the QS record to be inserted
    /// into the QS buffer.
    /// \sa
    static void filterOff(uint8_t const rec);

    /// \brief Mark the begin of a QS record \a rec
    ///
    /// This function must be called at the beginning of each QS record.
    /// This function should be called indirectly through the macro #QS_BEGIN,
    /// or #QS_BEGIN_NOCRIT, depending if it's called in a normal code or from
    /// a critical section.
    static void begin(uint8_t const rec);

    /// \brief Mark the end of a QS record \a rec
    ///
    /// This function must be called at the end of each QS record.
    /// This function should be called indirectly through the macro #QS_END,
    /// or #QS_END_NOCRIT, depending if it's called in a normal code or from
    /// a critical section.
    static void end(void);

    // unformatted data elements output ......................................

    /// \brief output uint8_t data element without format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u8_(uint8_t const d);

    /// \brief Output uint16_t data element without format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u16_(uint16_t d);

    /// \brief Output uint32_t data element without format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u32_(uint32_t d);

    /// \brief Output zero-terminated ASCII string element without format
    /// information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void str_(char_t const *s);

    /// \brief Output zero-terminated ASCII string element  allocated in ROM
    /// without format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void str_ROM_(char_t const Q_ROM * Q_ROM_VAR s);

    // formatted data elements output ........................................

    /// \brief Output uint8_t data element with format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u8(uint8_t const format, uint8_t const d);

    /// \brief output uint16_t data element with format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u16(uint8_t const format, uint16_t d);

    /// \brief Output uint32_t data element with format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u32(uint8_t const format, uint32_t d);

    /// \brief Output 32-bit floating point data element with format
    /// information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void f32(uint8_t const format, float32_t const d);

    /// \brief Output 64-bit floating point data element with format
    /// information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void f64(uint8_t const format, float64_t const d);

    /// \brief Output zero-terminated ASCII string element with format
    /// information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void str(char_t const *s);

    /// \brief Output zero-terminated ASCII string element allocated in ROM
    /// with format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void str_ROM(char_t const Q_ROM * Q_ROM_VAR s);

    /// \brief Output memory block of up to 255-bytes with format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void mem(uint8_t const *blk, uint8_t size);

#if (QS_OBJ_PTR_SIZE == 8) || (QS_FUN_PTR_SIZE == 8)
    /// \brief Output uint64_t data element without format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u64_(uint64_t d);

    /// \brief Output uint64_t data element with format information
    /// \note This function is only to be used through macros, never in the
    /// client code directly.
    static void u64(uint8_t format, uint64_t d);
#endif

    // QS buffer access ......................................................

    /// \brief Byte-oriented interface to the QS data buffer.
    ///
    /// This function delivers one byte at a time from the QS data buffer.
    /// The function returns the byte in the least-significant 8-bits of the
    /// 16-bit return value if the byte is available. If no more data is
    /// available at the time, the function returns QS_EOD (End-Of-Data).
    ///
    /// \note QS::getByte() is NOT protected with a critical section.
    static uint16_t getByte(void);

    /// \brief Block-oriented interface to the QS data buffer.
    ///
    /// This function delivers a contiguous block of data from the QS data
    /// buffer. The function returns the pointer to the beginning of the
    /// block, and writes the number of bytes in the block to the location
    /// pointed to by \a pNbytes. The argument \a pNbytes is also used as
    /// input to provide the maximum size of the data block that the caller
    /// can accept.
    ///
    /// If no bytes are available in the QS buffer when the function is
    /// called, the function returns a NULL pointer and sets the value
    /// pointed to by \a pNbytes to zero.
    ///
    /// \note Only the NULL return from QS::getBlock() indicates that the QS
    /// buffer is empty at the time of the call. The non-NULL return often
    /// means that the block is at the end of the buffer and you need to call
    /// QS::getBlock() again to obtain the rest of the data that "wrapped
    /// around" to the beginning of the QS data buffer.
    ///
    /// \note QS::getBlock() is NOT protected with a critical section.
    static uint8_t const *getBlock(uint16_t * const pNbytes);

    //////////////////////////////////////////////////////////////////////////
    // platform-dependent callback functions to be implemented by clients

    // platform-specific callback functions, need to be implemented by clients
    /// \brief Callback to startup the QS facility
    ///
    /// This is a platform-dependent "callback" function invoked through the
    /// macro #QS_INIT. You need to implement this function in your
    /// application. At a minimum, the function must configure the QS buffer
    /// by calling QS::initBuf(). Typically, you will also want to open/
    /// configure the QS output channel, such as a serial port, or a file.
    /// The void* argument \a arg can be used to pass parameter(s) needed to
    /// configure the output channel.
    ///
    /// The function returns true if the QS initialization was successful,
    /// or false if it failed.
    ///
    /// The following example illustrates an implementation of QS_onStartup():
    /// \include qs_startup.cpp
    static bool onStartup(void const *arg);

    /// \brief Callback to cleanup the QS facility
    ///
    /// This is a platform-dependent "callback" function invoked through the
    /// macro #QS_EXIT. You need to implement this function in your
    /// application. The main purpose of this function is to close the QS
    /// output channel, if necessary.
    static void onCleanup(void);

    /// \brief Callback to flush the QS trace data to the host
    ///
    /// This is a platform-dependent "callback" function to flush the QS
    /// trace buffer to the host. The function typically busy-waits until all
    /// the data in the buffer is sent to the host. This is acceptable only
    /// in the initial transient.
    static void onFlush(void);

    /// \brief Callback to obtain a timestamp for a QS record.
    ///
    /// This is a platform-dependent "callback" function invoked from the
    /// macro #QS_TIME_ to add the time stamp to the QS record.
    ///
    /// \note Some of the pre-defined QS records from QP do not output the
    /// time stamp. However, ALL user records do output the time stamp.
    /// \note QS::onGetTime() is called in a critical section and should not
    /// exit critical section.
    ///
    /// The following example shows using a system call to implement QS
    /// time stamping:
    /// \include qs_onGetTime.cpp
    static QSTimeCtr onGetTime(void);

    //////////////////////////////////////////////////////////////////////////
    // Global and Local QS filters
    static uint8_t glbFilter_[32];                ///< global on/off QS filter
    static void const *smObj_;         ///< state machine for QEP local filter
    static void const *aoObj_;       ///< active object for QF/QK local filter
    static void const *mpObj_;            ///<  event pool for QF local filter
    static void const *eqObj_;             ///<  raw queue for QF local filter
    static void const *teObj_;            ///<  time event for QF local filter
    static void const *apObj_;///<  generic object Application QF local filter

    //////////////////////////////////////////////////////////////////////////
    // Miscallaneous
                                    /// tick counter for the QS_QF_TICK record
    static QSTimeCtr tickCtr_;
};

/// \brief Enumerates data formats recognized by QS
///
/// QS uses this enumeration is used only internally for the formatted user
/// data elements.
enum QSType {
    QS_I8_T,                                  ///< signed 8-bit integer format
    QS_U8_T,                                ///< unsigned 8-bit integer format
    QS_I16_T,                                ///< signed 16-bit integer format
    QS_U16_T,                              ///< unsigned 16-bit integer format
    QS_I32_T,                                ///< signed 32-bit integer format
    QS_U32_T,                              ///< unsigned 32-bit integer format
    QS_F32_T,                                ///< 32-bit floating point format
    QS_F64_T,                                ///< 64-bit floating point format
    QS_STR_T,                         ///< zero-terminated ASCII string format
    QS_MEM_T,                         ///< up to 255-bytes memory block format
    QS_SIG_T,                                         ///< event signal format
    QS_OBJ_T,                                       ///< object pointer format
    QS_FUN_T,                                     ///< function pointer format
    QS_I64_T,                                ///< signed 64-bit integer format
    QS_U64_T,                              ///< unsigned 64-bit integer format
    QS_U32_HEX_T                    ///< unsigned 32-bit integer in hex format
};

/// \brief critical section nesting level
///
/// \note Not to be used by Clients directly, only in ports of QF
extern uint8_t QF_critNest_;

QP_END_


//////////////////////////////////////////////////////////////////////////////
// Macros for adding QS instrumentation to the client code

/// \brief Initialize the QS facility.
///
/// This macro provides an indirection layer to invoke the QS initialization
/// routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
/// \sa QS::onStartup(), example of setting up a QS filter in #QS_FILTER_IN
#define QS_INIT(arg_)           (QP_ QS::onStartup(arg_))

/// \brief Cleanup the QS facility.
///
/// This macro provides an indirection layer to invoke the QS cleanup
/// routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
/// \sa QS::onCleanup()
#define QS_EXIT()               (QP_ QS::onCleanup())

/// \brief Global Filter ON for a given record type \a rec.
///
/// This macro provides an indirection layer to call QS::filterOn() if #Q_SPY
/// is defined, or do nothing if #Q_SPY is not defined.
///
/// The following example shows how to use QS filters:
/// \include qs_filter.cpp
#define QS_FILTER_ON(rec_)      (QP_ QS::filterOn(static_cast<uint8_t>(rec_)))

/// \brief Global filter OFF for a given record type \a rec.
///
/// This macro provides an indirection layer to call QS::filterOff() if #Q_SPY
/// is defined, or do nothing if #Q_SPY is not defined.
///
/// \sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_OFF(rec_)    (QP_ QS::filterOff(static_cast<uint8_t>(rec_)))

/// \brief Local Filter for a given state machine object \a obj_.
///
/// This macro sets up the state machine object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument \a obj_
/// is the pointer to the state machine object that you want to monitor.
///
/// The state machine object filter allows you to filter QS records pertaining
/// only to a given state machine object. With this filter disabled, QS will
/// output records from all state machines in your application. The object
/// filter is disabled by setting the state machine pointer to NULL.
///
/// The state machine filter affects the following QS records:
/// ::QS_QEP_STATE_ENTRY, ::QS_QEP_STATE_EXIT, ::QS_QEP_STATE_INIT,
/// ::QS_QEP_INIT_TRAN, ::QS_QEP_INTERN_TRAN, ::QS_QEP_TRAN,
/// and ::QS_QEP_IGNORED.
///
/// \note Because active objects are state machines at the same time,
/// the state machine filter (#QS_FILTER_SM_OBJ) pertains to active
/// objects as well. However, the state machine filter is more general,
/// because it can be used only for state machines that are not active
/// objects, such as "Orthogonal Components".
///
/// \sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_SM_OBJ(obj_)  (QP_ QS::smObj_ = (obj_))

/// \brief Local Filter for a given active object \a obj_.
///
/// This macro sets up the active object local filter if #Q_SPY is defined,
/// or does nothing if #Q_SPY is not defined. The argument \a obj_ is the
/// pointer to the active object that you want to monitor.
///
/// The active object filter allows you to filter QS records pertaining
/// only to a given active object. With this filter disabled, QS will
/// output records from all active objects in your application. The object
/// filter is disabled by setting the active object pointer \a obj_ to NULL.
///
/// The active object filter affects the following QS records:
/// ::QS_QF_ACTIVE_ADD, ::QS_QF_ACTIVE_REMOVE, ::QS_QF_ACTIVE_SUBSCRIBE,
/// ::QS_QF_ACTIVE_UNSUBSCRIBE, ::QS_QF_ACTIVE_POST_FIFO,
/// ::QS_QF_ACTIVE_POST_LIFO, ::QS_QF_ACTIVE_GET, and ::QS_QF_ACTIVE_GET_LAST.
///
/// \sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_AO_OBJ(obj_)  (QP_ QS::aoObj_ = (obj_))

/// \brief Local Filter for a given memory pool object \a obj_.
///
/// This macro sets up the memory pool object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument \a obj_
/// is the pointer to the memory buffer used during the initialization of the
/// event pool with QF::poolInit().
///
/// The memory pool filter allows you to filter QS records pertaining
/// only to a given memory pool. With this filter disabled, QS will
/// output records from all memory pools in your application. The object
/// filter is disabled by setting the memory pool pointer \a obj_ to NULL.
///
/// The memory pool filter affects the following QS records:
/// ::QS_QF_MPOOL_INIT, ::QS_QF_MPOOL_GET, and ::QS_QF_MPOOL_PUT.
///
/// \sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_MP_OBJ(obj_)  (QP_ QS::mpObj_ = (obj_))

/// \brief Filter for a given event queue object \a obj_.
///
/// This macro sets up the event queue object filter if #Q_SPY is defined,
/// or does nothing if #Q_SPY is not defined. The argument \a obj_ is the
/// pointer to the "raw" thread-safe queue object you want to monitor.
///
/// The event queue filter allows you to filter QS records pertaining
/// only to a given event queue. With this filter disabled, QS will
/// output records from all event queues in your application. The object
/// filter is disabled by setting the event queue pointer \a obj_ to NULL.
///
/// The event queue filter affects the following QS records:
/// ::QS_QF_EQUEUE_INIT, ::QS_QF_EQUEUE_POST_FIFO, ::QS_QF_EQUEUE_POST_LIFO,
/// ::QS_QF_EQUEUE_GET, and ::QS_QF_EQUEUE_GET_LAST.
///
/// \sa Example of using QS filters in #QS_FILTER_IN documentation
#define QS_FILTER_EQ_OBJ(obj_)  (QP_ QS::eqObj_ = (obj_))

/// \brief Local Filter for a given time event object \a obj_.
///
/// This macro sets up the time event object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument \a obj_
/// is the pointer to the time event object you want to monitor.
///
/// The time event filter allows you to filter QS records pertaining
/// only to a given time event. With this filter disabled, QS will
/// output records from all time events in your application. The object
/// filter is disabled by setting the time event pointer \a obj_ to NULL.
///
/// The time event filter affects the following QS records:
/// ::QS_QF_TIMEEVT_ARM, ::QS_QF_TIMEEVT_AUTO_DISARM,
/// ::QS_QF_TIMEEVT_DISARM_ATTEMPT, ::QS_QF_TIMEEVT_DISARM,
/// ::QS_QF_TIMEEVT_REARM, ::QS_QF_TIMEEVT_POST, and ::QS_QF_TIMEEVT_PUBLISH.
///
/// \sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_TE_OBJ(obj_)  (QP_ QS::teObj_ = (obj_))

/// \brief Local Filter for a generic application object \a obj_.
///
/// This macro sets up the local application object filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument \a obj_
/// is the pointer to the application object you want to monitor.
///
/// The application object filter allows you to filter QS records pertaining
/// only to a given application object. With this filter disabled, QS will
/// output records from all application-records enabled by the global filter.
/// The local filter is disabled by setting the time event pointer \a obj_
/// to NULL.
///
/// \sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_AP_OBJ(obj_)  (QP_ QS::apObj_ = (obj_))


//////////////////////////////////////////////////////////////////////////////
// Macros to generate user QS records

#define QS_GLB_FILTER_(rec_) \
    ((QP_ QS::glbFilter_[static_cast<uint8_t>(rec_) >> 3] \
      & (static_cast<uint8_t>(1U << (static_cast<uint8_t>(rec_) \
                                     & static_cast<uint8_t>(7))))) \
             != static_cast<uint8_t>(0))

/// \brief Begin a QS user record without entering critical section.
#define QS_BEGIN_NOCRIT(rec_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && ((QP_ QS::apObj_ == static_cast<void *>(0)) \
            || (QP_ QS::apObj_ == (obj_)))) \
    { \
        QP_ QS::begin(static_cast<uint8_t>(rec_)); \
        QS_TIME_();

/// \brief End a QS user record without exiting critical section.
#define QS_END_NOCRIT() \
    QS_END_NOCRIT_()

                                               // QS-specific critical section
#ifndef QF_CRIT_STAT_TYPE
    /// \brief This is an internal macro for defining the critical section
    /// status type.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// provides the definition of the critical section status variable.
    /// Otherwise this macro is empty.
    /// \sa #QF_CRIT_STAT_TYPE
    ///
    #define QS_CRIT_STAT_

    /// \brief This is an internal macro for entering a critical section.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_ENTRY passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_ENTRY is invoked with a dummy parameter.
    /// \sa #QF_CRIT_ENTRY
    ///
    #define QS_CRIT_ENTRY_()    QF_CRIT_ENTRY(dummy)

    /// \brief This is an internal macro for exiting a critical section.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_EXIT passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_EXIT is invoked with a dummy parameter.
    /// \sa #QF_CRIT_EXIT
    ///
    #define QS_CRIT_EXIT_()     QF_CRIT_EXIT(dummy)

#else
    #define QS_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QS_CRIT_ENTRY_()    QF_CRIT_ENTRY(critStat_)
    #define QS_CRIT_EXIT_()     QF_CRIT_EXIT(critStat_)
#endif

/// \brief Begin a user QS record with entering critical section.
///
/// The following example shows how to build a user QS record using the
/// macros #QS_BEGIN, #QS_END, and the formatted output macros: #QS_U8 and
/// #QS_STR.
/// \include qs_user.cpp
/// \note Must always be used in pair with #QS_END
#define QS_BEGIN(rec_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && ((QP_ QS::apObj_ == static_cast<void *>(0)) \
            || (QP_ QS::apObj_ == (obj_)))) \
    { \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP_ QS::begin(static_cast<uint8_t>(rec_)); \
        QS_TIME_();

/// \brief End a QS record with exiting critical section.
/// \sa example for #QS_BEGIN
/// \note Must always be used in pair with #QS_BEGIN
#define QS_END() \
    QS_END_()


//////////////////////////////////////////////////////////////////////////////
// Macros for use inside other macros or internally in the QP code

/// \brief Internal QS macro to begin a QS record with entering critical
/// section.
/// \note This macro is intended to use only inside QP components and NOT
/// at the application level. \sa #QS_BEGIN
#define QS_BEGIN_(rec_, objFilter_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && (((objFilter_) == static_cast<void *>(0)) \
            || ((objFilter_) == (obj_)))) \
    { \
        QS_CRIT_ENTRY_(); \
        QP_ QS::begin(static_cast<uint8_t>(rec_));

/// \brief  Internal QS macro to end a QS record with exiting critical
/// section.
/// \note This macro is intended to use only inside QP components and NOT
/// at the application level. \sa #QS_END
#define QS_END_() \
        QP_ QS::end(); \
        QS_CRIT_EXIT_(); \
    }

/// \brief Internal QS macro to begin a QS record without entering critical
/// section.
/// \note This macro is intended to use only inside QP components and NOT
/// at the application level. \sa #QS_BEGIN_NOCRIT
#define QS_BEGIN_NOCRIT_(rec_, objFilter_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && (((objFilter_) == static_cast<void *>(0)) \
            || ((objFilter_) == (obj_)))) \
    { \
        QP_ QS::begin(static_cast<uint8_t>(rec_));

/// \brief Internal QS macro to end a QS record without exiting critical
/// section.
/// \note This macro is intended to use only inside QP components and NOT
/// at the application level. \sa #QS_END_NOCRIT
#define QS_END_NOCRIT_() \
        QP_ QS::end(); \
    }

#if (Q_SIGNAL_SIZE == 1)
    /// \brief Internal QS macro to output an unformatted event signal
    /// data element
    /// \note the size of the pointer depends on the macro #Q_SIGNAL_SIZE.
    #define QS_SIG_(sig_)    (QP_ QS::u8_(static_cast<uint8_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 2)
    #define QS_SIG_(sig_)    (QP_ QS::u16_(static_cast<uint16_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 4)
    #define QS_SIG_(sig_)    (QP_ QS::u32_(static_cast<uint32_t>(sig_)))
#endif

/// \brief Internal QS macro to output an unformatted uint8_t data element
#define QS_U8_(data_)        (QP_ QS::u8_(data_))

/// \brief Internal QS macro to output an unformatted uint16_t data element
#define QS_U16_(data_)       (QP_ QS::u16_(data_))

/// \brief Internal QS macro to output an unformatted uint32_t data element
#define QS_U32_(data_)       (QP_ QS::u32_(data_))


#if (QS_OBJ_PTR_SIZE == 1)
    #define QS_OBJ_(obj_)    (QP_ QS::u8_(reinterpret_cast<uint8_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 2)
    #define QS_OBJ_(obj_)    (QP_ QS::u16_(reinterpret_cast<uint16_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4)
    #define QS_OBJ_(obj_)    (QP_ QS::u32_(reinterpret_cast<uint32_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8)
    #define QS_OBJ_(obj_)    (QP_ QS::u64_(reinterpret_cast<uint64_t>(obj_)))
#else

    /// \brief Internal QS macro to output an unformatted object pointer
    /// data element
    /// \note the size of the pointer depends on the macro #QS_OBJ_PTR_SIZE.
    /// If the size is not defined the size of pointer is assumed 4-bytes.
    #define QS_OBJ_(obj_)    (QP_ QS::u32_(reinterpret_cast<uint32_t>(obj_)))
#endif


#if (QS_FUN_PTR_SIZE == 1)
    #define QS_FUN_(fun_)    (QP_ QS::u8_(reinterpret_cast<uint8_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 2)
    #define QS_FUN_(fun_)    (QP_ QS::u16_(reinterpret_cast<uint16_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 4)
    #define QS_FUN_(fun_)    (QP_ QS::u32_(reinterpret_cast<uint32_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 8)
    #define QS_FUN_(fun_)    (QP_ QS::u64_(reinterpret_cast<uint64_t>(fun_)))
#else

    /// \brief Internal QS macro to output an unformatted function pointer
    /// data element
    /// \note the size of the pointer depends on the macro #QS_FUN_PTR_SIZE.
    /// If the size is not defined the size of pointer is assumed 4-bytes.
    #define QS_FUN_(fun_)    (QP_ QS::u32_(reinterpret_cast<uint32_t>(fun_)))
#endif

/// \brief Internal QS macro to output a zero-terminated ASCII string
/// data element
#define QS_STR_(msg_)        (QP_ QS::str_(msg_))

/// \brief Internal QS macro to output a zero-terminated ASCII string
/// allocated in ROM data element
#define QS_STR_ROM_(msg_)    (QP_ QS::str_ROM_(msg_))

//////////////////////////////////////////////////////////////////////////////
// Macros for use in the client code

/// \brief Output formatted int8_t to the QS record
#define QS_I8(width_, data_) \
    (QP_ QS::u8(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_I8_T)), (data_)))

/// \brief Output formatted uint8_t to the QS record
#define QS_U8(width_, data_) \
    (QP_ QS::u8(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_U8_T)), (data_)))

/// \brief Output formatted int16_t to the QS record
#define QS_I16(width_, data_) \
    (QP_ QS::u16(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_I16_T)), (data_)))

/// \brief Output formatted uint16_t to the QS record
#define QS_U16(width_, data_) \
    (QP_ QS::u16(static_cast<uint8_t>((((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_U16_T)), (data_)))

/// \brief Output formatted int32_t to the QS record
#define QS_I32(width_, data_) \
    (QP_ QS::u32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_I32_T)), (data_)))

/// \brief Output formatted uint32_t to the QS record
#define QS_U32(width_, data_) \
    (QP_ QS::u32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_U32_T)), (data_)))

/// \brief Output formatted 32-bit floating point number to the QS record
#define QS_F32(width_, data_) \
    (QP_ QS::f32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_F32_T)), (data_)))

/// \brief Output formatted 64-bit floating point number to the QS record
#define QS_F64(width_, data_) \
    (QP_ QS::f64(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_F64_T)), (data_)))

/// \brief Output formatted int64_t to the QS record
#define QS_I64(width_, data_) \
    (QP_ QS::u64(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_I64_T)), (data_)))

/// \brief Output formatted uint64_t to the QS record
#define QS_U64(width_, data_) \
    (QP_ QS::u64(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_U64_T)), (data_)))

/// \brief Output formatted uint32_t to the QS record
#define QS_U32_HEX(width_, data_) \
    (QP_ QS::u32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP_ QS_U32_HEX_T)), (data_)))

/// \brief Output formatted zero-terminated ASCII string to the QS record
#define QS_STR(str_)            (QP_ QS::str(str_))

/// \brief Output formatted zero-terminated ASCII string from ROM
/// to the QS record
#define QS_STR_ROM(str_)        (QP_ QS::str_ROM(str_))

/// \brief Output formatted memory block of up to 255 bytes to the QS
/// record
#define QS_MEM(mem_, size_)     (QP_ QS::mem((mem_), (size_)))


#if (QS_OBJ_PTR_SIZE == 1)
    #define QS_OBJ(obj_)        (QP_ QS::u8(QS_OBJ_T, (uint8_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 2)
    #define QS_OBJ(obj_)        (QP_ QS::u16(QS_OBJ_T, (uint16_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4)
    #define QS_OBJ(obj_)        (QP_ QS::u32(QS_OBJ_T, (uint32_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8)
    #define QS_OBJ(obj_)        (QP_ QS::u64(QS_OBJ_T, (uint64_t)(obj_)))
#else
    /// \brief Output formatted object pointer to the QS record
    #define QS_OBJ(obj_)        (QP_ QS::u32(QS_OBJ_T, (uint32_t)(obj_)))
#endif


#if (QS_FUN_PTR_SIZE == 1)
    #define QS_FUN(fun_)        (QP_ QS::u8(QS_FUN_T, (uint8_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 2)
    #define QS_FUN(fun_)        (QP_ QS::u16(QS_FUN_T, (uint16_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 4)
    #define QS_FUN(fun_)        (QP_ QS::u32(QS_FUN_T, (uint32_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 8)
    #define QS_FUN(fun_)        (QP_ QS::u64(QS_FUN_T, (uint64_t)(fun_)))
#else
    /// \brief Output formatted function pointer to the QS record
    #define QS_FUN(fun_)        (QP_ QS::u32(QS_FUN_T, (uint32_t)(fun_)))
#endif


/// \brief Reset the QS session.
///
/// This trace record should be generated at the beginning of the QS session.
/// It informs the QSPY host application that the new session has been started.
#define QS_RESET() do { \
    if (QS_GLB_FILTER_(QP_ QS_QP_RESET)) { \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP_ QS::begin(static_cast<uint8_t>(QP_ QS_QP_RESET)); \
        QP_ QS::end(); \
        QS_CRIT_EXIT_(); \
        QP_ QS::onFlush(); \
    } \
} while (false)

/// \brief Output signal dictionary record
///
/// A signal dictionary record associates the numerical value of the signal
/// and the binary address of the state machine that consumes that signal
/// with the human-readable name of the signal.
///
/// Providing a signal dictionary QS record can vastly improve readability of
/// the QS log, because instead of dealing with cryptic machine addresses the
/// QSpy host utility can display human-readable names.
///
/// A signal dictionary entry is associated with both the signal value \a sig_
/// and the state machine \a obj_, because signals are required to be unique
/// only within a given state machine and therefore the same numerical values
/// can represent different signals in different state machines.
///
/// For the "global" signals that have the same meaning in all state machines
/// (such as globally published signals), you can specify a signal dictionary
/// entry with the \a obj_ parameter set to NULL.
///
/// The following example shows the definition of signal dictionary entries
/// in the initial transition of the Table active object. Please note that
/// signals HUNGRY_SIG and DONE_SIG are associated with the Table state
/// machine only ("me" \a obj_ pointer). The EAT_SIG signal, on the other
/// hand, is global (0 \a obj_ pointer):
/// \include qs_sigDic.cpp
///
/// \note The QSpy log utility must capture the signal dictionary record
/// in order to use the human-readable information. You need to connect to
/// the target before the dictionary entries have been transmitted.
///
/// The following QSpy log example shows the signal dictionary records
/// generated from the Table initial transition and subsequent records that
/// show human-readable names of the signals:
/// \include qs_sigLog.txt
///
/// The following QSpy log example shows the same sequence of records, but
/// with dictionary records removed. The human-readable signal names are not
/// available.
/// \include qs_sigLog0.txt
#define QS_SIG_DICTIONARY(sig_, obj_) do { \
    if (QS_GLB_FILTER_(QP_ QS_SIG_DIC)) { \
        static char_t const Q_ROM Q_ROM_VAR sig_name_[] = #sig_; \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP_ QS::begin(static_cast<uint8_t>(QP_ QS_SIG_DIC)); \
        QS_SIG_(sig_); \
        QS_OBJ_(obj_); \
        QS_STR_ROM_(&sig_name_[0]); \
        QP_ QS::end(); \
        QS_CRIT_EXIT_(); \
        QP_ QS::onFlush(); \
    } \
} while (false)

/// \brief Output object dictionary record
///
/// An object dictionary record associates the binary address of an object
/// in the target's memory with the human-readable name of the object.
///
/// Providing an object dictionary QS record can vastly improve readability of
/// the QS log, because instead of dealing with cryptic machine addresses the
/// QSpy host utility can display human-readable object names.
///
/// The following example shows the definition of object dictionary entry
/// for the Table active object:
/// \include qs_objDic.cpp
#define QS_OBJ_DICTIONARY(obj_) do { \
    if (QS_GLB_FILTER_(QP_ QS_OBJ_DIC)) { \
        static char_t const Q_ROM Q_ROM_VAR obj_name_[] = #obj_; \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP_ QS::begin(static_cast<uint8_t>(QP_ QS_OBJ_DIC)); \
        QS_OBJ_(obj_); \
        QS_STR_ROM_(&obj_name_[0]); \
        QP_ QS::end(); \
        QS_CRIT_EXIT_(); \
        QP_ QS::onFlush(); \
    } \
} while (false)

/// \brief Output function dictionary record
///
/// A function dictionary record associates the binary address of a function
/// in the target's memory with the human-readable name of the function.
///
/// Providing a function dictionary QS record can vastly improve readability
/// of the QS log, because instead of dealing with cryptic machine addresses
/// the QSpy host utility can display human-readable function names.
///
/// The example from #QS_SIG_DICTIONARY shows the definition of a function
/// dictionary.
#define QS_FUN_DICTIONARY(fun_) do { \
    if (QS_GLB_FILTER_(QP_ QS_FUN_DIC)) { \
        static char_t const Q_ROM Q_ROM_VAR fun_name_[] = #fun_; \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP_ QS::begin(static_cast<uint8_t>(QP_ QS_FUN_DIC)); \
        QS_FUN_(fun_); \
        QS_STR_ROM_(&fun_name_[0]); \
        QP_ QS::end(); \
        QS_CRIT_EXIT_(); \
        QP_ QS::onFlush(); \
    } \
} while (false)

/// \brief Output user QS record dictionary record
///
/// A user QS record dictionary record associates the numerical value of a
/// user record with the human-readable identifier.
#define QS_USR_DICTIONARY(rec_) do { \
    if (QS_GLB_FILTER_(QP_ QS_USR_DIC)) { \
        static char_t const Q_ROM Q_ROM_VAR usr_name_[] = #rec_; \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP_ QS::begin(static_cast<uint8_t>(QP_ QS_USR_DIC)); \
        QS_U8_(static_cast<uint8_t>(rec_)); \
        QS_STR_ROM_(&usr_name_[0]); \
        QP_ QS::end(); \
        QS_CRIT_EXIT_(); \
        QP_ QS::onFlush(); \
    } \
} while (false)

/// \brief Output the assertion violation
#define QS_ASSERTION(module_, loc_) do { \
    QS_BEGIN_NOCRIT_(QP_ QS_ASSERT, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U16_(static_cast<uint16_t>(loc_)); \
        QS_STR_ROM_(module_); \
    QS_END_NOCRIT_() \
    QP_ QS::onFlush(); \
} while (false)

/// \brief Flush the QS trace data to the host
///
/// This macro invokes the QS::flush() platform-dependent callback function
/// to flush the QS trace buffer to the host. The function typically
/// busy-waits until all the data in the buffer is sent to the host.
/// This is acceptable only in the initial transient.
#define QS_FLUSH()   (QP_ QS::onFlush())

/// \brief Output the critical section entry record
#define QF_QS_CRIT_ENTRY() \
    QS_BEGIN_NOCRIT_(QP_ QS_QF_CRIT_ENTRY, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_((uint8_t)(++QF_critNest_)); \
    QS_END_NOCRIT_()

/// \brief Output the critical section exit record
#define QF_QS_CRIT_EXIT() \
    QS_BEGIN_NOCRIT_(QP_ QS_QF_CRIT_EXIT, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_((uint8_t)(QF_critNest_--)); \
    QS_END_NOCRIT_()

/// \brief Output the interrupt entry record
#define QF_QS_ISR_ENTRY(isrnest_, prio_) \
    QS_BEGIN_NOCRIT_(QP_ QS_QF_ISR_ENTRY, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_(isrnest_); \
        QS_U8_(prio_); \
    QS_END_NOCRIT_()

/// \brief Output the interrupt exit record
#define QF_QS_ISR_EXIT(isrnest_, prio_) \
    QS_BEGIN_NOCRIT_(QP_ QS_QF_ISR_EXIT,  \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_(isrnest_); \
        QS_U8_(prio_); \
    QS_END_NOCRIT_()

/// \brief Execute an action that is only necessary for QS output
#define QF_QS_ACTION(act_)      (act_)

#endif                                                                 // qs_h
