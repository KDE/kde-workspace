/***************************************************************************
 *   Copyright 2011 Davide Bettio <davide.bettio@kdemail.net>              *
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1
import org.kde.plasma.extras 0.1 as PlasmaExtras


PlasmaCore.Dialog {
    id: lastNotificationPopup

    function popup()
    {

        notificationsView.positionViewAtBeginning()

        var pos = lastNotificationPopup.popupPosition(notificationIcon, Qt.AlignCenter)
        lastNotificationPopup.x = pos.x
        lastNotificationPopup.y = pos.y
        lastNotificationPopup.visible = true
        lastNotificationTimer.interval = Math.max(4000, Math.min(60*1000, notificationsModel.get(0).expireTimeout))
        lastNotificationTimer.restart()
        notificationsView.currentIndex = 0
    }

    location: plasmoid.location
    windowFlags: Qt.WindowStaysOnTopHint
    Component.onCompleted: {
        setAttribute(Qt.WA_X11NetWmWindowTypeDock, true)
    }

    mainItem: Item {
        id: mainItem
        width: theme.defaultFont.mSize.width * 30
        height: theme.defaultFont.mSize.width * 10

        Timer {
            id: lastNotificationTimer
            interval: 4000
            repeat: false
            running: false
            onTriggered: lastNotificationPopup.visible = false
        }

        ListView {
            id: notificationsView
            snapMode: ListView.SnapOneItem
            anchors.fill: parent
            model: notificationsModel
            interactive: false
            delegate: MouseEventListener {
                width: mainItem.width
                height: mainItem.height
                property int startX: 0
                property int startY: 0
                property int startScreenX: 0
                property int startScreenY: 0
                onPressed: {
                    startX = mouse.x + lastNotificationPopup.margins.left
                    startY = mouse.y + lastNotificationPopup.margins.top
                    startScreenX = mouse.screenX
                    startScreenY = mouse.screenY
                    lastNotificationTimer.running = false
                }
                onReleased: {
                    //FIXME: bind startdragdistance
                    if (Math.sqrt(Math.pow(startScreenX - mouse.screenX, 2) + Math.pow(startScreenY - mouse.screenY, 2)) > 4) {
                        lastNotificationTimer.restart()
                    } else {
                        lastNotificationPopup.visible = false
                        lastNotificationTimer.running = false
                    }
                }
                onPositionChanged: {
                    lastNotificationPopup.x = mouse.screenX - startX
                    lastNotificationPopup.y = mouse.screenY - startY
                }
                QIconItem {
                    id: appIconItem
                    icon: model.appIcon
                    width: theme.largeIconSize
                    height: theme.largeIconSize
                    visible: !imageItem.visible
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                    }
                }
                QImageItem {
                    id: imageItem
                    anchors.fill: appIconItem
                    smooth: true
                    image: model.image
                    visible: nativeWidth > 0
                }
                PlasmaComponents.Label {
                    id: lastNotificationText
                    text: model.body
                    anchors {
                        left: appIconItem.right
                        right: actionsColumn.left
                        top: parent.top
                        bottom: parent.bottom
                        leftMargin: 6
                        rightMargin: 6
                    }
                    //textFormat: Text.PlainText
                    verticalAlignment: Text.AlignVCenter
                    color: theme.textColor
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                }
                Column {
                    id: actionsColumn
                    spacing: 6
                    anchors {
                        right: parent.right
                        rightMargin: 6
                        verticalCenter: parent.verticalCenter
                    }
                    Repeater {
                        model: actions
                        PlasmaComponents.Button {
                            text: model.text
                            width: theme.defaultFont.mSize.width * 8
                            height: theme.defaultFont.mSize.width * 2
                            onClicked: {
                                executeAction(source, model.id)
                                actionsColumn.visible = false
                            }
                        }
                    }
                }
            }
        }
        PlasmaCore.Svg {
            id: arrowsSvg
            imagePath: "widgets/arrows"
        }
        PlasmaCore.SvgItem {
            svg: arrowsSvg
            elementId: "up-arrow"
            width: theme.smallIconSize
            height: width
            visible: notificationsView.currentIndex > 0
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }
            MouseArea {
                anchors.fill: parent
                onClicked: notificationsView.currentIndex = Math.max(0, notificationsView.currentIndex-1)
            }
        }
        PlasmaCore.SvgItem {
            svg: arrowsSvg
            elementId: "down-arrow"
            width: theme.smallIconSize
            height: width
            visible: notificationsView.currentIndex < notificationsView.count-1
            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
            MouseArea {
                anchors.fill: parent
                onClicked: notificationsView.currentIndex = Math.min(notificationsView.count-1, notificationsView.currentIndex+1)
            }
        }
    }
}
