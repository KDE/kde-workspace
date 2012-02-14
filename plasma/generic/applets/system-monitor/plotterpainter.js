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

.pragma library

var height;
var width;

var graphPadding;
var horizSpace;
var vertSpace;

// [i][0] = x, [i][1] = y
var points = [];
var canvas;
var context;

var clearNeeded = false;

var gridPainted = false;

function addSample(y)
{
    // adding a new sample, making a new element that contains x and y
    // set x
//    var xPos = graphPadding * 2;
    points.push([points.length, 2]);

    var index = 0;
    if (points.length != 0) {
        index = points.length - 1;
    }
    points[index][0] = graphPadding + (horizSpace * (points.length));

    // set y
    points[index][1] = y;

    debug("SSAMPLE LIST, X VALUE: " + points[points.length - 1][0] + " POINTS.LENGTH: " + points.length + "Y VALUE" + y);
    debug("sample list: " + points);
    debug("requesting new paint event");
}

function debug(str)
{
    print("PlotterPainter::" + arguments.callee.caller.name.toString() + "() OUTPUT: " + str);
}

function shiftLeft()
{
    debug("");
    //shift all x points left some
    for (var i = 0; i < points.length; ++i) {

        if (points[i][0] <= 0 || (points[i][0] - horizSpace) <= 0) {
            //FIXME: pretty sure this LEAKS. also, not sure how to use splice properly
            points.splice(i, 2);
        }

        points[i][0] -= horizSpace;

    }

    clearNeeded = true;
}

function init(width, height)
{
    //set global vars
    this.width = width;
    this.height = height;

    graphPadding = 20;

    var divisor = points.length;

    if (divisor == 0) {
        divisor = this.width;
    }


    debug("POOOINTS LENGTH: " + points.length);
    debug("width: " + this.width);

    // TODO: find a scalar, mostly for vertSpace
    horizSpace = 10;
    vertSpace  = 1 //height - (graphPadding * (points.length));

    //form an array of an array
    points.push([]);
}

/**
 * Advances the plotter (shifts all points left) by 1 interval
 * Should be called every tick of the plotter sampler.
 */
function advancePlotter()
{
    debug("");
    var yPercent = Math.floor(Math.random() * 100);
    debug("randomly generated number: " + yPercent);
    debug("$$$$$$$ vertSspace: " + vertSpace);
    debug("$$$$$$$ yPercent: " + yPercent);
    debug("$$$$$$$ graphPadding: " + graphPadding);
    var yPos = (400 * (yPercent / 100) + graphPadding * 2);
    debug("randomly generated y pos: " + yPos);
    addSample(height - yPos);

    if ((points.length * horizSpace) >= width - 50) {
        shiftLeft();
    }
}

function paint(canvas, context)
{
    if (clearNeeded) {
        context.clearRect(0, 0, width, height);
        //we only draw the grid once. it only needs to be redrawn on certain
        //events, like clearing the entire thing
        gridPainted = false;
    }

    //set global vars
    this.canvas = canvas;
    this.context = context;
    //nothing to paint if 0
    if (points.length != 0) {

        debug("WIDTH: " + width);
        debug("PAINT HEIGHT: " + height);

        drawLines();
        fillPath();
        if (!gridPainted) {
            drawGrid(context);
            gridPainted = true;
        }
    }
}

function drawLines()
{
    // Draw Lines
//    print("plotterPainter::paint starting to draw lines, xPos: " + xPos);

    context.beginPath();

    //HACK we start at 1.
    for(var i = 1; i < points.length; ++i) {
        debug("length: " + points.length + " i has value: " + i);
        debug("x value: " + points[i][0] + " y value: " + points[i][1]);

        // FIXME: TEXT IS BROKEN, UPSTREAM
        // context.text("POINT" , points[i][0], points[i][1]);

        var x;
        var y;
        if(points.length > 1) {
            x = points[i - 1][0] - graphPadding;
            y = points[i - 1][1];

            context.moveTo(points[i][0] - graphPadding, points[i][1]);

            var cp1x = x
            var cp1y = y - 20
            var cp2x = x + 10
            var cp2y = y + 20

            context.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
        } else {
            context.lineTo(points[i][0] - graphPadding, points[i][1]);
        }
    }

    context.stroke();
    context.closePath();

}

function fillPath()
{

    //fill path (everything below the line graph)
    context.beginPath();
    context.moveTo(graphPadding * 2, height - graphPadding)
    context.fillStyle = "rgba(255, 0, 0, 1)"

    //FIXME: because i have no fucking clue where this comes from, or why it's offset
    var offset = 20;
    var x;
    var y;
    for(var i = 0; i < points.length; ++i) {
        x = points[i][0] - offset;
        y = points[i][1];

        var cp1x = x 
        var cp1y = y - 20
        var cp2x = x + 10
        var cp2y = y + 20

        context.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y);
//        context.lineTo(points[i][0] - offset, points[i][1]);
        debug("points[i][1] " + points[i][1]);
    }

    context.lineTo(i * horizSpace - offset, height - graphPadding);
    context.lineTo(0, height - graphPadding);

    context.closePath();
    if (points.length > 1) {
        context.fill();
    }

}

function drawGrid(context)
{
    debug("painting on width: " + width);
    debug("painting on height: " + height);

    // Draw Axis
    context.lineWidth = 1;
    context.strokeStyle = "rgba(0, 0, 0, 0.3)"

    for (var y = 0; y < height - graphPadding; y += height/20) {
        context.moveTo(graphPadding, y);
        context.lineTo(width - graphPadding, y);
    }
    context.stroke();

    context.beginPath();
    context.moveTo(graphPadding, graphPadding);
    context.lineTo(graphPadding,height - graphPadding);
    context.lineTo(width - graphPadding, height - graphPadding);
    context.stroke();
    context.closePath();
}

function mouseMoved(x, y)
{
    //FIXME: implement binary search, instead of linear
    for (var i = 0; i < points.length; ++i) {
        if (points[i][0] - 50 < x > points[i][0] + 50) {
            debug("****** MOUSE HIT A POINT, NEAREST POINT WE FOUND: " + points[i][0]);
            context.text(points[i][0], points[i][0], points[i][1]);
        }
    }
}
