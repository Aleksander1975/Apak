#-------------------------------------------------
#
# Project created by QtCreator 2021-11-26T12:01:23
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = /home/user/ProjectData/APAK/ZNRecovery/zn_picker
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    task_editor.cpp \
    ../../../../../job/svlib/SvAbstractLogger/1.2/sv_abstract_logger.cpp \
    ../../../../../job/svlib/SvWidgetLogger/1.1/sv_widget_logger.cpp \
    ../../../../../job/svlib/SvSettings/1.0/sv_settings.cpp \
    zn_picker.cpp \
    ../../../../../job/Modus/global/signal/sv_signal.cpp \
    zn_select_system_dialog.cpp

HEADERS  += mainwindow.h \
    task_editor.h \
    ../../../../../job/svlib/SvAbstractLogger/1.2/sv_abstract_logger.h \
#    ../../ZNReader/src/zn_reader_defs.h \
    ../../../../../job/Modus_Libs/APAK/storages/zn_k1/src/zn_global.h \
    ../../../../../job/svlib/SvWidgetLogger/1.1/sv_widget_logger.h \
    ../../../../../job/svlib/SvSettings/1.0/sv_settings.h \
    zn_picker.h \
    ../../recovery_defs.h \
    ../../../../../job/Modus/global/signal/sv_signal.h \
    zn_select_system_dialog.h

FORMS    += mainwindow.ui \
    task_editor_dialog.ui \
    select_system_dialog.ui

RESOURCES += \
    ../res/res.qrc
