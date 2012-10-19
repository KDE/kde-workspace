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
//import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.qtextracomponents 0.1 as QtExtras

Item {
    id: toolBoxButton
    width: iconSize
    height: iconSize

    y: 0
    x: main.width - toolBoxButton.width
    z: toolBox.z + 1

    property string cornerElement: "desktop-northeast"
    property bool isCorner: ((state == "topleft") || (state == "topright") ||
                             (state == "bottomright") || (state == "bottomleft"))
    /*
        cornerElement = "desktop-northwest";
        cornerElement = "desktop-northeast";
        cornerElement = "desktop-southeast";
        cornerElement = "desktop-southwest";
     */
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
        svg: PlasmaCore.Svg {
            imagePath: "widgets/toolbox"
        }
        elementId: cornerElement
        anchors.fill: parent

    }

    QtExtras.QIconItem {
        id: toolBoxIcon
        anchors { top: parent.top; right: parent.right; }
        width: iconSize/2
        height: iconSize/2
        icon: "plasma"
    }

    onXChanged: updateState()
    onYChanged: updateState()

    function updateState() {
        var _m = 2;
        var _s = "UNKNOWN";
        var container = main;
        //print("    w: " + container.width +"x"+container.height+" : "+x+"/"+y);
        if (x < _m) { // || container.x > (drag.maximumX - toolBoxButton.width - 12)) {
            if (y < _m) {
                _s = "topleft"
            } else if (y > (container.width - toolBoxButton.height - _m)) {
                _s = "bottomleft"
            } else {
                _s = "left";
            }
        } else if (x > (container.width - toolBoxButton.width - _m)) {
            if (y < _m) {
                _s = "topright"
            } else if (y > (container.height- toolBoxButton.height - _m)) {
                _s = "bottomright"
            } else {
                _s = "right";
            }
        } else {
            if (y < _m) {
                _s = "top"
            } else if (y > (container.height - toolBoxButton.height - _m)) {
                _s = "bottom"
            } else {
                _s = "we shouldn't end up here";
            }
        }
        //print("  aand? " + _s);
        toolBoxButton.state = _s;
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
                print(" Changing axis to " + s);
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
