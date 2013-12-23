/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0 as QtExtras

Item {
    id: toolBoxButton

    property QtObject proxy: plasmoid.toolBox
    property string text: plasmoid.activityName == "" ? "Activity Name That Is 37 Miles Long" : plasmoid.activityName
    property string cornerElement: "desktop-northeast"
    property bool skipNextUpdate: false
    property bool isCorner: ((state == "topleft") || (state == "topright") ||
                             (state == "bottomright") || (state == "bottomleft"))
    property bool isHorizontal: (state == "top") || (state == "bottom")

    width: 128; height: 128
    y: 0
    x: main.width - toolBoxButtonFrame.width
    //z: toolBox.z + 1

    state: "topright" // FIXME: read default value from config

    onXChanged: updateState()
    onYChanged: updateState()

    function updateState()
    {
        if (skipNextUpdate) {
            skipNextUpdate = false;
            return;
        }
        var _m = 2;
        var _s = "";
        var container = main;
        //print("    w: " + container.width +"x"+container.height+" : "+x+"/"+y+" tbw: " + toolBoxButtonFrame.mWidth);
        if (x <= _m) {
            if (y <= _m) {
                _s = "topleft"
            } else if (y >= (container.height - toolBoxButtonFrame.mHeight - _m)) {
                _s = "bottomleft"
            } else {
                _s = "left";
            }
        } else if (x >= (container.width - toolBoxButtonFrame.mWidth - _m)) {
            if (y <= _m) {
                _s = "topright"
            } else if (y >= (container.height- toolBoxButtonFrame.mHeight - _m)) {
                _s = "bottomright"
            } else {
                _s = "right";
            }
        } else {
            if (y <= _m) {
                _s = "top"
            } else if (y >= (container.height - toolBoxButtonFrame.mHeight - _m)) {
                _s = "bottom"
            } else {
                //print("Error: Reached invalid state in ToolBoxButton.updateState()")
            }
        }
        if (_s != "") {
            if (_s == "topright" || _s == "bottomright" || _s == "right") {
                skipNextUpdate = true;
                toolBoxButton.x = main.width - toolBoxButton.width;
            }
            if (_s == "bottomleft" || _s == "bottomright" || _s == "bottom") {
                skipNextUpdate = true;
                toolBoxButton.y = main.height - toolBoxButton.height;
            }
            toolBoxButton.state = _s;
            configSaveTimer.running = true;
        }
    }

    Timer {
        id: configSaveTimer
        interval: 5000
        running: false
        onTriggered: {
            plasmoid.writeConfig("ToolBoxButtonState", toolBoxButton.state);
            plasmoid.writeConfig("ToolBoxButtonX", toolBoxButton.x);
            plasmoid.writeConfig("ToolBoxButtonY", toolBoxButton.y);
            print("Saved coordinates for ToolBox in config: " + toolBoxButton.x + "x" +toolBoxButton.x);
        }
    }

    PlasmaCore.SvgItem {
        id: cornerSvg
        svg: toolBoxSvg
        elementId: cornerElement
        anchors.fill: parent
        //anchors.margins: 8
        opacity: isCorner ? 1 : 0
        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                var corner = ""
                if (s == "topleft") {
                    corner = "desktop-northwest";
                } else if (s == "topright") {
                    corner = "desktop-northeast";
                } else if (s == "bottomright") {
                    corner = "desktop-southeast";
                } else if (s == "bottomleft") {
                    corner = "desktop-southwest";
                }
                toolBoxButton.cornerElement = corner;
            }
        }
        Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.InOutExpo; } }
    }

    PlasmaCore.FrameSvgItem {
        id: toolBoxButtonFrame
        imagePath: "widgets/toolbox"
        anchors.fill: parent

        property int borderWidth: toolBoxButton.isHorizontal ? toolBoxSvg.elementSize("left").width + toolBoxSvg.elementSize("right").width : toolBoxSvg.elementSize("left").width
        property int borderHeight: !toolBoxButton.isHorizontal ? toolBoxSvg.elementSize("top").height + toolBoxSvg.elementSize("bottom").height: toolBoxSvg.elementSize("bottom").height

        property int mWidth: iconSize + borderWidth + activityName.paintedWidth
        property int mHeight: (activityName.text == "") ? mWidth+2 : iconSize + borderHeight + activityName.paintedWidth+2

        opacity: !isCorner ? 1 : 0.01
        enabledBorders: PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder | PlasmaCore.FrameSvg.BottomBorder;

        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                var h = iconSize + toolBoxButtonFrame.borderHeight;
                if (s == "top") {
                    // resize frame
                    width = toolBoxButtonFrame.mWidth;
                    height = h;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "right") {
                    // resize frame
                    width = iconSize + toolBoxSvg.leftBorder;
                    height = toolBoxButtonFrame.mHeight;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "bottom") {
                    // resize frame
                    width = toolBoxButtonFrame.mWidth;
                    height = h;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "left") {
                    // resize frame
                    width = iconSize + toolBoxSvg.rightBorder;
                    height = toolBoxButtonFrame.mHeight;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder;
                } else {
                    width = iconSize+toolBoxSvg.leftBorder
                    height = iconSize + toolBoxSvg.topBorder
                }
            }
        }
        Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.InOutExpo; } }

        Component.onCompleted: {
            print(" borders are: " + borderWidth+" and " + borderHeight + " " + toolBoxSvg.elementSize("top").height);
            print(" mSize are: " + mWidth+" and " + mHeight + " " + iconSize);
        }
    }
    PlasmaComponents.Label {
        id: activityName
        opacity: (!isCorner && (toolBoxButton.state == "top" || toolBoxButton.state == "bottom"))? 1 : 0.01
        text: toolBoxButton.text
        anchors { left: toolBoxIcon.right; right: parent.right; verticalCenter: toolBoxIcon.verticalCenter; leftMargin: 4; }
    }

    PlasmaComponents.Label {
        id: activityNameVertical
        horizontalAlignment: Text.AlignBottom
        x: toolBoxIcon.x
        y: toolBoxIcon.y
        opacity: (!isCorner && (toolBoxButton.state == "left" || toolBoxButton.state == "right"))? 1 : 0.01
        transform: Rotation { angle: 90 }
        text: toolBoxButton.text
        anchors {
            top: toolBoxIcon.bottom;
            left: toolBoxIcon.left;
            leftMargin: (paintedHeight+iconSize-activityNameVertical.font.pixelSize);
            topMargin: 4
            rightMargin: -6
        }
    }

    PlasmaCore.IconItem {
        id: toolBoxIcon
        anchors { top: parent.top; right: parent.right; margins: 4; topMargin: 12; }
        width: iconSize
        height: iconSize
        enabled: buttonMouse.containsMouse || toolBoxItem.showing
        source: "plasma"
        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                var t = toolBoxButton;
                var _lm = toolBoxSvg.leftBorder;
                var _tm = toolBoxSvg.topBorder;

                toolBoxIcon.anchors.top = undefined;
                toolBoxIcon.anchors.left = undefined;
                toolBoxIcon.anchors.bottom = undefined;
                toolBoxIcon.anchors.right = undefined;
                //toolBoxIcon.anchors.horizontalCenter = undefined;
                //toolBoxIcon.anchors.verticalCenter = undefined;
                var _m = 2;
                toolBoxIcon.anchors.leftMargin = _m;
                toolBoxIcon.anchors.rightMargin = _m;
                toolBoxIcon.anchors.topMargin = _m;
                toolBoxIcon.anchors.bottomMargin = _m;

                if (s == "topleft") {
                    toolBoxIcon.anchors.top = t.top;
                    toolBoxIcon.anchors.left = t.left;
                } else if (s == "topright") {
                    toolBoxIcon.anchors.top = t.top;
                    toolBoxIcon.anchors.right = t.right;
                } else if (s == "bottomright") {
                    toolBoxIcon.anchors.bottom = t.bottom;
                    toolBoxIcon.anchors.right = t.right;
                } else if (s == "bottomleft") {
                    toolBoxIcon.anchors.bottom = t.bottom;
                    toolBoxIcon.anchors.left = t.left;
                } else if (s == "top") {
                    toolBoxIcon.anchors.top = t.top;
                    toolBoxIcon.anchors.left = t.left;
                    toolBoxIcon.anchors.leftMargin = _lm;
                } else if (s == "right") {
                    toolBoxIcon.anchors.top = t.top;
                    toolBoxIcon.anchors.right = t.right;
                    toolBoxIcon.anchors.topMargin = _tm;
                } else if (s == "bottom") {
                    toolBoxIcon.anchors.bottom = t.bottom;
                    toolBoxIcon.anchors.left = t.left;
                    toolBoxIcon.anchors.leftMargin = _lm;
                } else if (s == "left") {
                    toolBoxIcon.anchors.top = t.top;
                    toolBoxIcon.anchors.left = t.left;
                    toolBoxIcon.anchors.topMargin = _tm;
                }
            }
        }
    }

    MouseArea {
        id: buttonMouse

        property QtObject container: main

        anchors.fill: parent

        drag.target: plasmoid.immutable ? undefined : toolBoxButton
        drag.minimumX: 0
        drag.maximumX: container.width - toolBoxButton.width
        drag.minimumY: 0
        drag.maximumY: container.height - toolBoxButton.height

        hoverEnabled: true

        onClicked: {
            print ("click state now: " + toolBoxItem.state);
            toolBoxItem.showing = !toolBoxItem.showing;
            //proxy.showing  = toolBoxItem.state != "expanded";
        }
        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                if (isCorner) {
                    buttonMouse.drag.axis = Drag.XAxis | Drag.YAxis;
                } else if (s == "top" || s == "bottom" ) {
                    buttonMouse.drag.axis = Drag.XAxis;
                } else {
                    buttonMouse.drag.axis = Drag.YAxis;
                }
            }
        }
        Connections {
            target: plasmoid
            onImmutableChanged: {
                buttonMouse.drag.target = plasmoid.immutable ? undefined : toolBoxButton
            }
        }
    }
    states: [
        State {
            name: "topleft"
        },
        State {
            name: "top"
        },
        State {
            name: "topright"
        },
        State {
            name: "right"
        },
        State {
            name: "bottomright"
        },
        State {
            name: "bottom"
        },
        State {
            name: "bottomleft"
        },
        State {
            name: "topleft"
        },
        State {
            name: "left"
        }
    ]
}
