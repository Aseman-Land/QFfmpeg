#include "ffmpegtools.h"

#include <QTime>
#include <QFile>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

FfmpegTools::FfmpegTools(QObject *parent) :
    QObject(parent)
{
    tempDirectory = QCoreApplication::applicationDirPath();
}

FfmpegTools::~FfmpegTools()
{

}

qreal FfmpegTools::getProgress(const QString &output, qint32 duration, qint32 from_ms = 0)
{
    QString time = QStringLiteral("");
    qreal progress;

    QRegExp rx(QStringLiteral("time=(\\d\\d:\\d\\d:\\d\\d.\\d\\d)"));
    int pos = rx.indexIn(output);
    if (pos > -1) {
        time = rx.cap(1);
    }
    if(time == QStringLiteral("")) {
        progress = -1;
    } else {
        QTime currentTime = QTime::fromString(time);
        qint32 currentTimeMs = QTime(0, 0, 0).msecsTo(currentTime) - from_ms;
        progress = currentTimeMs / static_cast<qreal>(duration);
    }
    if(progress >= 1)
        progress = 0.99999;

    return progress;
}

qint32 FfmpegTools::getRemainingTime(ProcessStatus processStatus, qreal progress)
{
    if(processStatus.lastProgress == 0.0) {
        return -1;
    }
    qreal averageSpeed = progress / processStatus.startTime.msecsTo(QTime::currentTime());
    qreal speed = (progress - processStatus.lastProgress) / processStatus.lastTime.msecsTo(QTime::currentTime());
    qreal overallSpeed = ( averageSpeed + speed ) / 2;
    qint32 remainingTime = static_cast<qint32>(( 1 - progress ) / overallSpeed);
    return remainingTime;
}

void FfmpegTools::cut(const QString &inputFile, qint32 from_ms, qint32 to_ms, const QString &outputFile, std::function<void (qreal, const QString &, qint32)> callback)
{
    QProcess *ffmpegProcess = new QProcess(this);

    QTime startTime(0, 0);
    QTime endTime(0, 0);
    startTime = startTime.addMSecs(from_ms);
    endTime = endTime.addMSecs(to_ms - from_ms);
    qint32 duration = to_ms - from_ms;

    QStringList options = {QStringLiteral("-y"),  QStringLiteral("-ss"), startTime.toString(QStringLiteral("HH:mm:s.zzz")), QStringLiteral("-i"), inputFile, QStringLiteral("-to"), endTime.toString(QStringLiteral("HH:mm:s.zzz")), /*"-c", "copy", */outputFile, QStringLiteral("-avoid_negative_ts"), QStringLiteral("make_zero"), QStringLiteral("-hide_banner")};
    ffmpegProcess->start(QCoreApplication::applicationDirPath() + "/" + QStringLiteral("ffmpeg"), options);

    processStatus[ffmpegProcess].startTime = QTime::currentTime();
    processStatus[ffmpegProcess].lastProgress = 0;

    connect(ffmpegProcess, &QProcess::readyReadStandardOutput, [=](){
        QByteArray output = ffmpegProcess->readAllStandardOutput();
        qreal progress = getProgress(QString::fromUtf8(output), duration);
        if(progress == -1)
            return;

        qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

        processStatus[ffmpegProcess].lastTime = QTime::currentTime();
        processStatus[ffmpegProcess].lastProgress = progress;

        callback(progress, QString::fromUtf8(output), remainingTime);
    });

    connect(ffmpegProcess, &QProcess::readyReadStandardError, [=](){
        QByteArray output = ffmpegProcess->readAllStandardError();
        qreal progress = getProgress(QString::fromUtf8(output), duration);
        if(progress == -1)
            return;

        qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

        processStatus[ffmpegProcess].lastTime = QTime::currentTime();
        processStatus[ffmpegProcess].lastProgress = progress;

        callback(progress, QString::fromUtf8(output), remainingTime);
    });

    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
        callback(1, "exitCode:" + QString::number(exitCode) + " exitStatus:" + QString::number(exitStatus), 10);
        ffmpegProcess->deleteLater();
    });
}

void FfmpegTools::merge(const QStringList &inputFiles, const QString &outputFile, std::function<void (qreal, const QString &, qint32)> callback)
{
	QFile::remove(QCoreApplication::applicationDirPath() + "/list");
	
    int *jobs = new int(0);
    QList<Metadata> *metas = new QList<Metadata>();
    for(const auto &file: inputFiles) {
        getData(file, [jobs, metas, this, inputFiles, outputFile, callback](Metadata metadata) {
            metas->append(metadata);
            (*jobs)--;
            if(*jobs == 0) {
                qint32 totalDuration = 0;
                for(int i = 0; i < metas->size(); i++) {
                    totalDuration += metas->at(i).duration;
                }
                QFile file(QCoreApplication::applicationDirPath() + "/list");
                file.open(QIODevice::ReadWrite);
                for(const auto &fileName: inputFiles) {
                    QString text = "file '" + fileName + "'\n";
                    file.write(text.toUtf8());
                }
                file.close();

                QProcess *ffmpegProcess = new QProcess(this);
                ffmpegProcess->setWorkingDirectory(QCoreApplication::applicationDirPath());
                QStringList options = {QStringLiteral("-y"), QStringLiteral("-f"), QStringLiteral("concat"), QStringLiteral("-safe"), QStringLiteral("0"), QStringLiteral("-i"), QStringLiteral("list"), /*"-c", "copy", */outputFile, QStringLiteral("-hide_banner")};
                ffmpegProcess->start(QCoreApplication::applicationDirPath() + "/" + QStringLiteral("ffmpeg"), options);

                processStatus[ffmpegProcess].startTime = QTime::currentTime();
                processStatus[ffmpegProcess].lastProgress = 0;

                connect(ffmpegProcess, &QProcess::readyReadStandardOutput, [=](){
                    QByteArray output = ffmpegProcess->readAllStandardOutput();
                    qreal progress = getProgress(QString::fromUtf8(output), totalDuration);

                    qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

                    processStatus[ffmpegProcess].lastTime = QTime::currentTime();
                    processStatus[ffmpegProcess].lastProgress = progress;

                    callback(progress, QString::fromUtf8(output), remainingTime);
                });

                connect(ffmpegProcess, &QProcess::readyReadStandardError, [=](){
                    QByteArray output = ffmpegProcess->readAllStandardError();
                    qreal progress = getProgress(QString::fromUtf8(output), totalDuration);

                    qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

                    processStatus[ffmpegProcess].lastTime = QTime::currentTime();
                    processStatus[ffmpegProcess].lastProgress = progress;

                    callback(progress, QString::fromUtf8(output), remainingTime);
                });

                connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
                    callback(1, "exitCode:" + QString::number(exitCode) + " exitStatus:" + QString::number(exitStatus), 10);

                    QFile::remove(QCoreApplication::applicationDirPath() + "/list");

                    QDir dir(tempDirectory + "/temp");
                    dir.removeRecursively();

                    delete jobs;
                    delete metas;
                    ffmpegProcess->deleteLater();
                });
            }
        });
        (*jobs)++;
    }
}

void FfmpegTools::convert(const QString &inputFile, const QSize &resolution, qreal frameRate, const QString &codec, const QString &outputFile, std::function<void (qreal, const QString &, qint32)> callback)
{
    getData(inputFile, [=](Metadata metadata){
        QProcess *ffmpegProcess = new QProcess(this);
        qreal fps;
        if(frameRate == 0.0) {
            fps = metadata.fps;
        } else {
            fps = frameRate;
        }

        QStringList options = {QStringLiteral("-y"),  QStringLiteral("-i"), inputFile, QStringLiteral("-r"), QString::number(fps), QStringLiteral("-c:v"), QStringLiteral("libx264"), QStringLiteral("-c:a"), QStringLiteral("copy"), QStringLiteral("-vf"), "scale=" + QString::number(resolution.width()) + ":" + QString::number(resolution.height()), outputFile};
        ffmpegProcess->start(QCoreApplication::applicationDirPath() + "/" + QStringLiteral("ffmpeg"), options);

        processStatus[ffmpegProcess].startTime = QTime::currentTime();
        processStatus[ffmpegProcess].lastProgress = 0;

        connect(ffmpegProcess, &QProcess::readyReadStandardOutput, [=](){
            QByteArray output = ffmpegProcess->readAllStandardOutput();
            qreal progress = getProgress(QString::fromUtf8(output), metadata.duration);

            qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

            processStatus[ffmpegProcess].lastTime = QTime::currentTime();
            processStatus[ffmpegProcess].lastProgress = progress;

            callback(progress , QString::fromUtf8(output), remainingTime);
        });

        connect(ffmpegProcess, &QProcess::readyReadStandardError, [=](){
            QByteArray output = ffmpegProcess->readAllStandardError();
            qreal progress = getProgress(QString::fromUtf8(output), metadata.duration);

            qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

            processStatus[ffmpegProcess].lastTime = QTime::currentTime();
            processStatus[ffmpegProcess].lastProgress = progress;

            callback(progress, QString::fromUtf8(output), remainingTime);
        });

        connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            callback(1, "exitCode:" + QString::number(exitCode) + " exitStatus:" + QString::number(exitStatus), 10);
            ffmpegProcess->deleteLater();
        });
    });
}

void FfmpegTools::getData(const QString &inputFile, std::function<void (Metadata)> callback)
{
    QProcess *ffmpegProcess = new QProcess(this);
    QStringList options = {QStringLiteral("-y"), QStringLiteral("-i"), inputFile, QStringLiteral("-hide_banner")};
    ffmpegProcess->start(QCoreApplication::applicationDirPath() + "/" + QStringLiteral("ffmpeg"), options);

    connect(ffmpegProcess, &QProcess::readyReadStandardOutput, this, [ffmpegProcess, this](){
        buffers[ffmpegProcess] += ffmpegProcess->readAllStandardOutput();
    });

    connect(ffmpegProcess, &QProcess::readyReadStandardError, this, [=](){
        buffers[ffmpegProcess] += ffmpegProcess->readAllStandardError();
    });

    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [ffmpegProcess, this, callback](){
        QByteArray output = buffers.value(ffmpegProcess);
        Metadata metadata;
        QRegExp rx(QStringLiteral("Duration:\\s(\\d\\d:\\d\\d:\\d\\d.\\d\\d).+Video:\\s([^\\s]+).+\\,\\s(\\d+)x(\\d+).+(\\d+\\.*\\d*)\\s+fps"));
        int pos = rx.indexIn(QString::fromUtf8(output));
        if(pos > -1) {
            QTime time(0, 0);
            metadata.duration = QTime(0, 0, 0).msecsTo(QTime::fromString(rx.cap(1)));
            metadata.codec = rx.cap(2);
            metadata.resolution = QSize(rx.cap(3).toInt(), rx.cap(4).toInt());
            metadata.fps = rx.cap(5).toDouble();
        } else {
            metadata.duration = -1;
            metadata.resolution = QSize(-1, -1);
            metadata.fps = -1;
        }
        callback(metadata);
        ffmpegProcess->deleteLater();
    });

    connect(ffmpegProcess, &QProcess::destroyed, this, [ffmpegProcess, this](){
       buffers.remove(ffmpegProcess);
    });
}

void FfmpegTools::render(QList<FfmpegTools::Render> renderList, const QSize &resolution, qreal frameRate, const QString &outputFile, std::function<void (qreal, const QString &, qint32)> callback)
{
    QDir dir(tempDirectory + "/temp");
    dir.removeRecursively();
    qreal *totalProgress = new qreal(0);
    const qreal singleProgress = 1.0 / (2 * renderList.length() + 1);
    auto topCallback = [callback, totalProgress, singleProgress](qreal progress, const QString &log, qint32 remainingTime) {
        if(progress == -1)
            return;
        qreal newProgress = progress * singleProgress;
        callback(*totalProgress + newProgress, log, remainingTime);
        if(progress == 1) {
            *totalProgress += singleProgress;
        }
    };
    QDir dir2;
    dir2.mkpath(tempDirectory + "/temp/cut");
    dir2.mkpath(tempDirectory + "/temp/convert");
    int *jobs = new int(0);
    QHash<QString, FfmpegTools::Metadata> *renderListData = new QHash<QString, FfmpegTools::Metadata>();
    for(const auto &renderItem: renderList) {
        getData(renderItem.inputFile, [outputFile, resolution, renderList, topCallback, this, jobs, renderItem, renderListData](FfmpegTools::Metadata metadata) {
            (*jobs)--;
            renderListData->insert(renderItem.inputFile, metadata);
            if(*jobs == 0) {
                delete jobs;
                render_cut(outputFile, renderList, renderList, resolution, *renderListData, topCallback);
            }
        });
        (*jobs)++;
    }
}

void FfmpegTools::takeScreenshot(const QString &inputFile, qint32 capture_time, const QString &outputFile, std::function<void (const QString &log)> callback)
{
    QTime time(0, 0);
    time = time.addMSecs(capture_time);

    QProcess *ffmpegProcess = new QProcess(this);
    QStringList options = {QStringLiteral("-y"), QStringLiteral("-ss"), time.toString(QStringLiteral("HH:mm:s.zzz")), QStringLiteral("-i"), inputFile, outputFile, QStringLiteral("-hide_banner")};
    ffmpegProcess->start(QCoreApplication::applicationDirPath() + "/" + QStringLiteral("ffmpeg"), options);

    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [ffmpegProcess, callback](){
        callback(QString::fromUtf8(ffmpegProcess->readAllStandardOutput()));
        ffmpegProcess->deleteLater();
    });
}

void FfmpegTools::setTempDirectory(const QString &tempDir)
{
    tempDirectory = tempDir;
}

QString FfmpegTools::getTempDirectory() const
{
    return tempDirectory;
}

void FfmpegTools::render_cut(const QString &outputFile, const QList<FfmpegTools::Render> renderListCopy, QList<FfmpegTools::Render> renderList, const QSize &resolution, const QHash<QString, FfmpegTools::Metadata> &renderListData, std::function<void (qreal, const QString &, qint32)> callback, qint32 nameKey)
{
    if(renderList.isEmpty()) {
        render_convert(outputFile, renderListCopy, renderListCopy, resolution, renderListData, callback);
        return;
    }

    Render renderItem = renderList.takeFirst();
    if((renderItem.startTime == 0 && renderItem.endTime == renderListData[renderItem.inputFile].duration) || (renderItem.startTime == 0 && renderItem.endTime == 0)) {
        QFile::copy(renderItem.inputFile, tempDirectory + "/temp/cut/" + QString::number(nameKey) + renderItem.inputFile.mid(renderItem.inputFile.lastIndexOf(QStringLiteral("."))));
        render_cut(outputFile, renderListCopy, renderList, resolution, renderListData, callback, nameKey + 1);
    } else {
        cut(renderItem.inputFile, renderItem.startTime, renderItem.endTime, tempDirectory + "/temp/cut/" + QString::number(nameKey) + renderItem.inputFile.mid(renderItem.inputFile.lastIndexOf(QStringLiteral("."))), [=](qreal progress, const QString &log, qint32 remainingTime) {
            callback(progress, log, remainingTime);
            if(progress == 1.0) {
                render_cut(outputFile, renderListCopy, renderList, resolution, renderListData, callback, nameKey + 1);
            }
        });
    }
}

void FfmpegTools::render_convert(const QString &outputFile, const QList<Render> renderListCopy, QList<FfmpegTools::Render> renderList, const QSize &resolution, const QHash<QString, FfmpegTools::Metadata> &renderListData, std::function<void (qreal progress, const QString &log, qint32 remainingTime)> callback, qint32 nameKey)
{
    if(renderList.isEmpty()) {
        QStringList inputFiles;
        for(int i = 0; i < renderListCopy.size(); i++) {
            inputFiles.append(tempDirectory + "/temp/convert/" + QString::number(i) + renderListCopy.at(i).inputFile.mid(renderListCopy.at(i).inputFile.lastIndexOf(QStringLiteral("."))));
        }
        merge(inputFiles, outputFile, callback);
        return;
    }

    Render renderItem = renderList.takeFirst();
    if(renderListData[renderItem.inputFile].resolution == resolution) {
        QFile::copy(tempDirectory + "/temp/cut/" + QString::number(nameKey) + renderItem.inputFile.mid(renderItem.inputFile.lastIndexOf(QStringLiteral("."))), tempDirectory + "/temp/convert/" + QString::number(nameKey) + renderItem.inputFile.mid(renderItem.inputFile.lastIndexOf(QStringLiteral("."))));
        render_convert(outputFile, renderListCopy, renderList, resolution, renderListData, callback, nameKey + 1);
    } else {
        convert(tempDirectory + "/temp/cut/" + QString::number(nameKey) + renderItem.inputFile.mid(renderItem.inputFile.lastIndexOf(QStringLiteral("."))), resolution, renderListData[renderItem.inputFile].fps, renderListData[renderItem.inputFile].codec, tempDirectory + "/temp/convert/" + QString::number(nameKey) + renderItem.inputFile.mid(renderItem.inputFile.lastIndexOf(QStringLiteral("."))), [=](qreal progress, const QString &log, qint32 remainingTime) {
            callback(progress, log, remainingTime);
            if(progress == 1.0) {
                render_convert(outputFile, renderListCopy, renderList, resolution, renderListData, callback, nameKey + 1);
            }
        });
    }
}

void FfmpegTools::setMetaData(const QString &inputFile, QHash<QString, QString> metadata, const QString &outputFile, std::function<void (qreal, const QString &, qint32)> callback)
{
    QStringList metadataList;
    QHashIterator<QString, QString> i(metadata);
    while(i.hasNext()) {
        i.next();
        metadataList << QStringLiteral("-metadata");
        metadataList << i.key() + "=" + i.value();
    }
    getData(inputFile, [=](Metadata metadata){
        QProcess *ffmpegProcess = new QProcess(this);
        QStringList options = {QStringLiteral("-y"),  QStringLiteral("-i"), inputFile, QStringLiteral("-c"), QStringLiteral("copy")};
        options.append(metadataList);
        options.append(outputFile);
        ffmpegProcess->start(QCoreApplication::applicationDirPath() + "/" + QStringLiteral("ffmpeg"), options);

        processStatus[ffmpegProcess].startTime = QTime::currentTime();
        processStatus[ffmpegProcess].lastProgress = 0;

        connect(ffmpegProcess, &QProcess::readyReadStandardOutput, [=](){
            QByteArray output = ffmpegProcess->readAllStandardOutput();
            qreal progress = getProgress(QString::fromUtf8(output), metadata.duration);

            qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

            processStatus[ffmpegProcess].lastTime = QTime::currentTime();
            processStatus[ffmpegProcess].lastProgress = progress;

            callback(progress , QString::fromUtf8(output), remainingTime);
        });

        connect(ffmpegProcess, &QProcess::readyReadStandardError, [=](){
            QByteArray output = ffmpegProcess->readAllStandardError();
            qreal progress = getProgress(QString::fromUtf8(output), metadata.duration);

            qint32 remainingTime = getRemainingTime(processStatus.value(ffmpegProcess), progress);

            processStatus[ffmpegProcess].lastTime = QTime::currentTime();
            processStatus[ffmpegProcess].lastProgress = progress;

            callback(progress, QString::fromUtf8(output), remainingTime);
        });

        connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            callback(1, "exitCode:" + QString::number(exitCode) + " exitStatus:" + QString::number(exitStatus), 10);
            ffmpegProcess->deleteLater();
        });
    });
}

void FfmpegTools::getMetaData(const QString &inputFile, std::function<void (QHash<QString, QString>)> callback)
{
    QProcess *ffmpegProcess = new QProcess(this);
    QStringList options = {QStringLiteral("-y"), QStringLiteral("-i"), inputFile, QStringLiteral("-hide_banner")};
    ffmpegProcess->start(QCoreApplication::applicationDirPath() + "/" + QStringLiteral("ffmpeg"), options);

    connect(ffmpegProcess, &QProcess::readyReadStandardOutput, this, [ffmpegProcess, this](){
        buffers[ffmpegProcess] += ffmpegProcess->readAllStandardOutput();
    });

    connect(ffmpegProcess, &QProcess::readyReadStandardError, this, [=](){
        buffers[ffmpegProcess] += ffmpegProcess->readAllStandardError();
    });

    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [ffmpegProcess, this, callback](){
        QByteArray output = buffers.value(ffmpegProcess);
        QHash<QString, QString> metadata;
        QRegExp rx(QStringLiteral("([^\\s]+)\\s+:\\s([^\\s]+)"));
        int pos = 0;
        while((pos = rx.indexIn(QString::fromUtf8(output), pos)) != -1) {
            metadata.insert(rx.cap(1), rx.cap(2));
            pos += rx.matchedLength();
        }
        callback(metadata);
        ffmpegProcess->deleteLater();
    });

    connect(ffmpegProcess, &QProcess::destroyed, this, [ffmpegProcess, this](){
       buffers.remove(ffmpegProcess);
    });
}
