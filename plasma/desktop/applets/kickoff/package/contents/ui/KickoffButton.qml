/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.kickoff 0.1 as Kickoff
import org.kde.qtextracomponents 0.1

PlasmaComponents.TabButton {
    id: button
    property string iconSource
    property alias text: labelElement.text
    implicitHeight: iconElement.height + labelElement.implicitHeight + iconElement.anchors.topMargin + labelElement.anchors.topMargin + labelElement.anchors.bottomMargin

    QIconItem {
        id: iconElement
        icon: QIcon(iconSource)
        width: theme.largeIconSize
        height: width
        anchors {
            topMargin: 5
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
    }
    PlasmaComponents.Label {
        id: labelElement
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        font.pointSize: theme.smallestFont.pointSize
        anchors {
            bottom: parent.bottom
            top: iconElement.bottom
            left: parent.left
            right: parent.right
            topMargin: 5
            bottomMargin: 5
        }
    }
    MouseArea {
        enabled: root.switchTabsOnHover
        anchors.fill: parent
        hoverEnabled: root.switchTabsOnHover
        onEntered: {
            clickTimer.pendingButton = button
            clickTimer.restart()
        }
        onExited: clickTimer.running = false
        onClicked: button.clicked()
    }
}
