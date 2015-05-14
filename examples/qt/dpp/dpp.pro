#-----------------------------------------------------------------------------
# Product: DPP console exampe for Qt
# Last Updated for Version: QP/C++ 5.4.0/Qt 5.x
# Date of the Last Update:  2015-05-03
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) Quantum Leaps, LLC. All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
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
# Web:   www.state-machine.com
# Email: info@state-machine.com
#-----------------------------------------------------------------------------

TEMPLATE = app

QT      += core
QT      -= gui
CONFIG  += console
CONFIG  -= app_bundle
TARGET   = dpp
DEFINES += QT_NO_STATEMACHINE

QPCPP = ../../..

INCLUDEPATH = . \
    $(QPCPP)/include \
    $(QPCPP)/ports/qt \

SOURCES += \
    main.cpp \
    bsp.cpp \
    table.cpp \
    philo.cpp

HEADERS += \
    dpp.h \
    bsp.h

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
