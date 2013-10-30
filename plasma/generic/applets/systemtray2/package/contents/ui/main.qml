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
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.private.systemtray 2.0 as SystemTray

Item {
    id: root
    objectName: "SystemTrayRootItem"
//     width: 256
//     height: 32
//     anchors {
//         left: parent.left
//         right: parent.right
//         verticalCenter: parent.verticalCenter
//     }

//     width: 356
//     height: 356

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property bool fillWidth: !vertical
    property bool fillHeight: vertical

//
    property int preferredWidth: 100
    property int preferredHeight: plasmoid.configuration.itemSize

    property int implicitWidth: 100
    property int implicitHeight: 256

    property int minimumWidth: 1000
    property int minimumHeight: 1000

    property int _h: plasmoid.configuration.itemSize
    property int itemSpacing: 2

    property Component compactRepresentation: CompactRepresentation {
        systrayhost: host
    }

    //Rectangle { color: "blue"; width: 200; height: 48; }
    SystemTray.Host {
        id: host
        rootItem: hiddenView
    }
    ListView {
        id: hiddenView
        objectName: "hiddenView"

        anchors {
            fill: parent
        }
//         cellWidth: _h + itemSpacing
//         cellHeight: _h + itemSpacing
        //orientation: Qt.Horizontal
        //interactive: false
        spacing: 4
        //Rectangle { anchors.fill: parent; color: "blue"; opacity: 0.2; }

        model: host.hiddenTasks

        delegate: TaskDelegate {}
    }

}