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

    property variant savedPos
    property bool customPosition: false
    onCustomPositionChanged: {
        setAttribute(Qt.WA_X11NetWmWindowTypeToolTip, customPosition);
    }

    function popup(notification)
    {
        if (!lastNotificationPopup.visible) {
            lastNotificationsModel.clear()
        }
        lastNotificationsModel.append(notification)

        setCustomPosition(lastNotificationPopup.savedPos, false)

        lastNotificationPopup.visible = true
        lastNotificationTimer.interval = Math.max(4000, Math.min(60*1000, notificationsModel.get(0).expireTimeout))
        notificationsView.currentIndex = lastNotificationsModel.count - 1
        lastNotificationTimer.restart()
    }

    function setCustomPosition(pos, writeConfig)
    {
        var popupPos = lastNotificationPopup.popupPosition(notificationIcon)
        var finalPos

        //custom
        if ((pos.x >= 0 || pos.y >= 0) &&
            (Math.abs(popupPos.x - pos.x) > 40 ||
            Math.abs(popupPos.y - pos.y) > 40)) {
            finalPos = pos
            if (writeConfig) {
                plasmoid.writeConfig("CustomPosition", pos)
                lastNotificationPopup.savedPos = pos
                lastNotificationPopup.customPosition = true
            }
        } else {
            finalPos = popupPos
            if (writeConfig) {
                plasmoid.writeConfig("CustomPosition", QPoint(-1,-1))
                lastNotificationPopup.savedPos = QPoint(-1,-1)
                lastNotificationPopup.customPosition = false
            }
        }
        lastNotificationPopup.x = finalPos.x
        lastNotificationPopup.y = finalPos.y
    }

    location: customPosition ? Floating : plasmoid.location
    windowFlags: Qt.WindowStaysOnTopHint
    Component.onCompleted: {
        setAttribute(Qt.WA_X11NetWmWindowTypeDock, true)

        lastNotificationPopup.savedPos = plasmoid.readConfig("CustomPosition")
        setCustomPosition(lastNotificationPopup.savedPos, true)
        plasmoid.popupEvent.connect(lastNotificationPopup.popupEvent)
    }

    function popupEvent(popupShowing)
    {
        if(popupShowing) {
           lastNotificationPopup.visible = false
        }
    }

    mainItem: MouseEventListener {
        id: mainItem
        width: maximumWidth
        height: maximumHeight
        property int maximumWidth: theme.defaultFont.mSize.width * 35
        property int maximumHeight: theme.defaultFont.mSize.width * 8
        property int minimumWidth: maximumWidth
        property int minimumHeight: maximumHeight

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
            if ((navigationButtonsColumn.visible && mouse.x < navigationButtonsColumn.width) ||
                Math.sqrt(Math.pow(startScreenX - mouse.screenX, 2) + Math.pow(startScreenY - mouse.screenY, 2)) > 4) {
                lastNotificationTimer.restart()
            } else {
                lastNotificationPopup.visible = false
                lastNotificationTimer.running = false
            }

            setCustomPosition(QPoint(Math.max(0, mouse.screenX - startX), Math.max(mouse.screenY - startY)), true)
        }
        onPositionChanged: {
            setCustomPosition(QPoint(Math.max(0, mouse.screenX - startX), Math.max(0, mouse.screenY - startY)), false)
        }
        onWheelMoved: {
            lastNotificationTimer.restart()
            if (notificationsView.moving) {
                return
            }

            if (wheel.delta > 0) {
                notificationsView.currentIndex = Math.max(0, notificationsView.currentIndex-1)
            } else {
                notificationsView.currentIndex = Math.min(notificationsView.count-1, notificationsView.currentIndex+1)
            }
        }

        Timer {
            id: lastNotificationTimer
            interval: 4000
            repeat: false
            running: false
            onTriggered: lastNotificationPopup.visible = false
        }

        ListView {
            id: notificationsView
            clip: true
            snapMode: ListView.SnapOneItem
            orientation: ListView.Horizontal
            anchors.fill: parent
            model: ListModel {
                id: lastNotificationsModel
            }
            interactive: false
            delegate: Item {
                width: notificationsView.width
                height: notificationsView.height
                QIconItem {
                    id: appIconItem
                    icon: model.appIcon
                    width: theme.largeIconSize
                    height: theme.largeIconSize
                    visible: !imageItem.visible
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: navigationButtonsColumn.width
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
                    maximumLineCount: 4
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

        Column {
            id: navigationButtonsColumn
            visible: backButton.enabled || nextButton.enabled
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }

            PlasmaComponents.ToolButton {
                id: nextButton
                iconSource: "go-next"
                width: theme.smallMediumIconSize
                height: mainItem.height/2 - 4
                enabled: notificationsView.currentIndex < notificationsView.count-1
                onClicked: {
                    lastNotificationTimer.restart()
                    notificationsView.currentIndex = Math.min(notificationsView.count-1, notificationsView.currentIndex+1)
                }
            }

            PlasmaComponents.ToolButton {
                id: backButton
                iconSource: "go-previous"
                width: theme.smallMediumIconSize
                height: mainItem.height/2 - 4
                enabled: notificationsView.currentIndex > 0
                onClicked: {
                    lastNotificationTimer.restart()
                    notificationsView.currentIndex = Math.max(0, notificationsView.currentIndex-1)
                }
            }
        }
    }
}
