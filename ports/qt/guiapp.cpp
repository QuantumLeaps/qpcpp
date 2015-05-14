/// @file
/// @brief QP/C++ port to Qt
/// @cond
///***************************************************************************
/// Last Updated for Version: QP 5.4.0/Qt 5.x
/// Last updated on  2015-05-03
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
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // include QS port
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

Q_DEFINE_THIS_MODULE("guiapp")

//****************************************************************************
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
        static_cast<QP::QActive *>(m_act)->dispatch(qpevt); // dispatch to AO
        QP::QF::gc(qpevt); // garbage collect the QP evt
        return true; // event recognized and handled
    }
    else {
        return QApplication::event(e); // delegate to the superclass
    }
}

//............................................................................
void GuiQActive::start(uint_fast8_t const prio,
                       QEvt const *qSto[], uint_fast16_t const /*qLen*/,
                       void * const stkSto, uint_fast16_t const /*stkSize*/,
                       QEvt const * const ie)
{
    Q_REQUIRE((static_cast<uint_fast8_t>(0) < prio)
              && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
              && (qSto == (QEvt const **)0)/* does not need per-actor queue */
              && (stkSto == static_cast<void *>(0))); // AOs don't need stack

    setPrio(prio);  // set the QF priority of this AO
    QF::add_(this); // make QF aware of this AO
    static_cast<GuiApp *>(QApplication::instance())->registerAct(this);

    this->init(ie); // execute initial transition (virtual call)
    QS_FLUSH(); // flush the trace buffer to the host
}
//............................................................................
#ifndef Q_SPY
bool GuiQActive::post_(QEvt const * const e, uint_fast16_t const /*margin*/)
#else
bool GuiQActive::post_(QEvt const * const e, uint_fast16_t const /*margin*/,
                       void const * const sender)
#endif
{
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::priv_.aoObjFilter, this)
        QS_TIME_();                  // timestamp
        QS_OBJ_(sender);             // the sender object
        QS_SIG_(e->sig);             // the signal of the event
        QS_OBJ_(this);               // this active object
        QS_2U8_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_(0);                  // number of free entries (not used)
        QS_EQC_(0);                  // min number of free entries (not used)
    QS_END_NOCRIT_()

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }
    QF_CRIT_EXIT_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e));
    return true;
}
//............................................................................
void GuiQActive::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS::priv_.aoObjFilter, this)
        QS_TIME_();                  // timestamp
        QS_SIG_(e->sig);             // the signal of this event
        QS_OBJ_(this);               // this active object
        QS_2U8_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_(0);                  // number of free entries (not used)
        QS_EQC_(0);                  // min number of free entries (not used)
    QS_END_NOCRIT_()

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }
    QF_CRIT_EXIT_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e),
                                Qt::HighEventPriority);
}

//****************************************************************************
void GuiQMActive::start(uint_fast8_t const prio,
                        QEvt const *qSto[], uint_fast16_t const /*qLen*/,
                        void * const stkSto, uint_fast16_t const /*stkSize*/,
                        QEvt const * const ie)
{
    Q_REQUIRE((static_cast<uint_fast8_t>(0) < prio)
              && (prio <= static_cast<uint_fast8_t>(QF_MAX_ACTIVE))
              && (qSto == (QEvt const **)0)/* does not need per-actor queue */
              && (stkSto == static_cast<void *>(0))); // AOs don't need stack

    setPrio(prio);  // set the QF priority of this active object
    QF::add_(this); // make QF aware of this active object
    static_cast<GuiApp *>(QApplication::instance())->registerAct(this);

    this->init(ie); // execute initial transition (virtual call)
    QS_FLUSH();     // flush the trace buffer to the host
}
//............................................................................
#ifndef Q_SPY
bool GuiQMActive::post_(QEvt const * const e, uint_fast16_t const /*margin*/)
#else
bool GuiQMActive::post_(QEvt const * const e, uint_fast16_t const /*margin*/,
                        void const * const sender)
#endif
{
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS::priv_.aoObjFilter, this)
        QS_TIME_();                  // timestamp
        QS_OBJ_(sender);             // the sender object
        QS_SIG_(e->sig);             // the signal of the event
        QS_OBJ_(this);               // this active object
        QS_2U8_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_(0);                  // number of free entries (not used)
        QS_EQC_(0);                  // min number of free entries (not used)
    QS_END_NOCRIT_()

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }
    QF_CRIT_EXIT_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e));
    return true;
}
//............................................................................
void GuiQMActive::postLIFO(QEvt const * const e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS::priv_.aoObjFilter, this)
        QS_TIME_();                  // timestamp
        QS_SIG_(e->sig);             // the signal of this event
        QS_OBJ_(this);               // this active object
        QS_2U8_(QF_EVT_POOL_ID_(e),  /* the poolID of the event */
                QF_EVT_REF_CTR_(e)); // the ref Ctr of the event
        QS_EQC_(0);                  // number of free entries (not used)
        QS_EQC_(0);                  // min number of free entries (not used)
    QS_END_NOCRIT_()

    // is it a dynamic event?
    if (QF_EVT_POOL_ID_(e) != static_cast<uint8_t>(0)) {
        QF_EVT_REF_CTR_INC_(e); // increment the reference counter
    }
    QF_CRIT_EXIT_();

    // QCoreApplication::postEvent() is thread-safe per Qt documentation
    QCoreApplication::postEvent(QApplication::instance(), new QP_Event(e),
                                Qt::HighEventPriority);
}

} // namespace QP
