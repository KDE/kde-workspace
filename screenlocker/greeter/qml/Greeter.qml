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
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    signal accepted()
    signal switchUserClicked()
    signal canceled()
    property alias cancelEnabled: cancelButton.visible
    property bool switchUserEnabled

    GreeterItem {
        id: greeter
        objectName: "greeter"
        anchors.centerIn: parent
        anchors.bottomMargin: 20
        Keys.onEnterPressed: verify()
        Keys.onReturnPressed: verify()
    }
    PlasmaComponents.Label {
        id: message
        text: ""
        anchors.horizontalCenter: lockMessage.horizontalCenter
        anchors.bottom: lockMessage.top
        anchors.bottomMargin: 20
    }
    PlasmaComponents.Label {
        id: capsLockMessage
        text: i18n("Warning: Caps Lock on")
        anchors.horizontalCenter: lockMessage.horizontalCenter
        anchors.bottom: message.top
        anchors.bottomMargin: 5
        visible: capsLockOn
    }
    PlasmaComponents.Label {
        id: lockMessage
        text: kscreenlocker_userName.empty ? i18n("The session is locked") : i18n("The session has been locked by %1", kscreenlocker_userName)
        anchors.bottom: greeter.top
        anchors.horizontalCenter: greeter.horizontalCenter
        anchors.bottomMargin: 5
    }
    KeyboardItem {
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
            visible: switchUserEnabled
            onClicked: switchUserClicked()
        }
        PlasmaWidgets.PushButton {
            id: unlock
            text: i18n("Unlock")
            icon: QIcon("object-unlocked")
            onClicked: greeter.verify()
        }
        PlasmaWidgets.PushButton {
            id: cancelButton
            text: i18n("Cancel")
            icon: QIcon("dialog-cancel")
            onClicked: canceled()
            visible: false
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
        onGreeterAccepted: accepted()
    }
}