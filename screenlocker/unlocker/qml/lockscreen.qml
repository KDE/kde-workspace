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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.screenlocker 1.0 as ScreenLocker

Rectangle {
    id: lockScreen
    width: 800
    height: 600
    color: Qt.rgba(0, 0, 0, 1.0)
    state: "UNLOCK"
    signal unlockRequested()
    signal startNewSession()
    signal activateSession(int index)
    property string userName
    property bool switchUserSupported
    property bool startNewSessionSupported
    property bool capsLockOn

    PlasmaCore.Theme {
        id: theme
    }

    function switchSession(index) {
        lockScreen.state = "UNLOCK";
        lockScreen.activateSession(index);
    }

    Image {
        id: background
        anchors.fill: parent
        source: theme.wallpaperPathForSize(parent.width, parent.height)
    }

    PlasmaCore.FrameSvgItem {
        id: dialog
        anchors.centerIn: parent
        imagePath: "translucent/dialogs/background"
        width: parent.width/2
        height: parent.height/2
    }
    Item {
        id: unlockUI
        signal accepted()

        anchors.centerIn: dialog

        ScreenLocker.UnlockerItem {
            id: unlocker
            objectName: "unlocker"
            anchors.centerIn: parent
            anchors.bottomMargin: 20
            Keys.onEnterPressed: verify()
            Keys.onReturnPressed: verify()
        }
        Text {
            id: message
            text: ""
            anchors.horizontalCenter: lockMessage.horizontalCenter
            anchors.bottom: lockMessage.top
            anchors.bottomMargin: 20
        }
        Text {
            id: capsLockMessage
            text: i18n("Warning: Caps Lock on")
            color: theme.textColor
            anchors.horizontalCenter: lockMessage.horizontalCenter
            anchors.bottom: message.top
            anchors.bottomMargin: 5
            visible: capsLockOn
        }
        Text {
            id: lockMessage
            text: userName.empty ? i18n("The session is locked") : i18n("The session has been locked by %1", userName)
            color: theme.textColor
            anchors.bottom: unlocker.top
            anchors.horizontalCenter: unlocker.horizontalCenter
            anchors.bottomMargin: 5
        }
        ScreenLocker.KeyboardItem {
            id: keyboard
            anchors.top: unlocker.bottom
            anchors.right: unlocker.right
            anchors.topMargin: 40
            anchors.bottomMargin: 20
        }
        Row {
            spacing: 5
            PlasmaWidgets.PushButton {
                id: switchUser
                text: i18n("Switch User")
                icon: QIcon("fork")
                visible: switchUserSupported
                onClicked: lockScreen.state = "SESSION"
            }
            PlasmaWidgets.PushButton {
                id: unlock
                text: i18n("Unlock")
                icon: QIcon("object-unlocked")
                onClicked: unlocker.verify()
            }
            anchors.top: unlocker.bottom
            anchors.horizontalCenter: unlocker.horizontalCenter
            anchors.topMargin: 100
        }

        Connections {
            target: unlocker
            onGreeterFailed: {
                message.text = i18n("Unlocking failed");
                unlocker.enabled = false;
                switchUser.enabled = false;
                unlock.enabled = false;
            }
            onGreeterReady: {
                message.text = "";
                unlocker.enabled = true;
                switchUser.enabled = true;
                unlock.enabled = true;
            }
            onGreeterMessage: message.text = text
            onGreeterAccepted: unlockUI.accepted()
        }
    }

    Item {
        id: userSessionsUI
        width: dialog.width * 0.8
        height: dialog.height * 0.8
        anchors.centerIn: dialog
        ListView {
            id: userSessionsView
            width: parent.width * 0.5
            height: parent.height * 0.8
            anchors.centerIn: parent
            spacing: 5

            model: sessionModel
            delegate: Text {
                text: session + "(" + location + ")"
            }
            highlight: PlasmaCore.FrameSvgItem {
                imagePath: "widgets/viewitem"
                prefix: "hover"
            }
            focus: true
        }
        MouseArea {
            anchors.fill: userSessionsView
            onClicked: userSessionsView.currentIndex = userSessionsView.indexAt(mouse.x, mouse.y)
            onDoubleClicked: switchSession(userSessionsView.indexAt(mouse.x, mouse.y))
        }
        Text {
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
        Row {
            spacing: 5
            PlasmaWidgets.PushButton {
                id: activateSession
                text: i18n("Activate")
                icon: QIcon("fork")
                onClicked: switchSession(userSessionsView.currentIndex)
            }
            PlasmaWidgets.PushButton {
                id: newSession
                text: i18n("Start New Session")
                icon: QIcon("fork")
                visible: startNewSessionSupported
                onClicked: {
                    lockScreen.state = "UNLOCK";
                    lockScreen.startNewSession();
                }
            }
            PlasmaWidgets.PushButton {
                id: cancelSession
                text: i18n("Cancel")
                icon: QIcon("dialog-cancel")
                onClicked: lockScreen.state = "UNLOCK"
            }
            anchors.top: userSessionsUI.bottom
            anchors.horizontalCenter: userSessionsUI.horizontalCenter
            anchors.bottomMargin: 20
        }
    }

    states: [
        State {
            name: "UNLOCK"
            PropertyChanges {target: unlockUI; visible: true}
            PropertyChanges {target: userSessionsUI; visible: false}
        },
        State {
            name: "SESSION"
            PropertyChanges {target: unlockUI; visible: false}
            PropertyChanges {target: userSessionsUI; visible: true}
        }
    ]

    Connections {
        target: unlockUI
        onAccepted: unlockRequested()
    }
}