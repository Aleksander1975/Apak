QT -= gui

DEFINES += EXCHANGE_SIGNALS_LIBRARY

CONFIG += c++11 plugin


VERSION =   1.0.0    # major.minor.patch
DEFINES +=  LIB_VERSION=\\\"$$VERSION\\\"
DEFINES += "LIB_AUTHOR=\"\\\"Кон А.М.\\\"\""


TARGET = /home/user/Modus/lib/protocols/apak_exchange_signals_module
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

# Для отключения отладочного вывода через qDebug() в release-версии:
CONFIG (debug, debug|release){
    message ("Debug")
}else{
    message ("Release")
    DEFINES += QT_NO_DEBUG_OUTPUT
}

SOURCES += \
    ../../../../Modus/global/signal/sv_signal.cpp \
    exchange_signals.cpp

HEADERS += \
    ../../../../Modus/global/device/protocol/sv_abstract_protocol.h \
    ../../../../Modus/global/global_defs.h \
    ../../../../Modus/global/signal/sv_signal.h \
    protocol_params.h \
    ../../../../Modus/global/device/device_defs.h \
    exchange_signals.h \
    exchange_signals_global.h \
    frame_header.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
