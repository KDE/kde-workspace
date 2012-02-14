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

function addSample(y)
{
    // adding a new sample, making a new element that contains x and y
    // set x
//    var xPos = graphPadding * 2;
    points.push([points.length, 2]);
    points[points.length - 1][0] = horizSpace * (points.length);

    // set y
    points[points.length - 1][1] = y;

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
//    for (var i = 0; i < count; ++i) {
 //       points[i] -= horizSpace;
  //  }
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
    horizSpace = 15;
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


//    if ((points.length * horizSpace) >= width - 20) {
//        shiftLeft();
//    }
}

function paint(canvas, context)
{
    //nothing to paint if 0
    if (points.length != 0) {
        //set global vars
        this.canvas = canvas;
        this.context = context;

        debug("WIDTH: " + width);
        debug("PAINT HEIGHT: " + height);

        drawLines();
        fillPath();
//        drawGrid(context);
    }
}

function drawLines()
{
    // Draw Lines
//    print("plotterPainter::paint starting to draw lines, xPos: " + xPos);

    context.beginPath();

    //HACK we start at 1.
    for(var i = 1; i < points.length; ++i){
        debug("length: " + points.length + " i has value: " + i);
        debug("x value: " + points[i][0] + " y value: " + points[i][1]);

//        context.text("POINT" , points[i][0], points[i][1]);

        if(i == 0) {
            context.moveTo(points[i][0] - graphPadding, points[i][1]);
        } else {
            context.lineTo(points[i][0] - graphPadding, points[i][1]);
        }


    }
    context.stroke();
    context.closePath();

//    context.beginPath()
    //FIXME: TEXT IS BROKEN, UPSTREAM
 //   context.stroke();
    context.closePath;
}

function fillPath()
{

    //fill path (everything below the line graph)
    context.beginPath();
    context.moveTo(graphPadding * 2, height - graphPadding)
    context.fillStyle = "rgba(255, 0, 0, .1)"

    //FIXME: because i have no fucking clue where this comes from, or why it's offset
    var offset = 5;
    for(var i = 0; i < points.length; ++i)
    {
//        console.log("FILLING POS_Y: " + pos_y + " | lineTo: x: " + (i*horizSpace) + " | lineTo: y: " + pos_y);
        //FIXME: make it use x from points
        context.lineTo(i * horizSpace - offset, points[i][1]);
        debug("points[i][1] " + points[i][1]);
    }

    context.lineTo(i * horizSpace - offset, height - graphPadding);
    context.lineTo(0, height - graphPadding);

    context.closePath();
    context.fill();

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

