
INCLUDEPATH += $$PWD/include

INCLUDEPATH += $$PWD/include/third_party/FastCRC/
include( $$PWD/src/third_party/FastCRC/FastCRC.pri )

INCLUDEPATH += $$PWD/src/command_handler/
HEADERS += $$PWD/src/command_handler/command_impl.h

INCLUDEPATH += $$PWD/src/comm/
HEADERS += $$PWD/src/comm/sdk_protocol.h
SOURCES += $$PWD/src/comm/sdk_protocol.cpp
