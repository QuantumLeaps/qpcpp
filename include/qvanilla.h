/// \file
/// \brief platform-independent interface to the cooperative "vanilla" kernel.
/// \ingroup qF
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.0
/// Last updated on  2014-04-10
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#ifndef qvanilla_h
#define qvanilla_h

#include "qequeue.h" // "Vanilla" kernel uses the native QF event queue
#include "qmpool.h"  // "Vanilla" kernel uses the native QF memory pool
#include "qpset.h"   // "Vanilla" kernel uses the native QF priority set

//****************************************************************************
// QF configuration for the Vanilla kernel

//! This macro defines the type of the event queue used for the active objects
///
/// \note This is just an example of the macro definition. Typically, you need
/// to define it in the specific QF port file (qf_port.h). In case of the
/// Vanilla kernel, which always depends on the native QF queue, this macro
/// is defined at the level of the platform-independent interface vanilla.h.
#define QF_EQUEUE_TYPE     QEQueue

//****************************************************************************
// interface used only inside QF, but not in applications

#ifdef QP_IMPL
    // native event queue operations
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        Q_ASSERT((me_)->m_eQueue.m_frontEvt != static_cast<QEvt const *>(0))

    #define QACTIVE_EQUEUE_SIGNAL_(me_) \
        (QF_readySet_.insert((me_)->m_prio))

    #define QACTIVE_EQUEUE_ONEMPTY_(me_) \
        (QF_readySet_.remove((me_)->m_prio))

    // native QF event pool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_fast16_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

    extern "C" {
        #if (QF_MAX_ACTIVE <= 8)
            extern QP::QPSet8  QF_readySet_;     //!< ready set of AOs
        #else
            extern QP::QPSet64 QF_readySet_;     //!< ready set of AOs
        #endif
    } // extern "C"

#endif // QP_IMPL

#endif // qvanilla_h
