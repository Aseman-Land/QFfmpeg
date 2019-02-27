#ifndef FFMPEGJOB_H
#define FFMPEGJOB_H

#include <QObject>

class FfmpegJobPrivate;
class FfmpegJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int fromPosition READ fromPosition WRITE setFromPosition NOTIFY fromPositionChanged)
    Q_PROPERTY(int toPosition READ toPosition WRITE setToPosition NOTIFY toPositionChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)

public:
    FfmpegJob(QObject *parent = nullptr);
    virtual ~FfmpegJob();

    void setFromPosition(int fromPos);
    int fromPosition() const;

    void setToPosition(int toPos);
    int toPosition() const;

    void setSource(const QString &source);
    QString source() const;

Q_SIGNALS:
    void fromPositionChanged();
    void toPositionChanged();
    void sourceChanged();

public slots:

private:
    FfmpegJobPrivate *p;
};

#endif // FFMPEGJOB_H
