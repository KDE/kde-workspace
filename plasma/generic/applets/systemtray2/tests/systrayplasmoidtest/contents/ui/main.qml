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

    width: 48
    height: 48

    //Rectangle { anchors.fill: parent; color: "orange"; opacity: 0.8; }
    property Component compactRepresentation: Component {
        PlasmaCore.IconItem {
            //anchors.fill: parent
            source: "preferences-desktop-notification"
        }
    }


    PlasmaCore.IconItem {
        anchors.fill: parent
        source: "preferences-desktop-notification"
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

