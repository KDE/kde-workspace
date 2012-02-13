import QtQuick 2.0

import "plotterpainter.js" as PlotterPainter

Canvas {
    id: canvas
    width: 600
    height: 400

    //milliseconds
    property int sampleInterval: 5000
    Component.onCompleted: {
    }

    onPaint: {
        var context = getContext("2d");

//        PlotterPainter.sceneWidth = 400;
 //       PlotterPainter.sceneHeight = 400;
//        PlotterPainter.canvas = this
        print("onPaint HEIGHT: " + height);
        print("onPaint WIDTH: " + width);
        PlotterPainter.paint(this, context, width, height)
    }


    Timer {
        id: plotterTick
        interval: sampleInterval
        running: true
        repeat: true
        onTriggered: {
            PlotterPainter.advancePlotter();
            canvas.requestPaint();
        }
    }

}
