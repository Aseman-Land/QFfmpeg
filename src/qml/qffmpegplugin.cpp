#include "qffmpegplugin.h"

#include <QtQml>
#include <QUrl>

#include "ffmpegrender.h"
#include "ffmpegjob.h"
#include "ffmpegscreencapture.h"

void QFfmpegPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<FfmpegRender>(uri, 0, 2, "FfmpegRender");
    qmlRegisterType<FfmpegJob>(uri, 0, 2, "FfmpegJob");
    qmlRegisterType<FfmpegScreenCapture>(uri, 0, 2, "FFmpegScreenCapture");

    qmlRegisterType(QUrl(QStringLiteral("qrc:/QFfmpeg/qml/ScreenView.qml")), uri, 0, 2, "ScreenView");
}
