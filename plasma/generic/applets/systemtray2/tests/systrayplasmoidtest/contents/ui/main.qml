/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: taskRoot
    objectName: "systemtrayplasmoid"

//     width: 48
//     height: 48
    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property bool fillWidth: !vertical
    property bool fillHeight: vertical

    property int preferredWidth: 100
    property int preferredHeight: 22

    property int implicitWidth: 100
    property int implicitHeight: 256

    property int minimumWidth: 100
    property int minimumHeight: 100

    //Rectangle { anchors.fill: parent; color: "orange"; opacity: 0.8; }
    property Component compactRepresentation: Component {
        Rectangle {
            property bool fillWidth: !vertical
            property bool fillHeight: vertical

//             property int preferredWidth: 100
//             property int preferredHeight: 22

//             property int implicitWidth: 100
//             property int implicitHeight: 256
            //anchors.fill: parent;
            border.width: 2;
            border.color: "black";
            color: "green";
            opacity: 0.4;

            PlasmaCore.IconItem {
                //anchors.fill: parent
                source: "plasma"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: plasmoid.expanded = !plasmoid.expanded
            }
        }
    }


    Rectangle {
        anchors.fill: parent;
        border.width: 2;
        border.color: "black";
        color: "blue";
        opacity: 0.4;

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: "konqueror"
        }
    }

    Component.onCompleted: {
        print("Loaded plasmoid's main.qml.");
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            print(taskRoot.objectName + " has been clicked on.");
            print(" Is there a plasmoid? " + plasmoid != undefined);
            print(" Plasmoid ID: " + plasmoid.id);
        }
    }
}

