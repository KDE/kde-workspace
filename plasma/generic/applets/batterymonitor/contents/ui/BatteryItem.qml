/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2013 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as Components
import org.kde.qtextracomponents 0.1
import org.kde.locale 0.1 as KLocale
import "plasmapackage:/code/logic.js" as Logic

Item {
    id: batteryItem
    clip: true
    width: batteryColumn.width
    height: expanded ? batteryInfos.height + padding.margins.top + padding.margins.bottom * 2 + actionRow.height
                     : batteryInfos.height + padding.margins.top + padding.margins.bottom

    Behavior on height { PropertyAnimation {} }

    property bool expanded

    // NOTE: According to the UPower spec this property is only valid for primary batteries, however
    // UPower seems to set the Present property false when a device is added but not probed yet
    property bool isPresent: model["Plugged in"]

    property int remainingTime

    KLocale.Locale { id: locale }

    function updateSelection() {
        var hasFocus = batteryList.activeFocus && batteryList.activeIndex == index;
        var containsMouse = mouseArea.containsMouse;

        if (expanded && (hasFocus || containsMouse)) {
            padding.opacity = 1;
        } else if (expanded) {
            padding.opacity = 0.8;
        } else if (hasFocus || containsMouse) {
            padding.opacity = 0.65;
        } else {
            padding.opacity = 0;
        }
    }

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        Behavior on opacity { PropertyAnimation {} }
        anchors.fill: parent
    }

    onExpandedChanged: updateSelection()

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: updateSelection()
        onExited: updateSelection()
        onClicked: {
            oldIndex = batteryList.activeIndex
            batteryList.forceActiveFocus()
            batteryList.activeIndex = index
            batteryList.updateSelection(oldIndex,index)
            expanded = !expanded
        }
    }

    Item {
        id: batteryInfos
        height: Math.max(batteryIcon.height, batteryNameLabel.height + batteryPercentBar.height)

        anchors {
            top: parent.top
            topMargin: padding.margins.top
            left: parent.left
            leftMargin: padding.margins.left
            right: parent.right
            rightMargin: padding.margins.right
        }

        QIconItem {
            id: batteryIcon
            width: theme.iconSizes.dialog
            height: width
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
            icon: QIcon(Logic.iconForBattery(model,pluggedIn))
        }

        SequentialAnimation {
          id: chargeAnimation
          running: dialog.popupShown && model["State"] == "Charging" && model["Is Power Supply"]
          alwaysRunToEnd: true
          loops: Animation.Infinite

          NumberAnimation {
              target: batteryIcon
              properties: "opacity"
              from: 1.0
              to: 0.5
              duration: 750
              easing.type: Easing.InCubic
          }
          NumberAnimation {
              target: batteryIcon
              properties: "opacity"
              from: 0.5
              to: 1.0
              duration: 750
              easing.type: Easing.OutCubic
          }
        }

        Components.Label {
            id: batteryNameLabel
            anchors {
                verticalCenter: isPresent ? undefined : batteryIcon.verticalCenter
                top: isPresent ? parent.top : undefined
                left: batteryIcon.right
                leftMargin: 6
            }
            height: paintedHeight
            elide: Text.ElideRight
            text: model["Pretty Name"]
        }

        Components.Label {
            id: batteryStatusLabel
            anchors {
                top: batteryNameLabel.top
                left: batteryNameLabel.right
                leftMargin: 3
            }
            text: Logic.stringForBatteryState(model)
            height: paintedHeight
            visible: model["Is Power Supply"]
            color: "#77"+(theme.textColor.toString().substr(1))
        }

        Components.ProgressBar {
            id: batteryPercentBar
            anchors {
                bottom: parent.bottom
                left: batteryIcon.right
                leftMargin: 6
                right: batteryPercent.left
                rightMargin: 6
            }
            minimumValue: 0
            maximumValue: 100
            visible: isPresent
            value: parseInt(model["Percent"])
        }

        Components.Label {
            id: batteryPercent
            anchors {
                verticalCenter: batteryPercentBar.verticalCenter
                right: parent.right
            }
            visible: isPresent
            text: i18nc("Placeholder is battery percentage", "%1%", model["Percent"])
        }
    }

    Column {
        id: actionRow
        opacity: expanded ? 1 : 0
        width: parent.width
        anchors {
          top: batteryInfos.bottom
          topMargin: padding.margins.bottom
          left: parent.left
          leftMargin: padding.margins.left
          right: parent.right
          rightMargin: padding.margins.right
          bottomMargin: padding.margins.bottom
        }
        spacing: 4
        Behavior on opacity { PropertyAnimation {} }

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }
            elementId: "horizontal-line"
            height: lineSvg.elementSize("horizontal-line").height
            width: parent.width
        }

        Row {
            id: detailsRow
            width: parent.width
            spacing: 4

            Column {
                id: labelsColumn
                Components.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) { parent.width = paintedWidth; }
                    }
                    // FIXME Bound to AC adapter plugged in, not battery charging, see below
                    text: pluggedIn ? i18n("Time To Full:") : i18n("Time To Empty:")
                    visible: remainingTimeLabel.visible
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) { parent.width = paintedWidth; }
                    }
                    text: i18n("Capacity:")
                    visible: capacityLabel.visible
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) { parent.width = paintedWidth; }
                    }
                    text: i18n("Vendor:")
                    visible: vendorLabel.visible
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) { parent.width = paintedWidth; }
                    }
                    text: i18n("Model:")
                    visible: modelLabel.visible
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
            }
            Column {
                width: parent.width - labelsColumn.width - parent.spacing * 2
                Components.Label { // Remaining Time
                    id: remainingTimeLabel
                    height: paintedHeight
                    width: parent.width
                    elide: Text.ElideRight
                    // FIXME Uses overall remaining time, not bound to individual battery
                    text: locale.prettyFormatDuration(dialogItem.remainingTime)
                    visible: showRemainingTime && model["Is Power Supply"] && model["State"] != "NoCharge" && text != "" && dialogItem.remainingTime > 0
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label { // Capacity
                    id: capacityLabel
                    height: paintedHeight
                    width: parent.width
                    elide: Text.ElideRight
                    text: i18nc("Placeholder is battery capacity", "%1%", model["Capacity"])
                    visible: model["Is Power Supply"] &&  model["Capacity"] != "" && typeof model["Capacity"] == 'number'
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label { // Vendor
                    id: vendorLabel
                    height: paintedHeight
                    width: parent.width
                    elide: Text.ElideRight
                    text: model["Vendor"]
                    visible: model["Vendor"] != "" && typeof model["Vendor"] == 'string'
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                Components.Label { // Model
                    id: modelLabel
                    height: paintedHeight
                    width: parent.width
                    elide: Text.ElideRight
                    text: model["Product"]
                    visible: model["Product"] != "" && typeof model["Product"] == 'string'
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
            }
        }
    }
}

