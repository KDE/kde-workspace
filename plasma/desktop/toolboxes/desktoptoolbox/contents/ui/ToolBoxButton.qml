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

    property string text: plasmoid.activityName
    property string cornerElement: "desktop-northeast"
    property bool skipNextUpdate: false
    property bool isCorner: ((state == "topleft") || (state == "topright") ||
                             (state == "bottomright") || (state == "bottomleft"))
    property bool isHorizontal: (state == "top") || (state == "bottom")

    width: (isCorner || !isHorizontal)
            ? toolBoxIcon.width
            : toolBoxIcon.width + activityName.implicitWidth + 4;
    height: (isCorner || isHorizontal)
            ? toolBoxIcon.height
            : toolBoxIcon.height + activityName.implicitWidth + 4;
    y: 0
    x: main.width - toolBoxButtonFrame.width
    //z: toolBox.z + 1

    state: "topright" // FIXME: read default value from config

    onXChanged: updateState()
    onYChanged: updateState()

    function updateState() {
        if (skipNextUpdate) {
            skipNextUpdate = false;
            return;
        }
        var _m = 2;
        var _s = "";
        var container = main;
        //print("    w: " + container.width +"x"+container.height+" : "+x+"/"+y+" tbw: " + toolBoxButton.width);
        if (x <= _m) {
            if (y <= _m) {
                _s = "topleft"
            } else if (y >= (container.height - toolBoxButton.height - _m)) {
                _s = "bottomleft"
            } else {
                _s = "left";
            }
        } else if (x >= (container.width - toolBoxButton.width - _m)) {
            if (y <= _m) {
                _s = "topright"
            } else if (y >= (container.height- toolBoxButton.height - _m)) {
                _s = "bottomright"
            } else {
                _s = "right";
            }
        } else {
            if (y <= _m) {
                _s = "top"
            } else if (y >= (container.height - toolBoxButton.height - _m)) {
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
        anchors {
            fill: parent
            margins: -16
        }
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
        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }

        opacity: !isCorner ? 1 : 0
        enabledBorders: PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder | PlasmaCore.FrameSvg.BottomBorder;

        Connections {
            target: toolBoxButton
            onStateChanged: {
                var s = toolBoxButton.state;
                var h = iconSize + toolBoxButtonFrame.borderHeight;
                if (s == "top") {
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "right") {
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.BottomBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "bottom") {
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.LeftBorder;
                } else if (s == "left") {
                    toolBoxButtonFrame.enabledBorders = PlasmaCore.FrameSvg.TopBorder | PlasmaCore.FrameSvg.RightBorder | PlasmaCore.FrameSvg.BottomBorder;
                }
            }
        }
        Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.InOutExpo; } }
    }

    PlasmaCore.IconItem {
        id: toolBoxIcon
        anchors {
            top: parent.top;
            left: parent.left;
        }
        width: iconSize
        height: iconSize
        enabled: buttonMouse.containsMouse || toolBoxItem.showing
        source: "plasma"
    }

    PlasmaComponents.Label {
        id: activityName
        visible: !isCorner
        text: toolBoxButton.text

        anchors {
            fill: parent
            leftMargin: isHorizontal ? toolBoxIcon.width + 4 : 0;
            topMargin: isHorizontal ? 0 : toolBoxIcon.height * 2 + 4;
        }
        rotation: isHorizontal ? 0 : -90;
        transformOrigin: Item.Center
    }

    MouseArea {
        id: buttonMouse

        property QtObject container: main

        anchors {
            fill: parent
            margins: -8
        }

        drag {
            target: plasmoid.immutable ? undefined : toolBoxButton
            minimumX: 0
            maximumX: container.width - toolBoxIcon.width
            minimumY: 0
            maximumY: container.height - toolBoxIcon.height
        }

        hoverEnabled: true

        onClicked: {
            print ("click state now: " + toolBoxItem.state);
            toolBoxItem.showing = !toolBoxItem.showing;
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
