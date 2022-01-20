/// @file
/// @brief QF/C++ port to uC/OS-III (V3.08) kernel, all supported compilers
/// @cond
///***************************************************************************
/// Last updated for version 6.9.3
/// Last updated on  2021-04-08
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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

#ifndef QF_PORT_HPP
#define QF_PORT_HPP

// uC/OS-II event queue and thread types
#define QF_EQUEUE_TYPE       OS_Q
#define QF_THREAD_TYPE       OS_TCB
// The maximum number of active objects in the application
#define QF_MAX_ACTIVE ((OS_CFG_PRIO_MAX - 2 < 64) ? (OS_CFG_PRIO_MAX - 2) : 64U)

#include "os.h" // uC/OS-III API, port and compile-time configuration
                                  

// uC/OS-III crtitical section, NOTE1
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_INT_DIS_EN)
    /* QF_CRIT_STAT_TYPE  not defined */
    #define QF_CRIT_ENTRY(dummy) CPU_CRITICAL_ENTER()
    #define QF_CRIT_EXIT(dummy)  CPU_CRITICAL_EXIT()
#elif (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    #define QF_CRIT_STAT_TYPE    CPU_SR
    #define QF_CRIT_ENTRY(dummy) CPU_CRITICAL_ENTER()
    #define QF_CRIT_EXIT(dummy)  CPU_CRITICAL_EXIT()
#else
    #error Unsupported uC/OS-III critical section type
#endif // CPU_CFG_CRITICAL_METHOD

namespace QP {
                                           
                                                 
                                                

enum UCOS3_TaskAttrs {
    TASK_NAME_ATTR,
    TASK_OPT_ATTR
};

} // namespace QP
                                      

#include "qep_port.hpp"  // QEP port, includes the master uC/OS-II include
#include "qequeue.hpp"   // used for event deferral
#include "qmpool.hpp"    // native QF event pool
#include "qpset.hpp"     // this QP port uses the native QP priority set
#include "qf.hpp"        // QF platform-independent public interface

                                                                         
                                   

//****************************************************************************
// interface used only inside QF, but not in applications
//
#ifdef QP_IMPL

    // uC/OS-II crtitical section, NOTE1
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    /* internal uC/OS-II critical section operations, NOTE1 */
    #define QF_CRIT_STAT_       CPU_SR cpu_sr;
    #define QF_CRIT_E_()    CPU_CRITICAL_ENTER()
    #define QF_CRIT_X_()     CPU_CRITICAL_EXIT()
#endif // CPU_CFG_CRITICAL_METHOD

    // uC/OS-II-specific scheduler locking, see NOTE2
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) do { \
        if (OSIntNestingCtr == 0) {    \
            OS_ERR err;               \
            OSSchedLock(&err);         \
        }                              \
    } while (false)

    #define QF_SCHED_UNLOCK_() do { \
        if (OSIntNestingCtr == 0) {    \
            OS_ERR err;               \
            OSSchedLock(&err);         \
        } \
    } while (false)

    // uC/OS-III event pool operations...
    #define QF_EPOOL_TYPE_ OS_MEM*
    #define QF_EPOOL_INIT_(pool_, poolSto_, poolSize_, evtSize_) do {       \
        OS_ERR err;                                                          \
        OSMemCreate((pool_),"EPool",(poolSto_),(CPU_INT32U)((poolSize_)/(evtSize_)), \
                              (CPU_INT32U)(evtSize_), &err);                    \
        Q_ASSERT_ID(105, err == OS_ERR_NONE);                               \
    } while (false)

    #define QF_EPOOL_EVENT_SIZE_(pool_) ((pool_)->BlkSize)
    #define QF_EPOOL_GET_(pool_, e_, m_, qs_id_) do { \
        QF_CRIT_STAT_                                 \
        QF_CRIT_E_();                                 \
        if ((pool_)->NbrFree > (m_)) {             \
            OS_ERR err;                                \
            (e_) = (QEvt *)OSMemGet((pool_), &err);   \
            Q_ASSERT_ID(205, err == OS_ERR_NONE);     \
        }                                             \
        else {                                        \
            (e_) = nullptr;                           \
        }                                             \
        QF_CRIT_X_();                                 \
    } while (false)

    #define QF_EPOOL_PUT_(pool_, e_, qs_id_)    \
        OS_ERR err;                             \
        OSMemPut((pool_), (e_),&err);           \
        Q_ALLEGE_ID(305, err == OS_ERR_NONE)

#endif // ifdef QP_IMPL

                                   

                                                       
                                            
                                                 
                                       

                                  
                                                                   
                                                                

//****************************************************************************
// NOTE1:
// This QP port to uC/OS-III re-uses the exact same critical section mechanism
// as uC/OS-II. The goal is to make this port independent on the CPU or the
// toolchain by employing only the official uC/OS-II API. That way, all CPU
// and toolchain dependencies are handled internally by uC/OS-II.
//
// NOTE2:
// uC/OS-II provides only global scheduler locking for all thread priorities
// by means of OSSchedLock() and OSSchedUnlock(). Therefore, locking the
// scheduler only up to the specified lock priority is not supported.
              
//
         
                                                                           
                                                                     
                                                                          
                                                                           
                                                                            
                                                                             
                                                                          
                                                                        
                                                                   
                                                  
  
         
                                                                          
                                                                       
                                                                            
                                                                            
                                                                         
                                                                      
                                                                         
                                                                               
                                                                              
                                                                               
                           
  
         
                                                                             
                                                                     
                                                                             
                                                                         
                                                            
  

#endif // QF_PORT_HPP

