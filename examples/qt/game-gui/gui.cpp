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
#include <QtWidgets>
#include "gui.hpp"
//-----------------
#include "qpcpp.hpp"
#include "game.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

//............................................................................
Gui *Gui::instance;

//............................................................................
Gui::Gui(QWidget *parent)
  : QDialog(parent)
{
    instance = this;  // only one instance for the Gui

    setupUi(this);

    static quint8 const c_offColor[] = {  15U,  15U,  15U };
    m_display->init(BSP_SCREEN_WIDTH, 2U,
                    BSP_SCREEN_HEIGHT, 2U,
                    c_offColor);
}
//............................................................................
void Gui::onBtnPressed() { // slot
    m_button->setIcon(QPixmap(":/res/EK-BTN_DWN.png"));
    static QP::QEvt const fireEvt(GAME::PLAYER_TRIGGER_SIG);
    QP::QF::PUBLISH(&fireEvt, nullptr);
}
//............................................................................
void Gui::onBtnReleased() { // slot
    m_button->setIcon(QPixmap(":/res/EK-BTN_UP.png"));
}
//............................................................................
void Gui::onQuit() { // slot
    BSP_terminate(0);
}
//............................................................................
void Gui::wheelEvent(QWheelEvent *e) {
    if (e->delta() >= 0) {
        BSP_moveShipUp();
    }
    else {
        BSP_moveShipDown();
    }
}
