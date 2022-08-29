//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
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
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-08-25
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QP/C++ port to Qt

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"
#include "qassert.h"
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // include QS port
    #include "qs_pkg.hpp"   // QS package-scope internal interface
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

namespace {
    Q_DEFINE_THIS_MODULE("guiapp")
}

//============================================================================
namespace QP {

static QEvent::Type l_qp_event_type = QEvent::MaxUser;

//----------------------------------------------------------------------------
class QP_Event : public QEvent {
public:
    QP_Event(QP::QEvt const *e)
      : QEvent(l_qp_event_type),
        m_qpevt(e)
    {}

    QP::QEvt const *m_qpevt; // QP event pointer
};

//----------------------------------------------------------------------------
GuiApp::GuiApp(int &argc, char **argv)
  : QApplication(argc, argv),
    m_act(0)
{
    l_qp_event_type = static_cast<QEvent::Type>(QEvent::registerEventType());
}
//............................................................................
void GuiApp::registerAct(void *act) {
    Q_REQUIRE(m_act == 0); // the GUI active object must not be registered
    m_act = act;
}
//............................................................................
bool GuiApp::event(QEvent *e) {
    if (e->type() == l_qp_event_type) {
        QP::QEvt const *qpevt = (static_cast<QP_Event *>(e))->m_qpevt;
        QP::QActive *qpact = static_cast<QP::QActive *>(m_act);
        qpact->dispatch(qpevt, qpact->m_prio); // dispatch to AO
        QP::QF::gc(qpevt); // garbage collect the QP evt
        return true; // event recognized and handled
    }
    else {
        return QApplication::event(e); // delegate to the superclass
    }
}

//............................................................................
void GuiQActive::start(QPrioSpec const prioSpec,
                   QEvt const * * const qSto, std::uint_fast16_t const qLen,
                   void * const stkSto, std::uint_fast16_t const stkSize,
                   void const * const par)
{
    Q_REQUIRE((qSto == nullptr) /* no need for per-AO queue */
              && (stkSto == nullptr)); // no need for per-AO stack

    Q_UNUSED_PAR(stkSize);
    Q_UNUSED_PAR(qLen);

    setPrio(prioSpec); // set the priority specification of this AO
    register_(); // make QF aware of this active object

    static_cast<GuiApp *>(QApplication::instance())->registerAct(this);

    this->init(par, getPrio()); // execute initial transition (virtual call)
    QS_FLUSH(); // flush the trace buffer to the host
}
//............................................................................
bool GuiQActive::post_(QEvt const * const e,
                       std::uint_fast16_t const /*margin*/,
                       void const * const sender) noexcept
{
    Q_UNUSED(sender); // when Q_SPY is not defined

    QF_CRIT_STAT_
    QF_CRIT_E_();

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
        QS_TIME_PRE_();      // timestamp
        QS_OBJ_PRE_(sender); // the sender object
        QS_SIG_PRE_(e->sig); // the signal of the event
        QS_OBJ_PRE_(this);   // this active object
        QS_2U8_PRE_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_PRE_(0U);     // number of free entries (not used)
        QS_EQC_PRE_(0U);     // min number of free entries (not used)
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e));
    return true;
}
//............................................................................
void GuiQActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE_();      // timestamp
        QS_SIG_PRE_(e->sig); // the signal of this event
        QS_OBJ_PRE_(this);   // this active object
        QS_2U8_PRE_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_PRE_(0U);     // number of free entries (not used)
        QS_EQC_PRE_(0U);     // min number of free entries (not used)
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e),
                                Qt::HighEventPriority);
}

//============================================================================
void GuiQMActive::start(QPrioSpec const prioSpec,
                    QEvt const * * const qSto, std::uint_fast16_t const qLen,
                    void * const stkSto, std::uint_fast16_t const stkSize,
                    void const * const par)
{
    Q_REQUIRE((qSto == nullptr) /* no need for per-AO queue */
              && (stkSto == nullptr)); // no need for per-AO stack

    Q_UNUSED_PAR(stkSize);
    Q_UNUSED_PAR(qLen);

    setPrio(prioSpec); // set the priority specification  of this AO
    register_(); // make QF aware of this active object

    static_cast<GuiApp *>(QApplication::instance())->registerAct(this);

    this->init(par, getPrio()); // execute initial transition (virtual call)
    QS_FLUSH(); // flush the trace buffer to the host
}
//............................................................................
bool GuiQMActive::post_(QEvt const * const e,
                        std::uint_fast16_t const /*margin*/,
                        void const * const sender) noexcept
{
    Q_UNUSED(sender); // when Q_SPY is not defined

    QF_CRIT_STAT_
    QF_CRIT_E_();

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST, m_prio)
        QS_TIME_PRE_();      // timestamp
        QS_OBJ_PRE_(sender); // the sender object
        QS_SIG_PRE_(e->sig); // the signal of the event
        QS_OBJ_PRE_(this);   // this active object
        QS_2U8_PRE_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_PRE_(0U);     // number of free entries (not used)
        QS_EQC_PRE_(0U);     // min number of free entries (not used)
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e));
    return true;
}
//............................................................................
void GuiQMActive::postLIFO(QEvt const * const e) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != 0U) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_LIFO, m_prio)
        QS_TIME_PRE_();      // timestamp
        QS_SIG_PRE_(e->sig); // the signal of this event
        QS_OBJ_PRE_(this);   // this active object
        QS_2U8_PRE_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_PRE_(0U);     // number of free entries (not used)
        QS_EQC_PRE_(0U);     // min number of free entries (not used)
    QS_END_NOCRIT_PRE_()

    QF_CRIT_X_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e),
                                Qt::HighEventPriority);
}

} // namespace QP
