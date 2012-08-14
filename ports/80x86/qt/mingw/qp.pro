#-------------------------------------------------
#
# Project created by QtCreator 2012-05-09T19:56:54
#
#-------------------------------------------------

QT      += core gui
TARGET   = qp
TEMPLATE = lib
CONFIG  += staticlib
DEFINES += QT_NO_STATEMACHINE

INCLUDEPATH += . \
    ../../../../include \
    ../../../../qep/source \
    ../../../../qf/source

HEADERS +=  \
    qep_port.h \
    qf_port.h \
    qp_app.h \
    pixellabel.h

SOURCES += \
    qf_port.cpp \
    pixellabel.cpp \
    ../../../../qep/source/qep.cpp \
    ../../../../qep/source/qhsm_dis.cpp \
    ../../../../qep/source/qhsm_ini.cpp \
    ../../../../qep/source/qhsm_in.cpp \
    ../../../../qep/source/qhsm_top.cpp \
    ../../../../qf/source/qa_defer.cpp \
    ../../../../qf/source/qa_sub.cpp \
    ../../../../qf/source/qa_usub.cpp \
    ../../../../qf/source/qa_usuba.cpp \
    ../../../../qf/source/qeq_fifo.cpp \
    ../../../../qf/source/qeq_get.cpp \
    ../../../../qf/source/qeq_init.cpp \
    ../../../../qf/source/qeq_lifo.cpp \
    ../../../../qf/source/qf_act.cpp \
    ../../../../qf/source/qf_gc.cpp \
    ../../../../qf/source/qf_log2.cpp \
    ../../../../qf/source/qf_new.cpp \
    ../../../../qf/source/qf_pool.cpp \
    ../../../../qf/source/qf_psini.cpp \
    ../../../../qf/source/qf_pspub.cpp \
    ../../../../qf/source/qf_pwr2.cpp \
    ../../../../qf/source/qf_tick.cpp \
    ../../../../qf/source/qmp_get.cpp \
    ../../../../qf/source/qmp_init.cpp \
    ../../../../qf/source/qmp_put.cpp \
    ../../../../qf/source/qte_arm.cpp \
    ../../../../qf/source/qte_ctor.cpp \
    ../../../../qf/source/qte_ctr.cpp \
    ../../../../qf/source/qte_darm.cpp \
    ../../../../qf/source/qte_rarm.cpp

CONFIG(debug, debug|release) {
    DEFINES += Q_SPY

    SOURCES += \
        ../../../../qs/source/qs.cpp \
        ../../../../qs/source/qs_.cpp \
        ../../../../qs/source/qs_blk.cpp \
        ../../../../qs/source/qs_byte.cpp \
        ../../../../qs/source/qs_f32.cpp \
        ../../../../qs/source/qs_f64.cpp \
        ../../../../qs/source/qs_mem.cpp \
        ../../../../qs/source/qs_str.cpp \
        ../../../../qs/source/qs_u64.cpp

} else {
    DEFINES += NDEBUG
}
