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
    property bool isCorner: ((state == "topleft") || (state == "topright") ||
                             (state == "bottomright") || (state == "bottomleft"))
    property bool isHorizontal: (state != "left" && state != "right")

    rotation: isHorizontal ? 0 : -90;

    transform: Translate {
        x: state == "left" ? -height : state == "right" ? height : 0
        Behavior on x {
            NumberAnimation {
                duration: units.shortDuration * 3;
                easing.type: Easing.InOutExpo;
            }
        }
    }
    transformOrigin: Item.Center
    Behavior on rotation {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
    }
    Behavior on x {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
    }
    Behavior on y {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
    }

    width: isCorner ? toolBoxIcon.width : buttonLayout.width
    height: buttonLayout.height
    y: 0
    x: main.width - width

    state: ""

    onXChanged: updateState()
    onYChanged: updateState()

    function updateState() {
        var container = main;
        //print("    w: " + container.width +"x"+container.height+" : "+x+"/"+y+" tbw: " + toolBoxButton.width);

        var x = toolBoxButton.x;
        var y = toolBoxButton.y;

        var cornerSnap = toolBoxIcon.width;

        if (x < cornerSnap && y < cornerSnap) {
            toolBoxButton.state = "topleft";
        } else if (container.width - x - buttonLayout.width < cornerSnap && y < cornerSnap) {
            toolBoxButton.state = "topright";
        } else if (container.width - x - buttonLayout.width < cornerSnap && container.height - y - buttonLayout.height  < cornerSnap) {
            toolBoxButton.state = "bottomright";
        } else if (x < cornerSnap && container.height - y - buttonLayout.height < cornerSnap) {
            toolBoxButton.state = "bottomleft";
        //top diagonal half
        } else if (x > y) {
            //Top edge
            if (container.width - x > y ) {
                toolBoxButton.state = "top";
            //right edge
            } else {
                //toolBoxButton.transformOrigin = Item.BottomRight
                toolBoxButton.state = "right";
            }
        //bottom diagonal half
        } else {
            //Left edge
            if (container.height - y > x ) {
                //toolBoxButton.transformOrigin = Item.TopLeft
                toolBoxButton.state = "left";
            //Bottom edge
            } else {
                toolBoxButton.state = "bottom";
            }
        }
    }

    PlasmaCore.SvgItem {
        id: cornerSvg
        svg: toolBoxSvg
        elementId: cornerElement
        x: -width/2 + toolBoxIcon.width/2;
        y: x;
        width: toolBoxIcon.width * 5;
        height: width
        visible: isCorner
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
        Behavior on opacity { NumberAnimation { duration: units.shortDuration * 3; easing.type: Easing.InOutExpo; } }
    }

    PlasmaCore.FrameSvgItem {
        imagePath: "widgets/toolbox"
        anchors {
            fill: buttonLayout
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }

        visible: !isCorner
    }

    Row {
        id: buttonLayout
        PlasmaCore.IconItem {
            id: toolBoxIcon
            anchors.verticalCenter: parent.verticalCenter
            width: iconSize
            height: iconSize
            enabled: buttonMouse.containsMouse || toolBoxItem.showing
            source: "plasma"
            rotation: isHorizontal ? 0 : 90;
            transformOrigin: Item.Center
        }

        PlasmaComponents.Label {
            id: activityName
            opacity: isCorner ? 0 : 1
            text: toolBoxButton.text
        }
    }

    MouseArea {
        id: buttonMouse

        property QtObject container: main

        anchors {
            fill: parent
            margins: -10
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
        onReleased: {
            plasmoid.writeConfig("ToolBoxButtonState", toolBoxButton.state);
            plasmoid.writeConfig("ToolBoxButtonX", toolBoxButton.x);
            plasmoid.writeConfig("ToolBoxButtonY", toolBoxButton.y);
            print("Saved coordinates for ToolBox in config: " + toolBoxButton.x + ", " +toolBoxButton.x);
            main.placeToolBox();
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
