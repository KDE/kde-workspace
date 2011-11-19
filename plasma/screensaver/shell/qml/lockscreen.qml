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
import QtQuick 1.0
import org.kde.kscreenlocker 1.0

Item {
    id: lockScreen
    signal accepted()
    signal canceled()
    property alias notification: unlockUI.notification
    property alias capsLockOn: unlockUI.capsLockOn

    Greeter {
        id: unlockUI
        anchors.centerIn: dialog
        focus: true
        cancelEnabled: true

        Connections {
            onAccepted: lockScreen.accepted()
            onCanceled: lockScreen.canceled()
        }
    }
}
