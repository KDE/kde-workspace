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

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.qtextracomponents 2.0

Item {
    id: notificationItem

    width: popupFlickable.width
    height: jobGrid.childrenRect.height + (newDetailsItem.state == "expanded" ? units.largeSpacing : units.largeSpacing / 2)

    property int toolIconSize: units.iconSizes.smallMedium
    property int layoutSpacing: units.largeSpacing / 4
    property int animationDuration: 100
    property bool flatButtons: true

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

    property bool debug: false

    function friendlyLabel() {
        // Take the filename, might be improved
        var els = label0.split("/");
        if (els.length) {
            return els[els.length-1];
        }
        return f;
    }

    Rectangle {
        visible: notificationItem.debug
        color: "orange"
        opacity: 0.2
        anchors.fill: parent
    }

    Item {
        id: jobGrid
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        property int leftColWidth: Math.max(labelName0Text.paintedWidth, labelName1Text.paintedWidth)

        PlasmaExtras.Heading {
            id: infoLabel
            opacity: 0.6
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            level: 3
            text: getData(jobsSource.data, "infoMessage", '')
        }

        PlasmaComponents.Label {
            id: summary
            anchors {
                top: infoLabel.bottom
                left: parent.left
                right: expandButton.left
                rightMargin: notificationItem.layoutSpacing
            }
            elide: Text.ElideMiddle
            text: notificationItem.friendlyLabel()
        }

        PlasmaComponents.ToolButton {
            id: expandButton
            width: notificationItem.toolIconSize
            height: width
            flat: notificationItem.flatButtons
            iconSource: checked ? "list-remove" : "list-add"
            checkable: true
            anchors {
                right: parent.right
                top: summary.top
            }
        }

        Item {
            id: newDetailsItem

            property bool expanded: state == "expanded"
            state: expandButton.checked ? "expanded" : "collapsed"
            onStateChanged: {
                print("NNN expand state is now: " + state);
            }

            // 1st row
//             Rectangle {
//                 color: "yellow"
//                 visible: notificationItem.debug
//                 opacity: 0.3
//                 anchors {
//                     fill: parent
//                 }
//             }
            opacity: newDetailsItem.state == "expanded" ? 0.6 : 0
            //height: newDetailsItem.expanded ? childrenRect.height : 0
            //Behavior on height { NumberAnimation { duration: notificationItem.animationDuration } }
            Behavior on opacity { NumberAnimation { duration: notificationItem.animationDuration } }
            //clip: true
            anchors {
                top: summary.bottom
                topMargin: notificationItem.layoutSpacing
                left: parent.left
                right: parent.right
            }

            PlasmaComponents.Label {
                id: labelName0Text
                anchors {
                    top: parent.top
                    left: parent.left
                }
                width: jobGrid.leftColWidth
                height: paintedHeight

                font: theme.smallestFont
                text: labelName0 ? i18n("%1:", labelName0) : ''
                horizontalAlignment: Text.AlignRight
                visible: labelName0 != ''
            }

            PlasmaComponents.Label {
                id: label0Text
                anchors {
                    top: labelName0Text.top
                    left: labelName0Text.right
                    right: parent.right
                    leftMargin: notificationItem.layoutSpacing
                }
                height: paintedHeight
                font: theme.smallestFont
                text: label0 ? label0 : ''
                //width: parent.width - labelName0Text.width
                elide: Text.ElideMiddle
                visible: label0 != ''

                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    subText: label0Text.truncated ? label0 : ""
                }

            }

            // 2nd row
            PlasmaComponents.Label {
                id: labelName1Text
                anchors {
                    top: labelName0Text.bottom
                    topMargin: notificationItem.layoutSpacing
                    left: parent.left
                }
                width: jobGrid.leftColWidth
                height: paintedHeight

                font: theme.smallestFont
                text: labelName1 ? i18n("%1:", labelName1) : ''
                horizontalAlignment: Text.AlignRight
                visible: labelName1 != ''
            }
            PlasmaComponents.Label {
                id: label1Text

                anchors {
                    top: labelName1Text.top
                    left: labelName1Text.right
                    right: parent.right
                    leftMargin: notificationItem.layoutSpacing
                }

                font: theme.smallestFont
                height: paintedHeight
                text: label1 ? label1 : ''
                //width: parent.width - labelName0Text.width
                elide: Text.ElideMiddle
                visible: label1 != ''

                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    subText: label1Text.truncated ? label1 : ""
                }
            }

            Column {
                id: detailsColumn
                anchors {
                    top: labelName1Text.bottom
                    topMargin: notificationItem.layoutSpacing
                    right: parent.right
                    left: labelName1Text.right
                    leftMargin: notificationItem.layoutSpacing
                }
                spacing: notificationItem.layoutSpacing

                function formatByteSize(size) {
                    // Per IEC 60027-2

                    // Binary prefixes
                    //Tebi-byte             TiB             2^40    1,099,511,627,776 bytes
                    //Gibi-byte             GiB             2^30    1,073,741,824 bytes
                    //Mebi-byte             MiB             2^20    1,048,576 bytes
                    //Kibi-byte             KiB             2^10    1,024 bytes

                    var s;
                    // Gibi-byte
                    if ( size >= 1073741824.0 )
                    {
                        size = size /1073741824.0;
                        if ( size > 1024 ) { // Tebi-byte
                            // That's a huge amount, so display 3 digits
                            s = i18n("%1 TiB", Math.round(size / 1024.0 *1000) / 1000);
                        } else {
                            // Still quite large, display 3 digits
                            s = i18n("%1 GiB", Math.round(size *100) / 100);
                        }
                    }
                    // Mebi-byte
                    else if ( size >= 1048576.0 )
                    {
                        size = size /1048576.0;
                        s = i18n("%1 MiB", Math.round(size *10) / 10);
                    }
                    // Kibi-byte
                    else if ( size >= 1024.0 )
                    {
                        size = size /1024.0;
                        s = i18n("%1 KiB", Math.round(size *10) / 10);
                    }
                    // Just byte
                    else if ( size > 0 )
                    {
                        s = i18n("%1 B", Math.round(size *10) / 10);
                    }
                    // Nothing
                    else
                    {
                        s = i18n("0 B");
                    }
                    return s;
                }

                function localizeProcessedAmount(id) {
                    //if bytes localise the unit
                    if (jobsSource.data[modelData]["processedUnit"+id] == "bytes") {
                        return i18nc("How much many bytes (or whether unit in the locale has been copied over total", "%1 of %2",
                                formatByteSize(jobsSource.data[modelData]["processedAmount"+id]),
                                formatByteSize(jobsSource.data[modelData]["totalAmount"+id]))
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

                function prettyFormatDuration(msec) {
                    var minutes = Math.floor(msec / 1000 / 60);
                    var seconds = Math.round((msec % (1000 * 60)) / 1000);
                    var o;
                    if (minutes < 3) {
                        o = i18n("%1 min %2 sec", minutes, seconds);
                    } else {
                        o = i18n("%1 min", minutes);
                    }
                    return o;
                }

                PlasmaComponents.Label {
                    text: jobsSource.data[modelData] ? detailsColumn.localizeProcessedAmount(0) : ""
                    anchors.left: parent.left
                    font: theme.smallestFont
                    height: paintedHeight
                    visible: text != ""
                }
                PlasmaComponents.Label {
                    text: jobsSource.data[modelData] ? detailsColumn.localizeProcessedAmount(1) : ""
                    anchors.left: parent.left
                    font: theme.smallestFont
                    height: paintedHeight
                    visible: text != ""
                }
                PlasmaComponents.Label {
                    text: jobsSource.data[modelData] ? detailsColumn.localizeProcessedAmount(2) : ""
                    anchors.left: parent.left
                    font: theme.smallestFont
                    height: paintedHeight
                    visible: text != ""
                }
                PlasmaComponents.Label {
                    id: speedLabel
                    anchors.left: parent.left
                    font: theme.smallestFont
                    height: paintedHeight
                    text: notificationItem.eta > 0 ? i18nc("Speed and estimated time to completition", "%1 (%2 remaining)", speed, detailsColumn.prettyFormatDuration(notificationItem.eta)) : speed
                }
// FIXME: find a way to plot the signal
//                     PlasmaWidgets.SignalPlotter {
//                         id: plotter
//                         width: parent.width
//                         useAutoRange: true
//                         showVerticalLines: false
//                         unit: i18n("KiB/s")
//                         height: theme.mSize(theme.defaultFont).height * 5
//                         Component.onCompleted: plotter.addPlot(theme.highlightColor)
//                     }
//                     Connections {
//                         target: jobsSource
//                         onDataChanged: {
//                             plotter.addSample([jobsSource.data[modelData]["numericSpeed"]/1000])
//                         }
//                     }
                Item {
                    // just for spacing between details and progress bar
                    width: parent.width
                    height: notificationItem.layoutSpacing
                }
            }

            states: [
                State {
                    name: "expanded"
                    PropertyChanges {
                        target: newDetailsItem
                        height: newDetailsItem.childrenRect.height
                    }
                },
                State {
                    name: "collapsed"
                    PropertyChanges {
                        target: newDetailsItem
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
                                newDetailsItem.visible = true
                                newDetailsItem.clip = true
                            }
                        }
                        NumberAnimation {
                            duration: notificationItem.animationDuration
                            properties: "height"
                            easing.type: Easing.InOutQuad
                        }
                        ScriptAction {script: newDetailsItem.clip = false}
                    }
                },
                Transition {
                    from: "expanded"
                    to: "collapsed"
                    SequentialAnimation {
                        ScriptAction {script: newDetailsItem.clip = true}
                        NumberAnimation {
                            duration: notificationItem.animationDuration
                            properties: "height"
                            easing.type: Easing.InOutQuad
                        }
                        //TODO: delete the details?
                        ScriptAction {script: newDetailsItem.visible = false}
                    }
                }
            ]
        }
        Item {
            id: buttonsRow

            height: notificationItem.toolIconSize
            anchors {
                top: !newDetailsItem.opacity  ? summary.bottom : newDetailsItem.bottom
                left: parent.left
                right: parent.right

            }

            Rectangle {
                visible: notificationItem.debug
                color: "green"
                opacity: 0
                anchors.fill: parent
            }

            PlasmaComponents.ProgressBar {
                id: progressBar
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    right: pauseButton.left
                    rightMargin: notificationItem.layoutSpacing

                }
                //width: parent.width - pauseButton.width*2 - notificationItem.layoutSpacing*3
                width: 200
                height: 16
                orientation: Qt.Horizontal
                minimumValue: 0
                maximumValue: 100
                //percentage doesn't always exist, so doesn't get in the model
                value: getData(jobsSource.data, "percentage", 0)

//                     anchors {
//                         left: parent.left
//                         right: buttonsRow.left
//                         verticalCenter: parent.verticalCenter
//                         rightMargin: notificationItem.layoutSpacing
//                     }
            }
//                     anchors.right: parent.right
            PlasmaComponents.ToolButton {
                id: pauseButton
                width: notificationItem.toolIconSize
                height: width
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: stopButton.left
                    rightMargin: notificationItem.layoutSpacing
                }
                iconSource: notificationItem.jobstate == "suspended" ? "media-playback-start" : "media-playback-pause"
                flat: notificationItem.flatButtons
                onClicked: {
                    print("NNN Current: " + jobstate);
                    var operationName = "suspend"
                    if (notificationItem.jobstate == "suspended") {
                        operationName = "resume"
                    }
                    var service = jobsSource.serviceForSource(modelData)
                    var operation = service.operationDescription(operationName)
                    service.startOperationCall(operation)
                    print("NNN now: " + notificationItem.jobstate);
                }
            }
            PlasmaComponents.ToolButton {
                id: stopButton
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }
                width: notificationItem.toolIconSize
                height: width
                iconSource: "media-playback-stop"
                flat: notificationItem.flatButtons
                onClicked: {
                    var service = jobsSource.serviceForSource(modelData)
                    var operation = service.operationDescription("stop")
                    service.startOperationCall(operation)
                }
            }
        }
        /*
        Grid {
            anchors {
                left: parent.left
                right: parent.right
                rightMargin: notificationItem.layoutSpacing
            }
            //width: parent.width - notificationItem.layoutSpacing
            //x: 40
            spacing: notificationItem.layoutSpacing
            rows: 4
            columns: 2

//             QIconItem {
//                 icon: getData(jobsSource.data, "appIconName", '')
//                 width: notificationItem.toolIconSize
//                 height: width
// //                 anchors {
// //                     verticalCenter: progressItem.verticalCenter
// //                     right: progressItem.left
// //                     rightMargin: notificationItem.layoutSpacing
// //                 }
//             }
            Item {
                id: progressItem
                width: parent.width - labelName0Text.width
                height: childrenRect.height
            }
            PlasmaComponents.ToolButton {
                id: expandButton
                width: notificationItem.toolIconSize
                height: width
                flat: false
                iconSource: checked ? "list-remove" : "list-add"
                checkable: true
//                 anchors {
//                     right: speedLabel.left
//                     rightMargin: notificationItem.layoutSpacing
//                     verticalCenter: speedLabel.verticalCenter
//                 }
            }
            PlasmaComponents.Label {
                id: speedLabel
                text: eta > 0 ? i18nc("Speed and estimated time to completition", "%1 (%2 remaining)", speed, locale.prettyFormatDuration(eta)) : speed
            }
        }

        Item {
            id: detailsItem
            property bool expanded: state == "expanded"
            state: expandButton.checked ? "expanded" : "collapsed"
            onStateChanged: {
                print("NNN expand state is now: " + state);
            }
            anchors {
                top: summary.bottom
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
// FIXME: find a way to plot the signal
//                     PlasmaWidgets.SignalPlotter {
//                         id: plotter
//                         width: parent.width
//                         useAutoRange: true
//                         showVerticalLines: false
//                         unit: i18n("KiB/s")
//                         height: theme.mSize(theme.defaultFont).height * 5
//                         Component.onCompleted: plotter.addPlot(theme.highlightColor)
//                     }
//                     Connections {
//                         target: jobsSource
//                         onDataChanged: {
//                             plotter.addSample([jobsSource.data[modelData]["numericSpeed"]/1000])
//                         }
//                     }
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
                            duration: notificationItem.animationDuration
                            properties: "height"
                            easing: Easing.EaseInOut
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
                            duration: notificationItem.animationDuration
                            properties: "height"
                            easing: Easing.EaseInOut
                        }
                        //TODO: delete the details?
                        ScriptAction {script: detailsItem.visible = false}
                    }
                }
            ]
        }
        */

    }

    Component.onCompleted: {
        print("NNN ICON SIZE: " + toolIconSize + " " + units.iconSizes.smallMedium);
        print("NNN JOBSTATE: " + notificationItem.jobstate);
    }
}
