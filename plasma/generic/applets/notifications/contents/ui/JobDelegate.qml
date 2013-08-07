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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.qtextracomponents 0.1

PlasmaComponents.ListItem {
    id: notificationItem
    width: popupFlickable.width

    property int toolIconSize: theme.smallMediumIconSize
    property int layoutSpacing: 4

    function getData(data, name, defaultValue) {
        return data[modelData] ? (data[modelData][name] ? data[modelData][name] : defaultValue) : defaultValue;
    }

    property string labelName0: getData(jobsSource.data, "labelName0", '')
    property string label0: getData(jobsSource.data, "label0", '')
    property string labelName1: getData(jobsSource.data, "labelName1", '')
    property string label1: getData(jobsSource.data, "label1", '')
    property string jobstate: getData(jobsSource.data, "state", '')
    property int eta: getData(jobsSource.data, "eta", 0)
    property string speed: getData(jobsSource.data, "speed", '')

    Column {
        spacing: notificationItem.layoutSpacing
        width: parent.width
        PlasmaComponents.Label {
            text: getData(jobsSource.data, "infoMessage", '')
            font.bold: true
            color: theme.textColor
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Grid {
            anchors {
                left: parent.left
                right: parent.right
                rightMargin: notificationItem.layoutSpacing
            }
            spacing: notificationItem.layoutSpacing
            rows: 4
            columns: 2

            PlasmaComponents.Label {
                id: labelName0Text
                text: labelName0 ? i18n("%1:", labelName0) : ''
                width: Math.max(paintedWidth, labelName1Text.paintedWidth)
                horizontalAlignment: Text.AlignRight
                visible: labelName0 != ''
            }
            PlasmaComponents.Label {
                id: label0Text
                text: label0 ? label0 : ''
                width: parent.width - labelName0Text.width
                elide: Text.ElideMiddle
                visible: label0 != ''

                PlasmaCore.ToolTip {
                    target: label0Text
                    subText: label0Text.truncated ? label0 : ""
                }

            }
            PlasmaComponents.Label {
                id: labelName1Text
                text: labelName1 ? i18n("%1:", labelName1) : ''
                width: Math.max(paintedWidth, labelName0Text.paintedWidth)
                horizontalAlignment: Text.AlignRight
                visible: labelName1 != ''
            }
            PlasmaComponents.Label {
                id: label1Text
                text: label1 ? label1 : ''
                width: parent.width - labelName0Text.width
                elide: Text.ElideMiddle
                visible: label1 != ''

                PlasmaCore.ToolTip {
                    target: label1Text
                    subText: label1Text.truncated ? label1 : ""
                }
            }
            QIconItem {
                icon: getData(jobsSource.data, "appIconName", '')
                width: notificationItem.toolIconSize
                height: width
                anchors {
                    verticalCenter: progressItem.verticalCenter
                    right: progressItem.left
                    rightMargin: notificationItem.layoutSpacing
                }
            }
            Item {
                id: progressItem
                width: parent.width - labelName0Text.width
                height: childrenRect.height
                PlasmaComponents.ProgressBar {
                    width: parent.width - pauseButton.width*2 - theme.largeIconSize - notificationItem.layoutSpacing*3
                    height: 16
                    orientation: Qt.Horizontal
                    minimumValue: 0
                    maximumValue: 100
                    //percentage doesn't always exist, so doesn't get in the model
                    value: getData(jobsSource.data, "percentage", 0)

                    anchors {
                        left: parent.left
                        right: buttonsRow.left
                        verticalCenter: parent.verticalCenter
                        rightMargin: notificationItem.layoutSpacing
                    }
                }
                Row {
                    id: buttonsRow
                    spacing: notificationItem.layoutSpacing
                    anchors.right: parent.right
                    PlasmaComponents.ToolButton {
                        id: pauseButton
                        width: notificationItem.toolIconSize
                        height: width
                        iconSource: jobstate == "suspended" ? "media-playback-start" : "media-playback-pause"
                        flat: false
                        onClicked: {
                            var operationName = "suspend"
                            if (jobstate == "suspended") {
                                operationName = "resume"
                            }
                            var service = jobsSource.serviceForSource(modelData)
                            var operation = service.operationDescription(operationName)
                            service.startOperationCall(operation)
                        }
                    }
                    PlasmaComponents.ToolButton {
                        id: stopButton
                        width: notificationItem.toolIconSize
                        height: width
                        iconSource: "media-playback-stop"
                        flat: false
                        onClicked: {
                            var service = jobsSource.serviceForSource(modelData)
                            var operation = service.operationDescription("stop")
                            service.startOperationCall(operation)
                        }
                    }
                }
            }
            PlasmaComponents.ToolButton {
                id: expandButton
                width: notificationItem.toolIconSize
                height: width
                flat: false
                iconSource: checked ? "list-remove" : "list-add"
                checkable: true
                anchors {
                    right: speedLabel.left
                    rightMargin: notificationItem.layoutSpacing
                    verticalCenter: speedLabel.verticalCenter
                }
            }
            PlasmaComponents.Label {
                id: speedLabel
                text: eta > 0 ? i18nc("Speed and estimated time to completition", "%1 (%2 remaining)", speed, locale.prettyFormatDuration(eta)) : speed
            }
        }


        Item {
            id: detailsItem
            state: expandButton.checked ? "expanded" : "collapsed"
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: speedLabel.x
            }
            property Item contentsItem
            Component {
                id: detailsComponent
                Column {
                    id: detailsColumn
                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    function localizeProcessedAmount(id) {
                        //if bytes localise the unit
                        if (jobsSource.data[modelData]["processedUnit"+id] == "bytes") {
                            return i18nc("How much many bytes (or whether unit in the locale has been copied over total", "%1 of %2",
                                    locale.formatByteSize(jobsSource.data[modelData]["processedAmount"+id]),
                                    locale.formatByteSize(jobsSource.data[modelData]["totalAmount"+id]))
                        //else print something only if is interesting data (ie more than one file/directory etc to copy
                        } else if (jobsSource.data[modelData]["totalAmount"+id] > 1) {
                            return i18n( "%1 of %2 %3",
                                    jobsSource.data[modelData]["processedAmount"+id],
                                    jobsSource.data[modelData]["totalAmount"+id],
                                    jobsSource.data[modelData]["processedUnit"+id])
                        } else {
                            return ""
                        }
                    }
                    PlasmaComponents.Label {
                        text: jobsSource.data[modelData] ? detailsColumn.localizeProcessedAmount(0) : ""
                        anchors.left: parent.left
                        visible: text != ""
                    }
                    PlasmaComponents.Label {
                        text: jobsSource.data[modelData] ? detailsColumn.localizeProcessedAmount(1) : ""
                        anchors.left: parent.left
                        visible: text != ""
                    }
                    PlasmaComponents.Label {
                        text: jobsSource.data[modelData] ? detailsColumn.localizeProcessedAmount(2) : ""
                        anchors.left: parent.left
                        visible: text != ""
                    }
                    PlasmaWidgets.SignalPlotter {
                        id: plotter
                        width: parent.width
                        useAutoRange: true
                        showVerticalLines: false
                        unit: i18n("KiB/s")
                        height: theme.defaultFont.mSize.height * 5
                        Component.onCompleted: plotter.addPlot(theme.highlightColor)
                    }
                    Connections {
                        target: jobsSource
                        onDataChanged: {
                            plotter.addSample([jobsSource.data[modelData]["numericSpeed"]/1000])
                        }
                    }
                }
            }

            states: [
                State {
                    name: "expanded"
                    PropertyChanges {
                        target: detailsItem
                        height: detailsItem.childrenRect.height
                    }
                },
                State {
                    name: "collapsed"
                    PropertyChanges {
                        target: detailsItem
                        height: 0
                    }
                }
            ]
            transitions : [
                Transition {
                    from: "collapsed"
                    to: "expanded"
                    SequentialAnimation {
                        ScriptAction {
                            script: {
                                detailsItem.visible = true
                                detailsItem.clip = true
                                //create the contents if they don't exist yet
                                if (!detailsItem.contentsItem) {
                                    detailsItem.contentsItem = detailsComponent.createObject(detailsItem)
                                }
                            }
                        }
                        NumberAnimation {
                            duration: 250
                            properties: "height"
                            easing: PropertyAnimation.EaseInOut
                        }
                        ScriptAction {script: detailsItem.clip = false}
                    }
                },
                Transition {
                    from: "expanded"
                    to: "collapsed"
                    SequentialAnimation {
                        ScriptAction {script: detailsItem.clip = true}
                        NumberAnimation {
                            duration: 250
                            properties: "height"
                            easing: PropertyAnimation.EaseInOut
                        }
                        //TODO: delete the details?
                        ScriptAction {script: detailsItem.visible = false}
                    }
                }
            ]
        }
    }
}
