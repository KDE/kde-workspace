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
import org.kde.qtextracomponents 0.1
import org.kde.kscreenlocker 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: lockScreen
    signal unlockRequested()
    property alias capsLockOn: unlockUI.capsLockOn

    PlasmaCore.Theme {
        id: theme
    }

    Image {
        id: background
        anchors.fill: parent
        source: theme.wallpaperPathForSize(parent.width, parent.height)
    }

    PlasmaCore.FrameSvgItem {
        id: dialog
        anchors.centerIn: parent
        imagePath: "widgets/background"
        width: mainStack.currentPage.implicitWidth + margins.left + margins.right
        height: mainStack.currentPage.implicitHeight + margins.top + margins.bottom

        Behavior on height {
            NumberAnimation {
                duration: 250
            }
        }
        Behavior on width {
            NumberAnimation {
                duration: 250
            }
        }
        PlasmaComponents.PageStack {
            id: mainStack
            clip: true
            anchors {
                fill: parent
                leftMargin: dialog.margins.left
                topMargin: dialog.margins.top
                rightMargin: dialog.margins.right
                bottomMargin: dialog.margins.bottom
            }
            initialPage: unlockUI
        }
    }

    Greeter {
        id: unlockUI

        switchUserEnabled: userSessionsUI.switchUserSupported

        Connections {
            onAccepted: lockScreen.unlockRequested()
            onSwitchUserClicked: mainStack.push(userSessionsUI)
        }
    }


    // TODO: loader
    SessionSwitching {
        id: userSessionsUI
        visible: false

        Connections {
            onCancel: mainStack.pop()
            onActivateSession: mainStack.pop()
            onStartNewSession: mainStack.pop()
        }
    }
}
