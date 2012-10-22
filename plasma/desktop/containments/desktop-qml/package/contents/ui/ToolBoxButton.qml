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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtras

Item {
    id: toolBoxButton
    width: isCorner ? iconSize : iconSize
    height: isCorner ? iconSize: iconSize

    y: 0
    x: main.width - toolBoxButtonFrame.width
    z: toolBox.z + 1

    property string cornerElement: "desktop-northeast"
    property bool isCorner: ((state == "topleft") || (state == "topright") ||
                             (state == "bottomright") || (state == "bottomleft"))
    state: "topright" // FIXME: read default value from config
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
    PlasmaCore.SvgItem {
        id: cornerSvg
        svg: PlasmaCore.Svg {
            imagePath: "widgets/toolbox"
        }
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
        Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.InOutExpo; } }
    }

    PlasmaCore.FrameSvgItem {
        id: toolBoxButtonFrame
        imagePath: "widgets/background"
        anchors.fill: parent
        property int mWidth: iconSize + activityName.paintedWidth + 24
        property int mHeight: iconSize + activityName.paintedWidth + 36
        opacity: !isCorner ? 1 : 0.01
        enabledBorders: PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder | PlasmaCore.FrameSvg.BottomBorder;
        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                if (s == "top") {
                    // resize frame
                    width = iconSize + activityName.paintedWidth + 24
                    height = iconSize
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "right") {
                    // resize frame
                    width = iconSize;
                    height = toolBoxButtonFrame.mHeight;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "bottom") {
                    // resize frame
                    width = toolBoxButtonFrame.mWidth;
                    height = iconSize
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "left") {
                    // resize frame
                    width = iconSize;
                    height = toolBoxButtonFrame.mHeight;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder;
                } else {
                    width = iconSize
                    height = iconSize
                }
            }
        }
        Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.InOutExpo; } }
    }
    PlasmaComponents.Label {
        id: activityName
        opacity: (!isCorner && (toolBoxButton.state == "top" || toolBoxButton.state == "bottom"))? 1 : 0.01
        text: "Activity Name" + plasmoid.activityName
        anchors { left: toolBoxIcon.right; right: parent.right; verticalCenter: toolBoxIcon.verticalCenter; }
    }

    PlasmaComponents.Label {
        id: activityNameVertical
        horizontalAlignment: Text.AlignBottom
        x: toolBoxIcon.x
        y: toolBoxIcon.y
        opacity: (!isCorner && (toolBoxButton.state == "left" || toolBoxButton.state == "right"))? 1 : 0.01
        transform: Rotation { angle: 90 }
        text: "Activity Name" + plasmoid.activityName
        anchors {
            top: toolBoxIcon.bottom;
            left: toolBoxIcon.left;
            leftMargin: (paintedHeight+20-activityNameVertical.font.pixelSize);
            topMargin: 8;
            rightMargin: -6
        }
//         Rectangle { color: "green"; opacity: 0.5; anchors.fill: parent; }
    }

    QtExtras.QIconItem {
        id: toolBoxIcon
        anchors { top: parent.top; right: parent.right; margins: 4; }
        width: iconSize/2-6
        height: iconSize/2-6
        icon: "plasma"
        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                var t = toolBoxButton;
                var _m = 28;
                //var corner = ""
                toolBoxIcon.anchors.top = undefined;
                toolBoxIcon.anchors.left = undefined;
                toolBoxIcon.anchors.bottom = undefined;
                toolBoxIcon.anchors.right = undefined;
                //toolBoxIcon.anchors.horizontalCenter = undefined;
                //toolBoxIcon.anchors.verticalCenter = undefined;
                toolBoxIcon.anchors.leftMargin = 4;
                toolBoxIcon.anchors.rightMargin = 4;
                toolBoxIcon.anchors.topMargin = 4;
                toolBoxIcon.anchors.bottomMargin = 4;

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
                    toolBoxIcon.anchors.leftMargin = _m;
                } else if (s == "right") {
                    toolBoxIcon.anchors.top = t.top;
                    toolBoxIcon.anchors.right = t.right;
                    toolBoxIcon.anchors.topMargin = _m;
                    //toolBoxIcon.anchors.rightMargin = 0;
                } else if (s == "bottom") {
                    toolBoxIcon.anchors.bottom = t.bottom;
                    toolBoxIcon.anchors.left = t.left;
                    toolBoxIcon.anchors.leftMargin = _m;
                } else if (s == "left") {
                    toolBoxIcon.anchors.top = t.top;
                    toolBoxIcon.anchors.left = t.left;
                    toolBoxIcon.anchors.topMargin = _m;
                }
                //toolBoxButton.cornerElement = corner;
            }
        }
    }

    onXChanged: updateState()
    onYChanged: updateState()

    function updateState() {
        if (updateStateTimer.running) {
            updateStateTimer.running = false;
            return;
        }
        var _m = 2;
        var _s = ""; // will be changed
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
                print("Error: Reached invalid state in ToolBoxButton.updateState()")
            }
        }
        //print("  new state: " + _s);
        if (_s != ""){
            if (_s == "topright" || _s == "bottomright" || _s == "right") {
                updateStateTimer.tmpState = _s;
                updateStateTimer.start();
                toolBoxButton.x = main.width - toolBoxButton.width;
            }
            if (_s == "bottomleft" || _s == "bottomright" || _s == "bottom") {
                toolBoxButton.y = main.height - toolBoxButton.height;
                updateStateTimer.running = true;
            }
            toolBoxButton.state = _s;
        }
    }

    Timer {
        id: updateStateTimer
        interval: 1
        property string tmpState: ""
        running: false
        repeat: false
        onTriggered: {
            if (tmpState != "") {
                //print("Updating state to: " + tmpState);
                //toolBoxButton.state = tmpState;
                if (toolBoxButton.state == "topright" || toolBoxButton.state == "bottomright" || toolBoxButton.state == "right") {
                    //print("Moving to " + (main.width - toolBoxButton.width));
                    //toolBoxButton.x = main.width - toolBoxButton.width;
                }

            }

        }

    }

    MouseArea {
        id: buttonMouse
        property QtObject container: main
        anchors.fill: parent

        drag.target: toolBoxButton
        Connections {
            target: toolBoxButton
            onStateChanged: {
                s = toolBoxButton.state;
                //print(" Changing axis to " + s);
                if (isCorner) {
                    buttonMouse.drag.axis = Drag.XAxis | Drag.YAxis;
                } else if (s == "top" || s == "bottom" ) {
                    buttonMouse.drag.axis = Drag.XAxis;
                } else {
                    buttonMouse.drag.axis = Drag.YAxis;
                }
            }
        }
//         drag.axis: {
//             if (container.x < _m || container.x > (drag.maximumX - toolBoxButton.width - 12)) {
//                 print("both axes");
//                 return Drag.XAxis | Drag.YAxis;
//             } else if (false) {
//                 print("both axes");
//                 return Drag.XAxis;
//             } else {
//                 print("both axes");
//                 return Drag.YAxis;
//             }
//         }
        drag.minimumX: 0
        drag.maximumX: container.width - toolBoxButton.width
        drag.minimumY: 0
        drag.maximumY: container.height - toolBoxButton.height

        onClicked: {
            var qmlFile = (toolBox.state == "expanded") ? "ToolBoxDisappearAnimation.qml" : "ToolBoxAppearAnimation.qml";
            var component = Qt.createComponent(qmlFile);
            if (component.status == Component.Ready) {
                var ani = component.createObject(buttonMouse);
                ani.targetItem = toolBox;
                ani.start();
            }
        }
    }
}
