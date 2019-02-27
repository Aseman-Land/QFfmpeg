#ifndef QFFMPEGPLUGIN_H
#define QFFMPEGPLUGIN_H

#include <QQmlExtensionPlugin>

class QFfmpegPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);

};

#endif // QFFMPEGPLUGIN_H
