#-------------------------------------------------
#
# Project created by QtCreator 2022-04-12T18:40:30
#
#-------------------------------------------------

QT       -= gui

TARGET = /home/user/ProjectData/APAK/ZNRecovery/lib/radga
TEMPLATE = lib
CONFIG += c++11 plugin
DEFINES += ZN_RADGA_LIBRARY

SOURCES += radga.cpp \
    ../../../../../../job/Modus/global/signal/sv_signal.cpp

HEADERS += ../../../zn_abstract_outer_system.h \
    ../raduga_defs.h \
    ../../../../../../job/Modus/global/signal/sv_signal.h \
    radga.h \
    radga_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
