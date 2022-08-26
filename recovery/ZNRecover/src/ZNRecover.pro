#-------------------------------------------------
#
# Project created by QtCreator 2021-11-26T12:01:23
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = /home/user/ProjectData/APAK/ZNRecovery/znr
TEMPLATE = app

CONFIG += c++11

VERSION = 1.0.0    # major.minor.patch
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += main.cpp\
        mainwindow.cpp \
    zn_reader.cpp \
    ../../../../../job/svlib/SvAbstractLogger/1.2/sv_abstract_logger.cpp \
    ../../../../../job/svlib/SvWidgetLogger/1.1/sv_widget_logger.cpp \
    ../../../../../job/svlib/Network/Tcp/Client/1.0/sv_tcp_client.cpp \
    zn_select_dialog.cpp \
    ../../../../../job/Modus/global/signal/sv_signal.cpp \
    treemodel.cpp \
    treeitem.cpp \
    set_period_dialog.cpp \
    ../../../../svlib/SvSettings/1.0/sv_settings.cpp \
    coarse_data_picker.cpp

HEADERS  += mainwindow.h \
    zn_reader.h \
    ../../../../../job/svlib/SvAbstractLogger/1.2/sv_abstract_logger.h \
    ../../../../../job/svlib/SvWidgetLogger/1.1/sv_widget_logger.h \
    ../../../../../job/svlib/Network/Tcp/Client/1.0/sv_tcp_client.h \
    ../../../../../job/Modus_Libs/APAK/storages/zn_k1/src/zn_global.h \
    zn_select_dialog.h \
    ../../recovery_defs.h \
    ../../../../../job/Modus/global/signal/sv_signal.h \
    treemodel.h \
    treeitem.h \
    database.h \
    set_period_dialog.h \
    ../../../../svlib/SvSettings/1.0/sv_settings.h \
    coarse_data_picker.h

FORMS    += mainwindow.ui \
    select_zn_dialog.ui \
    set_period_dialog.ui

RESOURCES += \
    ../res/res.qrc
