
include( $$PWD/include/sdk_core_include.pri )
include( $$PWD/src/sdk_core_src.pri )

INCLUDEPATH += $$PWD/include/third_party/spdlog

CONFIG      *= link_pkgconfig
PKGCONFIG   *= apr-1

LIBS        *= -lboost_system
