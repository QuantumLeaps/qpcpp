//============================================================================
// Product: QP/C++ GUI example for Qt5
// Last updated for version 6.6.0
// Last updated on  2019-07-30
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com>
// <info@state-machine.com>
//============================================================================
#include "gui.hpp"
//-----------------
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

//............................................................................
static Gui *l_instance;


//............................................................................
Gui::Gui(QObject *parent)
  : QObject(parent)
{
    l_instance = this; // initialize the instance (Singleton)
    engine.rootContext()->setContextProperty("gui", (QObject*)l_instance);

    for (int i = 0; i < N_PHILO; ++i) {
        m_State.append("res/thinking.png");
    }

    BSP_randomSeed(123U);
}
//............................................................................
Gui *Gui::instance() {
    return l_instance;
}
//............................................................................
void Gui::show() {
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
}
//............................................................................
QList<QString> Gui::State() const {
    return m_State;
}
//............................................................................
void Gui::setState(const QList<QString> &state) {
    m_State = state;
    emit stateChanged();
}
//............................................................................
QString Gui::Button() const {
    return m_Button;
}
//............................................................................
void Gui::setButton(const QString& button) {
    m_Button = button;
    emit buttonChanged();
}
//............................................................................
void Gui::onPausePressed() { // slot
    static QP::QEvt const e(DPP::PAUSE_SIG);
    QP::QF::PUBLISH(&e, nullptr);
    qDebug("onPausePressed");
}
//............................................................................
void Gui::onPauseReleased() { // slot
    static QP::QEvt const e(DPP::SERVE_SIG);
    QP::QF::PUBLISH(&e, nullptr);
    qDebug("onPauseReleased");
}
//............................................................................
void Gui::onQuit() { // slot
    static QP::QEvt const e(DPP::TERMINATE_SIG);
    QP::QF::PUBLISH(&e, nullptr);
    qDebug("onQuit");
}
