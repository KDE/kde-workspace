/*
 * Copyright 2013 Jörg Ehrichs <joerg.ehrichs@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

/**
 * This item represents the entry for one service
 *
 * Contains the name of the indexer, the status message and icon
 * as well as button to suspend/resume and show the settings
 */
Item {
    id: serviceItem
    property string uid
    property string name
    property bool buttons

    property string statusMsg
    property int status
    property bool isAvailable
    property bool isSuspended
    property bool isActive
    property int percent

    height: labelsColumn.childrenRect.height
    width: parent.width

    states: [
        State {
            name: "ENABLED"
            when: (isAvailable && !isSuspended)
                PropertyChanges { target: startStopAction;  iconSource: "media-playback-stop"}
                PropertyChanges { target: statusIcon; icon: "user-online"}
                PropertyChanges { target: startStopAction; enabled: buttons}

        },
        State {
            name: "SUSPENDED"
            when: (isAvailable && isSuspended)
                PropertyChanges { target: startStopAction; iconSource: "media-playback-start"}
                PropertyChanges { target: statusIcon; icon: "user-invisible"}
                PropertyChanges { target: startStopAction; enabled: buttons}
        },
        State {
            name: "OFFLINE"
            when: (!isAvailable)
            PropertyChanges { target: statusIcon; icon: "user-offline"}
            PropertyChanges { target: startStopAction; visible: false}
        }
    ]

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }

    Column {
        spacing: 10
        width: parent.width

        id: labelsColumn
        anchors {
            left : parent.left
            right: buttonRow.left
            topMargin: 5
            leftMargin: 5
            rightMargin: 5
            bottomMargin: 10
        }

        PlasmaComponents.Label {
            id: serviceName

            text: name
            font.bold: true
            font.pointSize: theme.desktopFont.pointSize + 4
        }

        Row {
            id:statusRow
            spacing: 5
            width: parent.width

            QIconItem {
                id: statusIcon
                icon: "user-offline"
                height: 16
                width: 16
            }
            PlasmaComponents.Label {
                id: serviceStatus
                anchors {
                    verticalCenter: statusIcon.verticalCenter
                }

                width: parent.width - statusIcon.width
                height: 3*theme.smallestFont.pointSize

                text: statusMsg
                font.italic: true
                font.pointSize: theme.smallestFont.pointSize
                color: "#99"+(theme.textColor.toString().substr(1))
                wrapMode: Text.Wrap
            }
        }

        PlasmaCore.SvgItem {
            id: separator

            svg: lineSvg
            elementId: "horizontal-line"
            width: parent.width + buttonRow.width
            height: lineSvg.elementSize("horizontal-line").height
        }
    }

    PlasmaComponents.ButtonRow {
        id: buttonRow
        exclusive: false

        anchors {
            right: parent.right
            rightMargin: 5
        }

        PlasmaComponents.Button {
            id: startStopAction
            iconSource: "media-playback-stop"
            visible: buttons

            onClicked: {
                service = nepomukSource.serviceForSource(uid);
                if (serviceItem.state == "ENABLED") {
                    operation = service.operationDescription("suspend");
                    operation.predicate = "suspend";
                }
                else {
                    operation = service.operationDescription("resume");
                    operation.predicate = "resume";
                }
                service.startOperationCall(operation);
            }
        }

        PlasmaComponents.Button {
            id: settingsAction
            iconSource: "configure"

            onClicked: {
                service = nepomukSource.serviceForSource(uid);
                operation = service.operationDescription("settings");
                operation.predicate = "settings";
                service.startOperationCall(operation);
            }
        }
    }
}
