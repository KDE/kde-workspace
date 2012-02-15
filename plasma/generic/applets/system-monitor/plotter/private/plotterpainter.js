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

// the scalar that gets multiplied to scale it up or down.
//  if it is 1 then it is not scaled at all
// all members in points[][] are scaled accordingly well, only y values
var scalar = 1;

var pointMousedOver = false;

function addSample(y)
{
    // adding a new sample, making a new element that contains x and y
    // set x
//    var xPos = graphPadding * 2;
if (points.length > 6)
    return;

    var xValue = graphPadding + (horizSpace * (points.length));

    var yValue = y;

    points.push({x: xValue, y: yValue});

    if (y < 0 + graphPadding) {
//        downscale(y);
    } else if (y > height / 2) {
//        upscale(y);
    }

    debug("SSAMPLE LIST, X VALUE: " + points[points.length - 1][0] + " POINTS.LENGTH: " + points.length + "Y VALUE" + y);
    debug("sample list: " + points);
    debug("requesting new paint event");
}

function downscale(y)
{
    // pick a scalar that's close
    scalar = 1.1;

    debug("*** $$$ downscaling values, found one too big");
    // it's too big, scale all of it down
    for (var i = 0; i < points.length; ++i) {
        points[i].y = Math.abs(points[i].y) * scalar;
    }

    //let it be known we need to clear it because all points got shifted downward
    clearNeeded = true;
}

function debug(str)
{
//    print("PlotterPainter::" + arguments.callee.caller.name.toString() + "() OUTPUT: " + str);
}

function shiftLeft()
{
    debug("");
    //shift all x points left some
    for (var i = 0; i < points.length; ++i) {

        if (points[i].x < graphPadding || (points[i].x - horizSpace) < graphPadding) {
            points.splice(i, 1);
        }

        points[i].x -= horizSpace;

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

    if ((points.length * horizSpace) >= width - graphPadding) {
        shiftLeft();
    }
}

function paint(canvas, context)
{
    if (clearNeeded) {
        print("clear needed");
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
        if (!gridPainted) {
            drawGrid(context);
            gridPainted = true;
        }
    }

}

function drawLines()
{
    // Draw Lines
    context.beginPath();

    context.strokeStyle = "rgba(255, 0, 0, 1)"
    context.fillStyle = "rgba(0, 255, 0, 1)"

    context.moveTo(graphPadding, height - graphPadding);

    //HACK we start at 1.
    for(var i = 1; i < points.length; ++i) {
        debug("length: " + points.length + " i has value: " + i);
        debug("x value: " + points[i].x + " y value: " + points[i].y);

        // FIXME: TEXT IS BROKEN, UPSTREAM
        // context.text("POINT" , points[i][0], points[i][1]);

        var x;
        var y;
        x = points[i].x;
        y = points[i].y;

        var cp1x = x - 5;
        var cp1y = y;
        var cp2x = x;
        var cp2y = y - 10;

        context.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x + 5, y);
    }

    context.lineTo(points[points.length - 1].x, height - graphPadding);
    context.lineTo(0, height - graphPadding);
    context.closePath();

    context.fill();
    context.stroke();
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
    context.beginPath();
    context.strokeStyle = "rgba(0, 0, 0, 1)"

    //FIXME: implement binary search, instead of linear
    for (var i = 0; i < points.length; ++i) {
        if (points[i].x - 5 < x && x < points[i].x + 5)  {
            if (pointMousedOver == false ) {
                context.clearRect(0, 0, width, height);
                paint(canvas, context);
            }

            context.text(points[i].x, points[i].x, points[i].y);
            pointMousedOver = true;
        } else if (i == points.length - 1) {
            pointMousedOver = false;
        }
    }

breakout:
    context.stroke();
    context.closePath();
}
