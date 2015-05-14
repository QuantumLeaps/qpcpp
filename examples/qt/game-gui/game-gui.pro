#-----------------------------------------------------------------------------
# Product: Game-GUI exampe for Qt5
# Last Updated for Version: QP 5.4.0/Qt 5.x
# Date of the Last Update:  2015-05-04
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) Quantum Leaps, LLC. All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Alternatively, this program may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GNU General Public License and are specifically designed for
# licensees interested in retaining the proprietary status of their code.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Contact information:
# Web  : http://www.state-machine.com
# Email: info@state-machine.com
#-----------------------------------------------------------------------------

TEMPLATE = app
QT      += core gui widgets
DEFINES += QT_NO_STATEMACHINE

TARGET   = game-gui

INCLUDEPATH = . \
    $(QPCPP)/include \
    $(QPCPP)/ports/qt \

SOURCES += \
    main.cpp \
    gui.cpp \
    bsp.cpp \
    missile.cpp \
    mine2.cpp \
    mine1.cpp \
    tunnel.cpp \
    ship.cpp

HEADERS  += \
    game.h \
    bsp.h \
    gui.h

FORMS += gui.ui

CONFIG(debug, debug|release) {
    DEFINES += Q_SPY
    INCLUDEPATH += $(QTOOLS)/qspy/include
    SOURCES += $(QTOOLS)/qspy/source/qspy.c
    HEADERS += qs_port.h
    LIBS += -L$(QPCPP)/ports/qt/mingw/debug
} else {
    LIBS += -L$(QPCPP)/ports/qt/mingw/release
}

LIBS += -lqp

RESOURCES = gui.qrc

win32:RC_FILE = gui.rc

