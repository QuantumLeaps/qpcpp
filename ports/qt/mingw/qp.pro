#-----------------------------------------------------------------------------
# Product: QP/C++ port to Qt5
# Last Updated for Version: 5.3.0
# Date of the Last Update:  2014-04-13
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
    ../../../qep/source \
    ../../../qf/source

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
    ../../../qep/source/qep.cpp \
    ../../../qep/source/qmsm_dis.cpp \
    ../../../qep/source/qmsm_ini.cpp \
    ../../../qep/source/qmsm_in.cpp \
    ../../../qep/source/qhsm_dis.cpp \
    ../../../qep/source/qhsm_ini.cpp \
    ../../../qep/source/qhsm_in.cpp \
    ../../../qep/source/qhsm_top.cpp \
    ../../../qf/source/qa_defer.cpp \
    ../../../qf/source/qa_fifo.cpp \
    ../../../qf/source/qa_get_.cpp \
    ../../../qf/source/qa_lifo.cpp \
    ../../../qf/source/qa_sub.cpp \
    ../../../qf/source/qa_usub.cpp \
    ../../../qf/source/qa_usuba.cpp \
    ../../../qf/source/qeq_fifo.cpp \
    ../../../qf/source/qeq_get.cpp \
    ../../../qf/source/qeq_init.cpp \
    ../../../qf/source/qeq_lifo.cpp \
    ../../../qf/source/qf_act.cpp \
    ../../../qf/source/qf_gc.cpp \
    ../../../qf/source/qf_log2.cpp \
    ../../../qf/source/qf_new.cpp \
    ../../../qf/source/qf_pool.cpp \
    ../../../qf/source/qf_psini.cpp \
    ../../../qf/source/qf_pspub.cpp \
    ../../../qf/source/qf_pwr2.cpp \
    ../../../qf/source/qf_tick.cpp \
    ../../../qf/source/qmp_get.cpp \
    ../../../qf/source/qmp_init.cpp \
    ../../../qf/source/qmp_put.cpp \
    ../../../qf/source/qte_arm.cpp \
    ../../../qf/source/qte_ctor.cpp \
    ../../../qf/source/qte_ctr.cpp \
    ../../../qf/source/qte_darm.cpp \
    ../../../qf/source/qte_rarm.cpp

CONFIG(debug, debug|release) {
    DEFINES += Q_SPY

    SOURCES += \
        ../../../qs/source/qs.cpp \
        ../../../qs/source/qs_.cpp \
        ../../../qs/source/qs_blk.cpp \
        ../../../qs/source/qs_byte.cpp \
        ../../../qs/source/qs_dict.cpp \
        ../../../qs/source/qs_f32.cpp \
        ../../../qs/source/qs_f64.cpp \
        ../../../qs/source/qs_mem.cpp \
        ../../../qs/source/qs_str.cpp \
        ../../../qs/source/qs_u64.cpp

} else {
    DEFINES += NDEBUG
}
