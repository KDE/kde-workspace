//.pragma library

var count = 6;

var graphPadding = 20;
var availableSpace = width - (graphPadding * count);
var horizSpace = availableSpace / count;
var vertSpace = height - (graphPadding * count);

function paint(canvas, context)
{
   // var points = new List();
    // Create an array of random y points
            var points = new Array();
    for(var i = 0;i < count;i++){
        // Get Ypos
        var yPercent  =  Math.floor(Math.random()*100)
        var yPos = (vertSpace * (yPercent / 100) + (graphPadding * 2));
        points.push(yPos);
    }

    // Draw Lines
    var xPos = graphPadding * 2;
    context.beginPath();
    for(i = 0;i < count;i++){
        if(i == 0) {
            context.moveTo(xPos, points[i]);
        } else {
            context.lineTo(xPos, points[i]);
        }

        //FIXME: TEXT IS BROKEN, UPSTREAM
    //            context.text(tempString , xPos, points[i]);

        xPos += horizSpace;
    }
    context.stroke();
    context.closePath();

    // Draw Dots
    var xPos = graphPadding * 2;
    for(i = 0;i < count;i++){
        drawCircle(context, xPos, points[i], count, "rgb(0, 0, 255)");
        xPos += horizSpace;
    }

    //fill path (everything below the line graph)
    context.beginPath();
    context.moveTo(graphPadding * 2, height)
    context.fillStyle = "rgba(255, 0, 0, 0.5)"
    for(var i = 0; i < points.length; i++)
    {
        var pos_y = points[i];
        console.log("FILLING POS_Y: " + pos_y + " | lineTo: x: " + (i*horizSpace) + " | lineTo: y: " + pos_y);

        context.lineTo(i * horizSpace + (graphPadding * 2), pos_y);
    }

    context.lineTo(i * horizSpace, height);
    context.lineTo(0, height);

    context.fill();

    context.closePath();

    drawGrid(context);
}

        
        function drawGrid(context) {
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
        
        function drawCircle(context, x, y, radius, colour){
            context.save();
            context.fillStyle = colour;
            context.beginPath();
            context.arc(x-1, y-1, radius,0,Math.PI*2,true); // Outer circle
            context.fill();
            context.restore();
        }
        