load(qt_build_config)

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

CONFIG += c++11

TARGET = QtFfmpeg
QT = core

MODULE = qffmpeg

load(qt_module)

DEFINES += LIBQT_FFMPEG_LIBRARY

HEADERS += \
    $$PWD/ffmpegtools.h

SOURCES += \
    $$PWD/ffmpegtools.cpp
