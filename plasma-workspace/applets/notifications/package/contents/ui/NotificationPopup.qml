/*
 *   Copyright 2014 Martin Klapetek <mklapetek@kde.org>
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

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0

PlasmaCore.Dialog {
    id: notificationPopup

    location: PlasmaCore.Types.Floating
    type: PlasmaCore.Dialog.Dock
    flags: Qt.WindowStaysOnTopHint

    property var notificationProperties

    onVisibleChanged: {
        notificationTimer.running = visible;
    }

    function populatePopup(notification)
    {
        notificationProperties = notification
        print("Populating things");
        notificationTimer.interval = notification.expireTimeout
        titleLabel.text = notification.summary
        bodyLabel.text = notification.body
        appIconItem.icon = notification.appIcon
        actionsRepeater.model = notification.actions
    }

    Behavior on y {
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.OutQuad
        }
    }

    mainItem: MouseEventListener {
        id: mainItem
        height: 6 * units.gridUnit
        width: 19 * units.gridUnit

        state: "controlsHidden"
        hoverEnabled: true

        onContainsMouseChanged: {
            if (containsMouse) {
                mainItem.state = "controlsShown"
                notificationTimer.running = false
            } else {
                mainItem.state = "controlsHidden"
                notificationTimer.restart()
            }
        }

        QIconItem {
            id: appIconItem
            height: units.iconSizes.large
            width: height
            visible: true// !imageItem.visible
            anchors {
                left: parent.left
                top: parent.top
                leftMargin: units.smallSpacing
                topMargin: units.smallSpacing
                rightMargin: units.smallSpacing
                bottomMargin: units.smallSpacing
            }
        }

        PlasmaComponents.Label {
            id: titleLabel
            font.weight: Font.Bold
            elide: Text.ElideRight
            anchors {
                left: appIconItem.right
                right: closeButton.left
                top: parent.top
                rightMargin: units.smallSpacing //settingsButton.visible ? settingsButton.width + closeButton.width : closeButton.width
                leftMargin: units.largeSpacing
                topMargin: units.smallSpacing
            }
            onLinkActivated: Qt.openUrlExternally(link)
        }


        /*
         * this extra item is for clip the overflowed body text
         * maximumLineCount cannot control the behavior of rich text,
         * so manual clip is required.
         */
//         Item {
//             id: bodyLabelClip
//             clip: true
//             height: Math.min(parent.height - (titleLabel.height+titleLabel.y), bodyLabel.height)
//             property bool tallText: bodyLabelClip.height >= (bodyLabelClip.parent.height - (titleLabel.height+titleLabel.y)*2)
//             anchors {
//                 top: tallText ? titleLabel.bottom : undefined
//                 verticalCenter: tallText ? undefined : parent.verticalCenter
//                 left: appIconItem.right
// //                 right: actionsColumn.left
//                 leftMargin: 6
//                 rightMargin: 6
//             }
            PlasmaComponents.Label {
                id: bodyLabel
                color: theme.textColor
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: 4
                verticalAlignment: Text.AlignTop
                onLinkActivated: Qt.openUrlExternally(link)
                anchors {
                    left: appIconItem.right
                    right: actionsColumn.left
                    top: titleLabel.bottom
                    bottom: parent.bottom
                    leftMargin: units.largeSpacing
                }
            }
//         }

            PlasmaComponents.ToolButton {
                id: closeButton
                opacity: 0
                iconSource: "window-close"
                width: units.iconSizes.smallMedium
                height: width
                anchors {
                    right: parent.right
                    top: parent.top
                }
                onClicked: {
                    closeNotification(notificationProperties.source)
                    notificationPopup.close()
                }
            }

            Column {
                id: actionsColumn
                spacing: 6
                anchors {
                    top: closeButton.bottom
                    right: parent.right
                    rightMargin: 6
                    verticalCenter: parent.verticalCenter
                }
                Repeater {
                    id: actionsRepeater
                    model: new Array()
                    PlasmaComponents.Button {
                        text: modelData.text
                        width: theme.mSize(theme.defaultFont).width * 8
                        height: theme.mSize(theme.defaultFont).width * 2
                        onClicked: {
                            executeAction(notificationProperties.source, modelData.id)
                            actionsColumn.visible = false
                            notificationPopup.close()
                        }
                    }
                }
            }

        Timer {
            id: notificationTimer
            repeat: false
            running: false
            onTriggered: {
                if (!notificationProperties.isPersistent) {
                    closeNotification(notificationProperties.source)
                }
                notificationPopup.close()
            }
        }

        states: [
        State {
            name: "controlsShown"
            PropertyChanges {
                target: closeButton
                opacity: 1
            }
//             PropertyChanges {
//                 target: settingsButton
//                 opacity: 1
//             }
        },
        State {
            name: "controlsHidden"
            PropertyChanges {
                target: closeButton
                opacity: 0
            }
//             PropertyChanges {
//                 target: settingsButton
//                 opacity: 0
//             }
        }
        ]
        transitions: [
        Transition {
            NumberAnimation {
                properties: "opacity"
                easing.type: Easing.InOutQuad
                duration: units.longDuration
            }
        }
        ]

    }

}
