/***************************************************************************
 *   Copyright 2011 Davide Bettio <davide.bettio@kdemail.net>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.mobilecomponents 0.1 as PlasmaComponents

Item {
    id: notificationsApplet
    state: "default"
    width: 32
    height: 32

    Component.onCompleted: {
        //plasmoid.popupIcon = QIcon("preferences-desktop-notification")
    }

    states: [
        State {
            name: "default"
            PropertyChanges {
                target: notificationSvgItem
                elementId: "notification-disabled"
            }
            PropertyChanges {
                target: countText
                visible: false
            }
        },
        State {
            name: "new-notifications"
            PropertyChanges {
                target: notificationSvgItem
                elementId: "notification-empty"
            }
            PropertyChanges {
                target: countText
                visible: true
            }
        }
    ]

    PlasmaCore.Svg {
        id: notificationSvg
        imagePath: "icons/notification"
    }

    PlasmaCore.SvgItem {
        id: notificationSvgItem
        svg: notificationSvg
        elementId: "notification-disabled"
        anchors.fill: parent
        Text {
            id: countText
            text: notificationsList.count
            anchors.centerIn: parent
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (popup.visible) {
                    popup.visible = false
                } else {
                    var pos = popup.popupPosition(notificationsApplet, Qt.AlignCenter)
                    popup.x = pos.x
                    popup.y = pos.y
                    popup.visible = true
                }
            }
        }
    }


    PlasmaCore.DataModel {
        id: notificationsModel
        dataSource: notificationsSource
    }

    PlasmaCore.DataSource {
        id: notificationsSource
        engine: "notifications"
        interval: 0

        onSourceAdded: {
            connectSource(source);
        }

        onConnectedSourcesChanged: {
            if (connectedSources.length > 0) {
                notificationsApplet.state = "new-notifications"
            } else {
                notificationsApplet.state = "default"
            }
        }
    }

    PlasmaCore.Dialog {
        id: popup
        mainItem: ListView {
            id: notificationsList
            width: 400
            height: 250
            model: notificationsModel
            anchors.fill: parent
            clip: true
            delegate: ListItem {
                Row {
                    spacing: 6
                    PlasmaWidgets.IconWidget {
                        icon: QIcon(appIcon)
                    }
                    Column {
                        spacing: 3
                        Text {
                            text: appName + ": " + body
                        }
                        Text {
                            text: body
                        }
                    }
                }
            }
        }
    }
}
