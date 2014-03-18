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
import org.kde.plasma.plasmoid 2.0

Item {
    id: toolBoxButton

    property string text: main.Plasmoid.activityName
    property bool isCorner: !buttonMouse.dragging &&
                            ((state == "topleft") || (state == "topright") ||
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
        enabled: visible
    }
    Behavior on x {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible

    }
    Behavior on y {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible
    }

    width: buttonLayout.width
    height: buttonLayout.height

    //x and y default to 0, so top left would be correct
    //If the position is anything else it will updated via onXChanged during intialisation
    state: "topleft"

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

        if (!buttonMouse.pressed) {
            main.placeToolBox(toolBoxButton.state);
        }
    }

    PlasmaCore.FrameSvgItem {
        id: backgroundFrame
        imagePath: isCorner ? "widgets/translucentbackground" : "widgets/background"
        x: -margins.left
        y: -margins.top
        width: (isCorner ? buttonLayout.height : buttonLayout.width) + margins.left + margins.right
        height: buttonLayout.height + margins.top + margins.bottom
        Behavior on width {
            NumberAnimation {
                duration: units.longDuration;
                easing.type: Easing.InOutQuad;
            }
        }
    }

    Row {
        id: buttonLayout
        x: !buttonMouse.dragging && isCorner ? units.smallSpacing*2 : 0

        Behavior on x {
            NumberAnimation {
                duration: units.longDuration;
                easing.type: Easing.InOutQuad;
            }
        }

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
        property int pressedX
        property int pressedY
        property bool dragging: false

        anchors {
            fill: parent
            margins: -10
        }

        drag {
            target: main.Plasmoid.immutable ? undefined : toolBoxButton
            minimumX: 0
            maximumX: container.width - toolBoxIcon.width
            minimumY: 0
            maximumY: container.height - toolBoxIcon.height
        }

        hoverEnabled: true

        onPressed: {
            pressedX = toolBoxButton.x
            pressedY = toolBoxButton.y
        }
        onPositionChanged: {
            if (pressed && (Math.abs(toolBoxButton.x - pressedX) > iconSize ||
                Math.abs(toolBoxButton.y - pressedY) > iconSize)) {
                dragging = true;
            }
        }
        onClicked: {
            print ("click state now: " + toolBoxItem.state);
            toolBoxItem.showing = !toolBoxItem.showing;
        }
        onReleased: {
            main.Plasmoid.configuration.ToolBoxButtonState = toolBoxButton.state;
            main.Plasmoid.configuration.ToolBoxButtonX = toolBoxButton.x;
            main.Plasmoid.configuration.ToolBoxButtonY = toolBoxButton.y;
            print("Saved coordinates for ToolBox in config: " + toolBoxButton.x + ", " +toolBoxButton.x);
            if (dragging) {
                main.placeToolBox();
            }
            dragging = false;
        }
        onCanceled: dragging = false;
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
