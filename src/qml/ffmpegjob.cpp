#include "ffmpegjob.h"

class FfmpegJobPrivate {
public:
    int fromPosition;
    int toPosition;
    QString source;
};

FfmpegJob::FfmpegJob(QObject *parent) :
    QObject(parent)
{
    p = new FfmpegJobPrivate;
}

FfmpegJob::~FfmpegJob()
{
    delete p;
}

void FfmpegJob::setFromPosition(int fromPos)
{
    if(p->fromPosition == fromPos)
        return;

    p->fromPosition = fromPos;
    Q_EMIT fromPositionChanged();
}

int FfmpegJob::fromPosition() const
{
    return p->fromPosition;
}

void FfmpegJob::setToPosition(int toPos)
{
    if(p->toPosition == toPos)
        return;

    p->toPosition = toPos;
    Q_EMIT toPositionChanged();
}

int FfmpegJob::toPosition() const
{
    return p->toPosition;
}

void FfmpegJob::setSource(const QString &source)
{
    if(p->source == source)
        return;

    p->source = source;
    Q_EMIT sourceChanged();
}

QString FfmpegJob::source() const
{
    return p->source;
}
