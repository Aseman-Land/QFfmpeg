import QtQuick 2.9
import QtQuick.Window 2.2
import QFfmpeg 0.2

Item {
    property alias source: screenCapture.source
    property alias output: screenCapture.output
    property alias position: screenCapture.position
    property alias ffmpegPath: screenCapture.ffmpegPath
    property alias sourceSize: screenView.sourceSize
    property alias verticalAlignment: screenView.verticalAlignment
    property alias fillMode: screenView.fillMode
    Image {
        id: screenView
        anchors.fill: parent
    }
    FFmpegScreenCapture {
        id: screenCapture
        onSourceChanged: {
            screenCapture.start()
        }
        onFinished: {
            screenView.source= output
        }
    }
}
