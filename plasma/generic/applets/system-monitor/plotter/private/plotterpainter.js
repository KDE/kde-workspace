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

/* actually a multidimensional array
* array of arrays
* each points[i] represents a sample set.
* A sample set is an entire set of graph data that is logically
* separated from everything all other sets. e.g. downstream and upstream,
* read I/O and write I/O, etc. each points[i][j] indicates 1 dot/point
* on that particular sampleSet
*/
var points = Array();

var canvas;
var context = null;

var clearNeeded = false;

var gridPainted = false;
var backgroundPainted = false;

var GraphTypeEnum = {
    BarGraph: "BarGraph",
    FilledLineGraph: "FilledLineGraph"
}

// what this graph is currently being rendered as
// a bar graph or a filled line graph
var graphType = GraphTypeEnum.FilledLineGraph; //BarGraph

// the scalar that gets multiplied to scale it up or down.
//  if it is 1 then it is not scaled at all
// all members in points[][] are scaled accordingly well, only y values
var scalar = 0;

var pointMousedOver = false;

// holds if we have hover text needing painted, and where at
// access via hoverText.bool = true, etc.
var hoverText = {visible: false, x: 0, y: 0, clearNeeded: false}

/*
 * @p sampleSet corresponds to the subscript in points[subscript]
 */
function addSample(y, sampleSet, fillColor)
{
    var pointsIndex = sampleSet;

    if (points[pointsIndex] === undefined) {
        points.push(new Array());
 //       print("PUSHED NEW ARRAY TO POINTS");
        points[pointsIndex].push(new Array());
//        points[points.length - 1][points
    }

    var xValue = graphPadding + (horizSpace * (points[pointsIndex].length));

    var yValue = y;

        points[pointsIndex].push( { x: xValue, originalY: yValue, scaledY: yValue, fillColor: fillColor } );

 //   downscaleOne(y);

    if (y < 0 + graphPadding) {
//        downscaleAll(y);
    } else if (y > height / 2) {
//        upscale(y);
    }

}

/**
 * Attempts to downscale 1 (new!) point to match the rest.
 * Applicable only if the plotter has already been downscaled
 * If it has not, nothing happens and you'll need to call downscaleAll(y)
 */
function downscaleOne(y)
{
    //no scalar applied if 0
    if (scalar != 0) {
        var index = points.length - 1;
        points[index][sampleSetLength].scaledY = Math.abs(points[index][sampleSetLength].scaledY + ((height - graphPadding) * scalar));

        if (points[index][sampleSetLength].scaledY > height - graphPadding) {
            points[index][sampleSetLength].scaledY = height - graphPadding;
        }
    }
}

function downscaleAll(y)
{
    // pick a scalar that's close
    scalar = 0.05;

    debug("*** $$$ downscaling values, found one too big");
    // it's too big, scale all of it down
    for (var i = 0; i < points.length; ++i) {

        points[i].scaledY = Math.abs(points[i].scaledY + ((height - graphPadding) * scalar));

        if (points[i].scaledY > height - graphPadding) {
            //yeah, it's assumed to be 0 now, truncation i guess
            points[i].scaledY = height - graphPadding;
        }
    }

    //let it be known we need to clear it because all points got shifted downward
    clearNeeded = true;
}

function debug(str)
{
   // print("PlotterPainter::" + arguments.callee.caller.name.toString() + "() OUTPUT: " + str);
}

function shiftLeft()
{
    debug("");
    //shift all x points left some
    for (var i = 0; i < points.length; ++i) {
        for (var j = 0; j < points[i].length; ++j) {

            if (points[i][j].x < graphPadding || (points[i][j].x - horizSpace) < graphPadding) {
                //FIXME: SHIFTS LEFT FAR,  FAR TOO SOON, THE GRAPH CANT KEEP UP HARDLY
//                points[i].splice(j, 1);
            }

            print("*****^^^^^** SHIFTLEFT VALUE, BEFORE: " + points[i][j].x);
            points[i][j].x -= horizSpace;
            print("*****^^^^^** SHIFTLEFT VALUE, AFTER: " + points[i][j].x);

            if (hoverText.visible = true) {
                //shift hovertext to the left actually
                hoverText.x -= horizSpace;
                canvas.requestPaint();
            }
        }
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
    var yPercent2 = Math.floor(Math.random() * 100);
    debug("randomly generated number: " + yPercent);
    debug("$$$$$$$ vertSspace: " + vertSpace);
    debug("$$$$$$$ yPercent: " + yPercent);
    debug("$$$$$$$ graphPadding: " + graphPadding);
    var yPos = (height * (yPercent / 100) + graphPadding * 2);
    var yPos2 = (height * (yPercent2 / 100) + graphPadding * 2);
    debug("randomly generated y pos: " + yPos);
    addSample(height - yPos, 0, "rgba(0, 150, 255, .5)");
    addSample(height - yPos2, 1, "rgba(0, 255, 0, .6)");

    var index = points.length - 1;
    if ((points[index][points[index].length - 1].x * horizSpace) >= width - graphPadding) {
        //FIXME: shiftleft is broken..occurs far, far too often
     //   shiftLeft();
    }
}

function paint(canvas, context)
{
    //set global vars
    this.canvas = canvas;
    this.context = context;

    if (clearNeeded) {
        context.clearRect(0, 0, width, height);
        //we only draw the grid once. it only needs to be redrawn on certain
        //events, like clearing the entire thing
        gridPainted = false;
        backgroundPainted = false;
    }

    if (hoverText.visible == false && hoverText.clearNeeded == true) {
        //FIXME: trigger repaint only on the previous area. This doesn't work because there's some bug where it paints more than one

        // i'm guessign it's a program flow control issue.
        // context.clearRect(hoverText.x - 50, hoverText.y - 50, hoverText.x + 50, hoverText.y + 50);

        context.clearRect(0, 0, width, height);
        hoverText.clearNeeded = false;
        gridPainted = false;
        backgroundPainted = false;
    }

    //FIXME: HACK, if i put the painting in the if statement of
    //backgroundPainted, it doesn't get called. well, it does. but doesn't paint wtf?
    backgroundPainted = true;
    gridPainted = false;

    context.beginPath();
    context.fillStyle = "rgba(70, 0, 0, 1)";
    context.fillRect(0, 0, width, height);
    context.closePath();

    //nothing to paint if 0
    if (points.length != 0) {

        debug("WIDTH: " + width);
        debug("PAINT HEIGHT: " + height);

        if (!gridPainted) {
            drawGrid(context);
            gridPainted = true;
        }

        if (graphType == GraphTypeEnum.FilledLineGraph) {
            drawLines();
        } else if (graphType == GraphTypeEnum.BarGraph) {
            drawBarGraph();
        }

        if (hoverText.visible == true) {
            context.beginPath();
            context.strokeStyle = "rgba(0, 0, 255, 1)";
            context.strokeWidth = 2;
            context.fillStyle = "rgba(50, 0, 0, 1)";

            context.text(Math.round(height - graphPadding - hoverText.y), hoverText.x, hoverText.y + 10);
            context.closePath();
            context.stroke();
            context.fill();
        }
    }
}

function drawBackground()
{
    context.beginPath();
    context.fillStyle = "rgba(50, 0, 0, 1)";
    context.fillRect(0, 0, width, height);
    context.closePath();
}

function drawBarGraph()
{
    // Draw Bars
    context.beginPath();

    context.strokeStyle = "rgba(0, 255, 0, 1)"

    context.moveTo(graphPadding, height - graphPadding);

    for (var i = 0; i < points.length; ++i) {
        for (var j = 0; j < points[i].length; ++j) {
            var x;
            var y;

            x = points[i][j].x;
            y = points[i][j].scaledY;

            context.fillStyle = points[i][j].fillColor;
            context.rect(x, y, horizSpace, (height - graphPadding) - y);
        }
    context.closePath();
    context.fill();
    context.stroke();
    }

//    var grd = context.createLinearGradient(graphPadding, graphPadding, graphPadding, height - graphPadding);
//    grd.addColorStop(0, "rgba(0, 255, 0, 0.5)");
 //   grd.addColorStop(1, "rgba(0, 180, 0, 0.2)");
  //  context.fillStyle = grd;
}

function drawLines()
{
    // Draw Lines

    for (var i = 0; i < points.length; ++i) {

    context.beginPath();
        context.moveTo(graphPadding, height - graphPadding);

        for (var j = 0; j < points[i].length; ++j) {
            var x;
            var y;
            x = points[i][j].x;
            y = points[i][j].scaledY;

            var cp1x = x - 5;
            var cp1y = y;
            var cp2x = x;
            var cp2y = y - 10;

            context.strokeStyle = points[i][j].fillColor
            context.fillStyle = points[i][j].fillColor;
            context.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x + 5, y);

            if (j < points[i].length - 1) {
                x = points[i][j + 1].x;
                cp1x = x - 5;
                cp2x = x - 5;
                y = points[i][j + 1].scaledY;
                cp1y = y;
                cp2y = y - 10;
              context.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x + 5, y);
            } else {
              context.lineTo(points[i][j].x, height - graphPadding);
            }
        }

        context.lineTo(graphPadding, height - graphPadding);
        context.closePath();
        context.fill();
        context.stroke();
    }


//    var grd = context.createLinearGradient(graphPadding, graphPadding, graphPadding, height - graphPadding);
 //   grd.addColorStop(0, "rgba(0, 255, 0, 0.5)");
  //  grd.addColorStop(1, "rgba(0, 180, 0, 0.2)");
   // context.fillStyle = grd;
}

function drawGrid(context)
{
    debug("painting on width: " + width);
    debug("painting on height: " + height);

    // Draw Axis
    context.lineWidth = 1;
    //NOTE: set to .5 alpha or so to get paint regions easily
    context.strokeStyle = "rgba(0, 0, 0, 1)"

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
        if (points[i].x - 10 < x && x < points[i].x + 10)  {
            hoverText.visible = true;
            hoverText.x = points[i].x;
            hoverText.y = points[i].originalY;
            canvas.requestPaint();
            // we found one, who cares about anything else
            break;
        } else if (i == points.length - 1) {
            // we've hit the end (otherwise we'd have broken out of this loop)
            // so there are no matching points.
            hoverText.visible = false;
            // wipe the previous one, we don't care
            hoverText.clearNeeded = true;
            canvas.requestPaint();
        }
    }
}
