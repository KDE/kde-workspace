/*
 *   Copyright 2011 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

PlasmaComponents.ListItem {
    id: notificationItem
    opacity: 1-Math.abs(x)/width
    width: popupFlickable.width
    property int toolIconSize: theme.smallMediumIconSize
    property int layoutSpacing: 4

    visible: appTabBar.currentTab == allAppsTab || appTabBar.currentTab.text == appName

    Component.onCompleted: {
        allApplicationsModel.addApplication(appIcon, appName)
        mainScrollArea.height = mainScrollArea.implicitHeight
    }
    Component.onDestruction: {
        allApplicationsModel.removeApplication(model.appName)
        mainScrollArea.height = mainScrollArea.implicitHeight
    }
    Timer {
        interval: 30*60*1000
        repeat: false
        running: true
        onTriggered: {
            notificationsModel.remove(index)
        }
    }


    MouseArea {
        width: parent.width
        height: childrenRect.height
        drag {
            target: notificationItem
            axis: Drag.XAxis
        }
        onReleased: {
            if (notificationItem.x < -notificationItem.width/2) {
                removeAnimation.exitFromRight = false
                removeAnimation.running = true
            } else if (notificationItem.x > notificationItem.width/2 ) {
                removeAnimation.exitFromRight = true
                removeAnimation.running = true
            } else {
                resetAnimation.running = true
            }
        }
        SequentialAnimation {
            id: removeAnimation
            property bool exitFromRight: true
            NumberAnimation {
                target: notificationItem
                properties: "x"
                to: removeAnimation.exitFromRight ? notificationItem.width-1 : 1-notificationItem.width
                duration: 250
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: notificationItem
                properties: "height"
                to: 0
                duration: 250
                easing.type: Easing.InOutQuad
            }
            ScriptAction {
                script: notificationsModel.remove(index)
            }
        }
        SequentialAnimation {
            id: resetAnimation
            NumberAnimation {
                target: notificationItem
                properties: "x"
                to: 0
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        Column {
            spacing: notificationItem.layoutSpacing
            width: parent.width
            Item {
                width: parent.width
                height: summaryLabel.height

                PlasmaComponents.Label {
                    id: summaryLabel
                    text: summary
                    font.bold: true
                    height: paintedHeight
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    maximumLineCount: 4
                }

                PlasmaComponents.ToolButton {
                    iconSource: "window-close"
                    width: notificationItem.toolIconSize
                    height: width
                    onClicked: removeAnimation.running = true
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                }
            }

            Item {
                height: childrenRect.height
                width: parent.width
                QIconItem {
                    id: appIconItem
                    icon: QIcon(appIcon)
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
                    image: model.image
                    smooth: true
                    visible: nativeWidth > 0
                }
                PlasmaComponents.ContextMenu {
                    id: contextMenu
                    PlasmaComponents.MenuItem {
                        text: i18n("Copy")
                        onClicked: bodyText.copy()
                    }
                    PlasmaComponents.MenuItem {
                        text: i18n("Select All")
                        onClicked: bodyText.selectAll()
                    }
                }
                MouseArea {
                    anchors {
                        left: appIconItem.right
                        right: actionsColumn.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 6
                        rightMargin: 6
                    }
                    acceptedButtons: Qt.RightButton
                    height: bodyText.paintedHeight
                    preventStealing: true
                    onPressed: contextMenu.open(mouse.x, mouse.y)
                    TextEdit {
                        id: bodyText
                        anchors.fill: parent
                        text: body
                        color: theme.textColor
                        font.capitalization: theme.defaultFont.capitalization
                        font.family: theme.defaultFont.family
                        font.italic: theme.defaultFont.italic
                        font.letterSpacing: theme.defaultFont.letterSpacing
                        font.pointSize: theme.defaultFont.pointSize
                        font.strikeout: theme.defaultFont.strikeout
                        font.underline: theme.defaultFont.underline
                        font.weight: theme.defaultFont.weight
                        font.wordSpacing: theme.defaultFont.wordSpacing
                        selectByMouse: true
                        readOnly: true
                        wrapMode: Text.Wrap
                    }
                }
                Column {
                    id: actionsColumn
                    spacing: notificationItem.layoutSpacing
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
    }
}
