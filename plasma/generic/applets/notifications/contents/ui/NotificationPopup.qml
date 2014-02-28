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
import QtQuick.Layouts 1.1
import QtQml 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.qtextracomponents 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras

import QtQuick.Window 2.1

PlasmaCore.Dialog {
    id: notificationPopup

    location: PlasmaCore.Types.Floating
    type: PlasmaCore.Dialog.Dock
    flags: Qt.WindowStaysOnTopHint

//     property variant savedPos
//     property bool customPosition: false

    property var notificationProperties

    onVisibleChanged: {
        if (visible) {
            notificationTimer.running = true;
        }
    }

//     onHeightChanged:  setCustomPosition(notificationPopup.savedPos, false)
//     onWidthChanged:   setCustomPosition(notificationPopup.savedPos, false)

    function populatePopup(notification)
    {
        notificationProperties = notification

        notificationTimer.interval = notification.expireTimeout
//         notificationTimer.running = true;


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
        width: 18 * units.gridUnit

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
//             text: notificationProperties.body
//             visible: model.summary.length > 0
//             height: model.summary.length > 0 ? paintedHeight : 0
//             horizontalAlignment: Text.AlignHCenter
//             color: theme.textColor
            elide: Text.ElideRight
            anchors {
                left: appIconItem.right
                right: closeButton.left
                rightMargin: units.smallSpacing //settingsButton.visible ? settingsButton.width + closeButton.width : closeButton.width
                top: parent.top
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
//                 width: parent.width
                //textFormat: Text.PlainText
                color: theme.textColor
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: 4
                onLinkActivated: Qt.openUrlExternally(link)
                anchors {
                    left: appIconItem.right
                    top: titleLabel.bottom
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
                    notificationPopup.visible = false
                    //FIXME
//                     closeNotification(notificationsModel.get((notificationsView.count-1)-notificationsView.currentIndex).source)
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
                    id :actionsRepeater
                    model: new Array()
                    PlasmaComponents.Button {
                        text: modelData.text
                        width: theme.mSize(theme.defaultFont).width * 8
                        height: theme.mSize(theme.defaultFont).width * 2
                        onPressedChanged: {
                            if (pressed) {
//                                 mainItem.buttonPressed = true
                            } else {
//                                 mainItem.buttonPressed = false
                            }
                        }
                        onClicked: {
//                             executeAction(source, model.id)
//                             actionsColumn.visible = false
                        }
                    }
                }
            }

        Timer {
            id: notificationTimer
            repeat: false
            running: false
            onTriggered: {
                notificationPopup.visible = false
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
