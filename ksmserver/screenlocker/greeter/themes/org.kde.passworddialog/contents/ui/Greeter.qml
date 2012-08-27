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
    property bool capsLockOn
    implicitWidth: layoutItem.width + theme.defaultFont.mSize.width * 4 + 12
    implicitHeight: layoutItem.height + 12

    anchors {
        fill: parent
        margins: 6
    }

    Column {
        id: layoutItem
        anchors.centerIn: parent
        spacing: theme.defaultFont.mSize.height/2


        PlasmaComponents.Label {
            id: message
            text: ""
            anchors.horizontalCenter: parent.horizontalCenter
            font.bold: true
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }
            opacity: text == "" ? 0 : 1
        }



        PlasmaComponents.Label {
            id: capsLockMessage
            text: i18n("Warning: Caps Lock on")
            anchors.horizontalCenter: parent.horizontalCenter
            opacity: capsLockOn ? 1 : 0
            height: capsLockOn ? paintedHeight : 0
            font.bold: true
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }
        }


        PlasmaComponents.Label {
            id: lockMessage
            text: kscreenlocker_userName.empty ? i18n("The session is locked") : i18n("The session has been locked by %1", kscreenlocker_userName)
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Item {
            width: greeter.width
            height: greeter.height
            anchors.horizontalCenter: parent.horizontalCenter
            GreeterItem {
                id: greeter
                objectName: "greeter"

                Keys.onEnterPressed: verify()
                Keys.onReturnPressed: verify()
            }
            KeyboardItem {
                anchors.left: greeter.right
            }
            Timer {
                interval: 10
                running: true
                onTriggered: {
                    greeter.forceActiveFocus()
                    greeter.focus = true
                }
            }
        }

        PlasmaComponents.ButtonRow {
            id: buttonRow
            exclusive: false
            spacing: theme.defaultFont.mSize.width / 2
            anchors.horizontalCenter: parent.horizontalCenter

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
