/// @file
/// @brief QS/C++ platform-independent public interface.
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 5.6.2
/// Last updated on  2016-03-30
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, LLC. All rights reserved.
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qs_h
#define qs_h

#ifndef Q_SPY
    #error "Q_SPY must be defined to include qs.h"
#endif

#ifndef QS_TIME_SIZE

    //! The size (in bytes) of the QS time stamp. Valid values: 1, 2,
    //! or 4; default 4.
    /// @description
    /// This macro can be defined in the QS port file (qs_port.h) to
    /// configure the QP::QSTimeCtr type. Here the macro is not defined so
    /// the default of 4 byte is chosen.
    #define QS_TIME_SIZE 4
#endif

//! access element at index @p i_ from the base pointer @p base_
///
/// @note This macro encapsulates MISRA-C++ 2008 Rule 5-0-15 (pointer
/// arithmetic other than array indexing).
#define QS_PTR_AT_(base_, i_) (base_[i_])


//****************************************************************************
namespace QP {

//! Quantum Spy record types.
/// @description
/// This enumeration specifies the record types used in the QP components.
/// You can specify your own record types starting from QP::QS_USER offset.
/// Currently, the maximum of all records cannot exceed 256.
/// @sa QP::QS::filterOn() / QS_FILTER_ON() and QP::QS::filterOff() /
/// QS_FILTER_OFF()
enum QSpyRecords {
    // [0] QS session (not maskable)
    QS_EMPTY,             //!< QS record for cleanly starting a session

    // [1] QEP records
    QS_QEP_STATE_ENTRY,   //!< a state was entered
    QS_QEP_STATE_EXIT,    //!< a state was exited
    QS_QEP_STATE_INIT,    //!< an initial transition was taken in a state
    QS_QEP_INIT_TRAN,     //!< the top-most initial transition was taken
    QS_QEP_INTERN_TRAN,   //!< an internal transition was taken
    QS_QEP_TRAN,          //!< a regular transition was taken
    QS_QEP_IGNORED,       //!< an event was ignored (silently discarded)
    QS_QEP_DISPATCH,      //!< an event was dispatched (begin of RTC step)
    QS_QEP_UNHANDLED,     //!< an event was unhandled due to a guard

    // [10] QF records
    QS_QF_ACTIVE_ADD,     //!< an AO has been added to QF (started)
    QS_QF_ACTIVE_REMOVE,  //!< an AO has been removed from QF (stopped)
    QS_QF_ACTIVE_SUBSCRIBE,  //!< an AO subscribed to an event
    QS_QF_ACTIVE_UNSUBSCRIBE,//!< an AO unsubscribed to an event
    QS_QF_ACTIVE_POST_FIFO,  //!< an event was posted (FIFO) directly to an AO
    QS_QF_ACTIVE_POST_LIFO,  //!< an event was posted (LIFO) directly to an AO
    QS_QF_ACTIVE_GET, //!< an AO got an event and its queue is still not empty
    QS_QF_ACTIVE_GET_LAST,//!< an AO got an event and its queue is empty
    QS_QF_EQUEUE_INIT,    //!< an event queue was initialized
    QS_QF_EQUEUE_POST_FIFO,  //!< an event was posted (FIFO) to a raw queue
    QS_QF_EQUEUE_POST_LIFO,  //!< an event was posted (LIFO) to a raw queue
    QS_QF_EQUEUE_GET,     //!< get an event and queue still not empty
    QS_QF_EQUEUE_GET_LAST,//!< get the last event from the queue
    QS_QF_MPOOL_INIT,     //!< a memory pool was initialized
    QS_QF_MPOOL_GET,      //!< a memory block was removed from a memory pool
    QS_QF_MPOOL_PUT,      //!< a memory block was returned to a memory pool
    QS_QF_PUBLISH,        //!< an event was truly published to subscribers
    QS_QF_RESERVED8,
    QS_QF_NEW,            //!< new event creation
    QS_QF_GC_ATTEMPT,     //!< garbage collection attempt
    QS_QF_GC,             //!< garbage collection
    QS_QF_TICK,           //!< QP::QF::tickX() was called
    QS_QF_TIMEEVT_ARM,    //!< a time event was armed
    QS_QF_TIMEEVT_AUTO_DISARM, //!< a time event expired and was disarmed
    QS_QF_TIMEEVT_DISARM_ATTEMPT,//!< attempt to disarm a disarmed QTimeEvt
    QS_QF_TIMEEVT_DISARM, //!< true disarming of an armed time event
    QS_QF_TIMEEVT_REARM,  //!< rearming of a time event
    QS_QF_TIMEEVT_POST,   //!< a time event posted itself directly to an AO
    QS_QF_TIMEEVT_CTR,    //!< a time event counter was requested
    QS_QF_CRIT_ENTRY,     //!< critical section was entered
    QS_QF_CRIT_EXIT,      //!< critical section was exited
    QS_QF_ISR_ENTRY,      //!< an ISR was entered
    QS_QF_ISR_EXIT,       //!< an ISR was exited
    QS_QF_INT_DISABLE,    //!< interrupts were disabled
    QS_QF_INT_ENABLE,     //!< interrupts were enabled
    QS_QF_ACTIVE_POST_ATTEMPT, //!< attempt to post an evt to AO failed
    QS_QF_EQUEUE_POST_ATTEMPT, //!< attempt to post an evt to QEQueue failed
    QS_QF_MPOOL_GET_ATTEMPT,   //!< attempt to get a memory block failed
    QS_QF_RESERVED1,
    QS_QF_RESERVED0,

    // [50] QK/QV records
    QS_SCHED_LOCK,        //!< scheduler was locked
    QS_SCHED_UNLOCK,      //!< scheduler was unlocked
    QS_SCHED_NEXT,        //!< scheduler found next task to execute
    QS_SCHED_IDLE,        //!< scheduler became idle
    QS_SCHED_RESUME,      //!< scheduler resumed previous task (not idle)

    // [55] Additional QEP records
    QS_QEP_TRAN_HIST,     //!< a tran to history was taken
    QS_QEP_TRAN_EP,       //!< a tran to entry point into a submachine
    QS_QEP_TRAN_XP,       //!< a tran to exit  point out of a submachine
    QS_QEP_RESERVED1,
    QS_QEP_RESERVED0,

    // [60] Miscellaneous QS records
    QS_SIG_DICT,          //!< signal dictionary entry
    QS_OBJ_DICT,          //!< object dictionary entry
    QS_FUN_DICT,          //!< function dictionary entry
    QS_USR_DICT,          //!< user QS record dictionary entry
    QS_TARGET_INFO,       //!< reports the Target information
    QS_RESERVED0,
    QS_RX_STATUS,         //!< reports QS data receive status
    QS_TEST_STATUS,       //!< reports test status
    QS_PEEK_DATA,         //!< reports the data from the PEEK query
    QS_ASSERT_FAIL,       //!< assertion failed in the code

    // [70] Application-specific (User) QS records
    QS_USER               //!< the first record available to QS users
};

//! Specification of all QS records for the QP::QS::filterOn() and
//! QP::QS::filterOff()
uint_fast8_t const QS_ALL_RECORDS = static_cast<uint_fast8_t>(0xFF);

//! Constant representing End-Of-Data condition returned from the
//! QP::QS::getByte() function.
uint16_t const QS_EOD  = static_cast<uint16_t>(0xFFFF);


#if (QS_TIME_SIZE == 1)
    typedef uint8_t QSTimeCtr;
    #define QS_TIME_()   (QP::QS::u8_(QP::QS::onGetTime()))
#elif (QS_TIME_SIZE == 2)
    typedef uint16_t QSTimeCtr;
    #define QS_TIME_()   (QP::QS::u16_(QP::QS::onGetTime()))
#elif (QS_TIME_SIZE == 4)

    //! The type of the QS time stamp. This type determines the dynamic
    //! range of QS time stamps
    typedef uint32_t QSTimeCtr;

    //! Internal macro to output time stamp to the QS record
    #define QS_TIME_()   (QP::QS::u32_(QP::QS::onGetTime()))
#else
    #error "QS_TIME_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

//! QS ring buffer counter and offset type
typedef unsigned int QSCtr;

//! Quantum Spy logging facilities
/// @description
/// This class groups together QS services. It has only static members and
/// should not be instantiated.
class QS {
public:

    //! get the current QS version number string of the form X.Y.Z
    static char_t const *getVersion(void) {
        return versionStr;
    }

    //! Initialize the QS data buffer.
    static void initBuf(uint8_t sto[], uint_fast16_t const stoSize);

    //! Turn the global Filter on for a given record type @p rec.
    static void filterOn(uint_fast8_t const rec);

    //! Turn the global Filter off for a given record type @p rec.
    static void filterOff(uint_fast8_t const rec);

    //! Mark the begin of a QS record @p rec
    static void beginRec(uint_fast8_t const rec);

    //! Mark the end of a QS record @p rec
    static void endRec(void);

    // unformatted data elements output ......................................

    //! output uint8_t data element without format information
    static void u8_(uint8_t const d);

    //! output two uint8_t data elements without format information
    static void u8u8_(uint8_t const d1, uint8_t const d2);

    //! Output uint16_t data element without format information
    static void u16_(uint16_t d);

    //! Output uint32_t data element without format information
    static void u32_(uint32_t d);

    //! Output zero-terminated ASCII string element without format information
    static void str_(char_t const *s);


    // formatted data elements output ........................................

    //! Output uint8_t data element with format information
    static void u8(uint8_t const format, uint8_t const d);

    //! output uint16_t data element with format information
    static void u16(uint8_t format, uint16_t d);

    //! Output uint32_t data element with format information
    static void u32(uint8_t format, uint32_t d);

    //! Output 32-bit floating point data element with format information
    static void f32(uint8_t format, float32_t const d);

    //! Output 64-bit floating point data element with format information
    static void f64(uint8_t format, float64_t const d);

    //! Output zero-terminated ASCII string element with format information
    static void str(char_t const *s);

    //! Output memory block of up to 255-bytes with format information
    static void mem(uint8_t const *blk, uint8_t size);

#if (QS_OBJ_PTR_SIZE == 8) || (QS_FUN_PTR_SIZE == 8)
    //! Output uint64_t data element without format information
    static void u64_(uint64_t d);

    //! Output uint64_t data element with format information
    static void u64(uint8_t format, uint64_t d);
#endif  // (QS_OBJ_PTR_SIZE == 8) || (QS_FUN_PTR_SIZE == 8)

    //! Output signal dictionary record
    static void sig_dict(enum_t const sig, void const * const obj,
                         char_t const *name);

    //! Output object dictionary record
    static void obj_dict(void const * const obj,
                         char_t const *name);

    //! Output function dictionary record
    static void fun_dict(void (* const fun)(void),
                         char_t const *name);

    //! Output user dictionary record
    static void usr_dict(enum_t const rec,
                         char_t const *name);

    //! Initialize the QS RX data buffer
    static void rxInitBuf(uint8_t sto[], uint16_t const stoSize);

    //! Parse all bytes present in the QS RX data buffer
    static void rxParse(void);

    //! Obtain the number of free bytes in the QS RX data buffer
    static uint16_t rxGetNfree(void);

    //! put one byte into the QS RX lock-free buffer
    static void rxPut(uint8_t const b) {
        if (rxPriv_.head != static_cast<QSCtr>(0)) {
            if ((rxPriv_.head - rxPriv_.tail) != static_cast<QSCtr>(1)) {
                QS_PTR_AT_(rxPriv_.buf, rxPriv_.head) = b;
                --rxPriv_.head;
            }
        }
        else {
            if (rxPriv_.tail != rxPriv_.end) {
                QS_PTR_AT_(rxPriv_.buf, 0) = b;
                rxPriv_.head = rxPriv_.end;
            }
        }
    }

    // QS buffer access ......................................................

    //! Byte-oriented interface to the QS data buffer.
    static uint16_t getByte(void);

    //! Block-oriented interface to the QS data buffer.
    static uint8_t const *getBlock(uint16_t * const pNbytes);

    // platform-dependent callback functions to be implemented by clients ....

    //! Callback to startup the QS facility
    static bool onStartup(void const *arg);

    //! Callback to cleanup the QS facility
    static void onCleanup(void);

    //! Callback to flush the QS trace data to the host
    static void onFlush(void);

    //! Callback to obtain a timestamp for a QS record.
    static QSTimeCtr onGetTime(void);

    //! Callback function to reset the target (to be implemented in the BSP)
    static void onReset(void);

    //! Callback function to execute user commands (to be implemented in BSP)
    static void onCommand(uint8_t cmdId, uint32_t param);


    //! Enumerates data formats recognized by QS
    /// @description
    /// QS uses this enumeration is used only internally for the formatted
    /// user data elements.
    enum QSType {
        I8_T,         //!< signed 8-bit integer format
        U8_T,         //!< unsigned 8-bit integer format
        I16_T,        //!< signed 16-bit integer format
        U16_T,        //!< unsigned 16-bit integer format
        I32_T,        //!< signed 32-bit integer format
        U32_T,        //!< unsigned 32-bit integer format
        F32_T,        //!< 32-bit floating point format
        F64_T,        //!< 64-bit floating point format
        STR_T,        //!< zero-terminated ASCII string format
        MEM_T,        //!< up to 255-bytes memory block format
        SIG_T,        //!< event signal format
        OBJ_T,        //!< object pointer format
        FUN_T,        //!< function pointer format
        I64_T,        //!< signed 64-bit integer format
        U64_T,        //!< unsigned 64-bit integer format
        U32_HEX_T     //!< unsigned 32-bit integer in hex format
    };

    // private QS attributes .................................................

    uint8_t glbFilter[16];    //!< global on/off QS filter
    void const *smObjFilter;  //!< state machine for QEP local filter
    void const *aoObjFilter;  //!< active object for QF/QK local filter
    void const *mpObjFilter;  //!<  event pool for QF local filter
    void const *eqObjFilter;  //!<  raw queue for QF local filter
    void const *teObjFilter;  //!<  time event for QF local filter
    void const *apObjFilter;  //!<  generic object Application QF local filter

    uint8_t *buf;     //!< pointer to the start of the ring buffer
    QSCtr    end;     //!< offset of the end of the ring buffer
    QSCtr    head;    //!< offset to where next byte will be inserted
    QSCtr    tail;    //!< offset of where next record will be extracted
    QSCtr    used;    //!< number of bytes currently in the ring buffer
    uint8_t  seq;     //!< the record sequence number
    uint8_t  chksum;  //!< the checksum of the current record
    uint8_t  full;    //!< the ring buffer is temporarily full

    uint_fast8_t critNest; //!< critical section nesting level

    static QS priv_;

    static struct QSrxPriv {
        uint8_t *buf; //!< pointer to the start of the ring buffer
        QSCtr end;    //!< offset of the end of the ring buffer
        QSCtr head;   //!< offset to where next byte will be inserted
        QSCtr tail;   //!< offset of where next byte will be extracted
    } rxPriv_;
};

//! Quantum Spy Receive (RX) record types
/// @description
/// This enumeration specifies the record types for the QS receive channel
enum QSpyRxRecords {
    QS_RX_INFO,       //!< query Target info (ver, config, tstamp)
    QS_RX_COMMAND,    //!< execute a user-defined command in the Target
    QS_RX_RESET,      //!< reset the Target
    QS_RX_TICK,       //!< call QF_tick()
    QS_RX_PEEK,       //!< peek Target memory
    QS_RX_POKE,       //!< poke Target memory
    QS_RX_RESERVED7,  //!< reserved for future use
    QS_RX_RESERVED6,  //!< reserved for future use
    QS_RX_RESERVED5,  //!< reserved for future use
    QS_RX_RESERVED4,  //!< reserved for future use
    QS_RX_GLB_FILTER, //!< set global filters in the Target
    QS_RX_LOC_FILTER, //!< set local  filters in the Target
    QS_RX_AO_FILTER,  //!< set local AO filter in the Target
    QS_RX_RESERVED3,  //!< reserved for future use
    QS_RX_RESERVED2,  //!< reserved for future use
    QS_RX_RESERVED1,  //!< reserved for future use
    QS_RX_EVENT       //!< inject an event to the Target (post/publish)
};

} // namespace QP

//****************************************************************************
// Macros for adding QS instrumentation to the client code

//! Initialize the QS facility.
/// @description
/// This macro provides an indirection layer to invoke the QS initialization
/// routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
/// @sa QP::QS::onStartup(), example of setting up a QS filter in
/// QS_FILTER_ON()
#define QS_INIT(arg_)           (QP::QS::onStartup(arg_))

//! Cleanup the QS facility.
/// @description
/// This macro provides an indirection layer to invoke the QS cleanup
/// routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
/// @sa QP::QS::onCleanup()
#define QS_EXIT()               (QP::QS::onCleanup())

//! Global Filter ON for a given record type @p rec.
/// @description
/// This macro provides an indirection layer to call QP::QS::filterOn()
/// if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
///
/// @usage
/// The following example shows how to use QS filters:
/// @include qs_filter.cpp
#define QS_FILTER_ON(rec_)      (QP::QS::filterOn(static_cast<uint8_t>(rec_)))

//! Global filter OFF for a given record type @p rec.
/// @description
/// This macro provides an indirection layer to call QP::QS::filterOff()
/// if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_OFF(rec_)    (QP::QS::filterOff(static_cast<uint8_t>(rec_)))

//! Local Filter for a given state machine object @p obj_.
/// @description
/// This macro sets up the state machine object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the state machine object that you want to monitor.@n
/// @n
/// The state machine object filter allows you to filter QS records pertaining
/// only to a given state machine object. With this filter disabled, QS will
/// output records from all state machines in your application. The object
/// filter is disabled by setting the state machine pointer to NULL.@n
/// @n
/// The state machine filter affects the following QS records:
/// QP::QS_QEP_STATE_ENTRY, QP::QS_QEP_STATE_EXIT, QP::QS_QEP_STATE_INIT,
/// QP::QS_QEP_INTERN_TRAN, QP::QS_QEP_TRAN, and QP::QS_QEP_IGNORED.
///
/// @note
/// Because active objects are state machines at the same time, the state
/// machine filter (QS_FILTER_SM_OBJ) pertains to active objects as well.
/// However, the state machine filter is more general, because it can be
/// used only for state machines that are not active objects, such as
/// "Orthogonal Components".
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_SM_OBJ(obj_)  (QP::QS::priv_.smObjFilter = (obj_))

//! Local Filter for a given active object @p obj_.
/// @description
/// This macro sets up the active object local filter if #Q_SPY is defined,
/// or does nothing if #Q_SPY is not defined. The argument @p obj_ is the
/// pointer to the active object that you want to monitor.@n
/// @n
/// The active object filter allows you to filter QS records pertaining
/// only to a given active object. With this filter disabled, QS will
/// output records from all active objects in your application. The object
/// filter is disabled by setting the active object pointer @p obj_ to NULL.@n
/// @n
/// The active object filter affects the following QS records:
/// QP::QS_QF_ACTIVE_ADD, QP::QS_QF_ACTIVE_REMOVE, QP::QS_QF_ACTIVE_SUBSCRIBE,
/// QP::QS_QF_ACTIVE_UNSUBSCRIBE, QP::QS_QF_ACTIVE_POST_FIFO,
/// QP::QS_QF_ACTIVE_POST_LIFO, ::QS_QF_ACTIVE_GET, and
/// QP::QS_QF_ACTIVE_GET_LAST.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_AO_OBJ(obj_)  (QP::QS::priv_.aoObjFilter = (obj_))

//! Local Filter for a given memory pool object @p obj_.
/// @description
/// This macro sets up the memory pool object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the memory buffer used during the initialization of the
/// event pool with QP::QF::poolInit().@n
/// @n
/// The memory pool filter allows you to filter QS records pertaining
/// only to a given memory pool. With this filter disabled, QS will
/// output records from all memory pools in your application. The object
/// filter is disabled by setting the memory pool pointer @p obj_ to NULL.@n
/// @n
/// The memory pool filter affects the following QS records:
/// QP::QS_QF_MPOOL_INIT, QP::QS_QF_MPOOL_GET, and QP::QS_QF_MPOOL_PUT.
///
/// @sa Example of using QS filters in QS_FILTER_ON() documentation
#define QS_FILTER_MP_OBJ(obj_)  (QP::QS::priv_.mpObjFilter = (obj_))

//! Filter for a given event queue object @p obj_.
/// @description
/// This macro sets up the event queue object filter if #Q_SPY is defined,
/// or does nothing if #Q_SPY is not defined. The argument @p obj_ is the
/// pointer to the "raw" thread-safe queue object you want to monitor.@n
/// @n
/// The event queue filter allows you to filter QS records pertaining
/// only to a given event queue. With this filter disabled, QS will
/// output records from all event queues in your application. The object
/// filter is disabled by setting the event queue pointer @p obj_ to NULL.@n
/// @n
/// The event queue filter affects the following QS records:
/// QP::QS_QF_EQUEUE_INIT, QP::QS_QF_EQUEUE_POST_FIFO,
/// QP::QS_QF_EQUEUE_POST_LIFO, QP::QS_QF_EQUEUE_GET, and
/// QP::QS_QF_EQUEUE_GET_LAST.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_EQ_OBJ(obj_)  (QP::QS::priv_.eqObjFilter = (obj_))

//! Local Filter for a given time event object @p obj_.
/// @description
/// This macro sets up the time event object local filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the time event object you want to monitor.@n
/// @n
/// The time event filter allows you to filter QS records pertaining
/// only to a given time event. With this filter disabled, QS will
/// output records from all time events in your application. The object
/// filter is disabled by setting the time event pointer @p obj_ to NULL.@n
/// @n
/// The time event filter affects the following QS records:
/// QP::QS_QF_TIMEEVT_ARM, QP::QS_QF_TIMEEVT_AUTO_DISARM,
/// QP::QS_QF_TIMEEVT_DISARM_ATTEMPT, QP::QS_QF_TIMEEVT_DISARM,
/// QP::QS_QF_TIMEEVT_REARM, and QP::QS_QF_TIMEEVT_POST.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_TE_OBJ(obj_)  (QP::QS::priv_.teObjFilter = (obj_))

//! Local Filter for a generic application object @p obj_.
/// @description
/// This macro sets up the local application object filter if #Q_SPY is
/// defined, or does nothing if #Q_SPY is not defined. The argument @p obj_
/// is the pointer to the application object you want to monitor.@n
/// @n
/// The application object filter allows you to filter QS records pertaining
/// only to a given application object. With this filter disabled, QS will
/// output records from all application-records enabled by the global filter.
/// The local filter is disabled by setting the time event pointer @p obj_
/// to NULL.
///
/// @sa Example of using QS filters in #QS_FILTER_ON documentation
#define QS_FILTER_AP_OBJ(obj_)  (QP::QS::priv_.apObjFilter = (obj_))


//****************************************************************************
// Macros to generate user QS records

//! helper macro for checking the global QS filter
#define QS_GLB_FILTER_(rec_) \
    ((static_cast<uint_fast8_t>(QP::QS::priv_.glbFilter[ \
            static_cast<uint8_t>(rec_) >> 3]) \
      & static_cast<uint_fast8_t>(static_cast<uint8_t>(1U << \
               (static_cast<uint8_t>(rec_) & static_cast<uint8_t>(7))))) \
             != static_cast<uint_fast8_t>(0))

//! Begin a QS user record without entering critical section.
#define QS_BEGIN_NOCRIT(rec_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && ((QP::QS::priv_.apObjFilter == static_cast<void *>(0)) \
            || (QP::QS::priv_.apObjFilter == (obj_)))) \
    { \
        QP::QS::beginRec(static_cast<uint_fast8_t>(rec_)); \
        QS_TIME_();

//! End a QS user record without exiting critical section.
#define QS_END_NOCRIT() \
    QS_END_NOCRIT_()

// QS-specific critical section ..............................................
#ifdef QS_CRIT_ENTRY // separate QS critical section defined?

#ifndef QS_CRIT_STAT_TYPE
    #define QS_CRIT_STAT_
    #define QS_CRIT_ENTRY_()    QS_CRIT_ENTRY(dummy)
    #define QS_CRIT_EXIT_()     QS_CRIT_EXIT(dummy)
#else
    #define QS_CRIT_STAT_       QS_CRIT_STAT_TYPE critStat_;
    #define QS_CRIT_ENTRY_()    QS_CRIT_ENTRY(critStat_)
    #define QS_CRIT_EXIT_()     QS_CRIT_EXIT(critStat_)
#endif // QS_CRIT_ENTRY

#else // separate QS critical section not defined--use the QF definition
#ifndef QF_CRIT_STAT_TYPE
    //! This is an internal macro for defining the critical section
    //! status type.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// provides the definition of the critical section status variable.
    /// Otherwise this macro is empty.
    /// @sa #QF_CRIT_STAT_TYPE
    #define QS_CRIT_STAT_

    //! This is an internal macro for entering a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_ENTRY passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_ENTRY is invoked with a dummy parameter.
    /// @sa #QF_CRIT_ENTRY
    #define QS_CRIT_ENTRY_()    QF_CRIT_ENTRY(dummy)

    //! This is an internal macro for exiting a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes #QF_CRIT_EXIT passing the key variable as the parameter.
    /// Otherwise #QF_CRIT_EXIT is invoked with a dummy parameter.
    /// @sa #QF_CRIT_EXIT
    #define QS_CRIT_EXIT_()     QF_CRIT_EXIT(dummy)

#else
    #define QS_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QS_CRIT_ENTRY_()    QF_CRIT_ENTRY(critStat_)
    #define QS_CRIT_EXIT_()     QF_CRIT_EXIT(critStat_)
#endif // QF_CRIT_STAT_TYPE

#endif // QS_CRIT_ENTRY

//! Begin a user QS record with entering critical section.
/// @description
/// The following example shows how to build a user QS record using the
/// macros #QS_BEGIN, #QS_END, and the formatted output macros: #QS_U8 and
/// #QS_STR.
///
/// @note
/// Must always be used in pair with #QS_END
///
/// @include qs_user.cpp
#define QS_BEGIN(rec_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && ((QP::QS::priv_.apObjFilter == static_cast<void *>(0)) \
            || (QP::QS::priv_.apObjFilter == (obj_)))) \
    { \
        QS_CRIT_STAT_ \
        QS_CRIT_ENTRY_(); \
        QP::QS::beginRec(static_cast<uint_fast8_t>(rec_)); \
        QS_TIME_();

//! End a QS record with exiting critical section.
/// @sa example for #QS_BEGIN
/// @note Must always be used in pair with #QS_BEGIN
#define QS_END() \
    QS_END_()


//****************************************************************************
// Macros for use inside other macros or internally in the QP code

//! Internal QS macro to begin a QS record with entering critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_BEGIN
#define QS_BEGIN_(rec_, objFilter_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && (((objFilter_) == static_cast<void *>(0)) \
            || ((objFilter_) == (obj_)))) \
    { \
        QS_CRIT_ENTRY_(); \
        QP::QS::beginRec(static_cast<uint_fast8_t>(rec_));

//! Internal QS macro to end a QS record with exiting critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_END
#define QS_END_() \
        QP::QS::endRec(); \
        QS_CRIT_EXIT_(); \
    }

//! Internal QS macro to begin a QS record without entering critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_BEGIN_NOCRIT
#define QS_BEGIN_NOCRIT_(rec_, objFilter_, obj_) \
    if (QS_GLB_FILTER_(rec_) \
        && (((objFilter_) == static_cast<void *>(0)) \
            || ((objFilter_) == (obj_)))) \
    { \
        QP::QS::beginRec(static_cast<uint_fast8_t>(rec_));

//! Internal QS macro to end a QS record without exiting critical section.
/// @note
/// This macro is intended to use only inside QP components and NOT
/// at the application level. @sa #QS_END_NOCRIT
#define QS_END_NOCRIT_() \
        QP::QS::endRec(); \
    }

#if (Q_SIGNAL_SIZE == 1)
    //! Internal QS macro to output an unformatted event signal data element
    /// @note
    /// The size of the pointer depends on the macro #Q_SIGNAL_SIZE.
    #define QS_SIG_(sig_)    (QP::QS::u8_(static_cast<uint8_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 2)
    #define QS_SIG_(sig_)    (QP::QS::u16_(static_cast<uint16_t>(sig_)))
#elif (Q_SIGNAL_SIZE == 4)
    #define QS_SIG_(sig_)    (QP::QS::u32_(static_cast<uint32_t>(sig_)))
#endif

//! Internal QS macro to output an unformatted uint8_t data element
#define QS_U8_(data_)        (QP::QS::u8_(static_cast<uint8_t>(data_)))

//! Internal QS macro to output 2 unformatted uint8_t data elements
#define QS_2U8_(data1_, data2_) (QP::QS::u8u8_((data1_), (data2_)))

//! Internal QS macro to output an unformatted uint16_t data element
#define QS_U16_(data_)       (QP::QS::u16_(static_cast<uint16_t>(data_)))

//! Internal QS macro to output an unformatted uint32_t data element
#define QS_U32_(data_)       (QP::QS::u32_(static_cast<uint32_t>(data_)))


#if (QS_OBJ_PTR_SIZE == 1)
    #define QS_OBJ_(obj_)    (QP::QS::u8_(reinterpret_cast<uint8_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 2)
    #define QS_OBJ_(obj_)    (QP::QS::u16_(reinterpret_cast<uint16_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4)
    #define QS_OBJ_(obj_)    (QP::QS::u32_(reinterpret_cast<uint32_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8)
    #define QS_OBJ_(obj_)    (QP::QS::u64_(reinterpret_cast<uint64_t>(obj_)))
#else

    //! Internal QS macro to output an unformatted object pointer
    //! data element
    /// @note
    /// The size of the pointer depends on the macro #QS_OBJ_PTR_SIZE.
    /// If the size is not defined the size of pointer is assumed 4-bytes.
    #define QS_OBJ_(obj_)    (QP::QS::u32_(reinterpret_cast<uint32_t>(obj_)))
#endif


#if (QS_FUN_PTR_SIZE == 1)
    #define QS_FUN_(fun_)    (QP::QS::u8_(reinterpret_cast<uint8_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 2)
    #define QS_FUN_(fun_)    (QP::QS::u16_(reinterpret_cast<uint16_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 4)
    #define QS_FUN_(fun_)    (QP::QS::u32_(reinterpret_cast<uint32_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 8)
    #define QS_FUN_(fun_)    (QP::QS::u64_(reinterpret_cast<uint64_t>(fun_)))
#else

    //! Internal QS macro to output an unformatted function pointer
    //! data element
    /// @note
    /// The size of the pointer depends on the macro #QS_FUN_PTR_SIZE.
    /// If the size is not defined the size of pointer is assumed 4-bytes.
    #define QS_FUN_(fun_)    (QP::QS::u32_(reinterpret_cast<uint32_t>(fun_)))
#endif

//! Internal QS macro to output a zero-terminated ASCII string
/// data element
#define QS_STR_(msg_)        (QP::QS::str_(msg_))


//****************************************************************************
// Macros for use in the client code

//! Output formatted int8_t to the QS record
#define QS_I8(width_, data_) \
    (QP::QS::u8(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::I8_T)), (data_)))

//! Output formatted uint8_t to the QS record
#define QS_U8(width_, data_) \
    (QP::QS::u8(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::U8_T)), (data_)))

//! Output formatted int16_t to the QS record
#define QS_I16(width_, data_) \
    (QP::QS::u16(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::I16_T)), (data_)))

//! Output formatted uint16_t to the QS record
#define QS_U16(width_, data_) \
    (QP::QS::u16(static_cast<uint8_t>((((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::U16_T)), (data_)))

//! Output formatted int32_t to the QS record
#define QS_I32(width_, data_) \
    (QP::QS::u32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::I32_T)), (data_)))

//! Output formatted uint32_t to the QS record
#define QS_U32(width_, data_) \
    (QP::QS::u32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::U32_T)), (data_)))

//! Output formatted 32-bit floating point number to the QS record
#define QS_F32(width_, data_) \
    (QP::QS::f32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::F32_T)), (data_)))

//! Output formatted 64-bit floating point number to the QS record
#define QS_F64(width_, data_) \
    (QP::QS::f64(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::F64_T)), (data_)))

//! Output formatted int64_t to the QS record
#define QS_I64(width_, data_) \
    (QP::QS::u64(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::I64_T)), (data_)))

//! Output formatted uint64_t to the QS record
#define QS_U64(width_, data_) \
    (QP::QS::u64(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::U64_T)), (data_)))

//! Output formatted uint32_t to the QS record
#define QS_U32_HEX(width_, data_) \
    (QP::QS::u32(static_cast<uint8_t>((static_cast<uint8_t>((width_) << 4)) \
        | static_cast<uint8_t>(QP::QS::U32_HEX_T)), (data_)))

//! Output formatted zero-terminated ASCII string to the QS record
#define QS_STR(str_)            (QP::QS::str(str_))

//! Output formatted memory block of up to 255 bytes to the QS record
#define QS_MEM(mem_, size_)     (QP::QS::mem((mem_), (size_)))


#if (QS_OBJ_PTR_SIZE == 1)
    #define QS_OBJ(obj_)        (QP::QS::u8(QS_OBJ_T, (uint8_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 2)
    #define QS_OBJ(obj_)        (QP::QS::u16(QS_OBJ_T, (uint16_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4)
    #define QS_OBJ(obj_)        (QP::QS::u32(QS_OBJ_T, (uint32_t)(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8)
    #define QS_OBJ(obj_)        (QP::QS::u64(QS_OBJ_T, (uint64_t)(obj_)))
#else
    //! Output formatted object pointer to the QS record
    #define QS_OBJ(obj_)        (QP::QS::u32(QS_OBJ_T, (uint32_t)(obj_)))
#endif


#if (QS_FUN_PTR_SIZE == 1)
    #define QS_FUN(fun_)        (QP::QS::u8(QS_FUN_T, (uint8_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 2)
    #define QS_FUN(fun_)        (QP::QS::u16(QS_FUN_T, (uint16_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 4)
    #define QS_FUN(fun_)        (QP::QS::u32(QS_FUN_T, (uint32_t)(fun_)))
#elif (QS_FUN_PTR_SIZE == 8)
    #define QS_FUN(fun_)        (QP::QS::u64(QS_FUN_T, (uint64_t)(fun_)))
#else
    //! Output formatted function pointer to the QS record
    #define QS_FUN(fun_)        (QP::QS::u32(QS_FUN_T, (uint32_t)(fun_)))
#endif


//! Output signal dictionary record
///
/// A signal dictionary record associates the numerical value of the signal
/// and the binary address of the state machine that consumes that signal
/// with the human-readable name of the signal.
///
/// Providing a signal dictionary QS record can vastly improve readability of
/// the QS log, because instead of dealing with cryptic machine addresses the
/// QSpy host utility can display human-readable names.
///
/// A signal dictionary entry is associated with both the signal value @p sig_
/// and the state machine @p obj_, because signals are required to be unique
/// only within a given state machine and therefore the same numerical values
/// can represent different signals in different state machines.
///
/// For the "global" signals that have the same meaning in all state machines
/// (such as globally published signals), you can specify a signal dictionary
/// entry with the @p obj_ parameter set to NULL.
///
/// The following example shows the definition of signal dictionary entries
/// in the initial transition of the Table active object. Please note that
/// signals HUNGRY_SIG and DONE_SIG are associated with the Table state
/// machine only ("me" @p obj_ pointer). The EAT_SIG signal, on the other
/// hand, is global (0 @p obj_ pointer):
/// @include qs_sigDic.cpp
///
/// @note The QSpy log utility must capture the signal dictionary record
/// in order to use the human-readable information. You need to connect to
/// the target before the dictionary entries have been transmitted.
///
/// The following QSpy log example shows the signal dictionary records
/// generated from the Table initial transition and subsequent records that
/// show human-readable names of the signals:
/// @include qs_sigLog.txt
///
/// The following QSpy log example shows the same sequence of records, but
/// with dictionary records removed. The human-readable signal names are not
/// available.
/// @include qs_sigLog0.txt
#define QS_SIG_DICTIONARY(sig_, obj_) do { \
    if (QS_GLB_FILTER_(QP::QS_SIG_DICT)) { \
        static char_t const sig_name_[] = #sig_; \
        QP::QS::sig_dict((sig_), (obj_), &sig_name_[0]); \
    } \
} while (false)

//! Output object dictionary record
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
/// @include qs_objDic.cpp
#define QS_OBJ_DICTIONARY(obj_) do { \
    if (QS_GLB_FILTER_(QP::QS_OBJ_DICT)) { \
        static char_t const obj_name_[] = #obj_; \
        QP::QS::obj_dict((obj_), &obj_name_[0]); \
    } \
} while (false)

//! Output function dictionary record
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
    if (QS_GLB_FILTER_(QP::QS_FUN_DICT)) { \
        static char_t const fun_name_[] = #fun_; \
        QP::QS::fun_dict((void (*)(void))(fun_), &fun_name_[0]); \
    } \
} while (false)

//! Output user QS record dictionary record
///
/// A user QS record dictionary record associates the numerical value of a
/// user record with the human-readable identifier.
#define QS_USR_DICTIONARY(rec_) do { \
    if (QS_GLB_FILTER_(QP::QS_USR_DICT)) { \
        static char_t const usr_name_[] = #rec_; \
        QP::QS::usr_dict((rec_), &usr_name_[0]); \
    } \
} while (false)

//! Output the assertion failure trace record
#define QS_ASSERTION(module_, loc_, delay_) do { \
    QS_BEGIN_NOCRIT_(QP::QS_ASSERT_FAIL, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U16_(static_cast<uint16_t>(loc_)); \
        QS_STR_(module_); \
    QS_END_NOCRIT_() \
    QP::QS::onFlush(); \
    for (uint32_t volatile delay_ctr_ = (delay_);  \
         delay_ctr_ > static_cast<uint32_t>(0); --delay_ctr_) \
    {} \
} while (false)

//! Flush the QS trace data to the host
///
/// This macro invokes the QP::QS::flush() platform-dependent callback
/// function to flush the QS trace buffer to the host. The function
/// typically busy-waits until all the data in the buffer is sent to
/// the host. This is acceptable only in the initial transient.
#define QS_FLUSH()   (QP::QS::onFlush())

//! Output the critical section entry record
#define QF_QS_CRIT_ENTRY() \
    QS_BEGIN_NOCRIT_(QP::QS_QF_CRIT_ENTRY, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_((uint8_t)(++QS::priv_.critNest)); \
    QS_END_NOCRIT_()

//! Output the critical section exit record
#define QF_QS_CRIT_EXIT() \
    QS_BEGIN_NOCRIT_(QP::QS_QF_CRIT_EXIT, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_((uint8_t)(QS::priv_.critNest--)); \
    QS_END_NOCRIT_()

//! Output the interrupt entry record
#define QF_QS_ISR_ENTRY(isrnest_, prio_) \
    QS_BEGIN_NOCRIT_(QP::QS_QF_ISR_ENTRY, \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_(isrnest_); \
        QS_U8_(prio_); \
    QS_END_NOCRIT_()

//! Output the interrupt exit record
#define QF_QS_ISR_EXIT(isrnest_, prio_) \
    QS_BEGIN_NOCRIT_(QP::QS_QF_ISR_EXIT,  \
        static_cast<void *>(0), static_cast<void *>(0)) \
        QS_TIME_(); \
        QS_U8_(isrnest_); \
        QS_U8_(prio_); \
    QS_END_NOCRIT_()

//! Execute an action that is only necessary for QS output
#define QF_QS_ACTION(act_)      (act_)

#endif // qs_h
