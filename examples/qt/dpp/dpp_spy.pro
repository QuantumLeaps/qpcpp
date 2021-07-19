#-----------------------------------------------------------------------------
# Product: DPP console exampe for Qt (console)
# Last updated for version 6.9.4
# Last updated on  2021-07-19
#
#                    Q u a n t u m  L e a P s
#                    ------------------------
#                    Modern Embedded Software
#
# Copyright (C) 2005-2021 Quantum Leaps. All rights reserved.
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
# along with this program. If not, see <www.gnu.org/licenses>.
#
# Contact information:
# <www.state-machine.com/licensing>
# <info@state-machine.com>
#-----------------------------------------------------------------------------

TEMPLATE = app

QT      += core
QT      -= gui
CONFIG  += console
CONFIG  -= app_bundle
CONFIG  += c++11
TARGET   = dpp
DEFINES += QT_NO_STATEMACHINE

QPCPP = ../../..

INCLUDEPATH = . \
    $$QPCPP/include \
    $$QPCPP/ports/qt

HEADERS += \
    dpp.hpp \
    bsp.hpp

SOURCES += \
    main.cpp \
    bsp.cpp \
    table.cpp \
    philo.cpp

##############################################################################
# NOTE:
# This project demonstrats how to build the QP/C++ framework from sources,
# as opposed to linking the QP/C++ library. The following headers and sources
# are included from QP/C++.
#
# NOTE:
# For this is a simple **console** application some modules from the Qt port
# are commented-out. (These modules would be needed in a GUI application).

# QP-Qt port headers/sources
HEADERS +=  \
    $$QPCPP/ports/qt/tickerthread.hpp \
    $$QPCPP/ports/qt/aothread.hpp \
#    $$QPCPP/ports/qt/guiapp.hpp \
#    $$QPCPP/ports/qt/guiactive.hpp \
#    $$QPCPP/ports/qt/pixellabel.hpp

SOURCES += \
    $$QPCPP/ports/qt/qf_port.cpp \
#    $$QPCPP/ports/qt/guiapp.cpp \
#    $$QPCPP/ports/qt/pixellabel.cpp


# QP/C++ headers/sources
SOURCES += \
    $$QPCPP/src/qf/qep_hsm.cpp \
    $$QPCPP/src/qf/qep_msm.cpp \
    $$QPCPP/src/qf/qf_act.cpp \
    $$QPCPP/src/qf/qf_actq.cpp \
    $$QPCPP/src/qf/qf_defer.cpp \
    $$QPCPP/src/qf/qf_dyn.cpp \
    $$QPCPP/src/qf/qf_mem.cpp \
    $$QPCPP/src/qf/qf_ps.cpp \
    $$QPCPP/src/qf/qf_qact.cpp \
    $$QPCPP/src/qf/qf_qeq.cpp \
    $$QPCPP/src/qf/qf_qmact.cpp \
    $$QPCPP/src/qf/qf_time.cpp \
    $$QPCPP/include/qstamp.cpp

INCLUDEPATH += $$QPCPP/src


CONFIG(debug, debug|release) {

    # NOTE:
    # To include Q-SPY software tracing in the Debug configuration,
    # please un-comment the following lines of code.

    DEFINES += Q_SPY

    # QS software tracing sources
    SOURCES += \
        $$QPCPP/src/qs/qs.cpp \
        $$QPCPP/src/qs/qs_fp.cpp \
        $$QPCPP/src/qs/qs_64bit.cpp

    HEADERS += $$QPCPP/ports/qt/qs_port.hpp

    # NOTE:
    # The "qspy.c" component is needed only when you perform the formatted
    # output directly in the Target. The following declarations assume that
    # the QTools collection is installed and that the QTOOLS environment
    # variable is set to point to this QTools installation directory.

    INCLUDEPATH += $(QTOOLS)/qspy/include
    SOURCES += $(QTOOLS)/qspy/source/qspy.c

} else {
    # Release build configuartion
    DEFINES += NDEBUG
}
