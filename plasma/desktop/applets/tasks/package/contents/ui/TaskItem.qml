/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    id: taskItem

    property string item_id
    property alias icon: taskIcon.icon
    property alias label: taskName.text
    property bool focused
    property alias showLabel: taskName.visible
    signal clicked

    PlasmaCore.FrameSvgItem {
        anchors.fill: parent
        imagePath: "widgets/tasks"
        prefix: mouseArea.containsMouse ? "hover" : (focused ? "focus" : "normal")
    }

    QIconItem {
        id: taskIcon
        width: showLabel ? 16 : Math.min(taskItem.width-20, taskItem.height-20)
        height: width
        anchors {
            verticalCenter: parent.verticalCenter
        }
        x: showLabel ? 10 : (parent.width-width)/2
    }

    Text {
        id: taskName
        anchors {
            verticalCenter: parent.verticalCenter
            left: taskIcon.right
            leftMargin: 5
            right: parent.right
            rightMargin: 10
        }
        clip: true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
    }
}
