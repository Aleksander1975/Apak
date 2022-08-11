#-------------------------------------------------
#
# Project created by QtCreator 2022-04-12T18:40:30
#
#-------------------------------------------------

QT       -= gui

TARGET = /home/user/ProjectData/APAK/ZNRecovery/lib/skm
TEMPLATE = lib
CONFIG += c++11 plugin
DEFINES += ZN_SKM_LIBRARY

SOURCES += skm.cpp \
    ../../../../../../job/Modus/global/signal/sv_signal.cpp

HEADERS += ../../../zn_abstract_outer_system.h \
    ../skm_defs.h \
    ../../../../../../job/Modus/global/signal/sv_signal.h \
    skm.h \
    skm_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
