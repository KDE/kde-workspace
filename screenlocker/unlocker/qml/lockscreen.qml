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

        ScreenLocker.GreeterItem {
            id: greeter
            objectName: "greeter"
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
            anchors.bottom: greeter.top
            anchors.horizontalCenter: greeter.horizontalCenter
            anchors.bottomMargin: 5
        }
        ScreenLocker.KeyboardItem {
            id: keyboard
            anchors.top: greeter.bottom
            anchors.right: greeter.right
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
                onClicked: greeter.verify()
            }
            anchors.top: greeter.bottom
            anchors.horizontalCenter: greeter.horizontalCenter
            anchors.topMargin: 100
        }

        Connections {
            target: greeter
            onGreeterFailed: {
                message.text = i18n("Unlocking failed");
                greeter.enabled = false;
                switchUser.enabled = false;
                unlock.enabled = false;
            }
            onGreeterReady: {
                message.text = "";
                greeter.enabled = true;
                switchUser.enabled = true;
                unlock.enabled = true;
            }
            onGreeterMessage: message.text = text
            onGreeterAccepted: unlockUI.accepted()
        }
    }

    // TODO: loader
    SessionSwitching {
        id: userSessionsUI
        model: sessionModel
        startNewSessionSupported: lockScreen.startNewSessionSupported
        width: dialog.width * 0.8
        height: dialog.height * 0.8
        anchors.centerIn: dialog
        Connections {
            onCancel: lockScreen.state = "UNLOCK"
            onStartNewSessionClicked: lockScreen.startNewSession()
            onActivateSessionClicked: {
                lockScreen.activateSession(index)
                lockScreen.state = "UNLOCK"
            }
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