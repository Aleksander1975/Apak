#-------------------------------------------------
#
# Project created by QtCreator 2022-03-14T10:49:06
#
#-------------------------------------------------

QT       += widgets core gui printsupport network

TARGET = /home/user/ProjectData/APAK/ZNRecovery/zn_graph
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
    mainwindow.cpp \
    treeitem.cpp \
    treemodel.cpp \
    sv_chartwidget.cpp \
    sv_graph.cpp \
    ../../../../../../c++/qcustomplot/qcustomplot.cpp \
    ../../../../../../c++/svlib/SvSettings/1.0/sv_settings.cpp \
    ../../../../../../c++/Modus/global/signal/sv_signal.cpp \
    ../../../../../../c++/svlib/SvAbstractLogger/1.2/sv_abstract_logger.cpp \
    ../../../../../../c++/svlib/SvProgressBarDialog/1.0/progressbar_dialog.cpp \
    zn_select_data_dialog.cpp

HEADERS  += mainwindow.h \
    sv_chartwidget.h \
    sv_graph.h \
    treeitem.h \
    treemodel.h \
    ../../recovery_defs.h \
    ../../zn_abstract_outer_system.h \
    ../../../../../../c++/qcustomplot/qcustomplot.h \
    ../../../../../../c++/svlib/SvException/1.1/sv_exception.h \
    ../../../../../../c++/svlib/SvSettings/1.0/sv_settings.h \
    ../../../../../../c++/Modus_Libs/APAK/storages/zn_k1/src/zn_global.h \
    ../../../../../../c++/Modus/global/signal/sv_signal.h \
    ../../../../../../c++/svlib/SvAbstractLogger/1.2/sv_abstract_logger.h \
    ../../../../../../c++/Modus/global/device/protocol/sv_abstract_protocol.h \
    ../../../../../../c++/svlib/SvProgressBarDialog/1.0/progressbar_dialog.h \
    zn_select_data_dialog.h

FORMS    += mainwindow.ui \
    sv_chartwidget.ui \
    ../../../../../../c++/svlib/SvProgressBarDialog/1.0/progressbar_dialog.ui \
    zn_select_data_dialog.ui \
    sv_graph_params_dialog.ui

INCLUDEPATH += ../../../../../../c++/qcustomplot

RESOURCES += \
    res.qrc
