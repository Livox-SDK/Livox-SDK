
INCLUDEPATH += $$PWD
HEADERS     += $$files( $$PWD/*.h   )
SOURCES     += $$files( $$PWD/*.cpp )

include( $$PWD/command_handler/src_command_handler.pri )
include( $$PWD/data_handler/src_data_handler.pri )
include( $$PWD/base/src_base.pri )
include( $$PWD/comm/src_comm.pri )

include( $$PWD/third_party/FastCRC/FastCRC.pri )
