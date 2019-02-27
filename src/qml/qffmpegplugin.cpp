#include "qffmpegplugin.h"

#include <QtQml>

#include "ffmpegrender.h"
#include "ffmpegjob.h"
#include "ffmpegscreencapture.h"

void QFfmpegPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<FfmpegRender>(uri, 0, 1, "FfmpegRender");
    qmlRegisterType<FfmpegJob>(uri, 0, 1, "FfmpegJob");
    qmlRegisterType<FfmpegScreenCapture>(uri, 0, 1, "FFmpegScreenCapture");
}
