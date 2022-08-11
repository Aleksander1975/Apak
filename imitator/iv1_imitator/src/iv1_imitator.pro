QT -= gui

DEFINES += IV1_IMITATOR_LIBRARY

CONFIG += c++11 plugin

TARGET = /home/user/Modus/lib/protocols/apak_iv1_imitator
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

# Для генерации ошибки линкёра в случае наличия неопределённых
# ссылок (undefined references) при сборке разделяемой библиотеки:
QMAKE_LFLAGS += -Wno-unused-variable, -Wl,--no-undefined

CONFIG (release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

SOURCES += \
    ../../../../Modus/global/signal/sv_signal.cpp \
    iv1_imitator.cpp

HEADERS += \
    ../../../../Modus/global/device/protocol/sv_abstract_protocol.h \
    ../../../../Modus/global/global_defs.h \
    ../../../../Modus/global/signal/sv_signal.h \
    ../../../../Modus/global/device/device_defs.h \
    protocol_params.h \
    iv1_imitator.h \
    iv1_imitator_global.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
