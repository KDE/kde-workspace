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
var availableSpace;
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

    debug(arguments.callee.name.toString(), "sample list: " + points);
    debug(arguments.callee.name.toString(), "requesting new paint event");
}

function debug(caller, str)
{
    print("PlotterPainter::" + caller + "() " + str);
}

function shiftLeft()
{
    debug(arguments.callee.name.toString(), "");
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
    availableSpace  = width - (graphPadding * (points.length - 1));

    debug(arguments.callee.name.toString(), "POOOINTS LENGTH: " + points.length);

    horizSpace = availableSpace / (points.length - 1);
    vertSpace  = height - (graphPadding * (points.length - 1));
    //form an array of an array
    points.push([]);
}

/**
 * Advances the plotter (shifts all points left) by 1 interval
 * Should be called every tick of the plotter sampler.
 */
function advancePlotter()
{
    debug(arguments.callee.name.toString(), "advancePlotter()");
    var yPercent = Math.floor(Math.random() * 100);
    var yPos = (vertSpace * (yPercent / 100) + (graphPadding * 2));
    addSample(yPos);


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

        debug(arguments.callee.name.toString(), "WIDTH: " + width);
        debug(arguments.callee.name.toString(), "PAINT HEIGHT: " + height);

        drawLines();
        drawDots();
        fillPath();
        drawGrid(context);
    }
}

function drawDots()
{
    // Draw Dots
    for(var i = 0; i < points.length; ++i){
        drawCircle(context, points[i][0], points[i][1], (points.length - 1), "rgb(0, 0, 255)");
    }
}

function drawLines()
{
    // Draw Lines
//    print("plotterPainter::paint starting to draw lines, xPos: " + xPos);

    context.beginPath();

    for(var i = 0; i < points.length; ++i){
        debug(arguments.callee.name.toString(), "x value: " + points[i][0] + " y value: " + points[i][1]);
        if(i == 0) {
            context.moveTo(points[i][0], points[i][1]);
        } else {
            context.lineTo(points[i][0], points[i][1]);
        }

        //FIXME: TEXT IS BROKEN, UPSTREAM
    //            context.text(tempString , xPos, points[i]);

    }
    context.stroke();
    context.closePath();
}

function fillPath()
{

    //fill path (everything below the line graph)
    context.beginPath();
    context.moveTo(graphPadding * 2, height - graphPadding)
    context.fillStyle = "rgba(255, 0, 0, 0.5)"

    for(var i = 0; i < points.length; ++i)
    {
//        console.log("FILLING POS_Y: " + pos_y + " | lineTo: x: " + (i*horizSpace) + " | lineTo: y: " + pos_y);
        //FIXME: make it use x from points
        context.lineTo(i * horizSpace + (graphPadding * 2), points[i][1]);
    }

    context.lineTo(i * horizSpace, height - graphPadding);
    context.lineTo(0, height - graphPadding);

    context.fill();

    context.closePath();
}


function drawGrid(context)
{
    debug(arguments.callee.name.toString(), "painting on width: " + width);
    debug(arguments.callee.name.toString(), "painting on height: " + height);

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

function drawCircle(context, x, y, radius, colour)
{
    context.save();
    context.fillStyle = colour;
    context.beginPath();
    context.arc(x-1, y-1, radius,0,Math.PI*2,true); // Outer circle
    context.fill();
    context.restore();
}
