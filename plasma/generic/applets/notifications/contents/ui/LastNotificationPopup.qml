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

    onHeightChanged:  setCustomPosition(lastNotificationPopup.savedPos, false)
    onWidthChanged:   setCustomPosition(lastNotificationPopup.savedPos, false)

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
        mainItem.buttonPressed = false
    }

    function setCustomPosition(pos, writeConfig)
    {
        var popupPos = lastNotificationPopup.popupPosition(notificationIcon, Qt.AlignCenter)
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
        property int maximumHeight: theme.defaultFont.mSize.width * 10
        property int minimumWidth: maximumWidth
        property int minimumHeight: maximumHeight

        property int startX: 0
        property int startY: 0
        property int startScreenX: 0
        property int startScreenY: 0
        hoverEnabled: true
        property bool buttonPressed: false

        state: "controlsHidden"
        onContainsMouseChanged: {
            if (containsMouse) {
                mainItem.state = "controlsShown"
                lastNotificationTimer.running = false
            } else {
                mainItem.state = "controlsHidden"
                lastNotificationTimer.restart()
            }
        }
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
                buttonPressed ||
                Math.sqrt(Math.pow(startScreenX - mouse.screenX, 2) + Math.pow(startScreenY - mouse.screenY, 2)) > 4) {
            } else {
                lastNotificationPopup.visible = false
            }

            setCustomPosition(QPoint(Math.max(0, mouse.screenX - startX), Math.max(mouse.screenY - startY)), true)
        }
        onPositionChanged: {
            lastNotificationPopup.x = Math.max(0, mouse.screenX - startX)
            lastNotificationPopup.y = Math.max(0, mouse.screenY - startY)
        }
        onWheelMoved: {
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
                height: notificationsView.height
                width: notificationsView.width

                PlasmaComponents.Label {
                    id: titleLabel
                    text: model.summary
                    //font.weight: Font.Bold
                    visible: model.summary.length > 0
                    height: model.summary.length > 0 ? paintedHeight : 0
                    horizontalAlignment: Text.AlignHCenter
                    color: theme.textColor
                    elide: Text.ElideRight
                    anchors {
                        left: appIconItem.y > y + height ? parent.left : appIconItem.right
                        right: parent.right
                        rightMargin: settingsButton.visible ? settingsButton.width + closeButton.width : closeButton.width
                        top: parent.top
                        topMargin: 6
                        leftMargin: 6
                    }
                    onLinkActivated: plasmoid.openUrl(link)
                }

                QIconItem {
                    id: appIconItem
                    icon: model.appIcon
                    width: (model.appIcon.length > 0 || imageItem.visible) ? theme.largeIconSize : 0
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
                    image: model.image
                    smooth: true
                    fillMode: Image.PreserveAspectFit
                    visible: nativeWidth > 0
                }
               /*
                * this extra item is for clip the overflowed body text
                * maximumLineCount cannot control the behavior of rich text,
                * so manual clip is required.
                */
                Item {
                    id: bodyLabel
                    clip: true
                    height: Math.min(parent.height - (titleLabel.height+titleLabel.y), lastNotificationText.height)
                    property bool tallText: bodyLabel.height >= (bodyLabel.parent.height - (titleLabel.height+titleLabel.y)*2)
                    anchors {
                        top: tallText ? titleLabel.bottom : undefined
                        verticalCenter: tallText ? undefined : parent.verticalCenter
                        left: appIconItem.right
                        right: actionsColumn.left
                        leftMargin: 6
                        rightMargin: 6
                    }
                    PlasmaComponents.Label {
                        id: lastNotificationText
                        text: model.body
                        width: parent.width
                        //textFormat: Text.PlainText
                        color: theme.textColor
                        wrapMode: Text.Wrap
                        elide: Text.ElideRight
                        maximumLineCount: 4
                        onLinkActivated: plasmoid.openUrl(link)
                    }
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
                            onPressedChanged: {
                                if (pressed) {
                                    mainItem.buttonPressed = true
                                } else {
                                    mainItem.buttonPressed = false
                                }
                            }
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
            opacity: 0
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
                onPressedChanged: {
                    if (pressed) {
                        mainItem.buttonPressed = true
                    } else {
                        mainItem.buttonPressed = false
                    }
                }
                onClicked: {
                    notificationsView.currentIndex = Math.min(notificationsView.count-1, notificationsView.currentIndex+1)
                }
            }

            PlasmaComponents.ToolButton {
                id: backButton
                iconSource: "go-previous"
                width: theme.smallMediumIconSize
                height: mainItem.height/2 - 4
                enabled: notificationsView.currentIndex > 0
                onPressedChanged: {
                    if (pressed) {
                        mainItem.buttonPressed = true
                    } else {
                        mainItem.buttonPressed = false
                    }
                }
                onClicked: {
                    notificationsView.currentIndex = Math.max(0, notificationsView.currentIndex-1)
                }
            }
        }
        PlasmaComponents.ToolButton {
            id: closeButton
            opacity: 0
            iconSource: "window-close"
            width: theme.smallMediumIconSize
            height: width
            anchors {
                right: parent.right
                top: parent.top
            }
            onPressedChanged: {
                if (pressed) {
                    mainItem.buttonPressed = true
                } else {
                    mainItem.buttonPressed = false
                }
            }
            onClicked: {
                lastNotificationPopup.visible = false
                lastNotificationTimer.running = false
                closeNotification(notificationsModel.get((notificationsView.count-1)-notificationsView.currentIndex).source)
                notificationsModel.remove((notificationsView.count-1)-notificationsView.currentIndex)
            }
        }
        PlasmaComponents.ToolButton {
            id: settingsButton
            opacity: 0
            iconSource: "configure"
            width: theme.smallMediumIconSize
            height: width
            visible: notificationsModel.get((notificationsView.count-1)-notificationsView.currentIndex).configurable
            anchors {
                right: closeButton.left
                top: parent.top
                rightMargin: 5
            }
            onPressedChanged: {
                if (pressed) {
                    mainItem.buttonPressed = true
                } else {
                    mainItem.buttonPressed = false
                }
            }
            onClicked: {
                lastNotificationPopup.visible = false
                configureNotification(notificationsModel.get((notificationsView.count-1)-notificationsView.currentIndex).appRealName)
            }
        }
        states: [
            State {
                name: "controlsShown"
                PropertyChanges {
                    target: navigationButtonsColumn
                    opacity: 1
                }
                PropertyChanges {
                    target: closeButton
                    opacity: 1
                }
                PropertyChanges {
                    target: settingsButton
                    opacity: 1
                }
            },
            State {
                name: "controlsHidden"
                PropertyChanges {
                    target: navigationButtonsColumn
                    opacity: 0
                }
                PropertyChanges {
                    target: closeButton
                    opacity: 0
                }
                PropertyChanges {
                    target: settingsButton
                    opacity: 0
                }
            }
        ]
        transitions: [
            Transition {
             NumberAnimation {
                 properties: "opacity"
                 easing.type: Easing.InOutQuad
                 duration: 250
            }
         }
        ]
    }
}
