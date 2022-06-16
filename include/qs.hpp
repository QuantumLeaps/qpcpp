//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-06-15
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QS/C++ platform-independent public interface.

#ifndef QS_HPP
#define QS_HPP

#ifndef Q_SPY
    #error "Q_SPY must be defined to include qs.hpp"
#endif

//============================================================================
namespace QP {

#if (QS_TIME_SIZE == 1U)
    using QSTimeCtr = std::uint8_t;
#elif (QS_TIME_SIZE == 2U)
    using QSTimeCtr = std::uint16_t;
#elif (QS_TIME_SIZE == 4U)
    //! The type of the QS time stamp. This type determines the dynamic
    // range of QS time stamps
    //
    using QSTimeCtr = std::uint32_t;
#endif

//! QS ring buffer counter and offset type
using QSCtr = std::uint_fast16_t;

//! QS software tracing parameters for input (QS-RX)
//!
struct QSrx {
        void *currObj[8];    //!< current objects
        std::uint8_t *buf;   //!< pointer to the start of the ring buffer
        QSCtr end;           //!< offset of the end of the ring buffer
        QSCtr volatile head; //!< offset to where next byte will be inserted
        QSCtr volatile tail; //!< offset of where next byte will be extracted
#ifdef Q_UTEST
        QP::QPSet readySet;  //!< QUTEST ready-set of active objects
        bool inTestLoop;     //!< QUTest event loop is running
#endif
};

//! QS software tracing facilities for output (QS-TX)
//!
//! @description
//! This class groups together QS services. It has only static members and
//! should not be instantiated.
class QS {
public:
    //! Initialize the QS data buffer
    //!
    //! @description
    //! This function should be called from QP::QS::onStartup() to provide
    //! QS with the data buffer. The first argument `sto[]` is the address
    //! of the memory block, and the second argument `stoSize` is the size
    //! of this block [in bytes]. Currently the size of the QS buffer cannot
    //! exceed 64KB.
    //!
    //! @note
    //! QS can work with quite small data buffers, but you will start losing
    //! data if the buffer is too small for the bursts of tracing activity.
    //! The right size of the buffer depends on the data production rate and
    //! the data output rate. QS offers flexible filtering to reduce the data
    //! production rate.
    //!
    //! @note
    //! If the data output rate cannot keep up with the production rate,
    //! QS will start overwriting the older data with newer data. This is
    //! consistent with the "last-is-best" QS policy. The record sequence
    //! counters and check sums on each record allow the QSPY host utility
    //! to easily detect any data loss.
    //!
    static void initBuf(std::uint8_t * const sto,
                        std::uint_fast16_t const stoSize) noexcept;

    //! Set/clear the global Filter for a given QS record
    //!  or a group of records
    //!
    //! @description
    //! This function sets up the QS filter to enable record types specified
    //! in the `filter` parameter. The value #QS_ALL_RECORDS specifies to
    //! filter-in all records. This function should be called indirectly
    //! through the macro QS_GLB_FILTER()
    //!
    //! @param[in] filter  the QS record-d or group to enable in the filter,
    //!                 if positive or disable, if negative. The record-id
    //!                 numbers must be in the range -127..127.
    //! @note
    //! Filtering based on the record-type is only the first layer of
    //! filtering. The second layer is based on the object-type. Both filter
    //! layers must be enabled for the QS record to be inserted in the
    //! QS buffer.
    //!
    //! @sa QP::QS::locFilter_()
    //!
    static void glbFilter_(std::int_fast16_t const filter) noexcept;

    //! Set/clear the local Filter for a given object-id
    //!  or a group of object-ids
    //!
    //! @description
    //! This function sets up the local QS filter to enable or disable the
    //! given QS object-id or a group of object-ids @a filter.
    //! This function should be called indirectly through the macro
    //! QS_LOC_FILTER()
    //!
    //! @param[in] filter  the QS object-id or group to enable in the filter,
    //!                 if positive or disable, if negative. The qs_id numbers
    //!                 must be in the range 1..127.
    //! @note
    //! Filtering based on the object-id (local filter) is the second layer
    //! of filtering. The first layer is based on the QS record-type (global
    //! filter). Both filter layers must be enabled for the QS record to be
    //! inserted into the QS buffer.
    //!
    //! @sa QP::QS::glbFilter_()
    //!
    static void locFilter_(std::int_fast16_t const filter) noexcept;

    //! Mark the begin of a QS record `rec`
    //!
    //! @description
    //! This function must be called at the beginning of each QS record.
    //! This function should be called indirectly through the macro
    //! QS_BEGIN_ID(), or QS_BEGIN_NOCRIT(), depending if it's called in
    //! a normal code or from a critical section.
    //!
    static void beginRec_(std::uint_fast8_t const rec) noexcept;

    //! Mark the end of a QS record `rec`
    //!
    //! @description
    //! This function must be called at the end of each QS record.
    //! This function should be called indirectly through the macro QS_END(),
    //! or QS_END_NOCRIT(), depending if it's called in a normal code or from
    //! a critical section.
    //!
    static void endRec_(void) noexcept;

    // raw (unformatted) output of data elements .............................

    //! output std::uint8_t data element without format information
    static void u8_raw_(std::uint8_t const d) noexcept;

    //! output two std::uint8_t data elements without format information
    static void u8u8_raw_(std::uint8_t const d1,
                          std::uint8_t const d2) noexcept;

    //! Output std::uint16_t data element without format information
    static void u16_raw_(std::uint16_t d) noexcept;

    //! Output std::uint32_t data element without format information
    static void u32_raw_(std::uint32_t d) noexcept;

    //! Output obj pointer data element without format information
    static void obj_raw_(void const * const obj) noexcept;

    //! Output zero-terminated ASCII string element without format information
    static void str_raw_(char const *s) noexcept;


    // formatted data elements output ........................................

    //! Output std::uint8_t data element with format information
    //!
    //! @note
    //! This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void u8_fmt_(std::uint8_t const format,
                        std::uint8_t const d) noexcept;

    //! output std::uint16_t data element with format information
    //!
    //! @note
    //! This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void u16_fmt_(std::uint8_t format, std::uint16_t d) noexcept;

    //! Output std::uint32_t data element with format information
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void u32_fmt_(std::uint8_t format, std::uint32_t d) noexcept;

    //! Output 32-bit floating point data element with format information
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void f32_fmt_(std::uint8_t format, float32_t const d) noexcept;

    //! Output 64-bit floating point data element with format information
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void f64_fmt_(std::uint8_t format, float64_t const d) noexcept;

    //! Output zero-terminated ASCII string element with format information
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void str_fmt_(char const *s) noexcept;

    //! Output memory block of up to 255-bytes with format information
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void mem_fmt_(std::uint8_t const *blk, std::uint8_t size) noexcept;

    //! Output uint64_t data element without format information
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void u64_raw_(std::uint64_t d) noexcept;

    //! Output uint64_t data element with format information
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void u64_fmt_(std::uint8_t format, std::uint64_t d) noexcept;

    //! Output signal dictionary record
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void sig_dict_pre_(enum_t const sig, void const * const obj,
                              char const * const name) noexcept;

    //! Output object dictionary record
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void obj_dict_pre_(void const * const obj,
                              char const * const name) noexcept;

    //! Output predefined object-array dictionary record
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void obj_arr_dict_pre_(void const * const obj,
                                  std::uint_fast16_t const idx,
                                  char const * const name) noexcept;

    //! Output function dictionary record
    static void fun_dict_pre_(void (* const fun)(void),
                              char const * const name) noexcept;

    //! Output user dictionary record
    //!
    //! @note This function is only to be used through macros, never in the
    //! client code directly.
    //!
    static void usr_dict_pre_(enum_t const rec,
                              char const * const name) noexcept;

    //! Initialize the QS RX data buffer
    //!
    //! @description
    //! This function should be called from QS::onStartup() to provide QS-RX
    //! with the receive data buffer.
    //!
    //! @param[in]  sto[]   the address of the memory block
    //! @param[in]  stoSize the size of this block [bytes]. The size of the
    //!                     QS RX buffer cannot exceed 64KB.
    //!
    //! @note
    //! QS-RX can work with quite small data buffers, but you will start
    //! losing data if the buffer is not drained fast enough (e.g., in the
    //! idle task).
    //!
    //! @note
    //! If the data input rate exceeds the QS-RX processing rate, the data
    //! will be lost, but the QS protocol will notice that:
    //! (1) that the checksum in the incomplete QS records will fail; and
    //! (2) the sequence counter in QS records will show discontinuities.
    //!
    //! The QS-RX channel will report any data errors by sending the
    //! QS_RX_DATA_ERROR trace record.
    //!
    static void rxInitBuf(std::uint8_t * const sto,
                          std::uint16_t const stoSize) noexcept;

    //! Parse all bytes present in the QS RX data buffer
    static void rxParse(void);

    //! Obtain the number of free bytes in the QS RX data buffer
    //!
    //! @description
    //! This function is intended to be called from the ISR that reads the
    //! QS-RX bytes from the QSPY application. The function returns the
    //! conservative number of free bytes currently available in the buffer,
    //! assuming that the head pointer is not being moved concurrently.
    //! The tail pointer might be moving, meaning that bytes can be
    //! concurrently removed from the buffer.
    //!
    static std::uint16_t rxGetNfree(void) noexcept;

    //! Put one byte into the QS RX lock-free buffer
    static bool rxPut(std::uint8_t const b) noexcept;

    //! Set the "current object" in the Target
    //!
    //! @description
    //! This function sets the "current object" in the Target.
    //!
    static void setCurrObj(std::uint8_t obj_kind, void *obj_ptr) noexcept;

    //! Query the "current object" in the Target
    //!
    //!
    //! @description
    //! This function programmatically generates the response to the query
    //! for the "current object".
    //!
    static void queryCurrObj(std::uint8_t obj_kind) noexcept;

    // QS buffer access ......................................................
    //! Byte-oriented interface to the QS data buffer
    //!
    //! @description
    //! This function delivers one byte at a time from the QS data buffer.
    //!
    //! @returns
    //! the byte in the least-significant 8-bits of the 16-bit return
    //! value if the byte is available. If no more data is available at the
    //! time, the function returns QP::QS_EOD (End-Of-Data).
    //!
    //! @note
    //! QP::QS::getByte() is __not__ protected with a critical section.
    //!
    static std::uint16_t getByte(void) noexcept;

    //! Block-oriented interface to the QS data buffer
    //!
    //! @description
    //! This function delivers a contiguous block of data from the QS data
    //! buffer. The function returns the pointer to the beginning of the
    //! block, and writes the number of bytes in the block to the location
    //! pointed to by `pNbytes`. The argument `pNbytes` is also used as
    //! input to provide the maximum size of the data block that the caller
    //! can accept.
    //!
    //! @returns
    //! if data is available, the function returns pointer to the
    //! contiguous block of data and sets the value pointed to by `pNbytes`
    //! to the # available bytes. If data is available at the time the
    //! function is called, the function returns NULL pointer and sets the
    //! value pointed to by `pNbytes` to zero.
    //!
    //! @note
    //! Only the NULL return from QP::QS::getBlock() indicates that the QS
    //! buffer is empty at the time of the call. The non-NULL return often
    //! means that the block is at the end of the buffer and you need to call
    //! QP::QS::getBlock() again to obtain the rest of the data that
    //! "wrapped around" to the beginning of the QS data buffer.
    //!
    //! @note QP::QS::getBlock() is __not__ protected with a critical section.
    //!
    static std::uint8_t const *getBlock(
                               std::uint16_t * const pNbytes) noexcept;

    // platform-dependent callback functions to be implemented by clients ....

    //! Callback to startup the QS facility
    static bool onStartup(void const *arg);

    //! Callback to cleanup the QS facility
    static void onCleanup(void);

    //! Callback to flush the QS trace data to the host
    static void onFlush(void);

    //! Callback to obtain a timestamp for a QS record.
    static QSTimeCtr onGetTime(void);

    //! callback function to reset the Target (to be implemented in the BSP)
    static void onReset(void);

    //! Callback function to execute user commands (to be implemented in BSP)
    static void onCommand(std::uint8_t cmdId,
                          std::uint32_t param1,
                          std::uint32_t param2,
                          std::uint32_t param3);

    //! internal function to handle incoming (QS-RX) packet
    static void rxHandleGoodFrame_(std::uint8_t const state);

    //! internal function to produce the assertion failure trace record
    static void assertion_pre_(char const * const module, int_t const loc,
                               std::uint32_t const delay);

    //! internal function to produce the critical section entry record
    static void crit_entry_pre_(void);

    //! internal function to produce the critical section exit record
    static void crit_exit_pre_(void);

    //! internal function to produce the ISR entry record
    static void isr_entry_pre_(std::uint8_t const isrnest,
                               std::uint8_t const prio);

    //! internal function to produce the ISR exit record
    static void isr_exit_pre_(std::uint8_t const isrnest,
                              std::uint8_t const prio);

#ifdef Q_UTEST
    //! callback to setup a unit test inside the Target
    static void onTestSetup(void);

    //! callback to teardown after a unit test inside the Target
    static void onTestTeardown(void);

    //! callback to "massage" the test event before dispatching/posting it
    static void onTestEvt(QEvt *e);

    // callback to examine an event that is about to be posted
    static void onTestPost(void const *sender, QActive *recipient,
                           QEvt const *e, bool status);

    //! callback to run the test loop
    static void onTestLoop(void);

    //! internal function to process posted events during test
    static void processTestEvts_(void);

    //! internal function to process armed time events during test
    static void tickX_(std::uint_fast8_t const tickRate,
                       void const * const sender) noexcept;

    //! internal function to get the Test-Probe for a given API
    //!
    //! @description
    //! This function obtains the Test-Probe for a given API.
    //!
    //! @param[in]  api  pointer to the API function that requests its
    //!                  Test-Probe
    //!
    //! @returns Test-Probe data that has been received for the given API
    //! from the Host (running qutest). For any ginve API, the function
    //! returns the Test-Probe data in the same order as it was received
    //! from the Host. If there is no Test-Probe for a ginve API, or no more
    //! Test-Probes for a given API, the function returns zero.
    //!
    static std::uint32_t getTestProbe_(void (* const api)(void)) noexcept;

    //! internal function to pause test and enter the test event loop
    static void test_pause_(void);

#endif // Q_UTEST

    //! Enumerates data formats recognized by QS
    //!
    //! @description
    //! QS uses this enumeration is used only internally for the formatted
    //! user data elements.
    enum QSType : std::uint8_t {
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
        HEX_FMT       //!< HEX format for the "width" filed
    };

    //! Kinds of objects used in QS
    enum QSpyObjKind : std::uint8_t {
        SM_OBJ,       //!< state machine object for QEP
        AO_OBJ,       //!< active object
        MP_OBJ,       //!< event pool object
        EQ_OBJ,       //!< raw queue object
        TE_OBJ,       //!< time event object
        AP_OBJ,       //!< generic Application-specific object
        MAX_OBJ
    };

    enum OSpyObjCombnation : std::uint8_t {
        SM_AO_OBJ = MAX_OBJ //!< combination of SM and AO
    };

    //! template for forcing cast of member functions for function
    //! dictionaries and test probes.
    template<typename T_OUT, typename T_IN>
    static T_OUT force_cast(T_IN in) {
        union TCast {
            T_IN  in;
            T_OUT out;
        } u = { in };
        return u.out;
    }

    // private QS attributes .................................................
    std::uint8_t glbFilter[16];  //!< global on/off QS filter
    std::uint8_t locFilter[16];  //!< local  on/off QS filter
    void const *locFilter_AP; //!< deprecated local QS filter
    std::uint8_t *buf;    //!< pointer to the start of the ring buffer
    QSCtr    end;         //!< offset of the end of the ring buffer
    QSCtr volatile head;  //!< offset to where next byte will be inserted
    QSCtr volatile tail;  //!< offset of where next record will be extracted
    QSCtr volatile used;  //!< number of bytes currently in the ring buffer
    std::uint8_t volatile seq;    //!< the record sequence number
    std::uint8_t volatile chksum; //!< the checksum of the current record

    std::uint_fast8_t volatile critNest; //!< critical section nesting level

    static QS priv_;

    static QSrx rxPriv_;
};

//! QS pre-defined record types (TX channel)
//!
//! @description
//! This enumeration specifies the record types used in the QP components.
//! You can specify your own record types starting from QP::QS_USER offset.
//! Currently, the maximum of all records cannot exceed 125.
//!
//! @note
//! The QS records labeled as "not maskable" are always enabled and cannot
//! be turend off with the QS_GLB_FILTER() macro. Other QS trace records
//! can be disabled by means of the "global filters"
//!
//! @sa QS_GLB_FILTER() macro
//!
enum QSpyRecords : std::int8_t {
    // [0] QS session (not maskable)
    QS_EMPTY,             //!< QS record for cleanly starting a session

    // [1] SM records
    QS_QEP_STATE_ENTRY,   //!< a state was entered
    QS_QEP_STATE_EXIT,    //!< a state was exited
    QS_QEP_STATE_INIT,    //!< an initial transition was taken in a state
    QS_QEP_INIT_TRAN,     //!< the top-most initial transition was taken
    QS_QEP_INTERN_TRAN,   //!< an internal transition was taken
    QS_QEP_TRAN,          //!< a regular transition was taken
    QS_QEP_IGNORED,       //!< an event was ignored (silently discarded)
    QS_QEP_DISPATCH,      //!< an event was dispatched (begin of RTC step)
    QS_QEP_UNHANDLED,     //!< an event was unhandled due to a guard

    // [10] Active Object (AO) records
    QS_QF_ACTIVE_DEFER,   //!< AO deferred an event
    QS_QF_ACTIVE_RECALL,  //!< AO recalled an event
    QS_QF_ACTIVE_SUBSCRIBE,  //!< an AO subscribed to an event
    QS_QF_ACTIVE_UNSUBSCRIBE,//!< an AO unsubscribed to an event
    QS_QF_ACTIVE_POST,      //!< an event was posted (FIFO) directly to AO
    QS_QF_ACTIVE_POST_LIFO, //!< an event was posted (LIFO) directly to AO
    QS_QF_ACTIVE_GET,     //!< AO got an event and its queue is not empty
    QS_QF_ACTIVE_GET_LAST,//!< AO got an event and its queue is empty
    QS_QF_ACTIVE_RECALL_ATTEMPT, //!< AO attempted to recall an event

    // [19] Event Queue (EQ) records
    QS_QF_EQUEUE_POST,      //!< an event was posted (FIFO) to a raw queue
    QS_QF_EQUEUE_POST_LIFO, //!< an event was posted (LIFO) to a raw queue
    QS_QF_EQUEUE_GET,     //!< get an event and queue still not empty
    QS_QF_EQUEUE_GET_LAST,//!< get the last event from the queue

    // [23] Framework (QF) records */
    QS_QF_NEW_ATTEMPT,    //!< an attempt to allocate an event failed

    // [24] Memory Pool (MP) records
    QS_QF_MPOOL_GET,      //!< a memory block was removed from memory pool
    QS_QF_MPOOL_PUT,      //!< a memory block was returned to memory pool

    // [26] Additional Framework (QF) records
    QS_QF_PUBLISH,        //!< an event was published
    QS_QF_NEW_REF,        //!< new event reference was created
    QS_QF_NEW,            //!< new event was created
    QS_QF_GC_ATTEMPT,     //!< garbage collection attempt
    QS_QF_GC,             //!< garbage collection
    QS_QF_TICK,           //!< QP::QF::tickX() was called

    // [32] Time Event (TE) records
    QS_QF_TIMEEVT_ARM,    //!< a time event was armed
    QS_QF_TIMEEVT_AUTO_DISARM, //!< a time event expired and was disarmed
    QS_QF_TIMEEVT_DISARM_ATTEMPT,//!< attempt to disarm a disarmed QTimeEvt
    QS_QF_TIMEEVT_DISARM, //!< true disarming of an armed time event
    QS_QF_TIMEEVT_REARM,  //!< rearming of a time event
    QS_QF_TIMEEVT_POST,   //!< a time event posted itself directly to an AO

    // [38] Additional Framework (QF) records
    QS_QF_DELETE_REF,     //!< an event reference is about to be deleted
    QS_QF_CRIT_ENTRY,     //!< critical section was entered
    QS_QF_CRIT_EXIT,      //!< critical section was exited
    QS_QF_ISR_ENTRY,      //!< an ISR was entered
    QS_QF_ISR_EXIT,       //!< an ISR was exited
    QS_QF_INT_DISABLE,    //!< interrupts were disabled
    QS_QF_INT_ENABLE,     //!< interrupts were enabled

    // [45] Additional Active Object (AO) records
    QS_QF_ACTIVE_POST_ATTEMPT, //!< attempt to post an evt to AO failed

    // [46] Additional Event Queue (EQ) records
    QS_QF_EQUEUE_POST_ATTEMPT, //!< attempt to post an evt to QEQueue failed

    // [47] Additional Memory Pool (MP) records
    QS_QF_MPOOL_GET_ATTEMPT,   //!< attempt to get a memory block failed

    // [48] Scheduler (SC) records
    QS_MUTEX_LOCK,        //!< a mutex was locked
    QS_MUTEX_UNLOCK,      //!< a mutex was unlocked
    QS_SCHED_LOCK,        //!< scheduler was locked
    QS_SCHED_UNLOCK,      //!< scheduler was unlocked
    QS_SCHED_NEXT,        //!< scheduler found next task to execute
    QS_SCHED_IDLE,        //!< scheduler became idle
    QS_SCHED_RESUME,      //!< scheduler resumed previous task (not idle)

    // [55] Additional QEP records
    QS_QEP_TRAN_HIST,     //!< a tran to history was taken
    QS_QEP_TRAN_EP,       //!< a tran to entry point into a submachine
    QS_QEP_TRAN_XP,       //!< a tran to exit  point out of a submachine

    // [58] Miscellaneous QS records (not maskable)
    QS_TEST_PAUSED,       //!< test has been paused
    QS_TEST_PROBE_GET,    //!< reports that Test-Probe has been used
    QS_SIG_DICT,          //!< signal dictionary entry
    QS_OBJ_DICT,          //!< object dictionary entry
    QS_FUN_DICT,          //!< function dictionary entry
    QS_USR_DICT,          //!< user QS record dictionary entry
    QS_TARGET_INFO,       //!< reports the Target information
    QS_TARGET_DONE,       //!< reports completion of a user callback
    QS_RX_STATUS,         //!< reports QS data receive status
    QS_QUERY_DATA,        //!< reports the data from "current object" query
    QS_PEEK_DATA,         //!< reports the data from the PEEK query
    QS_ASSERT_FAIL,       //!< assertion failed in the code
    QS_QF_RUN,            //!< QF_run() was entered
};

//! QS user record group offsets for QS_GLB_FILTER()
enum QSpyUserOffsets : std::int16_t {
    QS_USER  = 100,          //!< the first record available to QS users
    QS_USER0 = QS_USER,      //!< offset for User Group 0
    QS_USER1 = QS_USER0 + 5, //!< offset of Group 1
    QS_USER2 = QS_USER1 + 5, //!< offset of Group 2
    QS_USER3 = QS_USER2 + 5, //!< offset of Group 3
    QS_USER4 = QS_USER3 + 5, //!< offset of Group 4
};

//! QS record groups for QS_GLB_FILTER()
enum QSpyRecordGroups : std::int16_t {
    QS_ALL_RECORDS = static_cast<std::uint8_t>(0xF0U), //!< all QS records
    QS_SM_RECORDS,  //!< State Machine QS records
    QS_AO_RECORDS,  //!< Active Object QS records
    QS_EQ_RECORDS,  //!< Event Queues QS records
    QS_MP_RECORDS,  //!< Memory Pools QS records
    QS_TE_RECORDS,  //!< Time Events QS records
    QS_QF_RECORDS,  //!< QF QS records
    QS_SC_RECORDS,  //!< Scheduler QS records
    QS_U0_RECORDS,  //!< User Group 100-104 records
    QS_U1_RECORDS,  //!< User Group 105-109 records
    QS_U2_RECORDS,  //!< User Group 110-114 records
    QS_U3_RECORDS,  //!< User Group 115-119 records
    QS_U4_RECORDS,  //!< User Group 120-124 records
    QS_UA_RECORDS   //!< All User records
};

//! QS ID offsets for QS_LOC_FILTER()
enum QSpyIdOffsets : std::int16_t {
    QS_AO_ID = 0,  //!< offset for AO priorities
    QS_EP_ID = 64, //!< offset for event-pool IDs
    QS_EQ_ID = 80, //!< offset for event-queue IDs
    QS_AP_ID = 96, //!< offset for Appl-spec IDs
};

//! QS ID groups for QS_LOC_FILTER()
enum QSpyIdGroups : std::int16_t {
    QS_ALL_IDS = 0xF0,            //!< all QS IDs
    QS_AO_IDS  = 0x80 + QS_AO_ID, //!< AO IDs (priorities)
    QS_EP_IDS  = 0x80 + QS_EP_ID, //!< event-pool IDs
    QS_EQ_IDS  = 0x80 + QS_EQ_ID, //!< event-queue IDs
    QS_AP_IDS  = 0x80 + QS_AP_ID, //!< Application-specific IDs
};

//! QS ID type for applying local filtering
struct QSpyId {
    std::uint8_t m_prio;
    std::uint_fast8_t getPrio(void) const noexcept {
        return static_cast<std::uint_fast8_t>(m_prio);
    }
};

} // namespace QP ************************************************************

#ifndef QS_TIME_SIZE

    //! The size (in bytes) of the QS time stamp. Valid values: 1U, 2U,
    //! or 4U; default 4U.
    //!
    //! @description
    //! This macro can be defined in the QS port file (qs_port.hpp) to
    //! configure the QP::QSTimeCtr type. Here the macro is not defined so
    //! the default of 4 byte is chosen.

    #define QS_TIME_SIZE 4U
#endif

#if (QS_TIME_SIZE == 1U)
    #define QS_TIME_PRE_() (QP::QS::u8_raw_(QP::QS::onGetTime()))
#elif (QS_TIME_SIZE == 2U)
    #define QS_TIME_PRE_() (QP::QS::u16_raw_(QP::QS::onGetTime()))
#elif (QS_TIME_SIZE == 4U)
    //! Internal macro to output time stamp to a QS record
    #define QS_TIME_PRE_() (QP::QS::u32_raw_(QP::QS::onGetTime()))
#else
    #error "QS_TIME_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

//============================================================================
namespace QP {

//! Constant representing End-Of-Data condition returned from the
//! QP::QS::getByte() function.
constexpr std::uint16_t QS_EOD  = 0xFFFFU;

//============================================================================
// QS receive channel

inline bool QS::rxPut(std::uint8_t const b) noexcept {
    QSCtr head = rxPriv_.head + 1U;
    if (head == rxPriv_.end) {
        head = 0U;
    }
    if (head != rxPriv_.tail) { // buffer NOT full?
        rxPriv_.buf[rxPriv_.head] = b;
        rxPriv_.head = head;
        return true;  // byte placed in the buffer
    }
    else {
        return false; // byte NOT placed in the buffer
    }
}

//============================================================================
#ifdef Q_UTEST

//! Dummy HSM class for testing (inherits QP::QHsm)
//!
//! @description
//! QHsmDummy is a test double for the role of "Orthogonal Components"
//! HSM objects in QUTest unit testing.
//!
class QHsmDummy : public QHsm {
public:
    QHsmDummy(void); // ctor

    void init(void const * const e,
              std::uint_fast8_t const qs_id) noexcept override;
    void init(std::uint_fast8_t const qs_id) noexcept override;
    void dispatch(QEvt const * const e,
                  std::uint_fast8_t const qs_id) noexcept override;
};

//! Dummy Active Object class for testing (inherits QP::QActive)
//!
//! @description
//! QActiveDummy is a test double for the role of collaborating active
//! objects in QUTest unit testing.
//!
class QActiveDummy : public QActive {
public:
    QActiveDummy(void); // ctor

    void start(std::uint_fast8_t const prio,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize,
               void const * const par) override;

    //! Overloaded start function (no initialization event)
    void start(std::uint_fast8_t const prio,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize) override
    {
        start(prio, qSto, qLen, stkSto, stkSize, nullptr);
    }

    void init(void const * const e,
              std::uint_fast8_t const qs_id) noexcept override;
    void init(std::uint_fast8_t const qs_id) noexcept override;
    void dispatch(QEvt const * const e,
                  std::uint_fast8_t const qs_id) noexcept override;
    bool post_(QEvt const * const e,
               std::uint_fast16_t const margin,
               void const * const sender) noexcept override;
    void postLIFO(QEvt const * const e) noexcept override;
};

constexpr std::uint8_t QUTEST_ON_POST {124U};

// interrupt nesting up-down counter
extern std::uint8_t volatile QF_intNest;

#endif // Q_UTEST

} // namespace QP

//============================================================================
// Macros for adding QS instrumentation to the client code

//! Initialize the QS facility
//!
//! @description
//! This macro provides an indirection layer to invoke the QS initialization
//! routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
//! @sa QP::QS::onStartup(), example of setting up a QS filter in
//! QS_GLB_FILTER()
#define QS_INIT(arg_)           (QP::QS::onStartup(arg_))

//! Cleanup the QS facility
//!
//! @description
//! This macro provides an indirection layer to invoke the QS cleanup
//! routine if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
//! @sa QP::QS::onCleanup()
#define QS_EXIT()               (QP::QS::onCleanup())

//! Global Filter ON for a given record type `rec_`
//!
//! @description
//! This macro provides an indirection layer to call QP::QS::filterOn()
//! if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
//!
//! @sa
//! - QP::QSpyRecordGroups - QS record groups that can be used as `rec_`
//! - QP::QSpyRecords - individual QS records that can be used as `rec_`
//!
//! @usage
//! The following example shows how to use QS filters:
//! @include qs_filter.cpp
#define QS_GLB_FILTER(rec_) \
    (QP::QS::glbFilter_(static_cast<std::int_fast16_t>(rec_)))

//! Local Filter for a given state machine object `qs_id`
//! @description
//! This macro provides an indirection layer to call QS_locFilter_()
//! if #Q_SPY is defined, or do nothing if #Q_SPY is not defined.
//!
//! @sa
//! - QP::QSpyIdGroups - QS ID groups that can be used as @p qs_id_
//! - QP::QSpyIdOffsets - QS ID offsets for @p qs_id_ (e.g., QS_AP_IDS + 5)
//!
//! The following example shows how to use QS filters:
//! @include qs_filter.cpp
#define QS_LOC_FILTER(qs_id_)  \
    (QP::QS::locFilter_(static_cast<std::int_fast16_t>(qs_id_)))

//============================================================================
// Macros to generate application-specific (user) QS records

//! Begin a user QS record with entering critical section
//!
//! @description
//! The following example shows how to build a user QS record using the
//! macros QS_BEGIN_ID(), QS_END(), and the formatted output macros:
//! QS_U8(), QS_STR(), etc.
//!
//! @note
//! Must always be used in pair with QS_END()
//!
//! @include qs_user.cpp
#define QS_BEGIN_ID(rec_, qs_id_)                                \
    if (QS_GLB_CHECK_(rec_) && QS_LOC_CHECK_(qs_id_)) {          \
        QS_CRIT_STAT_                                            \
        QS_CRIT_E_();                                            \
        QP::QS::beginRec_(static_cast<std::uint_fast8_t>(rec_)); \
        QS_TIME_PRE_();

//! End a QS record with exiting critical section.
//! @sa example for QS_BEGIN_ID()
//! @note Must always be used in pair with QS_BEGIN_ID()
#define QS_END()           \
        QP::QS::endRec_(); \
        QS_CRIT_X_();      \
    }

//! Begin a QS user record without entering critical section.
#define QS_BEGIN_NOCRIT(rec_, qs_id_)                            \
    if (QS_GLB_CHECK_(rec_) && QS_LOC_CHECK_(qs_id_)) {          \
        QP::QS::beginRec_(rec_);                                 \
        QS_TIME_PRE_();

//! End a QS user record without exiting critical section.
#define QS_END_NOCRIT()    \
        QP::QS::endRec_(); \
    }

#ifndef QS_REC_DONE
    //! macro to hook up user code when a QS record is produced
    #define QS_REC_DONE() (static_cast<void>(0))
#endif // QS_REC_DONE

//! helper macro for checking the global QS filter
#define QS_GLB_CHECK_(rec_)                                     \
    ((static_cast<std::uint_fast8_t>(QP::QS::priv_.glbFilter[   \
                static_cast<std::uint_fast8_t>(rec_) >> 3U])    \
          & (static_cast<std::uint_fast8_t>(1U)                 \
            << (static_cast<std::uint_fast8_t>(rec_) & 7U))) != 0U)

//! helper macro for checking the local QS filter
#define QS_LOC_CHECK_(qs_id_)                                   \
    ((static_cast<std::uint_fast8_t>(QP::QS::priv_.locFilter    \
                [static_cast<std::uint_fast8_t>(qs_id_) >> 3U]) \
          & (static_cast<std::uint_fast8_t>(1U)                 \
            << (static_cast<std::uint_fast8_t>(qs_id_) & 7U))) != 0U)

//============================================================================
// Facilities for QS ciritical section

// QS-specific critical section
#ifdef QS_CRIT_ENTRY // separate QS critical section defined?

#ifndef QS_CRIT_STAT_TYPE
    #define QS_CRIT_STAT_
    #define QS_CRIT_E_()    QS_CRIT_ENTRY(dummy)
    #define QS_CRIT_X_()     QS_CRIT_EXIT(dummy); QS_REC_DONE()
#else
    #define QS_CRIT_STAT_       QS_CRIT_STAT_TYPE critStat_;
    #define QS_CRIT_E_()    QS_CRIT_ENTRY(critStat_)
    #define QS_CRIT_X_()     QS_CRIT_EXIT(critStat_); QS_REC_DONE()
#endif // QS_CRIT_STAT_TYPE

#else // separate QS critical section not defined--use the QF definition
#ifndef QF_CRIT_STAT_TYPE
    //! This is an internal macro for defining the critical section
    //! status type
    //!
    //! @description
    //! The purpose of this macro is to enable writing the same code for the
    //! case when critical section status type is defined and when it is not.
    //! If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    //! provides the definition of the critical section status variable.
    //! Otherwise this macro is empty.
    //! @sa #QF_CRIT_STAT_TYPE
    #define QS_CRIT_STAT_

    //! This is an internal macro for entering a critical section
    //!
    //! @description
    //! The purpose of this macro is to enable writing the same code for the
    //! case when critical section status type is defined and when it is not.
    //! If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    //! invokes #QF_CRIT_ENTRY passing the key variable as the parameter.
    //! Otherwise #QF_CRIT_ENTRY is invoked with a dummy parameter.
    //! @sa #QF_CRIT_ENTRY
    #define QS_CRIT_E_()    QF_CRIT_ENTRY(dummy)

    //! This is an internal macro for exiting a critical section
    //!
    //! @description
    //! The purpose of this macro is to enable writing the same code for the
    //! case when critical section status type is defined and when it is not.
    //! If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    //! invokes #QF_CRIT_EXIT passing the key variable as the parameter.
    //! Otherwise #QF_CRIT_EXIT is invoked with a dummy parameter.
    //! @sa #QF_CRIT_EXIT
    #define QS_CRIT_X_()     QF_CRIT_EXIT(dummy); QS_REC_DONE()

#elif (!defined QS_CRIT_STAT_)
    #define QS_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QS_CRIT_E_()    QF_CRIT_ENTRY(critStat_)
    #define QS_CRIT_X_()     QF_CRIT_EXIT(critStat_); QS_REC_DONE()
#endif // simple unconditional interrupt disabling used

#endif // separate QS critical section not defined

//============================================================================
// Macros for use in the client code

//! Output formatted std::int8_t to the QS record
#define QS_I8(width_, data_)                                \
    (QP::QS::u8_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))          \
        | static_cast<std::uint8_t>(QP::QS::I8_T)), (data_)))

//! Output formatted std::uint8_t to the QS record
#define QS_U8(width_, data_)                                \
    (QP::QS::u8_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))          \
        | static_cast<std::uint8_t>(QP::QS::U8_T)), (data_)))

//! Output formatted std::int16_t to the QS record
#define QS_I16(width_, data_)                                \
    (QP::QS::u16_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))           \
        | static_cast<std::uint8_t>(QP::QS::I16_T)), (data_)))

//! Output formatted std::uint16_t to the QS record
#define QS_U16(width_, data_)                                     \
    (QP::QS::u16_fmt_(static_cast<std::uint8_t>((((width_) << 4)) \
        | static_cast<std::uint8_t>(QP::QS::U16_T)), (data_)))

//! Output formatted std::int32_t to the QS record
#define QS_I32(width_, data_)                                      \
    (QP::QS::u32_fmt_(                                             \
        static_cast<std::uint8_t>((static_cast<std::uint8_t>((width_) << 4)) \
        | static_cast<std::uint8_t>(QP::QS::I32_T)), (data_)))

//! Output formatted std::uint32_t to the QS record
#define QS_U32(width_, data_)                                \
    (QP::QS::u32_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))           \
        | static_cast<std::uint8_t>(QP::QS::U32_T)), (data_)))

//! Output formatted std::int64_t to the QS record
#define QS_I64(width_, data_)                                \
    (QP::QS::u64_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))           \
        | static_cast<std::uint8_t>(QP::QS::I64_T)), (data_)))

//! Output formatted std::uint64_t to the QS record
#define QS_U64(width_, data_)                                \
    (QP::QS::u64_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))           \
            | static_cast<std::uint8_t>(QP::QS::U64_T)), (data_)))

//! Output formatted 32-bit floating point number to the QS record
#define QS_F32(width_, data_)                                \
    (QP::QS::f32_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))           \
        | static_cast<std::uint8_t>(QP::QS::F32_T)), (data_)))

//! Output formatted 64-bit floating point number to the QS record
#define QS_F64(width_, data_)                                \
    (QP::QS::f64_fmt_(static_cast<std::uint8_t>(             \
        (static_cast<std::uint8_t>((width_) << 4))           \
        | static_cast<std::uint8_t>(QP::QS::F64_T)), (data_)))

//! Output formatted zero-terminated ASCII string to the QS record
#define QS_STR(str_)        (QP::QS::str_fmt_(str_))

//! Output formatted memory block of up to 255 bytes to the QS record
#define QS_MEM(mem_, size_) (QP::QS::mem_fmt_((mem_), (size_)))


#if (QS_OBJ_PTR_SIZE == 1U)
    #define QS_OBJ(obj_) (QP::QS::u8_fmt_(QP::QS::OBJ_T, \
        reinterpret_cast<std::uint8_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 2U)
    #define QS_OBJ(obj_) (QP::QS::u16_fmt_(QP::QS::OBJ_T, \
        reinterpret_cast<std::uint16_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 4U)
    #define QS_OBJ(obj_) (QP::QS::u32_fmt_(QP::QS::OBJ_T, \
        reinterpret_cast<std::uint32_t>(obj_)))
#elif (QS_OBJ_PTR_SIZE == 8U)
    #define QS_OBJ(obj_) (QP::QS::u64_fmt_(QP::QS::OBJ_T, \
        reinterpret_cast<std::uint64_t>(obj_)))
#else
    //! Output formatted object pointer to the QS record
    #define QS_OBJ(obj_) (QP::QS::u32_fmt_(QP::QS::OBJ_T, \
        reinterpret_cast<std::uint32_t>(obj_)))
#endif


#if (QS_FUN_PTR_SIZE == 1U)
    #define QS_FUN(fun_) (QP::QS::u8_fmt_(QP::QS::FUN_T, \
        reinterpret_cast<std::uint8_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 2U)
    #define QS_FUN(fun_) (QP::QS::u16_fmt_(QP::QS::FUN_T, \
        reinterpret_cast<std::uint16_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 4U)
    #define QS_FUN(fun_) (QP::QS::u32_fmt_(QP::QS::FUN_T, \
        reinterpret_cast<std::uint32_t>(fun_)))
#elif (QS_FUN_PTR_SIZE == 8U)
    #define QS_FUN(fun_) (QP::QS::u64_fmt_(QP::QS::FUN_T, \
        reinterpret_cast<std::uint64_t>(fun_)))
#else
    //! Output formatted function pointer to the QS record
    #define QS_FUN(fun_) (QP::QS::u32_fmt_(QP::QS::FUN_T, \
        reinterpret_cast<std::uint32_t>(fun_)))
#endif


#if (Q_SIGNAL_SIZE == 1U)
    #define QS_SIG(sig_, obj_) \
        QP::QS::u8_fmt_(QP::QS::SIG_T, static_cast<std::uint8_t>(sig_)); \
        QP::QS::obj_raw_(obj_)
#elif (Q_SIGNAL_SIZE == 2U)
    #define QS_SIG(sig_, obj_) \
        QP::QS::u16_fmt_(QP::QS::SIG_T, static_cast<std::uint16_t>(sig_)); \
        QP::QS::obj_raw_(obj_)
#elif (Q_SIGNAL_SIZE == 4U)
    #define QS_SIG(sig_, obj_) \
        QP::QS::u32_fmt_(QP::QS::SIG_T, static_cast<std::uint32_t>(sig_)); \
        QP::QS::obj_raw_(obj_)
#else
    //! Output formatted event signal (of type QP::QSignal) and
    //! the state machine object to the user QS record
    #define QS_SIG(sig_, obj_) \
        QP::QS::u16_fmt_(QP::QS::SIG_T, static_cast<std::uint16_t>(sig_)); \
        QP::QS::obj_raw_(obj_)
#endif

//============================================================================
//! Output signal dictionary record
//!
//! @description
//! A signal dictionary record associates the numerical value of the signal
//! and the binary address of the state machine that consumes that signal
//! with the human-readable name of the signal.
//!
//! Providing a signal dictionary QS record can vastly improve readability of
//! the QS log, because instead of dealing with cryptic machine addresses the
//! QSpy host utility can display human-readable names.
//!
//! A signal dictionary entry is associated with both the signal value @p sig_
//! and the state machine @p obj_, because signals are required to be unique
//! only within a given state machine and therefore the same numerical values
//! can represent different signals in different state machines.
//!
//! For the "global" signals that have the same meaning in all state machines
//! (such as globally published signals), you can specify a signal dictionary
//! entry with the @p obj_ parameter set to NULL.
//!
//! The following example shows the definition of signal dictionary entries
//! in the initial transition of the Table active object. Please note that
//! signals HUNGRY_SIG and DONE_SIG are associated with the Table state
//! machine only ("me" @p obj_ pointer). The EAT_SIG signal, on the other
//! hand, is global (0 @p obj_ pointer):
//! @include qs_sigDic.cpp
//!
//! @note The QSpy log utility must capture the signal dictionary record
//! in order to use the human-readable information. You need to connect to
//! the target before the dictionary entries have been transmitted.
//!
//! The following QSpy log example shows the signal dictionary records
//! generated from the Table initial transition and subsequent records that
//! show human-readable names of the signals:
//! @include qs_sigLog.txt
//!
//! The following QSpy log example shows the same sequence of records, but
//! with dictionary records removed. The human-readable signal names are not
//! available.
//! @include qs_sigLog0.txt
#define QS_SIG_DICTIONARY(sig_, obj_) \
    (QP::QS::sig_dict_pre_((sig_), (obj_), #sig_))

//! Output object dictionary record
//!
//! @description
//! An object dictionary record associates the binary address of an object
//! in the target's memory with the human-readable name of the object.
//!
//! Providing an object dictionary QS record can vastly improve readability of
//! the QS log, because instead of dealing with cryptic machine addresses the
//! QSpy host utility can display human-readable object names.
//!
//! The following example shows the definition of object dictionary entry
//! for the Table active object:
//! @include qs_objDic.cpp
#define QS_OBJ_DICTIONARY(obj_) \
    (QP::QS::obj_dict_pre_((obj_), #obj_))

//! Output object-array dictionary record
//!
//! @description
//! An object array dictionary record associates the binary address of the
//! object element in the target's memory with the human-readable name
//! of the object.
//!
//! Providing a dictionary QS record can vastly improve readability of
//! the QS log, because instead of dealing with cryptic machine addresses the
//! QSpy host utility can display human-readable object names.
//!
//! The following example shows the definition of object array dictionary
//! for `Philo::inst[n]` and `Philo::inst[n].m_timeEvt`:
//! @include qs_objObjDic.cpp
#define QS_OBJ_ARR_DICTIONARY(obj_, idx_) \
    (QP::QS::obj_arr_dict_pre_((obj_), (idx_), #obj_))

//! Output function dictionary record
//!
//! @description
//! A function dictionary record associates the binary address of a function
//! in the target's memory with the human-readable name of the function.
//!
//! Providing a function dictionary QS record can vastly improve readability
//! of the QS log, because instead of dealing with cryptic machine addresses
//! the QSpy host utility can display human-readable function names.
//!
//! The example from #QS_SIG_DICTIONARY shows the definition of a function
//! dictionary.
#define QS_FUN_DICTIONARY(fun_) \
    (QP::QS::fun_dict_pre_(     \
        QP::QS::force_cast<void (*)(void)>(fun_), #fun_))

//! Output user QS record dictionary record
//!
//! @description
//! A user QS record dictionary record associates the numerical value of a
//! user record with the human-readable identifier.
#define QS_USR_DICTIONARY(rec_) do {              \
    static char const usr_name_[] = #rec_;      \
    QP::QS::usr_dict_pre_((rec_), &usr_name_[0]); \
} while (false)

//! Produce the assertion failure trace record
#define QS_ASSERTION(module_, loc_, delay_) \
    (QP::QS::assertion_pre_((module_), (loc_), (delay_)))

//! Output the critical section entry record
#define QF_QS_CRIT_ENTRY() (QP::QS::crit_entry_pre_())

//! Output the critical section exit record
#define QF_QS_CRIT_EXIT()  (QP::QS::crit_exit_pre_())

//! Output the interrupt entry record
#define QF_QS_ISR_ENTRY(isrnest_, prio_) \
    (QP::QS::isr_entry_pre_((isrnest_), (prio_)))

//! Output the interrupt exit record
#define QF_QS_ISR_EXIT(isrnest_, prio_) \
    (QP::QS::isr_exit_pre_((isrnest_), (prio_)))

//! Flush the QS trace data to the host
//!
//! @description
//! This macro invokes the QP::QS::flush() platform-dependent callback
//! function to flush the QS trace buffer to the host. The function
//! typically busy-waits until all the data in the buffer is sent to
//! the host. This is acceptable only in the initial transient.
#define QS_FLUSH()   (QP::QS::onFlush())

//! Execute an action that is only necessary for QS output
#define QF_QS_ACTION(act_)      (act_)

//! macro to handle the QS output from the application
//! NOTE: if this macro is used, the application must define QS_output().
#define QS_OUTPUT()   (QS_output())

//! macro to handle the QS-RX input to the application
//! NOTE: if this macro is used, the application must define QS_rx_input().
#define QS_RX_INPUT() (QS_rx_input())

//============================================================================
// Macros for use in QUTest only

#ifdef Q_UTEST
    //! QS macro to define the Test-Probe for a given @p fun_
    #define QS_TEST_PROBE_DEF(fun_)                                       \
        std::uint32_t const qs_tp_ =                                      \
            QP::QS::getTestProbe_(QP::QS::force_cast<void (*)(void)>(fun_));

    //! QS macro to apply a Test-Probe
    #define QS_TEST_PROBE(code_)  \
        if (qs_tp_ != 0U) { code_ }

    //! QS macro to apply a Test-Probe
    #define QS_TEST_PROBE_ID(id_, code_)                       \
        if (qs_tp_ == static_cast<std::uint32_t>(id_)) { code_ }

    //! QS macro to pause test execution and enter the test event loop
    #define QS_TEST_PAUSE() (QP::QS::test_pause_())

#else
    // dummy definitions when not building for QUTEST
    #define QS_TEST_PROBE_DEF(fun_)
    #define QS_TEST_PROBE(code_)
    #define QS_TEST_PROBE_ID(id_, code_)
    #define QS_TEST_PAUSE()  ((void)0)
#endif // Q_UTEST

#endif // QS_HPP
