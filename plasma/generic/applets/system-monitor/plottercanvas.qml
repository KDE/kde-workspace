import QtQuick 2.0

import "plotterpainter.js" as PlotterPainter

Canvas {
    width: 600
    height: 400

    //milliseconds
    property int sampleInterval: 2000

    onPaint: {
        var context = getContext("2d");
        PlotterPainter.paint(this, context)
    }

    /**
     * Advances the plotter (shifts all points left) by 1 interval
     * Should be called every tick of the plotter sampler.
     */
    function advancePlotter() {
        
    }

    Timer {
        id: plotterTick
        interval: 2000
        running: true
        repeat: true
        onTriggered: advancePlotter()
    }

}
