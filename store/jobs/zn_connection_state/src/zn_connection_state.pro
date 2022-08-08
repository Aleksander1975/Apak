#-------------------------------------------------
#
# Project created by QtCreator 2021-09-16T16:54:16
#
#-------------------------------------------------

QT       -= gui

TARGET = zn_connection_state
TEMPLATE = lib

DEFINES += ZN_CONNECTION_STATE_LIBRARY

SOURCES += zn_connection_state.cpp \
    ../../../../../../Modus/global/signal/sv_signal.cpp

HEADERS += zn_connection_state.h\
        zn_connection_state_global.h \
    ../../../../../../Modus/global/job/sv_abstract_job.h \
    ../../../../../../Modus/global/job/job_config.h \
    ../../../../../../Modus/global/signal/sv_signal.h \
    zn_connection_state_defs.h \
    supported_signal_tags.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
