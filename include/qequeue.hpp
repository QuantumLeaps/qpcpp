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
//! @brief platform-independent fast "raw" thread-safe event queue interface

#ifndef QEQUEUE_HPP
#define QEQUEUE_HPP

//! @description
//! This header file must be included in all QF ports that use native QF
//! event queue for active objects. Also, this file needs to be included
//! in the QP/C++ library when the application uses QP::QMActive::defer() /
//! QP::QMActive::recall(). Finally, this file is also needed when the "raw"
//! thread-safe queues are used for communication between active objects
//! and non-framework entities, such as ISRs, device drivers, or legacy
//! code.

#ifndef QF_EQUEUE_CTR_SIZE

    //! The size (in bytes) of the ring-buffer counters used in the
    //! native QF event queue implementation. Valid values: 1U, 2U, or 4U;
    //! default 1U.
    //! @description
    //! This macro can be defined in the QF port file (qf_port.hpp) to
    //! configure the QP::QEQueueCtr type. Here the macro is not defined
    //! so the default of 1 byte is chosen.
    #define QF_EQUEUE_CTR_SIZE 1U
#endif

namespace QP {

#if (QF_EQUEUE_CTR_SIZE == 1U)
    //! The data type to store the ring-buffer counters based on
    //! the macro #QF_EQUEUE_CTR_SIZE.
    //! @description
    //! The dynamic range of this data type determines the maximum length
    //! of the ring buffer managed by the native QF event queue.
    using QEQueueCtr = std::uint8_t;
#elif (QF_EQUEUE_CTR_SIZE == 2U)
    using QEQueueCtr = std::uint16_t;
#elif (QF_EQUEUE_CTR_SIZE == 4U)
    using QEQueueCtr = std::uint32_t;
#else
    #error "QF_EQUEUE_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

//============================================================================
//! Native QF Event Queue class
//! @description
//! This structure describes the native QF event queue, which can be used as
//! the event queue for active objects, or as a simple "raw" event queue for
//! thread-safe event passing among non-framework entities, such as ISRs,
//! device drivers, or other third-party components.@n
//! @n
//! The native QF event queue is configured by defining the macro
//! #QF_EQUEUE_TYPE as QP::QEQueue in the specific QF port header file.@n
//! @n
//! The QP::QEQueue class contains only data members for managing an event
//! queue, but does not contain the storage for the queue buffer, which must
//! be provided externally during the queue initialization.@n
//! @n
//! The event queue can store only event pointers, not the whole events. The
//! internal implementation uses the standard ring-buffer plus one external
//! location that optimizes the queue operation for the most frequent case
//! of empty queue.@n
//! @n
//! The QP::QEQueue class is used with two sets of functions. One set is for
//! the active object event queue, which needs to block the active object
//! task when the event queue is empty and unblock it when events are posted
//! to the queue. The interface for the native active object event queue
//! consists of the following functions: QP::QMActive::post(),
//! QP::QMActive::postLIFO(), and QP::QMActive::get_(). Additionally the
//! function QP::QEQueue::init() is used to initialize the queue.@n
//! @n
//! The other set of functions, uses this class as a simple "raw" event
//! queue to pass events between entities other than active objects, such as
//! ISRs. The "raw" event queue is not capable of blocking on the get()
//! operation, but is still thread-safe because it uses QF critical section
//! to protect its integrity. The interface for the "raw" thread-safe queue
//! consists of the following functions: QP::QEQueue::post(),
//! QP::QEQueue::postLIFO(), and QP::QEQueue::get(). Additionally the
//! function QP::QEQueue::init() is used to initialize the queue.@n
//! @n
//! @note Most event queue operations (both the active object queues and
//! the "raw" queues) internally use  the QF critical section. You should be
//! careful not to invoke those operations from other critical sections when
//! nesting of critical sections is not supported.
class QEQueue {
private:

    //! pointer to event at the front of the queue
    //! @description
    //! All incoming and outgoing events pass through the m_frontEvt location.
    //! When the queue is empty (which is most of the time), the extra
    //! m_frontEvt location allows to bypass the ring buffer altogether,
    //! greatly optimizing the performance of the queue. Only bursts of events
    //! engage the ring buffer.@n
    //! @n
    //! The additional role of this attribute is to indicate the empty status
    //! of the queue. The queue is empty if the m_frontEvt location is NULL.
    QEvt const * volatile m_frontEvt;

    //! pointer to the start of the ring buffer
    QEvt const **m_ring;

    //! offset of the end of the ring buffer from the start of the buffer
    QEQueueCtr m_end;

    //! offset to where next event will be inserted into the buffer
    QEQueueCtr volatile m_head;

    //! offset of where next event will be extracted from the buffer
    QEQueueCtr volatile m_tail;

    //! number of free events in the ring buffer
    QEQueueCtr volatile m_nFree;

    //! minimum number of free events ever in the ring buffer.
    //! @note this attribute remembers the low-watermark of the ring buffer,
    //! which provides a valuable information for sizing event queues.
    //! @sa QP::QF::getQueueMin().
    QEQueueCtr m_nMin;

public:
    //! public default constructor
    QEQueue(void) noexcept;

    //! Initializes the native QF event queue
    //!
    //! @description
    //! Initialize the event queue by giving it the storage for the
    //! ring buffer.
    //!
    //! @param[in] qSto an array of pointers to QP::QEvt to serve as the
    //!                 ring buffer for the event queue
    //! @param[in] qLen the length of the qSto[] buffer (in QP::QEvt pointers)
    //!
    //! @note
    //! The actual capacity of the queue is qLen + 1, because of the extra
    //! location forntEvt.
    //!
    //! @note
    //! This function is also used to initialize the event queues of active
    //! objects in the built-int QV, QK and QXK kernels, as well as other
    //! QP ports to OSes/RTOSes that do provide a suitable message queue.
    //!
    void init(QEvt const *qSto[], std::uint_fast16_t const qLen) noexcept;

    //! Posts (FIFO) an event to the "raw" thread-safe QF event queue
    //!
    //! @description
    //! Post an event to the "raw" thread-safe event queue using the
    //! First-In-First-Out (FIFO) order.
    //!
    //! @param[in] e      pointer to the event to be posted to the queue
    //! @param[in] margin number of required free slots in the queue after
    //!                   posting the event. The special value
    //!                   QP::QF_NO_MARGIN means that this function will
    //!                   assert if posting
    //! @param[in] qs_id  QS-id of this state machine (for QS local filter)
    //!
    //! @note
    //! The QP::QF_NO_MARGIN value of the @p margin argument is special and
    //! denotes situation when the post() operation is assumed to succeed
    //! (event delivery guarantee). An assertion fires, when the event cannot
    //! be delivered in this case.
    //!
    //! @returns 'true' (success) when the posting succeeded with the provided
    //! margin and 'false' (failure) when the posting fails.
    //!
    //! @note
    //! This function can be called from any task context or ISR context.
    //!
    //! @sa QP::QEQueue::postLIFO(), QP::QEQueue::get()
    //!
    bool post(QEvt const * const e, std::uint_fast16_t const margin,
              std::uint_fast8_t const qs_id) noexcept;

    //! Posts (LIFO) an event to the "raw" thread-safe QF event queue
    //!
    //! @description
    //! Post an event to the "raw" thread-safe event queue using the
    //! Last-In-First-Out (LIFO) order.
    //!
    //! @param[in] e     pointer to the event to be posted to the queue
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @attention
    //! The LIFO policy should be used only with great __caution__,
    //! because it alters the order of events in the queue.
    //!
    //! @note
    //! This function can be called from any task context or ISR context.
    //!
    //! @note
    //! This function is used for the "raw" thread-safe queues and __not__
    //! for the queues of active objects.
    //!
    //! @sa
    //! QP::QEQueue::post(), QP::QEQueue::get(), QP::QActive::defer()
    //!
    void postLIFO(QEvt const * const e,
                  std::uint_fast8_t const qs_id) noexcept;

    //! Gets an event from the "raw" thread-safe QF event queue
    //!
    //! @description
    //! Retrieves an event from the front of the "raw" thread-safe queue and
    //! returns a pointer to this event to the caller.
    //!
    //! @param[in] qs_id QS-id of this state machine (for QS local filter)
    //!
    //! @returns
    //! pointer to event at the front of the queue, if the queue is
    //! not empty and NULL if the queue is empty.
    //!
    //! @note
    //! this function is used for the "raw" thread-safe queues and __not__
    //! for the queues of active objects.
    //!
    //! @sa
    //! QP::QEQueue::post(), QP::QEQueue::postLIFO(), QP::QActive::recall()
    //!
    QEvt const *get(std::uint_fast8_t const qs_id) noexcept;

    //! Gets the number of free slots currently in "raw" thread-safe
    //! QF event queue
    //!
    //! @note
    //! This operation needs to be used with caution because the
    //! number of free entries can change unexpectedly. The main intent for
    //! using this operation is in conjunction with event deferral. In this
    //! case the queue is accessed only from a single thread (by a single AO),
    //! so the number of free entries cannot change unexpectedly.
    //!
    //! @sa QP::QMActive::defer(), QP::QMActive::recall()
    //!
    QEQueueCtr getNFree(void) const noexcept {
        return m_nFree;
    }

    //! "raw" thread-safe QF event queue operation for obtaining the minimum
    //! number of free entries ever in the queue (a.k.a. "low-watermark").
    //!
    //! @description
    //! This operation needs to be used with caution because the
    //! "low-watermark" can change unexpectedly. The main intent for using
    //! this operation is to get an idea of queue usage to size the queue
    //! adequately.
    //!
    //! @returns the minimum number of free entries ever in the queue
    //! since init.
    QEQueueCtr getNMin(void) const noexcept {
        return m_nMin;
    }

    //! "raw" thread-safe QF event queue operation to find out if the queue
    //! is empty
    //! @note
    //! This operation needs to be used with caution because the
    //! queue status can change unexpectedly. The main intent for using
    //! this operation is in conjunction with event deferral. In this case
    //! the queue is accessed only from a single thread (by a single AO),
    //! so no other entity can post events to the queue.
    //!
    //! @sa QP::QMActive::defer(), QP::QMActive::recall()
    bool isEmpty(void) const noexcept {
        return m_frontEvt == nullptr;
    }

private:
    //! disallow copying of QEQueue
    QEQueue(QEQueue const &) = delete;

    //! disallow assignment of QEQueue
    QEQueue & operator=(QEQueue const &) = delete;

    friend class QF;
    friend class QActive;
    friend class QXThread;
    friend class QTicker;
    friend class QS;
};

} // namespace QP

#endif // QEQUEUE_HPP
