#-------------------------------------------------
#
# Project created by QtCreator 2012-05-10T10:48:40
#
#-------------------------------------------------
TEMPLATE = app

QT      += core gui
TARGET   = dpp-gui
DEFINES += QT_NO_STATEMACHINE

INCLUDEPATH = . \
    $(QPCPP)/include \
    $(QPCPP)/ports/qt/mingw \

SOURCES += \
    main.cpp \
    gui.cpp \
    bsp.cpp \
    table.cpp \
    philo.cpp

HEADERS += \
    dpp.h \
    gui.h \
    bsp.h

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
