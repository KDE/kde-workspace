/******************************************************************************
 *   Copyright (C) 2012 by Shaun Reich <shaun.reich@kdemail.net>              *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU General Public License as           *
 *   published by the Free Software Foundation; either version 2 of           *
 *   the License, or (at your option) any later version.                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/

import QtQuick 2.0

import "plotterpainter.js" as PlotterPainter

Canvas {
    id: canvas
    width: 600
    height: 400

    //milliseconds
    property int sampleInterval: 1000
    Component.onCompleted: {
        PlotterPainter.init(width, height);
    }

    onPaint: {
        var context = getContext("2d");

//        PlotterPainter.sceneWidth = 400;
 //       PlotterPainter.sceneHeight = 400;
//        PlotterPainter.canvas = this
        print("onPaint HEIGHT: " + height);
        print("onPaint WIDTH: " + width);
        PlotterPainter.paint(this, context)
    }


    Timer {
        id: plotterTick
        interval: sampleInterval
        running: true
        repeat: false
        onTriggered: {
            PlotterPainter.advancePlotter();
            canvas.requestPaint();
        }
    }

}
