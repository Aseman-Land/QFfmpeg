import QtQuick 2.9
import QtQuick.Window 2.2
import QFfmpeg 0.1

Item {
    property alias source: screenCapture.source
    property alias output: screenCapture.output
    property alias position: screenCapture.position
    property alias ffmpegPath: screenCapture.ffmpegPath
    property alias sourceSize: screenView.sourceSize
    property alias verticalAlignment: screenView.verticalAlignment
    Image {
        id: screenView
        anchors.fill: parent
        fillMode: fillMode
        sourceSize: sourceSize
        verticalAlignment: verticalAlignment
    }
    FFmpegScreenCapture {
        id: screenCapture
        source: source
        output: output
        position: position
        ffmpegPath: ffmpegPath
        onSourceChanged: {
            screenCapture.start()
        }
        onFinished: {
            screenView.source= output
        }
    }
}
