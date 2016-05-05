#-----------------------------------------------------------------------------
# Product: QP/C++ port to Qt5
# Last Updated for Version: QP/C++ 5.5.0/Qt 5.x
# Date of the Last Update:  2015-09-26
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
# http://www.state-machine.com
# mailto:info@state-machine.com
#-----------------------------------------------------------------------------

# NOTE:
# This project builds QP/C++ library (for Qt5). This libary is needed
# only in projects that link this library. (The example projects in
# the qpcpp/examples/qt/ directory build QP/C++ from sources and actually
# don't need the QP/C++ library).

# NOTE:
# After you build the QP/C++ library (libqp.a), you should copy the Debug
# version to the debug/ sub-directory and the Release version to the
# release/ sub-directory.

QT      += core gui widgets
TARGET   = qp
TEMPLATE = lib
CONFIG  += staticlib
DEFINES += QT_NO_STATEMACHINE

QPCPP = ../../..

INCLUDEPATH += .. \
    $$QPCPP/include \
    $$QPCPP/source

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
    $$QPCPP/source/qep_hsm.cpp \
    $$QPCPP/source/qep_msm.cpp \
    $$QPCPP/source/qf_act.cpp \
    $$QPCPP/source/qf_actq.cpp \
    $$QPCPP/source/qf_defer.cpp \
    $$QPCPP/source/qf_dyn.cpp \
    $$QPCPP/source/qf_mem.cpp \
    $$QPCPP/source/qf_ps.cpp \
    $$QPCPP/source/qf_qact.cpp \
    $$QPCPP/source/qf_qeq.cpp \
    $$QPCPP/source/qf_qmact.cpp \
    $$QPCPP/source/qf_time.cpp


CONFIG(debug, debug|release) {

    # NOTE:
    # The Debug configuration also uses Q-SPY software tracing.
    # The following defines, headers, and sources are needed only for
    # the QS target-resident component.

    DEFINES += Q_SPY

    SOURCES += \
        $$QPCPP/source/qs.cpp \
        $$QPCPP/source/qs_rx.cpp \
        $$QPCPP/source/qs_fp.cpp \
        $$QPCPP/source/qs_64bit.cpp

} else {

    # Release build configuartion
    DEFINES += NDEBUG
}
