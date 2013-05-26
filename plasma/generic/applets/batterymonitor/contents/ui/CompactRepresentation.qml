/*
*   Copyright 2011 Sebastian Kügler <sebas@kde.org>
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
import "plasmapackage:/code/logic.js" as Logic

ListView {
    id: view

    property int minimumWidth
    property int minimumHeight

    property bool showOverlay: false
    property bool hasBattery

    property QtObject pmSource
    property QtObject batteries

    property bool singleBattery

    model: singleBattery ? 1 : batteries

    anchors.fill: parent
    orientation: ListView.Horizontal
    interactive: false

    function isConstrained() {
        return (plasmoid.formFactor == Vertical || plasmoid.formFactor == Horizontal);
    }

    Component.onCompleted: {
        if (!isConstrained()) {
            minimumWidth = 32;
            minimumHeight = 32;
        }
        plasmoid.addEventListener('ConfigChanged', configChanged);
    }

    function configChanged() {
        showOverlay = plasmoid.readConfig("showBatteryString");
    }

    delegate: Item {
        id: batteryContainer

        property bool hasBattery: view.singleBattery ? batteries.count : model["Plugged in"]
        property int percent: view.singleBattery ? batteries.cumulativePercent : model["Percent"]
        property bool pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]

        width: view.width/view.count
        height: view.height

        property real size: Math.min(width, height)

        BatteryIcon {
            id: batteryIcon
            monochrome: true
            hasBattery: parent.hasBattery
            percent: parent.percent
            pluggedIn: parent.pluggedIn
            width: size; height: size
            anchors.centerIn: parent
        }

        Rectangle {
            id: labelRect
            // should be 40 when size is 90
            width: Math.max(parent.size*4/9, 35)
            height: width/2
            anchors.centerIn: parent
            color: theme.backgroundColor
            border.color: "grey"
            border.width: 2
            radius: 4
            opacity: hasBattery ? (showOverlay ? 0.7 : (isConstrained() ? 0 : mouseArea.containsMouse*0.7)) : 0

            Behavior on opacity { NumberAnimation { duration: 100 } }
        }

        Text {
            id: overlayText
            text: i18nc("overlay on the battery, needs to be really tiny", "%1%", percent);
            color: theme.textColor
            font.pixelSize: Math.max(parent.size/8, 11)
            anchors.centerIn: labelRect
            // keep the opacity 1 when labelRect.opacity=0.7
            opacity: labelRect.opacity/0.7
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: plasmoid.togglePopup()

        PlasmaCore.ToolTip {
            id: tooltip
            target: mouseArea
            image: batteries.tooltipImage
            subText: batteries.tooltipText
        }
    }
}
