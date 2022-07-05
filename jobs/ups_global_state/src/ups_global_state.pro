#-------------------------------------------------
#
# Project created by QtCreator 2021-09-16T16:54:16
#
#-------------------------------------------------

QT       -= gui

TARGET = /home/user/Modus/lib/jobs/apak_ups_global_state
TEMPLATE = lib

CONFIG += c++11

DEFINES += UPS_GLOBAL_STATE_LIBRARY

SOURCES += ups_global_state.cpp \
    ../../../../../Modus/global/signal/sv_signal.cpp

HEADERS += ups_global_state.h\
        ups_global_state_global.h \
    ../../../../../Modus/global/job/sv_abstract_job.h \
    ../../../../../Modus/global/job/job_config.h \
    ../../../../../Modus/global/signal/sv_signal.h \
    ups_global_state_defs.h \
    supported_signal_tags.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
