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
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.graphicslayouts 4.7
import org.kde.kscreenlocker 1.0

Item {
    signal accepted()
    signal switchUserClicked()
    signal canceled()
    property alias cancelEnabled: cancelButton.visible
    property alias notification: message.text
    property bool switchUserEnabled
    property alias capsLockOn: capsLockMessage.visible
    implicitWidth: layoutItem.width
    implicitHeight: layoutItem.height

    Column {
        id: layoutItem

        PlasmaComponents.Label {
            id: message
            text: ""
            anchors.horizontalCenter: lockMessage.horizontalCenter
            font.bold: true
        }

        PlasmaComponents.Label {
            id: capsLockMessage
            text: i18n("Warning: Caps Lock on")
            anchors.horizontalCenter: lockMessage.horizontalCenter
            visible: false
            font.bold: true
        }

        PlasmaComponents.Label {
            id: lockMessage
            text: kscreenlocker_userName.empty ? i18n("The session is locked") : i18n("The session has been locked by %1", kscreenlocker_userName)
            anchors.horizontalCenter: layoutItem.horizontalCenter
        }

        Row {
            GreeterItem {
                id: greeter
                objectName: "greeter"
            // anchors.fill: parent
                Keys.onEnterPressed: verify()
                Keys.onReturnPressed: verify()
            }
            KeyboardItem {
            }
        }

        PlasmaComponents.ButtonRow {
            id: buttonRow
            exclusive: false
            spacing: theme.defaultFont.mSize.width / 2

            PlasmaComponents.Button {
                id: switchUser
                text: i18n("Switch User")
                iconSource: "fork"
                visible: switchUserEnabled
                onClicked: switchUserClicked()
            }

            PlasmaComponents.Button {
                id: unlock
                text: i18n("Unlock")
                iconSource: "object-unlocked"
                onClicked: greeter.verify()
            }

            PlasmaComponents.Button {
                id: cancelButton
                text: i18n("Cancel")
                iconSource: "dialog-cancel"
                onClicked: canceled()
                visible: false
            }

            anchors.top: layoutItem.bottom
            anchors.horizontalCenter: layoutItem.horizontalCenter
        }
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
