#-----------------------------------------------------------------------------
# Product: QP/C++ port to Qt5
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

QT      += core gui widgets
TARGET   = qp
TEMPLATE = lib
CONFIG  += staticlib
DEFINES += QT_NO_STATEMACHINE

INCLUDEPATH += .. \
    ../../../include \
    ../../../source

HEADERS +=  \
    ../qep_port.h \
    ../qf_port.h \
    ../tickerthread.h \
    ../aothread.h \
    ../guiapp.h \
    ../guiactive.h \
    ../pixellabel.h

SOURCES += \
    ../qf_port.cpp \
    ../guiapp.cpp \
    ../pixellabel.cpp \
    ../../../source/qep_hsm.cpp \
    ../../../source/qep_msm.cpp \
    ../../../source/qf_act.cpp \
    ../../../source/qf_actq.cpp \
    ../../../source/qf_defer.cpp \
    ../../../source/qf_dyn.cpp \
    ../../../source/qf_mem.cpp \
    ../../../source/qf_ps.cpp \
    ../../../source/qf_qact.cpp \
    ../../../source/qf_qeq.cpp \
    ../../../source/qf_qmact.cpp \
    ../../../source/qf_time.cpp

CONFIG(debug, debug|release) {
    DEFINES += Q_SPY

    SOURCES += \
        ../../../source/qs.cpp \
        ../../../source/qs_fp.cpp \
        ../../../source/qs_64bit.cpp
} else {
    DEFINES += NDEBUG
}
