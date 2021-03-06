#ifndef FFMPEGSCREENCAPTURE_H
#define FFMPEGSCREENCAPTURE_H

#include <QObject>
#include <QUrl>
#include <QDebug>

#include <ffmpegtools.h>

class FfmpegScreenCapturePrivate;
class FfmpegScreenCapture : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QString output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(QString ffmpegPath READ ffmpegPath WRITE setFfmpegPath NOTIFY ffmpegPathChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit FfmpegScreenCapture(QObject *parent = nullptr);
    virtual ~FfmpegScreenCapture();

    void setSource(const QString &source);
    QString source() const;

    void setPosition(int position);
    int position() const;

    void setOutput(const QString &output);
    QString output() const;

    void setFfmpegPath(const QString &ffmpegPath);
    QString ffmpegPath() const;

    bool running() const;

Q_SIGNALS:
    void sourceChanged();
    void positionChanged();
    void outputChanged();
    void ffmpegPathChanged();
    void runningChanged();
    void finished(const QUrl &output);

public slots:
    void start();

private:
    FfmpegScreenCapturePrivate *p;
};

#endif // FFMPEGSCREENCAPTURE_H
