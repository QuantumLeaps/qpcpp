//============================================================================
// Product: GUI for PELICAN crossing example
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
#ifndef GUI_HPP
#define GUI_H

#include <QDialog>
#include "ui_gui.h"

class Gui : public QDialog, public Ui_Gui {
    Q_OBJECT

public:
    Gui(QWidget *parent = 0);
    static Gui *instance();

private slots:
    void onPedsPressed();
    void onPedsReleased();
    void onOnPressed();
    void onOnReleased();
    void onQuit();
};

#endif // GUI_HPP
