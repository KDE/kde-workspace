/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
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
import "plasmapackage:/code/logic.js" as Logic

Item {
    id: dialog
    property int actualHeight: batteryColumn.height + settingsColumn.height + separator.height + 10 // 10 = seprator margins

    property alias model: batteryLabels.model
    //property alias hasBattery: batteryIcon.hasBattery
    property bool pluggedIn

    property bool isBrightnessAvailable
    property alias screenBrightness: brightnessSlider.value

    property bool isKeyboardBrightnessAvailable
    property alias keyboardBrightness: keyboardBrightnessSlider.value

    property bool showRemainingTime
    property int remainingMsec

    property bool showSuspendButtons
    property bool offerSuspend
    property bool offerHibernate

    signal suspendClicked(int type)
    signal brightnessChanged(int screenBrightness)
    signal keyboardBrightnessChanged(int keyboardBrightness)
    signal powermanagementChanged(bool checked)

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
    }

    Column {
        id: batteryColumn
        spacing: 4
        width: parent.width
        anchors {
            left: parent.left
            right: parent.right
            top: plasmoid.location == BottomEdge ? parent.top : undefined
            bottom: plasmoid.location == BottomEdge ? undefined : parent.bottom
        }

        Repeater {
            id: batteryList
            model: dialog.model
            delegate: BatteryItem { }
        }
    }

    Column {
        id: settingsColumn
        spacing: 0
        width: parent.width

        anchors {
            left: parent.left
            right: parent.right
            top: plasmoid.location == BottomEdge ? undefined : parent.top
            bottom: plasmoid.location == BottomEdge ? parent.bottom : undefined
        }

        BrightnessItem {
            id: brightnessSlider
            icon: QIcon("video-display")
            label: i18n("Display Brightness")
            visible: isBrightnessAvailable
            onChanged: brightnessChanged(value)

        }

        BrightnessItem {
            id: keyboardBrightnessSlider
            icon: QIcon("input-keyboard")
            label: i18n("Keyboard Brightness")
            visible: isKeyboardBrightnessAvailable
            onChanged: keyboardBrightnessChanged(value)
        }

        PowerManagementItem {
            id: pmSwitch
            onEnabledChanged: powermanagementChanged(enabled)
        }
    }

    PlasmaCore.SvgItem {
        id: separator
        svg: PlasmaCore.Svg {
            id: lineSvg
            imagePath: "widgets/line"
        }
        elementId: "horizontal-line"
        height: lineSvg.elementSize("horizontal-line").height
        width: parent.width
        anchors {
            top: plasmoid.location == BottomEdge ? undefined : settingsColumn.bottom
            bottom: plasmoid.location == BottomEdge ? settingsColumn.top : undefined
            leftMargin: padding.margins.left
            rightMargin: padding.margins.right
            topMargin: 5
            bottomMargin: 5
        }
        /*anchors.top: plasmoid.location == BottomEdge ? settingsColumn.bottom : undefined
        anchors.bottom: plasmoid.location == BottomEdge ? undefined : settingsColumn.top*/
    }

    Column {
        id: labels
        spacing: 6
        anchors {
            top: parent.bottom // parent.top
            left: parent.left
            margins: 12
        }
        Repeater {
            model: dialog.model
            Components.Label {
                text: model["Pretty Name"] + ':'
                width: labels.width
                horizontalAlignment: Text.AlignRight
            }
        }

        Components.Label {
            text: i18n("AC Adapter:")
            anchors.right: parent.right
            anchors.bottomMargin: 12
        }

        Components.Label {
            text: i18nc("Label for remaining time", "Time Remaining:")
            visible: remainingTime.visible
            anchors.right: parent.right
        }

        Components.Label {
            text: i18nc("Label for power management inhibition", "Power management enabled:")
            anchors.right: parent.right
        }

        Components.Label {
            text: i18n("Screen Brightness:")
            visible: isBrightnessAvailable
            anchors.right: parent.right
        }

        //Row {
            Components.ToolButton {
                id: suspendButton
                iconSource: "system-suspend"
                text: i18nc("Suspend the computer to RAM; translation should be short", "Sleep")
                visible: true//showSuspendButtons && offerSuspend
                onClicked: suspendClicked(Logic.ram)
            }

            Components.ToolButton {
                id: hibernateButton
                iconSource: "system-suspend-hibernate"
                text: i18nc("Suspend the computer to disk; translation should be short", "Hibernate")
                visible: true//showSuspendButtons && offerHibernate
                onClicked: suspendClicked(Logic.disk)
            }
        //}
    }

    Column {
        id: values
        spacing: 6
        anchors {
            top: parent.bottom // parent.top
            left: labels.right
            margins: 12
        }

        Column {
            id: upperValues
            spacing: 6
            anchors {
                left: values.left
            }

            Repeater {
                id: batteryLabels
                Components.Label {
                    text: Logic.stringForState(model)
                    font.weight: Font.Bold
                }
            }

            Components.Label {
                text: dialog.pluggedIn ? i18n("Plugged in") : i18n("Not plugged in")
                font.weight: Font.Bold
                anchors.bottomMargin: 12
            }

            Components.Label {
                id: remainingTime
                text: Logic.formatDuration(remainingMsec);
                font.weight: Font.Bold
                visible: text!=""
            }
        }

        Column {
            id: lowerValues
            spacing: 6
            width: upperValues.width// + batteryIcon.width * 2
            anchors {
                left: values.left
            }
            Components.CheckBox {
                checked: true
                onClicked: powermanagementChanged(checked)
                x: 1
            }


        }
    }

    Row {
        anchors {
            //top: batteryView.bottom
            bottom: parent.bottom
            margins: 12
            right: parent.right
        }


    }

    /*BatteryIcon {
        id: batteryIcon
        monochrome: false
        pluggedIn: dialog.pluggedIn
        anchors {
            top: parent.top
            right: values.right
            topMargin: 12
        }
        width: height
        height: hibernateButton.height * 2
    }*/
}
