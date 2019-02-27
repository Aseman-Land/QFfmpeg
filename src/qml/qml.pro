TARGET  = qffmpeg
TARGETPATH = QFfmpeg
IMPORT_VERSION = 0.1

QT += core quick qml qffmpeg

HEADERS += \
    $$PWD/qffmpegplugin.h \
    $$PWD/ffmpegrender.h \
    $$PWD/ffmpegjob.h \
    ffmpegscreencapture.h

SOURCES += \
    $$PWD/qffmpegplugin.cpp \
    $$PWD/ffmpegrender.cpp \
    $$PWD/ffmpegjob.cpp \
    ffmpegscreencapture.cpp

load(qml_plugin)