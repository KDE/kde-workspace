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
    //width: isCorner ? 48 : iconSize * 2
    //height: isCorner ? 48 : iconSize * 2
    width: 128; height: 128

    y: 0
    x: main.width - toolBoxButtonFrame.width
    z: toolBox.z + 1

    property string cornerElement: "desktop-northeast"
    property bool skipNextUpdate: false
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
        Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.InOutExpo; } }
    }

    PlasmaCore.FrameSvgItem {
        id: toolBoxButtonFrame
        imagePath: "widgets/background"
        anchors.fill: parent
        property int mWidth: iconSize*3.5 + activityName.paintedWidth
        property int mHeight: (activityName.text == "") ? mWidth+2 : iconSize*3.5 + activityName.paintedWidth+2
        opacity: !isCorner ? 1 : 0.01
        enabledBorders: PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder | PlasmaCore.FrameSvg.BottomBorder;
        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                var h = iconSize*2.25;
                if (s == "top") {
                    // resize frame
                    width = toolBoxButtonFrame.mWidth;
                    height = h;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "right") {
                    // resize frame
                    width = h;
                    height = toolBoxButtonFrame.mHeight;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "bottom") {
                    // resize frame
                    width = toolBoxButtonFrame.mWidth;
                    height = h;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "left") {
                    // resize frame
                    width = h;
                    height = toolBoxButtonFrame.mHeight;
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder;
                } else {
                    width = iconSize*1.6
                    height = iconSize*1.6
                }
            }
        }
        Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.InOutExpo; } }
    }
    PlasmaComponents.Label {
        id: activityName
        opacity: (!isCorner && (toolBoxButton.state == "top" || toolBoxButton.state == "bottom"))? 1 : 0.01
        text: plasmoid.activityName
        anchors { left: toolBoxIcon.right; right: parent.right; verticalCenter: toolBoxIcon.verticalCenter; }
    }

    PlasmaComponents.Label {
        id: activityNameVertical
        horizontalAlignment: Text.AlignBottom
        x: toolBoxIcon.x
        y: toolBoxIcon.y
        opacity: (!isCorner && (toolBoxButton.state == "left" || toolBoxButton.state == "right"))? 1 : 0.01
        transform: Rotation { angle: 90 }
        text: plasmoid.activityName
        anchors {
            top: toolBoxIcon.bottom;
            left: toolBoxIcon.left;
            leftMargin: (paintedHeight+20-activityNameVertical.font.pixelSize);
            topMargin: 4
            rightMargin: -6
        }
//         Rectangle { color: "green"; opacity: 0.5; anchors.fill: parent; }
    }

    QtExtras.QIconItem {
        id: toolBoxIcon
        anchors { top: parent.top; right: parent.right; margins: 4; }
        width: iconSize
        height: iconSize
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
                toolBoxIcon.anchors.leftMargin = 0;
                toolBoxIcon.anchors.rightMargin = 0;
                toolBoxIcon.anchors.topMargin = 0;
                toolBoxIcon.anchors.bottomMargin = 0;

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
        if (skipNextUpdate) {
            skipNextUpdate = false;
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
                //print("Error: Reached invalid state in ToolBoxButton.updateState()")
            }
        }
        if (_s != ""){
            if (_s == "topright" || _s == "bottomright" || _s == "right") {
                skipNextUpdate = true;
                toolBoxButton.x = main.width - toolBoxButton.width;
            }
            if (_s == "bottomleft" || _s == "bottomright" || _s == "bottom") {
                skipNextUpdate = true;
                toolBoxButton.y = main.height - toolBoxButton.height;
            }
            toolBoxButton.state = _s;
            plasmoid.writeConfig("ToolBoxButtonState", toolBoxButton.state);
            plasmoid.writeConfig("ToolBoxButtonX", toolBoxButton.x);
            plasmoid.writeConfig("ToolBoxButtonY", toolBoxButton.y);
        }
        print("Saved coordinates for ToolBox in config: " + toolBoxButton.x + "x" +toolBoxButton.x);
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
                if (isCorner) {
                    buttonMouse.drag.axis = Drag.XAxis | Drag.YAxis;
                } else if (s == "top" || s == "bottom" ) {
                    buttonMouse.drag.axis = Drag.XAxis;
                } else {
                    buttonMouse.drag.axis = Drag.YAxis;
                }
            }
        }
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
