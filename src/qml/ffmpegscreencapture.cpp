#include "ffmpegscreencapture.h"

class FfmpegScreenCapturePrivate {
public:
    QString source;
    int position;
    QString output;
    bool running;
    FfmpegTools *ffmpegTools;
};

FfmpegScreenCapture::FfmpegScreenCapture(QObject *parent) : QObject(parent)
{
    p = new FfmpegScreenCapturePrivate;
    p->running = false;
}

FfmpegScreenCapture::~FfmpegScreenCapture()
{
    delete p;
}

void FfmpegScreenCapture::setSource(const QString &source)
{
    if(p->source == source)
        return;

    p->source = source;
    Q_EMIT sourceChanged();
}

QString FfmpegScreenCapture::source() const
{
    return p->source;
}

void FfmpegScreenCapture::setPosition(int position)
{
    if(p->position == position)
        return;

    p->position = position;
    Q_EMIT positionChanged();
}

int FfmpegScreenCapture::position() const
{
    return p->position;
}

void FfmpegScreenCapture::setOutput(const QString &output)
{
    if(p->output == output)
        return;

    p->output = output;
    Q_EMIT outputChanged();
}

QString FfmpegScreenCapture::output() const
{
    return p->output;
}

bool FfmpegScreenCapture::running() const
{
    return p->ffmpegTools;
}

void FfmpegScreenCapture::start()
{
    if(p->ffmpegTools)
        return;

    p->ffmpegTools = new FfmpegTools(this);
    p->ffmpegTools->takeScreenshot(p->source, p->position, p->output, [this](const QString log) {
        Q_EMIT finished(p->output);
        p->ffmpegTools->deleteLater();
        p->ffmpegTools = Q_NULLPTR;
    });
    Q_EMIT runningChanged();
}
