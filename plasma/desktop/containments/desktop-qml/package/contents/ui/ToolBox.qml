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
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtras

Item {
    id: toolBox
    width: iconSize
    height: iconSize
    state: "collapsed"

    property int expandedWidth: 200
    property int expandedHeight: 320
    property int iconSize: 32

    states: [
        State {
            name: "collapsed"
            PropertyChanges {
                target: toolBox
                width: iconSize
                height: iconSize
            }
            PropertyChanges { target: toolBoxFrame; opacity: 0.0; }
        },
        State {
            name: "expanded"
            PropertyChanges {
                target: toolBox
                width: expandedWidth
                height: expandedHeight
            }
            PropertyChanges { target: toolBoxFrame; opacity: 1.0; }
        }
    ]

    Behavior on width { NumberAnimation { duration: 350; easing.type: Easing.OutExpo; } }
    Behavior on height { NumberAnimation { duration: 350; easing.type: Easing.OutExpo; } }

    PlasmaCore.FrameSvgItem {
        id: toolBoxFrame
        imagePath: "widgets/background"
        anchors.fill: parent;
        anchors.margins: -4
        Behavior on opacity { NumberAnimation { duration: 250; easing.type: Easing.OutExpo; } }
        Column {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 12
            ActionButton {
                svg: iconsSvg
                elementId: "add"
                action: plasmoid.action("add widgets")
                //FIXME: WHY?
                Component.onCompleted: {
                    action.enabled = true
                }
            }

            ActionButton {
                id: configureButton
                svg: iconsSvg
                elementId: "configure"
                action: plasmoid.action("configure")
                //FIXME: WHY?
                Component.onCompleted: {
                    action.enabled = true
                }
            }
        }
    }

    QtExtras.QIconItem {
        id: toolBoxButton
        //text: "Add Applet"
        width: iconSize
        height: iconSize
        icon: "plasma"
        anchors { top: parent.top; right: parent.right; }
        //anchors { fill: parent }
        MouseArea {
            anchors.fill: parent
            onClicked: {

                toolBox.state = (toolBox.state == "expanded") ? "collapsed" : "expanded";
            }
        }
//         onClicked: {
//             var ap = "digital-clock";
//             print("Adding applet..." + ap);
//         }
    }
}
