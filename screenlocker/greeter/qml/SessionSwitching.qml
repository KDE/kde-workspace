/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    property alias model: userSessionsView.model
    property bool startNewSessionSupported
    signal activateSessionClicked(int index)
    signal startNewSessionClicked()
    signal cancel()
    ListView {
        id: userSessionsView
        width: parent.width / 2
        height: parent.height
        anchors.centerIn: parent

        delegate: PlasmaComponents.ListItem {
            content: PlasmaComponents.Label {
                text: session + "(" + location + ")"
            }
        }
        highlight: PlasmaComponents.Highlight {
            hover: true
            width: parent.width
        }
        focus: true
    }
    MouseArea {
        anchors.fill: userSessionsView
        onClicked: userSessionsView.currentIndex = userSessionsView.indexAt(mouse.x, mouse.y)
        onDoubleClicked: activateSessionClicked(userSessionsView.indexAt(mouse.x, mouse.y))
    }
    PlasmaComponents.Label {
        text: i18n("The current session will be hidden " +
                    "and a new login screen or an existing session will be displayed.\n" +
                    "An F-key is assigned to each session; " +
                    "F%1 is usually assigned to the first session, " +
                    "F%2 to the second session and so on. " +
                    "You can switch between sessions by pressing " +
                    "Ctrl, Alt and the appropriate F-key at the same time. " +
                    "Additionally, the KDE Panel and Desktop menus have " +
                    "actions for switching between sessions.",
                    7, 8)
        anchors {
            left: userSessionsView.right
            leftMargin: 10
        }
    }
    PlasmaComponents.ButtonRow {
        exclusive: false

        PlasmaComponents.Button {
            id: activateSession
            text: i18n("Activate")
            iconSource: "fork"
            onClicked: activateSessionClicked(userSessionsView.currentIndex)
        }
        PlasmaComponents.Button {
            id: newSession
            text: i18n("Start New Session")
            iconSource: "fork"
            visible: startNewSessionSupported
            onClicked: startNewSessionClicked()
        }
        PlasmaComponents.Button {
            id: cancelSession
            text: i18n("Cancel")
            iconSource: "dialog-cancel"
            onClicked: cancel()
        }
        anchors.top: userSessionsUI.bottom
        anchors.horizontalCenter: userSessionsUI.horizontalCenter
        anchors.bottomMargin: 20
    }
}
