QT -= gui

DEFINES += UPZ_LIBRARY

CONFIG += c++11 plugin

TARGET = /home/user/Modus/lib/protocols/apak_upz
TEMPLATE = lib

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../../../../Modus/global/signal/sv_signal.cpp \
    upz.cpp

HEADERS += \
    ../../../../../Modus/global/device/protocol/sv_abstract_protocol.h \
    ../../../../../Modus/global/global_defs.h \
    upz_global.h \
    ../../../../../Modus/global/signal/sv_signal.h \
    upz.h \
    protocol_params.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
