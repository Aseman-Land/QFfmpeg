#include "ffmpegrender.h"

#include "ffmpegtools.h"
#include <QDebug>

class FfmpegRenderPrivate {
public:
    QList<FfmpegJob*> items;
    qreal fps;
    QSize resolution;
    QString encoder;
    QString output;
    QString ffmpegPath;
    qreal progress;
    bool running;
    QString tempDirectory;
    FfmpegTools *ffmpegTools;
};

FfmpegRender::FfmpegRender(QObject *parent) : QObject(parent)
{
    p = new FfmpegRenderPrivate;
    p->fps = 0;
    p->encoder = "";
    p->progress = 0;
    p->running = false;
    p->ffmpegTools = Q_NULLPTR;
    p->ffmpegPath = "ffmpeg";
}

FfmpegRender::~FfmpegRender()
{
    delete p;
}

QQmlListProperty<FfmpegJob> FfmpegRender::items()
{
    return QQmlListProperty<FfmpegJob>(this, &p->items, QQmlListProperty<FfmpegJob>::AppendFunction(append),
                                       QQmlListProperty<FfmpegJob>::CountFunction(count),
                                       QQmlListProperty<FfmpegJob>::AtFunction(at),
                                       QQmlListProperty<FfmpegJob>::ClearFunction(clear) );
}

QList<FfmpegJob *> FfmpegRender::itemsList() const
{
    return p->items;
}

void FfmpegRender::setFPS(qreal fps)
{
    if(p->fps == fps)
        return;

    p->fps = fps;
    Q_EMIT fpsChanged();
}

qreal FfmpegRender::fps() const
{
    return p->fps;
}

void FfmpegRender::setResolution(QSize resolution)
{
    if(p->resolution == resolution)
        return;

    p->resolution = resolution;
    Q_EMIT resolutionChanged();
}

QSize FfmpegRender::resolution() const
{
    return p->resolution;
}

void FfmpegRender::setEncoder(const QString &encoder)
{
   if(p->encoder == encoder)
       return;

   p->encoder = encoder;
   Q_EMIT encoderChanged();
}

QString FfmpegRender::encoder() const
{
    return p->encoder;
}

void FfmpegRender::setOutput(const QString &output)
{
    if(p->output == output)
        return;

    p->output = output;
    Q_EMIT outputChanged();
}

QString FfmpegRender::output() const
{
    return p->output;
}

void FfmpegRender::setFfmpegPath(const QString &ffmpegPath)
{
    if(p->ffmpegPath == ffmpegPath)
        return;

    p->ffmpegPath = ffmpegPath;
    Q_EMIT ffmpegPathChanged();
}

QString FfmpegRender::ffmpegPath() const
{
    return p->ffmpegPath;
}

void FfmpegRender::setTempDirectory(const QString &tempDir)
{
    if(p->tempDirectory == tempDir)
        return;

    p->tempDirectory = tempDir;
    Q_EMIT tempDirectoryChanged();
}

QString FfmpegRender::tempDirectory() const
{
    return p->tempDirectory;
}

qreal FfmpegRender::progress() const
{
    return p->progress;
}

bool FfmpegRender::running() const
{
    return p->ffmpegTools;
}

void FfmpegRender::start()
{
    if(p->ffmpegTools || p->items.isEmpty())
        return;
    QList<FfmpegTools::Render> renderList;
    for(int i = 0; i < p->items.length(); i++) {
        FfmpegTools::Render renderobj;
        renderobj.inputFile = p->items.at(i)->source();
        renderobj.startTime = p->items.at(i)->fromPosition();
        renderobj.endTime = p->items.at(i)->toPosition();
        renderList.append(renderobj);
    }
    p->ffmpegTools = new FfmpegTools(this);
    p->ffmpegTools->setTempDirectory(p->tempDirectory);
    p->ffmpegTools->setFfmpegPath(p->ffmpegPath);
    p->ffmpegTools->render(renderList, p->resolution, p->fps, p->encoder, p->output, [this](qreal progress, const QString &log, qint32 remainingTime){
        setProgress(progress);
    });
    Q_EMIT runningChanged();
}

void FfmpegRender::stop()
{
    if(!p->ffmpegTools)
        return;
    p->ffmpegTools->deleteLater();
    p->ffmpegTools = 0;
    Q_EMIT runningChanged();
}

void FfmpegRender::append(QQmlListProperty<FfmpegJob> *p, FfmpegJob *v)
{
    FfmpegRender *ffmpegobj = static_cast<FfmpegRender*>(p->object);
    ffmpegobj->p->items.append(v);
    Q_EMIT ffmpegobj->itemsChanged();
}

int FfmpegRender::count(QQmlListProperty<FfmpegJob> *p)
{
    FfmpegRender *ffmpegobj = static_cast<FfmpegRender*>(p->object);
    return ffmpegobj->p->items.count();
}

QObject *FfmpegRender::at(QQmlListProperty<FfmpegJob> *p, int idx)
{
    FfmpegRender *ffmpegobj = static_cast<FfmpegRender*>(p->object);
    return ffmpegobj->p->items.at(idx);
}

void FfmpegRender::clear(QQmlListProperty<FfmpegJob> *p)
{
    FfmpegRender *ffmpegobj = static_cast<FfmpegRender*>(p->object);
    ffmpegobj->p->items.clear();
    Q_EMIT ffmpegobj->itemsChanged();
}

void FfmpegRender::setProgress(qreal progress)
{
    if(p->progress == progress)
        return;

    p->progress = progress;
    Q_EMIT progressChanged();
    if(progress == 1.0) {
        p->ffmpegTools->deleteLater();
        p->ffmpegTools = 0;
        Q_EMIT runningChanged();
        Q_EMIT finished();
    }
}
