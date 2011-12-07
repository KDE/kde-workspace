/*
 *   Copyright 2011 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.kscreenlocker 1.0

Item {
    id: lockScreen
    state: "UNLOCK"
    signal accepted()
    signal canceled()
    property alias notification: unlockUI.notification
    property alias capsLockOn: unlockUI.capsLockOn

    PlasmaCore.FrameSvgItem {
        id: dialog
        imagePath: "translucent/dialogs/background"
        width: unlockUI.implicitWidth * 1.5 + margins.left + margins.right
        height: unlockUI.implicitHeight * 1.5 + margins.top + margins.bottom
        anchors.centerIn: parent
    }

    Greeter {
        x: - implicitWidth/2
        id: unlockUI
        focus: true
        cancelEnabled: true
        switchUserEnabled: userSessionsUI.switchUserSupported

        Connections {
            onAccepted: lockScreen.accepted()
            onCanceled: lockScreen.canceled()
            onSwitchUserClicked: {
                // TODO: load if not loaded
                lockScreen.state = "SESSION"
            }
        }
    }
    SessionSwitching {
        id: userSessionsUI
        anchors {
            fill: dialog
            leftMargin: dialog.margins.left
            rightMargin: dialog.margins.right
            topMargin: dialog.margins.top
            bottomMargin: dialog.margins.bottom
        }
        Connections {
            onCancel: lockScreen.state = "UNLOCK"
            onActivateSession: lockScreen.state = "UNLOCK"
            onStartNewSession: lockScreen.state = "UNLOCK"
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
}
