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

    property alias icon: taskIcon.icon
    property alias label: taskName.text
    property bool focused
    signal focus

    PlasmaCore.FrameSvgItem {
        anchors.fill: parent
        imagePath: "widgets/tasks"
        prefix: focused ? "focus" : "normal"
    }

    QIconItem {
        id: taskIcon
        width: 16
        height: 16
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 10
        }
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
        anchors.fill: parent
        onClicked: parent.focus()
    }
}
