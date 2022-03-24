/**
* @file
* @brief "Experimental" QF/CPP port to Espressif ESP-IDF (version 4.x)
* @ingroup ports
* @cond
******************************************************************************
* Last updated for version 6.9.4
* Last updated on  2022-03-20
* Copyright (C) 2022 Victor Chavez
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005-2022 Quantum Leaps, LLC. All rights reserved.
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses>.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
******************************************************************************
* @endcond
*/
#define QP_IMPL           /* this is QP implementation */
#include "qf_port.hpp"      /* QF port */
#include "qf_pkg.hpp"
#include "qassert.h"
#ifdef Q_SPY              /* QS software tracing enabled? */
    #include "qs_port.hpp"  /* QS port */
    #include "qs_pkg.hpp"   /* QS package-scope internal interface */
#else
    #include "qs_dummy.hpp" /* disable the QS software tracing */
#endif /* Q_SPY */

#include "esp_log.h"
#include "esp_freertos_hooks.h"
#include "freertos/task.h"

#if ( configMAX_PRIORITIES < QF_MAX_ACTIVE )
    #error "FreeRTOS configMAX_PRIORITIES must not be less than QF_MAX_ACTIVE"
#endif


/* Global objects ----------------------------------------------------------*/
PRIVILEGED_DATA portMUX_TYPE QF_esp32mux = portMUX_INITIALIZER_UNLOCKED;

// namespace QP ==============================================================
namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

/* Local objects -----------------------------------------------------------*/
static void task_function(void *pvParameters); /* FreeRTOS task signature */


/*==========================================================================*/


void QF::init(void) {

}
/*..........................................................................*/
int_t QF::run(void) {
    onStartup();
   // produce the QS_QF_RUN trace record
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()
#ifndef ARDUINO
    //vTaskDelete(NULL);
#endif
    return 0; /* dummy return to make the compiler happy */
}
/*..........................................................................*/
void QF::stop(void) {
    onCleanup(); /* cleanup callback */
}
/*..........................................................................*/
void QActive::start(std::uint_fast8_t const prio,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{

    /* task name provided by the user in QF_setTaskName() or default name */
    char_t const *taskName = (m_thread != nullptr)
                             ? reinterpret_cast<char_t const *>(m_thread)
                             : static_cast<char_t const *>("AO");

    Q_REQUIRE_ID(200, (0U < prio)
        && (prio <= QF_MAX_ACTIVE) /* in range */
        && (qSto != nullptr) /* queue storage must be provided */
        && (qLen > 0U)             /* queue size must be provided */
       // && (stkSto != nullptr)   /* stack storage must be provided */
        && (stkSize > 0U));        /* stack size must be provided */

    /* create the event queue for the AO */
    m_eQueue.init(qSto, qLen);

    m_prio = prio;  /* save the QF priority */
	QF::add_(this); // make QF aware of this active object
    init(par, m_prio); /* the top-most initial tran. */
    QS_FLUSH(); /* flush the QS trace buffer to the host */
								
										
    /* statically create the FreeRTOS task for the AO */
    BaseType_t task_res = xTaskCreatePinnedToCore(
              &task_function,           /* the task function */
              taskName ,                /* the name of the task */
              stkSize/sizeof(portSTACK_TYPE), /* stack size */
              this,               /* the 'pvParameters' parameter */
              prio + tskIDLE_PRIORITY,  /* FreeRTOS priority */
              //static_cast<StackType_t *>(stkSto),    /* stack storage */
              &m_thread,              /* task buffer */
              static_cast<BaseType_t>(QP_CPU_NUM));            /* CPU number */
     
    Q_ENSURE_ID(210, task_res == pdPASS ); /* must be created */
}
/*..........................................................................*/
void QActive::setAttr(std::uint32_t attr1, void const *attr2) {
    /* this function must be called before QACTIVE_START(),
    * which implies that me->thread.pxDummy1 must not be used yet;
    */
    Q_REQUIRE_ID(300, m_thread == nullptr);
    switch (attr1) {
        case TASK_NAME_ATTR:
            /* temporarily store the name */
            m_thread = reinterpret_cast<QF_THREAD_TYPE>(const_cast<void *>(attr2)); /* cast 'const' away */
            break;
        /* ... */
    }
}

// thread for active objects -------------------------------------------------
void QF::thread_(QActive *act) {
    /* event-loop */
    for (;;) { /* for-ever */
        QEvt const *e = act->get_(); // wait for event
		act->dispatch(e, act->m_prio); // dispatch to the AO's state machine
        gc(e); // check if the event is garbage, and collect it if so
    }
}
/*..........................................................................*/
static void task_function(void *pvParameters) { /* FreeRTOS task signature */
    QActive *act = reinterpret_cast<QActive *>(pvParameters);
	QF::thread_(act);
}

/*==========================================================================*/
/* The "FromISR" QP APIs for the FreeRTOS port... */
#ifdef Q_SPY
bool QActive::postFromISR_(QEvt const * const e,
                           std::uint_fast16_t const margin, void *par,
                           void const * const sender) noexcept
#else
bool QActive::postFromISR_(QEvt const * const e,
                           std::uint_fast16_t const margin,
                           void *par) noexcept
#endif
{
    QEQueueCtr nFree; /* temporary to avoid UB for volatile access */
    bool status;

    /** @pre event pointer must be valid */
    Q_REQUIRE_ID(400, e != nullptr);

    portENTER_CRITICAL_ISR(&QF_esp32mux);
    nFree = m_eQueue.m_nFree; /* get volatile into the temporary */
    if (margin == QF_NO_MARGIN) {
        if (nFree > 0U) {
            status = true; /* can post */
        }
        else {
            status = false; /* cannot post */
            Q_ERROR_ID(410); /* must be able to post the event */
        }
    }
    else if (nFree > static_cast<QEQueueCtr>(margin)) {
        status = true; /* can post */
    }
    else {
        status = false; /* cannot post */
    }

    if (status) { /* can post the event? */
        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
            QS_TIME_PRE_();       /* timestamp */
            QS_OBJ_PRE_(sender);  /* the sender object */
            QS_SIG_PRE_(e->sig);  /* the signal of the event */
            QS_OBJ_PRE_(this);      /* this active object (recipient) */
            QS_2U8_PRE_(e->poolId_, e->refCtr_); /* pool Id & ref Count */
            QS_EQC_PRE_(nFree);   /* number of free entries */
            QS_EQC_PRE_(m_eQueue.m_nMin); /* min number of free entries */
        QS_END_NOCRIT_PRE_()

        /* is it a pool event? */
        if (e->poolId_ != 0U) {
            QF_EVT_REF_CTR_INC_(e); /* increment the reference counter */
        }
        --nFree; /* one free entry just used up */
        m_eQueue.m_nFree = nFree;    /* update the volatile */
        if (m_eQueue.m_nMin > nFree) {
            m_eQueue.m_nMin = nFree; /* update minimum so far */
        }

        /* empty queue? */
        if (m_eQueue.m_frontEvt == nullptr) {
            m_eQueue.m_frontEvt = e;    /* deliver event directly */
            portEXIT_CRITICAL_ISR(&QF_esp32mux);
            /* signal the event queue */
            vTaskNotifyGiveFromISR(static_cast<TaskHandle_t>(m_thread),
                                   static_cast<BaseType_t *>(par));
        }
        /* queue is not empty, insert event into the ring-buffer */
        else {
            /* insert event into the ring buffer (FIFO) */
            QF_PTR_AT_(m_eQueue.m_ring, m_eQueue.m_head) = e;
            if (m_eQueue.m_head == 0U) { /* need to wrap head? */
                m_eQueue.m_head = m_eQueue.m_end;   /* wrap around */
            }
            --m_eQueue.m_head; /* advance the head (counter clockwise) */
            portEXIT_CRITICAL_ISR(&QF_esp32mux);
        }
    }
    else {

        QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_ATTEMPT, m_prio)
            QS_TIME_PRE_();      /* timestamp */
            QS_OBJ_PRE_(sender); /* the sender object */
            QS_SIG_PRE_(e->sig); /* the signal of the event */
            QS_OBJ_PRE_(this);     /* this active object (recipient) */
            QS_2U8_PRE_(e->poolId_, e->refCtr_); /* pool Id & ref Count */
            QS_EQC_PRE_(nFree);  /* number of free entries */
            QS_EQC_PRE_(margin); /* margin requested */
        QS_END_NOCRIT_PRE_()

        portEXIT_CRITICAL_ISR(&QF_esp32mux);

        QF::gcFromISR(e); // recycle the event to avoid a leak
    }

    return status;
}

/*..........................................................................*/
#ifdef Q_SPY
void QF::publishFromISR_(QEvt const *e, void *par,
                         void const * const sender) noexcept
#else
void QF::publishFromISR_(QEvt const *e, void *par) noexcept
#endif
{
    QPSet subscrList; /* local, modifiable copy of the subscriber list */

    /** @pre the published signal must be within the configured range */
    Q_REQUIRE_ID(500, static_cast<enum_t>(e->sig) < QF_maxPubSignal_);

    portENTER_CRITICAL_ISR(&QF_esp32mux);

    QS_BEGIN_NOCRIT_PRE_(QS_QF_PUBLISH, 0U)
        QS_TIME_PRE_();          /* the timestamp */
        QS_OBJ_PRE_(sender);     /* the sender object */
        QS_SIG_PRE_(e->sig);     /* the signal of the event */
        QS_2U8_PRE_(e->poolId_, e->refCtr_);/* pool-Id & ref-Count */
    QS_END_NOCRIT_PRE_()

    /* is it a dynamic event? */
    if (e->poolId_ != 0U) {
        /* NOTE: The reference counter of a dynamic event is incremented to
        * prevent premature recycling of the event while the multicasting
        * is still in progress. At the end of the function, the garbage
        * collector step (QF_gcFromISR()) decrements the reference counter and
        * recycles the event if the counter drops to zero. This covers the
        * case when the event was published without any subscribers.
        */
        QF_EVT_REF_CTR_INC_(e);
    }

    /* make a local, modifiable copy of the subscriber list */
    subscrList = QF_PTR_AT_(QF_subscrList_, e->sig);
    portEXIT_CRITICAL_ISR(&QF_esp32mux);

    if (subscrList.notEmpty()) { /* any subscribers? */
		// the highest-prio subscriber
        std::uint_fast8_t p = subscrList.findMax();

        /* no need to lock the scheduler in the ISR context */
        do { /* loop over all subscribers */
            /* the prio of the AO must be registered with the framework */
            Q_ASSERT_ID(510, active_[p] != (QActive *)0);

			// POST_FROM_ISR() asserts internally if the queue overflows
            (void)active_[p]->POST_FROM_ISR(e, par, sender);

            subscrList.rmove(p); // remove the handled subscriber
			if (subscrList.notEmpty()) {  // still more subscribers?
                p = subscrList.findMax(); // the highest-prio subscriber
            }
            else {
                p = 0U; /* no more subscribers */
            }
        } while (p != 0U);
        /* no need to unlock the scheduler in the ISR context */
    }

    /* The following garbage collection step decrements the reference counter
    * and recycles the event if the counter drops to zero. This covers both
    * cases when the event was published with or without any subscribers.
    */
	gcFromISR(e);
}


/*..........................................................................*/
#ifdef Q_SPY
void QF::tickXfromISR_(std::uint_fast8_t const tickRate, void *par,
                       void const * const sender) noexcept
#else
void QF::tickXfromISR_(std::uint_fast8_t const tickRate, void *par) noexcept
#endif
{
    QTimeEvt *prev = &timeEvtHead_[tickRate];

    portENTER_CRITICAL_ISR(&QF_esp32mux);

    QS_BEGIN_NOCRIT_PRE_(QS_QF_TICK, 0U)
        ++prev->m_ctr;
        QS_TEC_PRE_(prev->m_ctr); /* tick ctr */
        QS_U8_PRE_(tickRate);   /* tick rate */
    QS_END_NOCRIT_PRE_()

    /* scan the linked-list of time events at this rate... */
    for (;;) {
        QTimeEvt *t = prev->m_next;  /* advance down the time evt. list */

        /* end of the list? */
        if (t == nullptr) {

            /* any new time events armed since the last run of QF_tickX_()? */
            if (timeEvtHead_[tickRate].m_act != nullptr) {

                /* sanity check */
                Q_ASSERT_ID(610, prev != nullptr);
                prev->m_next = QF::timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = nullptr;
                t = prev->m_next;  /* switch to the new list */
            }
            else {
                break; /* all currently armed time evts. processed */
            }
        }

        /* time event scheduled for removal? */
        if (t->m_ctr == 0U) {
            prev->m_next = t->m_next;
            /* mark time event 't' as NOT linked */
            t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);
            /* do NOT advance the prev pointer */
            /* exit crit. section to reduce latency */
            portEXIT_CRITICAL_ISR(&QF_esp32mux);
        }
        else {
            --t->m_ctr;

            /* is time event about to expire? */
            if (t->m_ctr ==  0U) {
                QActive *act = t->toActive(); // temp. for volatile

                /* periodic time evt? */
                if (t->m_interval != 0U) {
                    t->m_ctr = t->m_interval; /* rearm the time event */
                    prev = t; /* advance to this time event */
                }
                /* one-shot time event: automatically disarm */
                else {
                    prev->m_next = t->m_next;
                    /* mark time event 't' as NOT linked */
                    t->refCtr_ &= static_cast<std::uint8_t>(~TE_IS_LINKED);
                    /* do NOT advance the prev pointer */

                    QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_AUTO_DISARM,
                                         act->m_prio)
                        QS_OBJ_PRE_(t);        /* this time event object */
                        QS_OBJ_PRE_(act);      /* the target AO */
                        QS_U8_PRE_(tickRate);  /* tick rate */
                    QS_END_NOCRIT_PRE_()
                }

                QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_POST, act->m_prio)
                    QS_TIME_PRE_();            /* timestamp */
                    QS_OBJ_PRE_(t);            /* the time event object */
                    QS_SIG_PRE_(t->sig); /* signal of this time event */
                    QS_OBJ_PRE_(act);          /* the target AO */
                    QS_U8_PRE_(tickRate);      /* tick rate */
                QS_END_NOCRIT_PRE_()

                /* exit critical section before posting */
                portEXIT_CRITICAL_ISR(&QF_esp32mux);

                // asserts if queue overflows
                (void)act->POST_FROM_ISR(t, par, sender);
            }
            else {
                prev = t;         /* advance to this time event */
                /* exit crit. section to reduce latency */
                portEXIT_CRITICAL_ISR(&QF_esp32mux);
            }
        }
        /* re-enter crit. section to continue */
        portENTER_CRITICAL_ISR(&QF_esp32mux);
    }
    portEXIT_CRITICAL_ISR(&QF_esp32mux);
}


/*..........................................................................*/
QEvt *QF::newXfromISR_(std::uint_fast16_t const evtSize,
                       std::uint_fast16_t const margin,
                       enum_t const sig) noexcept
{
    QEvt *e;
    std::uint_fast8_t idx;

    /* find the pool index that fits the requested event size ... */
    for (idx = 0U; idx < QF_maxPool_; ++idx) {
        if (evtSize <= QF_EPOOL_EVENT_SIZE_(QF_pool_[idx])) {
            break;
        }
    }
    /* cannot run out of registered pools */
    Q_ASSERT_ID(710, idx < QF_maxPool_);


    /* get e -- platform-dependent */
#ifdef Q_SPY
    e = static_cast<QEvt *>(
              QF_pool_[idx].getFromISR(((margin != QF_NO_MARGIN)
                  ? margin : 0U),
                  static_cast<std::uint_fast8_t>(QS_EP_ID) + idx + 1U));
#else
    e = static_cast<QEvt *>(
              QF_pool_[idx].getFromISR(((margin != QF_NO_MARGIN)
                  ? margin : 0U), 0U));
#endif

    /* was e allocated correctly? */
    if (e != nullptr) {
        e->sig     = static_cast<QSignal>(sig); // set the signal
        e->poolId_ = static_cast<std::uint8_t>(idx + 1U); /* store the pool ID */
        e->refCtr_ = 0U; /* set the reference counter to 0 */

#ifdef Q_SPY
        portENTER_CRITICAL_ISR(&QF_esp32mux);
        QS_BEGIN_PRE_(QS_QF_NEW, static_cast<uint_fast8_t>(QS_EP_ID) + idx + 1U)
            QS_TIME_PRE_();         /* timestamp */
            QS_EVS_PRE_(evtSize);   /* the size of the event */
            QS_SIG_PRE_(sig);       /* the signal of the event */
        QS_END_NOCRIT_PRE_()
        portEXIT_CRITICAL_ISR(&QF_esp32mux);
#endif /* Q_SPY */
    }
    /* event cannot be allocated */
    else {
        /* must tolerate bad alloc. */
        Q_ASSERT_ID(720, margin != QF_NO_MARGIN);

#ifdef Q_SPY
        portENTER_CRITICAL_ISR(&QF_esp32mux);
        QS_BEGIN_PRE_(QS_QF_NEW_ATTEMPT, static_cast<uint_fast8_t>(QS_EP_ID) + idx + 1U)
            QS_TIME_PRE_();         /* timestamp */
            QS_EVS_PRE_(evtSize);   /* the size of the event */
            QS_SIG_PRE_(sig);       /* the signal of the event */
        QS_END_NOCRIT_PRE_()
        portEXIT_CRITICAL_ISR(&QF_esp32mux);
#endif /* Q_SPY */
    }
    return e; /* can't be NULL if we can't tolerate bad allocation */
}
/*..........................................................................*/
void QF::gcFromISR(QEvt const * const e) noexcept {

    /* is it a dynamic event? */
    if (e->poolId_ != 0U) {
        portENTER_CRITICAL_ISR(&QF_esp32mux);

        /* isn't this the last ref? */
        if (e->refCtr_ > 1U) {
            QF_EVT_REF_CTR_DEC_(e); /* decrements the ref counter */

            QS_BEGIN_NOCRIT_PRE_(QS_QF_GC_ATTEMPT,
                                 static_cast<uint_fast8_t>(e->poolId_))
                QS_TIME_PRE_();      /* timestamp */
                QS_SIG_PRE_(e->sig); /* the signal of the event */
                QS_2U8_PRE_(e->poolId_, e->refCtr_); /* pool Id & ref Count */
            QS_END_NOCRIT_PRE_()

            portEXIT_CRITICAL_ISR(&QF_esp32mux);
        }
        /* this is the last reference to this event, recycle it */
        else {
            std::uint_fast8_t idx =
                static_cast<std::uint_fast8_t>(e->poolId_) - 1U;

            QS_BEGIN_NOCRIT_PRE_(QS_QF_GC,
                                 static_cast<uint_fast8_t>(e->poolId_))
                QS_TIME_PRE_();         /* timestamp */
                QS_SIG_PRE_(e->sig);    /* the signal of the event */
                QS_2U8_PRE_(e->poolId_, e->refCtr_); /* pool Id & ref Count */
            QS_END_NOCRIT_PRE_()

            portEXIT_CRITICAL_ISR(&QF_esp32mux);

            /* pool ID must be in range */
            Q_ASSERT_ID(810, idx < QF_maxPool_);
#ifdef Q_EVT_VIRTUAL
            // explicitly exectute the destructor'
            // NOTE: casting 'const' away is legitimate,
            // because it's a pool event
            QF_EVT_CONST_CAST_(e)->~QEvt(); // xtor,
#endif
#ifdef Q_SPY
            // cast 'const' away, which is OK, because it's a pool event
            QF_pool_[idx].putFromISR(QF_EVT_CONST_CAST_(e),
                static_cast<uint_fast8_t>(QS_EP_ID) + e->poolId_);
#else
            QF_pool_[idx].putFromISR(QF_EVT_CONST_CAST_(e), 0U);
#endif
        }
    }
}


/*..........................................................................*/
void QMPool::putFromISR(void *b, std::uint_fast8_t const qs_id) noexcept {

    /** @pre # free blocks cannot exceed the total # blocks and
    * the block pointer must be from this pool.
    */
    Q_REQUIRE_ID(900, (m_nFree < m_nTot)
                      && QF_PTR_RANGE_(b, m_start, m_end));

    (void)qs_id; /* unused parameter (outside Q_SPY build configuration) */
    portENTER_CRITICAL_ISR(&QF_esp32mux);

    static_cast<QFreeBlock*>(b)->m_next =
        static_cast<QFreeBlock *>(m_free_head); // link into the free list
    m_free_head = b;      /* set as new head of the free list */
    ++m_nFree;            /* one more free block in this pool */

    QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_PUT, qs_id)
        QS_TIME_PRE_();         /* timestamp */
        QS_OBJ_PRE_(m_start); /* the memory managed by this pool */
        QS_MPC_PRE_(m_nFree); /* # free blocks in the pool */
    QS_END_NOCRIT_PRE_()

    portEXIT_CRITICAL_ISR(&QF_esp32mux);
}
/*..........................................................................*/
void *QMPool::getFromISR(std::uint_fast16_t const margin,
                         std::uint_fast8_t const qs_id) noexcept {
    QFreeBlock *fb;

    (void)qs_id; /* unused parameter (outside Q_SPY build configuration) */
    portENTER_CRITICAL_ISR(&QF_esp32mux);

    /* have more free blocks than the requested margin? */
    if (m_nFree > static_cast<QMPoolCtr>(margin)) {
        
        fb = static_cast<QFreeBlock *>(m_free_head);  // get a free block

        /* the pool has some free blocks, so a free block must be available */
        Q_ASSERT_ID(910, fb != nullptr);

        void *fb_next = fb->m_next; /* put volatile to a temporary to avoid UB */

        /* is the pool becoming empty? */
        --m_nFree; /* one less free block */
        if (m_nFree == 0) {
            /* pool is becoming empty, so the next free block must be NULL */
            Q_ASSERT_ID(920, fb_next == nullptr);

            m_nMin = 0U; /* remember that the pool got empty */
        }
        else {
            /* pool is not empty, so the next free block must be in range
            *
            * NOTE: the next free block pointer can fall out of range
            * when the client code writes past the memory block, thus
            * corrupting the next block.
            */
            Q_ASSERT_ID(930, QF_PTR_RANGE_(fb_next, m_start, m_end));

            /* is the number of free blocks the new minimum so far? */
            if (m_nMin > m_nFree) {
                m_nMin = m_nFree; /* remember the new minimum */
            }
        }

        m_free_head = fb_next; /* set the head to the next free block */

        QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_GET, qs_id)
            QS_TIME_PRE_();         /* timestamp */
            QS_OBJ_PRE_(m_start); /* the memory managed by this pool */
            QS_MPC_PRE_(m_nFree); /* # free blocks in the pool */
            QS_MPC_PRE_(m_nMin);  /* min # free blocks ever in the pool */
        QS_END_NOCRIT_PRE_()

    }
    /* don't have enough free blocks at this point */
    else {
        fb = nullptr;

        QS_BEGIN_NOCRIT_PRE_(QS_QF_MPOOL_GET_ATTEMPT, qs_id)
            QS_TIME_PRE_();         /* timestamp */
            QS_OBJ_PRE_(m_start); /* the memory managed by this pool */
            QS_MPC_PRE_(m_nFree); /* # free blocks in the pool */
            QS_MPC_PRE_(margin);    /* the requested margin */
        QS_END_NOCRIT_PRE_()
    }
    portEXIT_CRITICAL_ISR(&QF_esp32mux);

    return fb; /* return the pointer to memory block or NULL to the caller */
}


} // namespace QP
