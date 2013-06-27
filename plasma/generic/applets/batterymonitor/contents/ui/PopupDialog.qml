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
    property int actualHeight: batteryColumn.implicitHeight + settingsColumn.height + separator.height + 10 // 10 = separator margins

    property alias model: batteryList.model
    property bool pluggedIn

    property int remainingTime
    property bool showRemainingTime

    property bool popupShown // somehow plasmoid.popupShowing isn't working

    property bool isBrightnessAvailable
    property alias screenBrightness: brightnessSlider.value

    property bool isKeyboardBrightnessAvailable
    property alias keyboardBrightness: keyboardBrightnessSlider.value

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
            delegate: BatteryItem { showChargeAnimation: popupShown }
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
        visible: model.count
        anchors {
            top: plasmoid.location == BottomEdge ? undefined : settingsColumn.bottom
            bottom: plasmoid.location == BottomEdge ? settingsColumn.top : undefined
            leftMargin: padding.margins.left
            rightMargin: padding.margins.right
            topMargin: 5
            bottomMargin: 5
        }
    }
}
