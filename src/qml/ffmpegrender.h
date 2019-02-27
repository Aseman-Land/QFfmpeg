#ifndef FFMPEGRENDER_H
#define FFMPEGRENDER_H

#include <QObject>

#include <ffmpegtools.h>
#include <QQmlListProperty>

#include "ffmpegjob.h"

class FfmpegRenderPrivate;
class FfmpegRender : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal fps READ fps WRITE setFPS NOTIFY fpsChanged)
    Q_PROPERTY(QSize resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
    Q_PROPERTY(QString output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(QString ffmpegPath READ ffmpegPath WRITE setFfmpegPath NOTIFY ffmpegPathChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString tempDirectory READ tempDirectory NOTIFY tempDirectoryChanged)
    Q_PROPERTY(QQmlListProperty<FfmpegJob> items READ items NOTIFY itemsChanged)
    Q_CLASSINFO("DefaultProperty", "items")

public:
    explicit FfmpegRender(QObject *parent = nullptr);
    virtual ~FfmpegRender();

    QQmlListProperty<FfmpegJob> items();
    QList<FfmpegJob*> itemsList() const;

    void setFPS(qreal fps);
    qreal fps() const;

    void setResolution(QSize resolution);
    QSize resolution() const;

    void setOutput(const QString &output);
    QString output() const;

    void setFfmpegPath(const QString &ffmpegPath);
    QString ffmpegPath() const;

    void setTempDirectory(const QString &&tempDir);
    QString tempDirectory() const;

    qreal progress() const;

    bool running() const;

Q_SIGNALS:
    void itemsChanged();
    void fpsChanged();
    void resolutionChanged();
    void outputChanged();
    void ffmpegPathChanged();
    void progressChanged();
    void runningChanged();
    void tempDirectoryChanged();
    void finished();

public Q_SLOTS:
    void start();
    void stop();

private:
    static void append(QQmlListProperty<FfmpegJob> *p, FfmpegJob *v);
    static int count(QQmlListProperty<FfmpegJob> *p);
    static QObject *at(QQmlListProperty<FfmpegJob> *p, int idx);
    static void clear(QQmlListProperty<FfmpegJob> *p);

    void setProgress(qreal progress);

private:
    FfmpegRenderPrivate *p;
};

#endif // FFMPEGRENDER_H
